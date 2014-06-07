#include "blmcom_head.h"
#include "FleetSvc.h"
#include "VesselSvc.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "PortSvc.h"
#include "kSQL.h"
#include "Geom.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"
#include "json.h"
#include <string>

using namespace std;

IMPLEMENT_SERVICE_MAP(FleetSvc)

bool FleetSvc::CalculateCjDynamicFleet()
{
	SYSTEM_LOG("[FleetSvc::CalculateCjDynamicFleet] begin =======================");

	MySql* psql = CREATE_MYSQL;
	const char* sql =
		"SELECT t3.shipid, t3.mmsi, t3.name_cn, t3.dangerLevel, t3.isSpec, IF(t4.starttime IS NULL,0,1) AS YH \
		FROM ( \
		SELECT t2.shipid, t2.mmsi, t2.name_cn, t1.dangerLevel, t1.isSpec, t1.aptd \
		FROM t45_pilot_voy_info t1, t45_pilot_voy_ship t2 \
		WHERE T1.VOYAGE_ID = T2.VoyageID AND t1.voystatus IN (2,3,4) and t1.SurveyStatus != 2) t3 \
		LEFT JOIN t45_pilot_cj_illegal_event t4 ON t3.mmsi = t4.mmsi AND t4.eventid = 73 AND t4.endtime IS null ORDER BY t3.aptd";

	VecPilotShip pilotList;
	IntSet mmsiSet;

	// 1、取出引航申请表中的引航船舶
	CHECK_MYSQL_STATUS(psql->Query(sql), false);

	TPilotShip pilot;
	while(psql->NextRow())
	{
		READMYSQL_STR(shipid, pilot.shipid);
		READMYSQL_INT(mmsi, pilot.mmsi, 0);
		READMYSQL_STR(name_cn, pilot.nameCn);
		READMYSQL_INT(dangerLevel, pilot.dangerLevel, 0);
		READMYSQL_INT(isSpec, pilot.isSpec, 0);
		READMYSQL_INT(YH, pilot.isYH, 0);

		if(pilot.shipid[0]=='\0' || pilot.mmsi<100000000)
			continue;
		if(mmsiSet.find(pilot.mmsi) != mmsiSet.end())
			continue;

		pilotList.push_back(pilot);
		mmsiSet.insert(pilot.mmsi);
	}
	if(pilotList.empty())
	{
		SYSTEM_LOG("[FleetSvc::CalculateCjDynamicFleet] end, count:0 =======================");
		RELEASE_MYSQL_RETURN(psql, true);
	}

	clearCjDynamicFleet();
	// 2、计算船舶所在目录
	for(int i=0; i<(int)pilotList.size(); i++)
	{
		Tokens catList = getPilotCategory(pilotList[i]);
		for(int j=0; j<(int)catList.size(); j++)
			m_mapPilotDynamicFleet[catList[j]].push_back(pilotList[i]);
	}

	// 3、写入船队数据表
	std::stringstream outSql;
	outSql << "insert into t41_fleet_ship_dynamic(userid, shipid, categoryid, mmsi, shipname, update_dt) values ";

	bool bFirst = true;
	for(MapPilotDynamicFleet::iterator iter=m_mapPilotDynamicFleet.begin(); iter!=m_mapPilotDynamicFleet.end(); iter++)
	{
		VecPilotShip& pilots = iter->second;
		for(VecPilotShip::iterator _iter=pilots.begin(); _iter!=pilots.end(); _iter++)
		{
			if(bFirst)
			{
				outSql << (*_iter).toSqlValue(iter->first);
				bFirst = false;
			}
			else
			{
				outSql << "," << (*_iter).toSqlValue(iter->first);
			}
		}
	}

	psql->Execute("delete from t41_fleet_ship_dynamic where userid = ''");
	psql->Execute(outSql.str().c_str());
	RELEASE_MYSQL_RETURN(psql, true);

	SYSTEM_LOG("[FleetSvc::CalculateCjDynamicFleet] end, count:%d =======================", mmsiSet.size());
}

bool FleetSvc::CalDynamicFleet()
{
	SYSTEM_LOG("[FleetSvc::CalDynamicFleet] begin =======================");

	char tmp[1024];

	AISLIST aisList;
	g_VesselSvc::instance()->aisShip.GetRealAis(aisList);
	for(int i=0; i<(int)aisList.size(); i++)
	{
		AisRealtime& as = aisList.at(i);
		int regionid = GetRegionid(as.lon,as.lat);
		AreaMapIter regionit = m_mapPortArea.find(regionid);
		if(regionit!=m_mapPortArea.end())
		{
			for(int i = 0;i<(int)regionit->second.size();i++)
			{
				DFArea &area = regionit->second.at(i);
				if(m_mapPortid2DF.find(area.areaid) == m_mapPortid2DF.end()) 
					continue;

				PT point;
				point.x = as.lon;
				point.y = as.lat;
				POLYGON polygon = StringToPolygon(area.pointset);
				if(PtInPolygon(polygon, point))							//如果有这个点说明在港区内放入对应的动态船队的vec
				{			
					TokenMapIter fit = m_mapPortid2DF.find(area.areaid);
					if(fit != m_mapPortid2DF.end())						//说明有动态船队订阅了这个港口
					{		
						for(int j = 0;j<(int)fit->second.size();j++)
						{
							string& dyfleetid = fit->second.at(j);
							m_mapDF[dyfleetid].vecMmsi.push_back(as.mmsi);
						}
					}
				}
			}
		}

		regionit = m_mapUserArea.find(regionid);
		if(regionit!=m_mapUserArea.end())
		{
			for(int i = 0;i<(int)regionit->second.size();i++)
			{
				DFArea &area = regionit->second.at(i);
				sprintf(tmp, "%s %s", area.userid, area.areaid);
				string areaidstr(tmp);
				if(m_mapUserid2DF.find(areaidstr) == m_mapUserid2DF.end()) 
					continue;

				PT point;
				point.x = as.lon;
				point.y = as.lat;
				POLYGON polygon = StringToPolygon(area.pointset);
				if(PtInPolygon(polygon, point))						//如果有这个点说明在港区内放入对应的动态船队的vec
				{			
					TokenMapIter fit = m_mapUserid2DF.find(areaidstr);
					if(fit != m_mapUserid2DF.end())					//说明有动态船队订阅了这个自定义区域
					{	
						for(int j = 0;j<(int)fit->second.size();j++){
							string& dyfleetid = fit->second.at(j);
							m_mapDF[dyfleetid].vecMmsi.push_back(as.mmsi);
						}
					}
				}
			}
		}
	}
	
	calDynamicFleet2();
	calDynamicFleet3();
	saveToMysql();

	SYSTEM_LOG("[FleetSvc::CalDynamicFleet] end =======================");

	return true;
}

bool FleetSvc::LoadDynamicFleet()
{
	SYSTEM_LOG("[FleetSvc::LoadDynamicFleet] begin =======================");

	m_mapUserid2DF.clear();
	m_mapPortid2DF.clear();
	m_mapDF.clear();

	MySql* psql = CREATE_MYSQL;
	char* sql = "select userid,categoryid,dynamic_conditions from boloomodb.t41_fleet_dynamic_condition";
	CHECK_MYSQL_STATUS(psql->Query(sql), false);

	while(psql->NextRow())
	{
		DynamicFleet df;
		char _userid[50] = {0};
		char _categoryid[5] = {0};
		char _condition[512] = {0};
		READMYSQL_STR(userid,_userid);
		df.userid = _userid;
		READMYSQL_STR(categoryid,_categoryid);
		df.categoryid = _categoryid;
		READMYSQL_STR(dynamic_conditions,_condition);

		string contidion = _condition;
		Tokens conditionvec = StrSplit(contidion,"&");
		if (conditionvec.size()!=4) 
			continue;

		//区域条件的处理
		if(!conditionvec[0].empty())
		{
			parseAreaCondition(conditionvec[0],df);
			df.isAreaConEmpty = false;
		}else
			df.isAreaConEmpty = true;

		//目的港条件的处理
		if(!conditionvec[1].empty())
		{
			df.WillArrive = StrSplit(conditionvec[1],"|");
			df.isDestPortConEmpty = false;
		}else
			df.isDestPortConEmpty = true;

		//第二类条件的处理
		if(!conditionvec[2].empty())
			df.SecondCon = StrSplit(conditionvec[2],"|");

		//第三类条件的处理
		if(!conditionvec[3].empty())
			df.ThirdCon = StrSplit(conditionvec[3],"-");

		string mapkey = df.userid + string(" ") + df.categoryid;
		m_mapDF.insert(MapDF::value_type(mapkey,df));
	}

	SYSTEM_LOG("[FleetSvc::LoadDynamicFleet] end, mapUid2DF:%d, mapPid2DF:%d, mapDF:%d =======================", (int)m_mapUserid2DF.size(), (int)m_mapPortid2DF.size(), (int)m_mapDF.size());
	RELEASE_MYSQL_RETURN(psql, true);
}

bool FleetSvc::LoadWillArriveShip()
{
	SYSTEM_LOG("[FleetSvc::LoadWillArriveShip] begin =======================");

	//ACE_Guard<ACE_Thread_Mutex> al(willarrive_cs);
	//add test mutex
	ACE_Guard<ACE_Thread_Mutex> al(willarrive_cs_tmp);

	m_mapWillArrive.clear();

	MySql* psql = CREATE_MYSQL;
	char* sql = "select t1.mmsi,t1.expectdate,t1.portid,t2.name,t2.ship_type,t2.imo from boloomodb_s.t41_willarrive_port t1 left join aisdb.t41_ais_ship_realtime t2 on t1.mmsi = t2.mmsi";
	CHECK_MYSQL_STATUS(psql->Query(sql), false);

	while(psql->NextRow())
	{
		int  portid;
		WillArriveShip was;
		READMYSQL_INT(mmsi,was.mmsi,0);
		READMYSQL_INT(expectdate,was.expatedate,0);
		READMYSQL_INT(portid,portid,0);
		READMYSQL_STR(name,was.shipname);
		READMYSQL_STR(imo,was.imo);
		READMYSQL_INT(ship_type,was.shiptype,0);
		ArriveIter it = m_mapWillArrive.find(portid);
		if(it!=m_mapWillArrive.end()){
			it->second.push_back(was);
		}else{
			vector<WillArriveShip> a;
			a.push_back(was);
			m_mapWillArrive.insert(ArriveMap::value_type(portid,a));
		}
	}
	
	SYSTEM_LOG("[FleetSvc::LoadWillArriveShip] end, mapWillArrive:%d =======================", (int)m_mapWillArrive.size());
	RELEASE_MYSQL_RETURN(psql, true);
}

bool FleetSvc::LoadPortArea()
{
	SYSTEM_LOG("[FleetSvc::LoadPortArea] begin =======================");

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query("select a.range as pointset,a.port_id as areaid,a.regionSet from boloomodb.t41_port_area a"), false);
	
	DFArea onearea;
	while(psql->NextRow())
	{
		READMYSQL_STR(areaid,onearea.areaid);
		READMYSQL_STR(pointset,onearea.pointset);
		char _region[256];
		READMYSQL_STR(regionSet,_region);
		string region = _region;
		IntVector regionidvec = IntStrSplit(region,",");
		onearea.areatype = 31;
		for (int n = 0;n<(int)regionidvec.size();n++)
		{
			AreaMapIter mapit = m_mapPortArea.find(regionidvec.at(n));
			if (mapit!=m_mapPortArea.end())
				mapit->second.push_back(onearea);
			else
			{
				vector<DFArea> a;
				a.push_back(onearea);
				m_mapPortArea.insert(AreaMap::value_type(regionidvec.at(n),a));
			}
		}
	}
	
	SYSTEM_LOG("[FleetSvc::LoadPortArea] end, total:%d =======================", (int)m_mapPortArea.size());
	RELEASE_MYSQL_RETURN(psql, true);
}

bool FleetSvc::LoadUserArea()
{
	SYSTEM_LOG("[FleetSvc::LoadUserArea] begin =======================");

	m_mapUserArea.clear();

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query("select * from t12_user_myarea where isvalid = 1"), false);

	DFArea onearea;
	while(psql->NextRow())
	{
		READMYSQL_STR(USER_ID,onearea.userid);
		READMYSQL_STR(MYAREA_ID,onearea.areaid);
		READMYSQL_STR(MYAREA_NAME,onearea.name);
		const char* _pointsetstr = psql->GetField("XYS");
		Tokens vecpointset = StrSplit(NOTNULL(_pointsetstr),"#");
		IntSet regionset;
		string pointset;
		double lon,lat;
		for(int i=0;i<(int)vecpointset.size();i++)
		{
			StrReplace(vecpointset.at(i),"|",",");
			sscanf(vecpointset.at(i).c_str(),"%lf,%lf",&lon,&lat);
			regionset.insert(GetRegionid(lon,lat));
			pointset+=vecpointset.at(i);
			if(i+1<(int)vecpointset.size())
				pointset+="|";
		}
		snprintf(onearea.pointset, sizeof(onearea.pointset)-1, "%s", pointset.c_str());
		if(strlen(onearea.pointset) == sizeof(onearea.pointset)-1)
		{
			SYSTEM_LOG("[FleetSvc::LoadUserArea] area too many point:%d, area_id:%s", (int)vecpointset.size(), onearea.areaid);
			continue;
		}
		onearea.areatype = 51;
		fillregion(regionset);//填充regionid

		//把userarea和regionid放入map
		for (IntSetIter it = regionset.begin();it!=regionset.end();it++)
		{
			AreaMapIter mapit = m_mapUserArea.find(*it);
			if (mapit!=m_mapUserArea.end())
				mapit->second.push_back(onearea);
			else
			{
				vector<DFArea> a;
				a.push_back(onearea);
				m_mapUserArea.insert(AreaMap::value_type(*it,a));
			}
		}
	}
	
	SYSTEM_LOG("[FleetSvc::LoadUserArea] end, total:%d =======================", (int)m_mapUserArea.size());
	RELEASE_MYSQL_RETURN(psql, true);
}

bool FleetSvc::Start()
{
	if(!LoadPortArea())
		return false;

    if(!g_MessageService::instance()->RegisterCmd(MID_FLEETMGR, this))
        return false;

    m_mapPilotCategory.insert(std::make_pair("D01", "X类危险品船舶"));
    m_mapPilotCategory.insert(std::make_pair("D02", "特殊Y类危险品船舶"));
    m_mapPilotCategory.insert(std::make_pair("D03", "一级危险品船舶"));
    m_mapPilotCategory.insert(std::make_pair("D04", "特种船舶"));
    m_mapPilotCategory.insert(std::make_pair("D05", "其它危险品船舶"));
    m_mapPilotCategory.insert(std::make_pair("D06", "夜航船舶"));
    m_mapPilotCategory.insert(std::make_pair("D07", "其它引航船舶"));

    m_mapPilotDynamicFleet.insert(std::make_pair("D01", VecPilotShip()));
    m_mapPilotDynamicFleet.insert(std::make_pair("D02", VecPilotShip()));
    m_mapPilotDynamicFleet.insert(std::make_pair("D03", VecPilotShip()));
    m_mapPilotDynamicFleet.insert(std::make_pair("D04", VecPilotShip()));
    m_mapPilotDynamicFleet.insert(std::make_pair("D05", VecPilotShip()));
    m_mapPilotDynamicFleet.insert(std::make_pair("D06", VecPilotShip()));
    m_mapPilotDynamicFleet.insert(std::make_pair("D07", VecPilotShip()));

    SERVICE_MAP(SID_DFLTGETSHIP4CJ,FleetSvc,GetDynamicFleet4CJ);
    SERVICE_MAP(SID_DFLTGETSHIP,FleetSvc,GetDynamicFleetShip);
    SERVICE_MAP(SID_DFLTGETCONDITION,FleetSvc,GetFleetCondition);
    SERVICE_MAP(SID_DFLTSETCONDITION,FleetSvc,SetFleetCondition);
    SERVICE_MAP(SID_DFLTGETSUBSCRIBE,FleetSvc,GetDynamicFleetSub);
    SERVICE_MAP(SID_FLTDELEVENT,FleetSvc,DelFleetEvent);
    SERVICE_MAP(SID_FLTMANULVSLMATCH,FleetSvc,MatchManualAddVsl);
    SERVICE_MAP(SID_FLTSETSUB_BINDING,FleetSvc,SetSubscribeBinding);
    SERVICE_MAP(SID_FLTSETSUBSCRIBE,FleetSvc,SetSubscribe);
    SERVICE_MAP(SID_FLTSUBSCRIBELIST,FleetSvc,GetSubscribeList);
    SERVICE_MAP(SID_REMARKMANAGER,FleetSvc,FleetRemarkManager);
    SERVICE_MAP(SID_GETREMARKLIST,FleetSvc,GetRemarkList);
    SERVICE_MAP(SID_FLT_SHIPMANAGER,FleetSvc,FleetVslManager);
    SERVICE_MAP(SID_FLT_CATMANAGER,FleetSvc,FleetCategoryManager);
    SERVICE_MAP(SID_FLT_GETFLT,FleetSvc,GetFleetVslList);
	SERVICE_MAP(SID_FLEET_MANAGES_WEB,FleetSvc,FleetManagesWeb);
	SERVICE_MAP(SID_FLEET_SHIP_MOVE,FleetSvc,FleetShipMove);
	SERVICE_MAP(SID_EVENT_GETALL,FleetSvc,GetAllEventTmp);
	SERVICE_MAP(SID_EVENT_NEWTMP,FleetSvc,AddEventTmp);
	SERVICE_MAP(SID_EVENT_DELTMP,FleetSvc,DeleteEventTmp);
	SERVICE_MAP(SID_EVENT_UPDATETMP,FleetSvc,UpdateEventTmp);

    uint32 interval = 100;
    int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday()+ACE_Time_Value(interval), ACE_Time_Value(interval));
    if(timerId <= 0)
        return false;

    DEBUG_LOG("[FleetSvc::Start] OK......................................");
    return true;
}

FleetSvc::FleetSvc() : m_lastCjDynamic(0), m_lastWillArrive(0)
{

}

FleetSvc::~FleetSvc()
{

}

int FleetSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	SYSTEM_LOG("[FleetSvc::handle_timeout] begin ==============");

	time_t tNow = time(NULL);
	int cjInterval = g_MainConfig::instance()->GetCjDFCalTime();


	//长江动态船队计算
	if(cjInterval && tNow >= m_lastCjDynamic + cjInterval)
	{
		CalculateCjDynamicFleet();
		m_lastCjDynamic = (long)tNow;
	}

	//动态船队计算(100)
	if(g_MainConfig::instance()->GetDFCalEnable())
	{
		if(IsDFMapEmpty())
		{
			LoadUserArea();
			LoadDynamicFleet();
		}

		CalDynamicFleet();

		//WillArrival(3600)
		if(tNow >= m_lastWillArrive)
		{
			LoadWillArriveShip();
			m_lastWillArrive = (long)tNow;
		}
	}

	SYSTEM_LOG("[FleetSvc::handle_timeout] end ==============");
	return 0;
}

int FleetSvc::GetDynamicFleet4CJ(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::GetDynamicFleet4CJ]bad format:", jsonString, 1);

	string ci = root.getv("ci", "");
	string cidsqlstr;
	if(ci.empty())
		cidsqlstr = " and categoryid like 'D%'";
	else
		cidsqlstr = " and categoryid ='"+ci+"'";

	char sql[2048];
	MySql *psql = CREATE_MYSQL;

	//判断用户类型：是否是长江及代理用户
	bool isCjUser = false;
	std::string usersql = "select user_id from t00_user where user_id = '" + string(pUid) + "' and usertype in (50,51)";
	CHECK_MYSQL_STATUS(psql->Query(usersql.c_str()), 3);

	if(psql->NextRow())
		isCjUser = true;
	
	if(!isCjUser)
		RELEASE_MYSQL_RETURN(psql, GetDynamicFleetShip(pUid, jsonString,out));

	//1.获取船队分类
	std::map<string,FltCategory> CateMap;
	string csql="select categoryid,categoryname,create_dt,monitor_flag,trash_flag,v1.num from t41_fleet_category t1 left join t41_remark_count v1 on t1.fleet_id=v1.user_id and t1.categoryid=v1.object_id where fleet_id='"+string(pUid)+"'"+cidsqlstr;
	CHECK_MYSQL_STATUS(psql->Query(csql.c_str()), 3);

	FltCategory FltCateInfo;
	if (psql->GetRowCount()==0)//添加默认分类
	{
		sprintf("insert into t41_fleet_category(fleet_id,categoryid,categoryname,update_dt,create_dt,monitor_flag,remark_num,trash_flag) values('%s','%s','%s','%s','%s',1,0,0),('%s','%s','%s','%s','%s',1,0,0),('%s','%s','%s','%s','%s',1,0,0),('%s','%s','%s','%s','%s',1,0,0),('%s','%s','%s','%s','%s',1,0,0),('%s','%s','%s','%s','%s',1,0,0),('%s','%s','%s','%s','%s',1,0,0),('%s','%s','%s','%s','%s',1,0,0)",
				pUid,"D00","Default","","",pUid,"D01","X类危险品船舶","","",pUid,"D02","Y类危险品船舶","","",pUid,"D03","一级危险品船舶","","",pUid,"D04","特种船舶","","",pUid,"D05","其它危险品船舶","","",pUid,"D06","夜航船舶","","",pUid,"D07","其它引航船舶","","");
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		FltCateInfo.auto_flag=0;
		FltCateInfo.trash_flag=0;
		FltCateInfo.crdt[0]=0;
		FltCateInfo.remark_num=0;
		FltCateInfo.email=0;
		FltCateInfo.sms=0;
		strcpy(FltCateInfo.catId,"D00");
		strcpy(FltCateInfo.cname,"Default");
		CateMap["D00"]=FltCateInfo;
		strcpy(FltCateInfo.catId,"D01");
		strcpy(FltCateInfo.cname,"X类危险品船舶");
		CateMap["D01"]=FltCateInfo;
		strcpy(FltCateInfo.catId,"D02");
		strcpy(FltCateInfo.cname,"Y类危险品船舶");
		CateMap["D02"]=FltCateInfo;
		strcpy(FltCateInfo.catId,"D03");
		strcpy(FltCateInfo.cname,"一级危险品船舶");
		CateMap["D03"]=FltCateInfo;
		strcpy(FltCateInfo.catId,"D04");
		strcpy(FltCateInfo.cname,"特种船舶");
		CateMap["D04"]=FltCateInfo;
		strcpy(FltCateInfo.catId,"D05");
		strcpy(FltCateInfo.cname,"其它危险品船舶");
		CateMap["D05"]=FltCateInfo;
		strcpy(FltCateInfo.catId,"D06");
		strcpy(FltCateInfo.cname,"夜航船舶");
		CateMap["D06"]=FltCateInfo;
		strcpy(FltCateInfo.catId,"D07");
		strcpy(FltCateInfo.cname,"其它引航船舶");
		CateMap["D07"]=FltCateInfo;

		//默认条件
		char gmt0now[20];
		GmtNow(gmt0now);
		string values = FormatString("('%s','D01','dy-cj:xd','%s'),('%s','D02','dy-cj:yd','%s'),('%s','D03','dy-cj:1d','%s'),('%s','D04','dy-cj:spec','%s'),('%s','D05','dy-cj:od','%s'),('%s','D06','dy-cj:n','%s'),('%s','D07','dy-cj:op','%s')",pUid,gmt0now,pUid,gmt0now,pUid,gmt0now,pUid,gmt0now,pUid,gmt0now,pUid,gmt0now,pUid,gmt0now);
		string sql="insert into t41_fleet_dynamic_condition (userid,categoryid,dynamic_conditions,update_dt) values "+values;
		CHECK_MYSQL_STATUS(psql->Execute(sql.c_str())>=0, 3);
	}
	else
	{
		while(psql->NextRow())
		{
			READMYSQL_STR(categoryid, FltCateInfo.catId);
			READMYSQL_STR(categoryname, FltCateInfo.cname);
			READMYSQL_STR(create_dt, FltCateInfo.crdt);
			READMYSQL_INT(monitor_flag, FltCateInfo.auto_flag,0);
			READMYSQL_INT(num, FltCateInfo.remark_num,0);
			READMYSQL_INT(trash_flag, FltCateInfo.trash_flag,0);
			CateMap[FltCateInfo.catId]=FltCateInfo;
		}
	}

	//2.获取分类船舶
	string ssql="select categoryid,shipid,mmsi,shipname from t41_fleet_ship_dynamic where userid = ''"+cidsqlstr;
	CHECK_MYSQL_STATUS(psql->Query(ssql.c_str()), 3);

	while(psql->NextRow())
	{
		FltVessel FltVslInfo;
		char catid[5]= {'\0'};

		READMYSQL_STR(categoryid,catid);
		READMYSQL_STR(shipid, FltVslInfo.vid);
		READMYSQL_INT(mmsi, FltVslInfo.mmsi,-1);
		READMYSQL_STR(shipname, FltVslInfo.name);

		if (CateMap.find(catid)!=CateMap.end())
		{
			CateMap[catid].vslList.push_back(FltVslInfo);
		}
	}

	//3.获取分类短信、邮件订阅事件
	string csql1="select Categoryid,substype,subsevent,binding from aisdb.t41_aisevt_category_subscribe where userid='"+string(pUid)+"'" +cidsqlstr;
	CHECK_MYSQL_STATUS(psql->Query(csql1.c_str()), 3);

	while(psql->NextRow())
	{
		char catid[5]= {'\0'};
		char bindStr[64]= {'\0'};
		int subtype,subsevent;
		READMYSQL_STR(Categoryid, catid);
		READMYSQL_INT(substype, subtype,-1);
		READMYSQL_INT(subsevent, subsevent,0);
		READMYSQL_STR(binding, bindStr);

		if(CateMap.find(catid)!=CateMap.end())
		{
			if (subtype==1)
			{
				CateMap[catid].email=subsevent;
				CateMap[catid].emailStr=string(bindStr);
			}
			else if (subtype==2)
			{
				CateMap[catid].sms=subsevent;
				CateMap[catid].telnoStr=string(bindStr);
			}
		}
	}

	//封装返回串

	out<<"[";
	std::map<string,FltCategory>::iterator it=CateMap.begin();
	for (; it!=CateMap.end(); it++)
	{
		FltCategory _Category=it->second;
		vector<FltVessel> _FltVslList=_Category.vslList;
		if (it!=CateMap.begin())
			out<<",";

		char temp[512];
		sprintf(temp,"ci:\"%s\",ts:%d,rn:%d,mf:%d,als:%d,ale:%d,em:\"%s\",tn:\"%s\"",_Category.catId,_Category.trash_flag,_Category.remark_num,_Category.auto_flag,_Category.sms,_Category.email,_Category.emailStr.c_str(),_Category.telnoStr.c_str());
		out<<"{\"cn\":\""<<_Category.cname<<"\""
			<<",\"cr\":\""<<_Category.crdt<<"\","<<temp<<",shps:[";

		for (int j=0; j<(int)_FltVslList.size(); j++)
		{
			FltVessel _VslInfo=_FltVslList[j];
			if (j>0)
				out<<',';

			char temp[512];
			sprintf(temp,"mm:%d,st:%d,ts:%d,rn:%d,mf:%d,vf:%d",_VslInfo.mmsi,3,1,0,_Category.auto_flag,1);
			out<<"{\"si\":\""<<_VslInfo.vid<<"\""
				<<",\"sn\":\""<<_VslInfo.name<<"\""
				<<",\"al\":\""<<_VslInfo.alias<<"\""
				<<",\"cr\":\""<<_Category.crdt<<"\","
				<<temp<<"}";
		}
		out<<"]}";
	}
	out<<"]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::GetDynamicFleetShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::GetDynamicFleetShip]bad format:", jsonString, 1);
	string ci = root.getv("ci", "");
	string cidsqlstr;
	if(ci.empty())
		cidsqlstr = " and categoryid like 'D%'";
	else
		cidsqlstr = " and categoryid  ='"+ci+"'";

	char sql[1024];
	MySql *psql = CREATE_MYSQL;
	std::map<string,FltCategory> CateMap;

	//1.获取船队分类
	string csql="select categoryid,categoryname,create_dt,monitor_flag,trash_flag,v1.num from t41_fleet_category t1 left join t41_remark_count v1 on t1.fleet_id=v1.user_id and t1.categoryid=v1.object_id where trash_flag != 1 and fleet_id='"+string(pUid)+"'"+cidsqlstr;
	CHECK_MYSQL_STATUS(psql->Query(csql.c_str()), 3);

	FltCategory FltCateInfo;
	if (psql->GetRowCount()==0)//添加默认分类
	{
		char gmt0now[20];
		GmtNow(gmt0now);

		sprintf(sql, "insert into t41_fleet_category(fleet_id,categoryid,categoryname,update_dt,create_dt,monitor_flag,remark_num,trash_flag) values('%s','%s','%s','%s','%s',0,0,0)",
					pUid,"D00","My Fleet",gmt0now,gmt0now);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		
		FltCateInfo.auto_flag=0;
		FltCateInfo.trash_flag=0;
		strcpy(FltCateInfo.catId,"D00");
		strcpy(FltCateInfo.cname,"My Fleet");
		FltCateInfo.crdt[0]=0;
		FltCateInfo.remark_num=0;
		FltCateInfo.email=0;
		FltCateInfo.sms=0;
		CateMap["D00"]=FltCateInfo;
	}
	else
	{
		while(psql->NextRow())
		{
			READMYSQL_STR(categoryid, FltCateInfo.catId);
			READMYSQL_STR(categoryname, FltCateInfo.cname);
			READMYSQL_STR(create_dt, FltCateInfo.crdt);
			READMYSQL_INT(monitor_flag, FltCateInfo.auto_flag,0);
			READMYSQL_INT(num, FltCateInfo.remark_num,0);
			READMYSQL_INT(trash_flag, FltCateInfo.trash_flag,0);
			CateMap[FltCateInfo.catId]=FltCateInfo;
		}
	}

	//2.获取分类船舶
	string ssql="select categoryid,shipid,mmsi,shipname from t41_fleet_ship_dynamic where userid = '"+string(pUid)+"'"+cidsqlstr;
	CHECK_MYSQL_STATUS(psql->Query(ssql.c_str()), 3);

	while(psql->NextRow())
	{
		FltVessel FltVslInfo;
		char catid[5]= {'\0'};

		READMYSQL_STR(categoryid,catid);
		READMYSQL_STR(shipid, FltVslInfo.vid);
		READMYSQL_INT(mmsi, FltVslInfo.mmsi,-1);
		READMYSQL_STR(shipname, FltVslInfo.name);
		if (CateMap.find(catid)!=CateMap.end())
		{
			CateMap[catid].vslList.push_back(FltVslInfo);
		}
	}

	//3.获取分类短信、邮件订阅事件
	string csql1="select Categoryid,substype,subsevent,binding from aisdb.t41_aisevt_category_subscribe where userid='"+string(pUid)+"'" +cidsqlstr;
	CHECK_MYSQL_STATUS(psql->Query(csql1.c_str()), 3);

	while(psql->NextRow())
	{
		char catid[5]= {'\0'};
		char bindStr[64]= {'\0'};
		int subtype,subsevent;
		READMYSQL_STR(Categoryid, catid);
		READMYSQL_INT(substype, subtype,-1);
		READMYSQL_INT(subsevent, subsevent,0);
		READMYSQL_STR(binding, bindStr);

		if(CateMap.find(catid)!=CateMap.end())
		{
			if (subtype==1)
			{
				CateMap[catid].email=subsevent;
				CateMap[catid].emailStr=string(bindStr);
			}
			else if (subtype==2)
			{
				CateMap[catid].sms=subsevent;
				CateMap[catid].telnoStr=string(bindStr);
			}
		}
	}

	//封装返回串
	out<<'[';
	std::map<string,FltCategory>::iterator it=CateMap.begin();
	for (; it!=CateMap.end(); it++)
	{
		FltCategory _Category=it->second;
		vector<FltVessel> _FltVslList=_Category.vslList;
		if (it!=CateMap.begin())
			out<<',';

		char temp[512];
		sprintf(temp,"ci:\"%s\",ts:%d,rn:%d,mf:%d,als:%d,ale:%d,em:\"%s\",tn:\"%s\"",_Category.catId,_Category.trash_flag,_Category.remark_num,_Category.auto_flag,_Category.sms,_Category.email,_Category.emailStr.c_str(),_Category.telnoStr.c_str());
		out<<"{\"cn\":\""<<_Category.cname<<"\""
			<<",\"cr\":\""<<_Category.crdt<<"\","<<temp<<",shps:[";

		for (int j=0; j<(int)_FltVslList.size(); j++)
		{
			FltVessel _VslInfo=_FltVslList[j];
			if (j>0)
				out<<',';

			char temp[512];
			sprintf(temp,"mm:%d,st:%d,ts:%d,rn:%d,mf:%d,vf:%d",_VslInfo.mmsi,3,1,0,_Category.auto_flag,1);

			out<<"{\"si\":\""<<_VslInfo.vid<<"\""
				<<",\"sn\":\""<<_VslInfo.name<<"\""
				<<",\"al\":\""<<_VslInfo.alias<<"\""
				<<",\"cr\":\""<<_VslInfo.crdt<<"\","<<temp<<'}';

		}
		out<<"]}";
	}
	out<<"]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::GetFleetCondition(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::GetFleetCondition]bad format:", jsonString, 1);
	int size = root["ci"]->size();
	string catidstr;
	map<string,string> m_Condmap;
	for (int i=0; i<size; i++)
	{
		if (i>0)
		{
			catidstr+=",";
		}
		catidstr += FormatString("'%s'",root["ci"]->getv(i, ""));
		m_Condmap[root["ci"]->getv(i, "")] = "";
	}

	MySql *psql = CREATE_MYSQL;
	string sql="select categoryid,dynamic_conditions from t41_fleet_dynamic_condition where userid='"+string(pUid)+"' and categoryid in ("+catidstr+")";
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);

	while(psql->NextRow())
	{
		char dcond[512] = {0};
		char cid[5]= {0};
		READMYSQL_STR(categoryid,cid);
		READMYSQL_STR(dynamic_conditions,dcond);
		m_Condmap[string(cid)] = dcond;
	}

	out<<"{eid:0,cons:[";
	map<string,string>::iterator it  = m_Condmap.begin();
	while(it!=m_Condmap.end())
	{
		if (it!=m_Condmap.begin())
			out<<',';

		char temp[512];
		sprintf(temp,"{ci:\"%s\",cond:\"%s\"}",it->first.c_str(),it->second.c_str());
		out<<temp;
		it++;
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::SetFleetCondition(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::SetFleetCondition]bad format:", jsonString, 1);
	string cid = root.getv("ci", "");
	string con = root.getv("cond", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	string values = FormatString("'%s','%s','%s','%s'",pUid,cid.c_str(),con.c_str(),gmt0now);
	string sql="replace into t41_fleet_dynamic_condition (userid,categoryid,dynamic_conditions,update_dt) values ("+values+")";

	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Execute(sql.c_str())>=0, 3);
	
	if (con.size() > 5 && m_mapPilotDynamicFleet.empty())
	{
		DynamicFleet df;
		string contidion = con;
		df.userid = pUid;
		df.categoryid = cid;
		Tokens conditionvec = StrSplit(contidion,"&");
		if (conditionvec.size()!=4)  
			RELEASE_MYSQL_RETURN(psql, 1);

		//区域条件的处理
		if(!conditionvec[0].empty())
		{
			parseAreaCondition(conditionvec[0],df);
			df.isAreaConEmpty = false;
		}
		else
			df.isAreaConEmpty = true;

		//目的港条件的处理
		if(!conditionvec[1].empty())
		{
			df.WillArrive = StrSplit(conditionvec[1],"|");
			df.isDestPortConEmpty = false;
		}
		else
			df.isDestPortConEmpty = true;

		//第二类条件的处理
		if(!conditionvec[2].empty())
			df.SecondCon = StrSplit(conditionvec[2],"|");

		//第三类条件的处理
		if(!conditionvec[3].empty())
			df.ThirdCon = StrSplit(conditionvec[3],"-");

		string mapkey = df.userid + string(" ") + df.categoryid;
		m_mapDF.insert(pair<string,DynamicFleet>(mapkey,df));
	}

	if (con.size() < 5 )  //条件为空，删掉所有的船
	{
		char* dyshpsql = "delete from t41_fleet_ship_dynamic where userid = '%s' and categoryid = '%s'";
		string t = FormatString(dyshpsql,pUid,cid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(t.c_str())>=0, 3);
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::GetDynamicFleetSub(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::GetDynamicFleetSub]bad format:", jsonString, 1);
	int size= root["catid"]->size();
	string catidstr;
	map<string,VslSubscribeNew> m_SubsMap;
	for (int i=0; i<size; i++)
	{
		if (i>0)catidstr+=",";
		catidstr += FormatString("'%s'",root["catid"]->getv(i, ""));
		VslSubscribeNew _subscribe;
		_subscribe.emai=0;
		_subscribe.sms=0;
		string _key =root["catid"]->getv(i, "");
		m_SubsMap[_key]=_subscribe;
	}

	string sql="select categoryid,substype,subsevent from aisdb.t41_aisevt_category_subscribe where userid ='"+string(pUid)+"' and categoryid in ("+catidstr+")";
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);

	int Subtype,Events;
	char catid[8] = {0};
	while(psql->NextRow())
	{
		READMYSQL_STR(categoryid,catid);
		READMYSQL_INT(substype, Subtype,0);
		READMYSQL_INT(subsevent, Events,0);

		if (m_SubsMap.find(string(catid))!=m_SubsMap.end())
		{
			if (Subtype==1)		//邮件
			{
				m_SubsMap[string(catid)].emai=Events;
			}
			else if (Subtype==2)//短信
			{
				m_SubsMap[string(catid)].sms=Events;
			}
		}
	}

	//写数据
	out<<'[';
	std::map<string,VslSubscribeNew>::iterator it=m_SubsMap.begin();
	for (; it!=m_SubsMap.end(); it++)
	{
		if (it!=m_SubsMap.begin())
		{
			out<<',';
		}
		VslSubscribeNew _subsItem=it->second;
		out<<FormatString("[\"%s\",\"%d\",\"%d\"]",it->first.c_str(),_subsItem.emai,_subsItem.sms);
	}
	out<<']';

	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::DelFleetEvent(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::DelFleetEvent]bad format:", jsonString, 1);
	string transid = root.getv("tid", "");
	string eventid = root.getv("eid", "");
	long mmsi = root.getv("mm", 0);
	long datetime = root.getv("dt", 0);

	char sql[1024];
	MySql *psql = CREATE_MYSQL;
	sprintf(sql, "insert into aisdb.t41_asievt_tran_userdeleted(UserId,EventId,TransactionID,Starttime,mmsi) values('%s','%s','%s',%d,%d)",
				pUid,eventid.c_str(),transid.c_str(),datetime,mmsi);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::MatchManualAddVsl(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::MatchManualAddVsl]bad format:", jsonString, 1);
	string orgshipid = root.getv("osi", "");
	string newshipid = root.getv("nsi", "");
	long mmsi = root.getv("mm", 0);
	string shipname = root.getv("sn", "");
	string shipalias = root.getv("al", "");

	char sql[1024];
	MySql *psql = CREATE_MYSQL;
	sprintf(sql, "update t41_fleet_ship set shipid='%s',mmsi=%d,shipname='%s',alias='%s',valid_flag=1 where fleet_id='%s' and shipid='%s'",
			newshipid.c_str(),mmsi,shipname.c_str(),shipalias.c_str(),pUid,orgshipid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::SetSubscribeBinding(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::SetSubscribeBinding]bad format:", jsonString, 1);
	string catid = root.getv("ci", "");
	int substype = root.getv("tp", 0);			//1邮件 2短信
	string bind = root.getv("bind", "");		//当事件存在时 表示是否订阅标志 0,1
	
	char sql[1024];
	MySql *psql = CREATE_MYSQL;

	sprintf(sql, "select count(1) as count from aisdb.t41_aisevt_category_subscribe where userid= '%s' and Categoryid = '%s' and substype= %d",
				pUid,catid.c_str(),substype);
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	if(atoi(psql->GetField("count"))>0)			//存在,更新
	{
		sprintf(sql, "update aisdb.t41_aisevt_category_subscribe set binding='%s' where userid= '%s' and Categoryid = '%s' and substype= %d",
					bind.c_str(),pUid,catid.c_str(),substype);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else										// 不存在,插入
	{
		//获取GMT0的当前时间
		char gmt0now[20];
		GmtNow(gmt0now);

		sprintf(sql, "insert into aisdb.t41_aisevt_category_subscribe(Userid,Categoryid,substype,subsevent,creationdate,binding,isvalid) values('%s','%s',%d,%d,'%s','%s',1)",
					pUid,catid.c_str(),substype,0,gmt0now,bind.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::SetSubscribe(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::SetSubscribe]bad format:", jsonString, 1);
	string szUid = root.getv("uid", "");
	if(!szUid.empty())
		pUid = szUid.c_str();
	string catid = root.getv("ci", "");
	string evtId = root.getv("ei", "");
	int substype = root.getv("tp", 0); //1邮件 2短信
	int subs = root.getv("subs", 0);   //当事件存在时 表示是否订阅标志 0,1
	string lan = root.getv("lan", "");
	double timezone = root.getv("tz", 0.0);
	string bindstr = root.getv("bind", "");

	bool isSub=false;
	if (!evtId.empty())
	{
		isSub=(subs!=0);
	}

	char sql[1024];
	MySql *psql = CREATE_MYSQL;
	int binaryFlag = getEventBinaryFlag(evtId);
	vector<long> m_updtMMSI;
	int size = root["mms"]->size();
	
	for (int i=0; i<size; i++)
	{
		long _mmsi = root["mms"]->getv(i, 0);
		//判断是否存在
		sprintf(sql, "select count(1) as count from aisdb.t41_aisevt_ship_subscribe where userid= '%s' and mmsi = %d and substype= %d",
					pUid,_mmsi,substype);
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
		
		if(atoi(psql->GetField("count"))>0) //存在
		{
			m_updtMMSI.push_back(_mmsi);
		}
		else								// 不存在,插入
		{
			//获取GMT0的当前时间
			char gmt0now[20];
			GmtNow(gmt0now);

			string s;
			char *isql="insert into aisdb.t41_aisevt_ship_subscribe(Userid,Mmsi,substype,subsevent,creationdate,language,timezone,binding,isvalid) values('%s',%d,%d,%d,'%s','%s',%.1f,'%s',1)";
			if (!evtId.empty())
			{
				if (isSub)
				{
					s = FormatString(isql,pUid,_mmsi,substype,binaryFlag,gmt0now,lan.c_str(),timezone,bindstr.c_str());
				}
				else
				{
					s = FormatString(isql,pUid,_mmsi,substype,0,gmt0now,lan.c_str(),timezone,bindstr.c_str());
				}
			}
			else
			{
				s = FormatString(isql,pUid,_mmsi,substype,subs,gmt0now,lan.c_str(),timezone,bindstr.c_str());
			}

			CHECK_MYSQL_STATUS (psql->Execute(s.c_str())>=0, 3);
		}
	}

	//其他都存在,更新
	if (m_updtMMSI.size()>0)
	{
		string str, mmstr;
		for (int j=0; j<(int)m_updtMMSI.size(); j++)
		{
			if (j>0)
			{
				mmstr+=",";
			}

			char tmp[64];
			sprintf(tmp, "%d", m_updtMMSI[j]);
			mmstr+= string(tmp);
		}
		if (!evtId.empty())
		{
			if (isSub)
			{
				char *upsql="update aisdb.t41_aisevt_ship_subscribe set subsevent=subsevent|%d,binding='%s'where userid='%s' and substype=%d and mmsi in (%s)";
				str = FormatString(upsql,binaryFlag,bindstr.c_str(),pUid,substype,mmstr.c_str());
			}
			else
			{
				char *upsql="update aisdb.t41_aisevt_ship_subscribe set subsevent=subsevent&%d ,binding='%s'where userid='%s' and substype=%d and mmsi in (%s)";
				str = FormatString(upsql,~binaryFlag,bindstr.c_str(),pUid,substype,mmstr.c_str());
			}
		}
		else
		{
			char *upsql="update aisdb.t41_aisevt_ship_subscribe set subsevent=%d,binding='%s' where userid='%s' and substype=%d and mmsi in (%s)";
			str = FormatString(upsql,subs,bindstr.c_str(),pUid,substype,mmstr.c_str());
		}
		CHECK_MYSQL_STATUS(psql->Execute(str.c_str())>=0, 3);
	}

	//更新分类
	if (!catid.empty())
	{
		sprintf(sql, "select count(1) as count from aisdb.t41_aisevt_category_subscribe where userid= '%s' and Categoryid = '%s' and substype= %d",
					pUid,catid.c_str(),substype);
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
		
		if(atoi(psql->GetField("count"))>0) //存在,更新
		{
			string sss;
			if (!evtId.empty())
			{

				if (isSub)
				{
					char *upsql="update aisdb.t41_aisevt_category_subscribe set subsevent=subsevent|%d where userid= '%s' and Categoryid = '%s' and substype= %d";
					sss = FormatString(upsql,binaryFlag,pUid,catid.c_str(),substype);
				}
				else
				{
					char *upsql="update aisdb.t41_aisevt_category_subscribe set subsevent=subsevent&%d where userid= '%s' and Categoryid = '%s' and substype= %d";
					sss = FormatString(upsql,~binaryFlag,pUid,catid.c_str(),substype);
				}
			}
			else
			{
				char *upsql="update aisdb.t41_aisevt_category_subscribe set subsevent='%d' where userid= '%s' and Categoryid = '%s' and substype= %d";
				sss = FormatString(upsql,subs,pUid,catid.c_str(),substype);
			}

			CHECK_MYSQL_STATUS(psql->Execute(sss.c_str())>=0, 3);
		}
		else				//不存在,插入
		{

			//获取GMT0的当前时间
			char gmt0now[20];
			GmtNow(gmt0now);

			char *isql="insert into aisdb.t41_aisevt_category_subscribe(Userid,Categoryid,substype,subsevent,creationdate,isvalid) values('%s','%s',%d,%d,'%s',1)";
			string sss;
			if (!evtId.empty())
			{
				if (isSub)
				{
					sss = FormatString(isql,pUid,catid.c_str(),substype,binaryFlag,gmt0now);
				}
				else
				{
					sss = FormatString(isql,pUid,catid.c_str(),substype,0,gmt0now);
				}
			}
			else
			{
				sss = FormatString(isql,pUid,catid.c_str(),substype,subs,gmt0now);
			}

			CHECK_MYSQL_STATUS (psql->Execute(sss.c_str())>=0, 3);
		}
	}
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::GetSubscribeList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::GetSubscribeListNew]bad format:", jsonString, 1);
	string szUid = root.getv("uid", "");
	if(!szUid.empty())
		pUid = szUid.c_str();
	int size=root["mms"]->size();
	string mmstr;
	std::map<int,VslSubscribeNew> m_SubsMap;

	for (int i=0; i<size; i++)
	{
		if (i>0)
		{
			mmstr+=",";
		}
		
		int _mmsi=root["mms"]->getv(i, 0);
		char tmp[64];
		sprintf(tmp, "%d", _mmsi);
		mmstr+= (string)tmp;

		VslSubscribeNew _subscribe;
		_subscribe.mmsi=_mmsi;
		_subscribe.emai=0;
		_subscribe.sms=0;
		m_SubsMap[_mmsi]=_subscribe;
	}

	MySql *psql = CREATE_MYSQL;
	string sql="select mmsi,substype,subsevent,binding from aisdb.t41_aisevt_ship_subscribe where userid='"+string(pUid)+"' and mmsi in ("+mmstr+")";
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);
		
	int Mmsi,Subtype,Events;
	while(psql->NextRow())
	{
		char bindStr[64]= {'\0'};

		READMYSQL_INT(mmsi, Mmsi,0)
		READMYSQL_INT(substype, Subtype,0)
		READMYSQL_INT(subsevent, Events,0)
		READMYSQL_STR(binding, bindStr)
		if (m_SubsMap.find(Mmsi)!=m_SubsMap.end())
		{
			if (Subtype==1)//邮件
			{
				m_SubsMap[Mmsi].emai=Events;
				m_SubsMap[Mmsi].EmalStr=string(bindStr);
			}
			else if (Subtype==2)//短信
			{
				m_SubsMap[Mmsi].sms=Events;
				m_SubsMap[Mmsi].TelnoStr=string(bindStr);
			}
		}
	}

	//写数据
	out<<'[';
	std::map<int,VslSubscribeNew>::iterator it=m_SubsMap.begin();
	for (; it!=m_SubsMap.end(); it++)
	{
		if (it!=m_SubsMap.begin())
			out<<',';
		VslSubscribeNew _subsItem=it->second;
		out<<FormatString("[\"%d\",\"%d\",\"%d\",\"%s\",\"%s\"]",_subsItem.mmsi,_subsItem.emai,_subsItem.sms,_subsItem.TelnoStr.c_str(),_subsItem.EmalStr.c_str());
	}
	out<<']';
	
	RELEASE_MYSQL_RETURN(psql, 0);
}
int FleetSvc::GetFleetVslList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[FleetSvc::GetFleetVslList]bad format:", jsonString, 1);
	const char* uid = root.getv("uid", "");
	int isWeb= root.getv("web", 0);
	string  strSeq = root.getv("seq", "");

	if(strlen(uid))
		pUid = uid;

	char sql[1024];
	MySql *psql = CREATE_MYSQL;

    //1.获取船队分类
    std::map<string,FltCategory> CateMap;
    sprintf(sql, "select categoryid,categoryname,create_dt,monitor_flag,trash_flag,v1.num from t41_fleet_category t1 left join t41_remark_count v1 on t1.fleet_id=v1.user_id and t1.categoryid=v1.object_id and v1.type=0 where fleet_id ='%s'  and t1.categoryid not like 'D%%' ", pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	FltCategory FltCateInfo;
	if (psql->GetRowCount()==0)//添加默认分类
	{
		sprintf(sql, "insert into t41_fleet_category(fleet_id,categoryid,categoryname,update_dt,create_dt,monitor_flag,remark_num,trash_flag) values('%s','%s','%s','%s','%s',0,0,0)",
				pUid,"00","My Fleet","","");
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		FltCateInfo.auto_flag=0;
		FltCateInfo.trash_flag=0;
		strcpy(FltCateInfo.catId,"00");
		strcpy(FltCateInfo.cname,"My Fleet");
		FltCateInfo.crdt[0]=0;
		FltCateInfo.remark_num=0;
		FltCateInfo.email=0;
		FltCateInfo.sms=0;
		CateMap["00"]=FltCateInfo;
	}
	else
	{
		while(psql->NextRow())
		{
			READMYSQL_STR(categoryid, FltCateInfo.catId)
			READMYSQL_STR(categoryname, FltCateInfo.cname)
			READMYSQL_STR(create_dt, FltCateInfo.crdt)
			READMYSQL_INT(monitor_flag, FltCateInfo.auto_flag,0)
			READMYSQL_INT(num, FltCateInfo.remark_num,0)
			READMYSQL_INT(trash_flag, FltCateInfo.trash_flag,0)
			CateMap[FltCateInfo.catId]=FltCateInfo;
		}
	}

	//1.5 获取船队二级目录关系
	sprintf(sql, "select categoryid, parentid from t41_fleet_parent where fleet_id = '%s'", pUid);
	if(psql->Query(sql)) {
		while(psql->NextRow()) {
			char catId[10] = {'\0'};
			char parentId[10] = {'\0'};
			READMYSQL_STR(categoryid, catId);
			READMYSQL_STR(parentid, parentId);

			std::map<string,FltCategory>::iterator iter = CateMap.find(catId);
			if(iter != CateMap.end())
				strcpy(iter->second.parentCatId, parentId);
		}
	}

    //2.获取分类船舶
    sprintf(sql, "select categoryid,shipid,mmsi,shipname,alias,create_dt,monitor_flag,trash_flag,valid_flag,v1.num from t41_fleet_ship t1 left join t41_remark_count v1 on t1.fleet_id=v1.user_id and t1.shipid=v1.object_id and v1.type=1 where fleet_id ='%s'", pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    
	while(psql->NextRow())
	{
		FltVessel FltVslInfo;
		char catid[5]= {'\0'};

		READMYSQL_STR(categoryid,catid)
		READMYSQL_STR(shipid, FltVslInfo.vid);
		READMYSQL_INT(mmsi, FltVslInfo.mmsi,-1)
		READMYSQL_STR(shipname, FltVslInfo.name)
		READMYSQL_STR(alias, FltVslInfo.alias)
		READMYSQL_STR(create_dt,FltVslInfo.crdt)

		READMYSQL_INT(monitor_flag, FltVslInfo.auto_flag,0)
		READMYSQL_INT(num, FltVslInfo.remark_num,0)
		READMYSQL_INT(trash_flag, FltVslInfo.trash_flag,0)
		READMYSQL_INT(valid_flag,FltVslInfo.valid_flag,1)

		if (CateMap.find(catid)!=CateMap.end())
		{
			CateMap[catid].vslList.push_back(FltVslInfo);
		}
	}

    //3.获取分类短信、邮件订阅事件
    string csql1="select Categoryid,substype,subsevent,binding from aisdb.t41_aisevt_category_subscribe where userid='"+string(pUid)+"' and Categoryid not like 'D%'";
	CHECK_MYSQL_STATUS(psql->Query(csql1.c_str()), 3);
    
	while(psql->NextRow())
	{
		char catid[5]= {'\0'};
		char bindStr[64]= {'\0'};
		int subtype,subsevent;
		READMYSQL_STR(Categoryid, catid)
		READMYSQL_INT(substype, subtype,-1)
		READMYSQL_INT(subsevent, subsevent,0)
		READMYSQL_STR(binding, bindStr)

		if(CateMap.find(catid)!=CateMap.end())
		{
			if (subtype==1)
			{
				CateMap[catid].email=subsevent;
				CateMap[catid].emailStr=string(bindStr);
			}
			else if (subtype==2)
			{
				CateMap[catid].sms=subsevent;
				CateMap[catid].telnoStr=string(bindStr);
			}
		}
	}

    //封装返回串
	if(isWeb==1)
		out<<"{seq:\""<<strSeq<<"\",data:[";
	else
		out<<"[";
    std::map<string,FltCategory>::iterator it=CateMap.begin();
    for (; it!=CateMap.end(); it++)
    {
        FltCategory _Category=it->second;
        vector<FltVessel> _FltVslList=_Category.vslList;
        if (it!=CateMap.begin())
			out<<',';

        out<<"{\"cn\":\""<<_Category.cname<<"\""
           <<",\"cr\":\""<<_Category.crdt<<"\","
		   <<FormatString("ci:\"%s\",pi:\"%s\",ts:%d,rn:%d,mf:%d,als:%d,ale:%d,em:\"%s\",tn:\"%s\"",_Category.catId,_Category.parentCatId,_Category.trash_flag,_Category.remark_num,_Category.auto_flag,_Category.sms,_Category.email,_Category.emailStr.c_str(),_Category.telnoStr.c_str())
           <<",shps:[";

        for (size_t j=0; j<_FltVslList.size(); j++)
        {
            FltVessel _VslInfo=_FltVslList[j];
            if (j>0)
				out<<',';

            out<<"{\"si\":\""<<_VslInfo.vid<<"\""
               <<",\"sn\":\""<<_VslInfo.name<<"\""
               <<",\"al\":\""<<_VslInfo.alias<<"\""
               <<",\"cr\":\""<<_VslInfo.crdt<<"\","
               <<FormatString("mm:%d,st:%d,ts:%d,rn:%d,mf:%d,vf:%d",_VslInfo.mmsi,3,_VslInfo.trash_flag,_VslInfo.remark_num,_VslInfo.auto_flag,_VslInfo.valid_flag)
               <<'}';

        }

        out<<"]}";
    }
	if(isWeb==1)
		out<<"]}";
	else
		out<<"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::FleetCategoryManager(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::FleetCategoryManager]bad format:", jsonString, 1);
	const char* uid = root.getv("uid", "");
	if(strlen(uid))
		pUid = uid;
   
    string catId = root.getv("ci", "");
    string catname = root.getv("cn", "");
    int type = root.getv("type", 0);

    MySql *psql = CREATE_MYSQL;
    switch (type)
    {
    case 0:
        return _AddFleetCategory(pUid,catId.c_str(),catname.c_str(),psql,out);

    case 1:
        return _DelFleetCategory(pUid,catId.c_str(),psql,out);

    case 2:
        return _PurgeFleetCategory(pUid,catId.c_str(),psql,out);

    case 3:
        return _RecoverFleetCategory(pUid,catId.c_str(),psql,out);

    case 4:
        return _ModifyFleetCategory(pUid,catId.c_str(),catname.c_str(),psql,out);

    case 5:
        return _AutoPosFleetCategory(pUid,catId.c_str(),true,psql,out);

    case 6:
        return _AutoPosFleetCategory(pUid,catId.c_str(),false,psql,out);
    }

    RELEASE_MYSQL_RETURN(psql, 2);
}

int FleetSvc::FleetVslManager(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::FleetVslManager]bad format:", jsonString, 1);
	const char* uid = root.getv("uid", "");
	if(strlen(uid))
		pUid = uid;

    string catId = root.getv("ci", "");
    string shipid = root.getv("si", "");
    int mmsi = root.getv("mm", 0);
    string shipname = root.getv("sn", "");
    string shipalias = root.getv("al", "");
    int type = root.getv("type", 0);

    MySql *psql = CREATE_MYSQL;
    switch (type)
    {
    case 0:
        return _AddFleetVsl(pUid,catId.c_str(),mmsi,shipid.c_str(),shipname.c_str(),shipalias.c_str(),true,psql,out);

    case 1:
        return _AddFleetVsl(pUid,catId.c_str(),mmsi,shipid.c_str(),shipname.c_str(),shipalias.c_str(),false,psql,out);

    case 2:
        return _DelFleetVsl(pUid,mmsi,shipid.c_str(),psql,out);

    case 3:
        return _PurgeFleetVsl(pUid,mmsi,shipid.c_str(),psql,out);

    case 4:
        return _RecoverFleetVsl(pUid,mmsi,shipid.c_str(),psql,out);

    case 5:
        return _ModifyFleetAlias(pUid,shipid.c_str(),shipalias.c_str(),psql,out);

    case 6:
        return _AutoPosFleetVsl(pUid,shipid.c_str(),true,psql,out);

    case 7:
        return _AutoPosFleetVsl(pUid,shipid.c_str(),false,psql,out);

    }

    RELEASE_MYSQL_RETURN(psql, 2);
}
int FleetSvc::GetRemarkList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[FleetSvc::GetRemarkList]bad format:", jsonString, 1);
    int type = root.getv("cat", 0);

    string s;
    if (type==0)		//船舶
    {
        string shipid = root.getv("id", "");
        char *ssql="select remark_id,remark_dt,subject,body,top_flag,update_dt from t41_fleet_ship_remark where fleet_id = '%s' and shipid = '%s'";
        s = FormatString(ssql,pUid,shipid.c_str());
    }
    else if (type==1)	//分类
    {
        string catId = root.getv("id", "");
        char *csql="select remark_id,remark_dt,subject,body,top_flag,update_dt from t41_fleet_category_remark where fleet_id = '%s' and  categoryid ='%s'";
        s = FormatString(csql,pUid,catId.c_str());
    }
    else
    {
        return 2;
    }

    MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(s.c_str()), 3);

    out<<'[';
    int total = 0;
    while(psql->NextRow())
    {
		if(total != 0)
			out<<',';
        total++;

        const char *remarkid	= psql->GetField("remark_id");
        const char*subject		= psql->GetField("subject");
        const char*body			= psql->GetField("body");
        const char*remarkdt		= psql->GetField("remark_dt");
        const char*updatedt		= psql->GetField("update_dt");

        out<<"{\"ri\":\""<<NOTNULL(remarkid)<<"\""
           <<",\"su\":\""<<NOTNULL(subject)<<"\""
           <<",\"bd\":\""<<NOTNULL(body)<<"\""
           <<",\"rd\":\""<<NOTNULL(remarkdt)<<"\""
           <<",\"ud\":\""<<NOTNULL(updatedt)<<"\"}";
    }

    out<<']';
    RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::FleetRemarkManager(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[FleetSvc::FleetRemarkManager]bad format:", jsonString, 1);
    int cat = root.getv("cat", 0);
    string remarkid = root.getv("ri", "");
    string subject = root.getv("su", "");
    string body = root.getv("bd", "");
	string id = root.getv("id", "");
    int type = root.getv("type", 0);

    MySql *psql = CREATE_MYSQL;
    if(cat==0)//船舶
    {
        switch (type)
        {
        case 0:
            return _AddVslRemark(pUid,id.c_str(),subject.c_str(),body.c_str(),psql,out);
            break;
        case 1:
            return _DelVslRemark(pUid,id.c_str(),remarkid.c_str(),psql,out);
            break;
        case 2:
            return _ModVslRemark(pUid,remarkid.c_str(),subject.c_str(),body.c_str(),psql,out);
            break;
        }

    }
    else if(cat==1) //分类
    {
        switch (type)
        {
        case 0:
            return _AddCatRemark(pUid,id.c_str(),subject.c_str(),body.c_str(),psql,out);
            break;
        case 1:
            return _DelCatRemark(pUid,id.c_str(),remarkid.c_str(),psql,out);
            break;
        case 2:
            return _ModCatRemark(pUid,remarkid.c_str(),subject.c_str(),body.c_str(),psql,out);
            break;
        }
    }

    RELEASE_MYSQL_RETURN(psql, 2);
}

int FleetSvc::_AddCatRemark(const char *usrid,const char *catId,const char *subject,const char *body,MySql *psql,std::stringstream& out)
{
    string id;
    NextID(id,SequenceMgr::TYPE_REMARK_ID);
    const char* remark_id = id.c_str();

    //获取GMT0的当前时间
    char gmt0now[20];
    GmtNow(gmt0now);

	char sql[1024];
    sprintf(sql, "insert into t41_fleet_category_remark (remark_id,fleet_id,categoryid,remark_dt,subject,body,update_dt) values('%s','%s','%s','%s','%s','%s','%s')",
				remark_id,usrid,catId,gmt0now, SqlReplace(string(subject)).c_str(), SqlReplace(string(body)).c_str(),gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf(sql, "Update t41_fleet_category set remark_num=remark_num+1 where fleet_id='%s' and categoryid='%s'",usrid,catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out<<FormatString("{id:\"%s\"}",remark_id);
    RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_AddVslRemark(const char *usrid,const char *shipid,const char *subject,const char *body,MySql *psql,std::stringstream& out)
{
	string id;
    NextID(id,SequenceMgr::TYPE_REMARK_ID);
    const char* remark_id = id.c_str();

    //获取GMT0的当前时间
    char gmt0now[20];
    GmtNow(gmt0now);

	char sql[1024];
    sprintf(sql, "insert into t41_fleet_ship_remark(remark_id,fleet_id,shipid,remark_dt,subject,body,update_dt) values('%s','%s','%s','%s','%s','%s','%s')",
				remark_id,usrid,shipid,gmt0now, SqlReplace(string(subject)).c_str(), SqlReplace(string(body)).c_str(),gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf(sql, "Update t41_fleet_ship set remark_num=remark_num+1 where fleet_id='%s' and shipid='%s'",
				usrid,shipid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<<FormatString("{id:\"%s\"}",remark_id);
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_DelCatRemark(const char *usrid,const char *catId,const char *remarkid,MySql *psql,std::stringstream& out)
{
    char sql[1024];
	sprintf(sql, "delete from t41_fleet_category_remark where remark_id = '%s'",remarkid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    sprintf(sql, "Update t41_fleet_category set remark_num=remark_num-1 where fleet_id='%s' and categoryid='%s'", usrid,catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_DelVslRemark(const char *usrid,const char *shipid,const char *remarkid,MySql *psql,std::stringstream& out)
{
    char sql[1024];
	sprintf(sql, "delete from t41_fleet_ship_remark where remark_id='%s'", remarkid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    sprintf(sql, "Update t41_fleet_ship set remark_num=remark_num-1 where fleet_id='%s' and shipid='%s'", usrid,shipid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
   
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_ModCatRemark(const char *usrid,const char *remarkid,const char *subject,const char *body,MySql *psql,std::stringstream& out)
{
    //获取GMT0的当前时间
    char gmt0now[20];
    GmtNow(gmt0now);

	char sql[1024];
    sprintf(sql, "update t41_fleet_category_remark set subject = '%s',body = '%s',update_dt = '%s' where remark_id = '%s'",
				subject,body,gmt0now,remarkid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_ModVslRemark(const char *usrid,const char *remarkid,const char *subject,const char *body,MySql *psql,std::stringstream& out)
{
	//获取GMT0的当前时间
    char gmt0now[20];
    GmtNow(gmt0now);

	char sql[1024];
    sprintf(sql, "update t41_fleet_ship_remark set subject = '%s',body = '%s',update_dt = '%s' where remark_id = '%s'",
				subject,body,gmt0now,remarkid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}
//(pUid,catId.c_str(),mmsi,shipid.c_str(),shipname.c_str(),shipalias.c_str(),false,psql,out)
int FleetSvc::_AddFleetVsl(const char *usrid,const char *catId,int mmsi,const char *shipid,const char *shipname,const char *alias,bool manual,MySql *psql,std::stringstream& out)
{
	char sql[1024];
    if (!manual)
    {
        sprintf(sql, "select count(1) as count from t41_fleet_ship where fleet_id = '%s' and categoryid = '%s' and shipid = '%s'",
					usrid,catId,shipid);
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
		if(atoi(psql->GetField("count"))>0)
		{
			sprintf(sql, "update t41_fleet_ship set trash_flag=0 where fleet_id = '%s' and categoryid='%s' and shipid = '%s'",
					usrid,catId,shipid);
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

			RELEASE_MYSQL_RETURN(psql, 0);
		}
    }

    string manualVslId;
    if (manual)
    {
        string id;
        NextID(id,1);
        manualVslId="M"+id;
    }

    //获取GMT0的当前时间
    char gmt0now[20];
    GmtNow(gmt0now);
    sprintf(sql, "insert into t41_fleet_ship(fleet_id,categoryid,shipid,mmsi,shipname,alias,monitor_flag,trash_flag,remark_num,valid_flag,update_dt,create_dt) values('%s','%s','%s',%d,'%s','%s',0,0,0,%d,'%s','%s')",
			usrid,catId,manual?manualVslId.c_str():shipid,mmsi, SqlReplace(string(shipname)).c_str(), SqlReplace(string(alias)).c_str(),manual?2:1,gmt0now,gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	if (manual)
	{
		out<<FormatString("{id:\"%s\"}",manualVslId.c_str());
	}
    RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_DelFleetVsl(const char *usrid,int mmsi,const char *shipid,MySql *psql,std::stringstream& out)
{
	char sql[1024];
    char gmt0now[20];
    GmtNow(gmt0now);

    sprintf(sql, "update t41_fleet_ship set trash_flag=1,update_dt='%s' where fleet_id='%s' and shipid ='%s'",
				gmt0now,usrid,shipid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	sprintf(sql, "update aisdb.t41_aisevt_ship_subscribe set isvalid=0 where userid='%s' and mmsi =%d",
				usrid,mmsi);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf(sql, "delete from aisdb.t41_aisevt_ship_subscribe_hwd where userid='%s' and mmsi =%d",
		usrid,mmsi);
	psql->Execute(sql);
       
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_PurgeFleetVsl(const char *usrid,int mmsi,const char *shipid,MySql *psql,std::stringstream& out)
{
	char sql[1024];
    sprintf(sql, "delete from t41_fleet_ship where fleet_id ='%s' and shipid ='%s'", usrid,shipid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	sprintf(sql, "delete from aisdb.t41_aisevt_ship_subscribe where userid='%s' and mmsi =%d", usrid,mmsi);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
        
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_RecoverFleetVsl(const char *usrid,int mmsi,const char *shipid,MySql *psql,std::stringstream& out)
{
    char gmt0now[20];
    GmtNow(gmt0now);

	char sql[1024];
    sprintf(sql, "update t41_fleet_ship set trash_flag=0,update_dt='%s' where fleet_id='%s' and shipid ='%s'",
				gmt0now,usrid,shipid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	sprintf(sql, "update aisdb.t41_aisevt_ship_subscribe set isvalid=1 where userid='%s' and mmsi =%d",
				usrid,mmsi);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
       
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_ModifyFleetAlias(const char *usrid,const char *shipid,const char *alias,MySql *psql,std::stringstream& out)
{
    char sql[1024];
	sprintf(sql, "update t41_fleet_ship set alias = '%s' where fleet_id='%s' and shipid ='%s'",
				alias,usrid,shipid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_AutoPosFleetVsl(const char *usrid,const char *shipid,bool autoPos,MySql *psql,std::stringstream& out)
{
    char sql[1024];
	sprintf(sql, "update t41_fleet_ship set monitor_flag=%d where fleet_id='%s'and shipid ='%s'",
				autoPos?1:0,usrid,shipid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
   
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_AddFleetCategory(const char *usrid,const char *catId,const char *catname,MySql *psql,std::stringstream& out)
{
	char sql[1024];
    sprintf(sql, "select count(1) as count from t41_fleet_category where fleet_id = '%s' and categoryid = '%s'", usrid, catId);
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	if(atoi(psql->GetField("count"))>0)
		RELEASE_MYSQL_RETURN(psql, 0);

    //获取GMT0的当前时间
    char gmt0now[20];
    GmtNow(gmt0now);
    sprintf(sql, "insert into t41_fleet_category(fleet_id,categoryid,categoryname,update_dt,create_dt,monitor_flag,remark_num,trash_flag) values('%s','%s','%s','%s','%s',1,0,0)",
					usrid,catId, SqlReplace(catname).c_str(),gmt0now,gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_DelFleetCategory(const char *usrid,const char *catId,MySql *psql,std::stringstream& out)
{
    char sql[1024];

    char gmt0now[20];
    GmtNow(gmt0now);

    sprintf(sql, "update t41_fleet_ship set trash_flag=1,update_dt='%s' where fleet_id='%s' and categoryid ='%s'",
				gmt0now,usrid,catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


    sprintf(sql, "update t41_fleet_category set trash_flag=1 where fleet_id='%s' and categoryid ='%s'",
				usrid,catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_PurgeFleetCategory(const char *usrid,const char *catId,MySql *psql,std::stringstream& out)
{
    char sql[1024];

	sprintf(sql, "DELETE FROM aisdb.t41_aisevt_ship_subscribe_hwd t1, boloomodb.t41_fleet_ship t2 WHERE t1.fleet_id = t2.fleet_id AND t2.fleet_id = '%s' AND t1.mmsi = t2.mmsi", usrid);
	psql->Execute(sql);

	sprintf(sql, "delete from t41_fleet_ship where fleet_id ='%s' and categoryid='%s'", usrid,catId);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    sprintf(sql, "delete from t41_fleet_category where fleet_id='%s' and categoryid='%s'", usrid, catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    if(catId[0] == 'D')
    {
        sprintf(sql, "delete from t41_fleet_dynamic_condition where userid = '%s' and categoryid = '%s'",usrid,catId);
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

        //删除动态船队里的船
        sprintf(sql, "delete from t41_fleet_ship_dynamic where userid = '%s' and categoryid = '%s'",usrid,catId);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
   
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_RecoverFleetCategory(const char *usrid,const char *catId,MySql *psql,std::stringstream& out)
{
    char gmt0now[20];
    GmtNow(gmt0now);
	char sql[1024];

    sprintf(sql, "update t41_fleet_ship set trash_flag=0,update_dt='%s' where fleet_id='%s' and categoryid ='%s'",
					gmt0now,usrid,catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    

    sprintf(sql, "update t41_fleet_category set trash_flag=0 where fleet_id='%s' and categoryid ='%s'",
					usrid,catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_ModifyFleetCategory(const char *usrid,const char *catId,const char *catname,MySql *psql,std::stringstream& out)
{
    char sql[1024];
    char gmt0now[20];
    GmtNow(gmt0now);

    sprintf(sql, "update t41_fleet_category set categoryname='%s',update_dt='%s' where fleet_id ='%s' and categoryid='%s'",
				SqlReplace(string(catname)).c_str(),gmt0now,usrid,catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	RELEASE_MYSQL_RETURN(psql, 0);
}

int FleetSvc::_AutoPosFleetCategory(const char *usrid,const char *catId,bool autoPos,MySql *psql,std::stringstream& out)
{
	char sql[1024];
    sprintf(sql, "update t41_fleet_category set monitor_flag=%d where fleet_id='%s' and categoryid ='%s'",
			(autoPos?1:0),usrid,catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	
    sprintf(sql, "update t41_fleet_ship set monitor_flag=%d where fleet_id='%s'and categoryid ='%s'",
			(autoPos?1:0),usrid,catId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	RELEASE_MYSQL_RETURN(psql, 0);
}

bool FleetSvc::subscribeEvent(MySql* psql,const char* channel_type,const char* objectID,const Tokens& mmsiList,const char* lang)
{
	if(mmsiList.size()<=0)
		return false;

	//获取事件列表
	vector<int> eventList;

	char* sql = "select event_id from t41_fleet_eventsub where fleet_id = '%s'";
	string tsql = FormatString(sql,objectID);
	if(!psql->Query(tsql.c_str()))
		return false;

	int rnum = psql->GetRowCount();
	eventList.reserve(rnum);
	for(int j=0; j<rnum; ++j)
	{
		psql->NextRow();
		eventList.push_back( atoi((char*) psql->GetField("event_id")));
	}

	if(eventList.size()<=0)
	{
		return true;
	}

	//获取GMT0的当前时间
	char gmt0now[20];
	GmtNow(gmt0now);

	string asql("replace into aisdb.t41_aisevt_sysevt_para(subscribeID,EventID, Mmsi, Para1, Para2, Para3, CreationDate, Schedule, IsValid, ObjectID, type, language) values");
	for(int i=0; i<(int)mmsiList.size(); i++)
	{
		for(int j=0; j<(int)eventList.size(); j++)
		{
			if(i!=0 || j!=0)
			{
				asql += ",";
			}

			string subscribeID;
			NextID(subscribeID,SequenceMgr::TYPE_SUBSCRIBEID);

			asql += FormatString("('%s','%d',%s,'','','','%s',1,1,'%s','%s','%s')",subscribeID.c_str(),eventList[j],mmsiList[i].c_str(),gmt0now,objectID,channel_type,lang);
		}
	}

	if(psql->Execute(asql.c_str()) < 0)
		return false;
	
	return true;
}

bool FleetSvc::updateEvent(MySql* psql,const char* channel_type,const char* objectID,const vector<int>& eventList,const char* lang)
{
	//1.删除不在eventids中的注册事件
	string eventids;
	for(int i=0; i<(int)eventList.size(); ++i)
	{
		if(i!=0)
		{
			eventids += ",";
		}
		eventids += FormatString("'%d'",eventList[i]);
	}
	char* sql;
	string tsql ;
	if(eventList.size()>0)
	{
		sql= "delete from aisdb.t41_aisevt_sysevt_para where objectid = '%s' and eventid not in (%s)";
		tsql=FormatString(sql,objectID,eventids.c_str());
	}
	else
	{
		sql= "delete from aisdb.t41_aisevt_sysevt_para where objectid = '%s'";
		tsql=FormatString(sql,objectID);
	}

	if(psql->Execute(tsql.c_str())<0)
		return false;

	//2.获取mmsi列表
	Tokens mmsiList;
	sql = "select distinct mmsi as mmsi from t41_fleet_ship where fleet_id = '%s' and categoryid != '99' ";
	tsql=FormatString(sql,objectID);

	if(psql->Query(tsql.c_str()))
	{
		for(int j=0; j<psql->GetRowCount(); ++j)
		{
			psql->NextRow();
			const char* mmsi = psql->GetField("mmsi");
			if(mmsi != NULL && *mmsi != 0)
			{
				mmsiList.push_back(mmsi);
			}
		}
	}
	else
	{
		return false;
	}

	//3.插入新的事件
	if(mmsiList.size()>0 && eventList.size()>0)
	{
		//获取GMT0的当前时间
		char gmt0now[20];
		GmtNow(gmt0now);
		string  sql;
		sql+=FormatString("insert ignore into aisdb.t41_aisevt_sysevt_para(subscribeID,EventID, Mmsi, Para1, Para2, Para3, CreationDate, Schedule, IsValid, ObjectID, type, language) values");
		for(int i=0; i<(int)mmsiList.size(); i++)
		{
			for(int j=0; j<(int)eventList.size(); j++)
			{
				if(i!=0 || j!=0)
				{
					sql+=FormatString(",");
				}

				string subscribeID;
				NextID(subscribeID,SequenceMgr::TYPE_SUBSCRIBEID);

				sql+=FormatString("('%s','%d',%s,'','','','%s',1,1,'%s','%s','%s')",subscribeID.c_str(),eventList[j],mmsiList[i].c_str(),gmt0now,objectID,channel_type,lang);
			}
		}

		if(psql->Execute(sql.c_str())<0)
			return false;
	}

	//4.删除不在eventids中的注册事件
	sql = "update aisdb.t41_aisevt_sysevt_para set language ='%s'  where objectid = '%s'";
	tsql=FormatString(sql,lang,objectID);

	if(psql->Execute(tsql.c_str())<0)
		return false;

	return true;
}

bool FleetSvc::unsubscribeEvent(MySql* psql,const char* channel_type,const char* objectId)
{
	char* sql = "delete from aisdb.t41_aisevt_sysevt_para where objectId = '%s' and type = '%s'";
	string tsql ;
	tsql=FormatString(sql,objectId,channel_type);

	if(psql->Execute(tsql.c_str())<0)
		return false;
	
	return true;
}

bool FleetSvc::unsubscribeEvent(MySql* psql,const char* channel_type,const char* objectId,const char* mmsi)
{
	if(strcmp(mmsi,"")==0)
		return false;

	char* sql = "delete from aisdb.t41_aisevt_sysevt_para where objectId = '%s' and type = '%s' and mmsi=%s";
	string tsql ;
	tsql=FormatString(sql,objectId,channel_type,mmsi);

	if(psql->Execute(tsql.c_str())<0)
		return false;
	
	return true;
}

Tokens FleetSvc::getPilotCategory(TPilotShip& pilot)
{
	Tokens catList;

	if(pilot.dangerLevel == 1)
	{
		catList.push_back("DO1");
	}
	else if(pilot.dangerLevel == 2)
	{
		catList.push_back("D02");
	}
	else if(pilot.dangerLevel == 3)
	{
		catList.push_back("D03");
	}
	else if(pilot.dangerLevel == 5)
	{
		catList.push_back("D05");
	}

	if(pilot.isSpec)
		catList.push_back("D04");
	if(pilot.isYH)
		catList.push_back("D06");

	if(catList.empty())
		catList.push_back("D07");

	return catList;
}

void FleetSvc::clearCjDynamicFleet()
{
	for(MapPilotDynamicFleet::iterator iter = m_mapPilotDynamicFleet.begin(); iter != m_mapPilotDynamicFleet.end(); iter++)
		iter->second.clear();
}

int FleetSvc::getEventBinaryFlag(string eventid)
{
	int returnVal=0;

	int _EventidInt=atoi(eventid.c_str());
	switch (_EventidInt)
	{
	case 0://Under way
		returnVal=1;
		break;
	case 1://Anchor
		returnVal=2;
		break;
	case 2://"Not Under Command"
		returnVal=4;
		break;
	case 3: //"Restricted Manoeuvrability";
		returnVal=8;
		break;
	case 4://"Constrained by her draught";
		returnVal=16;
		break;
	case 5://"Moored";
		returnVal=32;
		break;
	case 6://"Aground";
		returnVal=64;
		break;
	case 7://"Engaged in Fishing";
		returnVal=128;
		break;
	case 8://"Under Way Sailing";
		returnVal=256;
		break;
	case 31://"Access Port";
		returnVal=512;
		break;
	case 32://"Leave Port";
		returnVal=1024;
		break;
	case 33://"Access Anchorage";
		returnVal=2048;
		break;
	case 34://"Leave Anchorage";
		returnVal=4096;
		break;
	case 37://"Access Dangerous Area";
		returnVal=8192;
		break;
	case 38://"Leave Dangerous Area";
		returnVal=16384;
		break;

	}

	return returnVal;
}

void FleetSvc::fillregion( IntSet& regionset )
{
	int front3numsmall = 999;
	int front3numbig = 0;
	int last3numsmall = 999;
	int last3numbig = 0;
	IntSetIter it = regionset.begin();
	while (it!=regionset.end())
	{
		int region = *it;
		if(region/1000 < front3numsmall) front3numsmall = region/1000;
		if(region/1000 > front3numbig) front3numbig = region/1000;
		if(region%1000 < last3numsmall) last3numsmall = region%1000;
		if(region%1000 > last3numbig) last3numbig = region%1000;
		it++;
	}
	regionset.clear();
	for(int i = front3numsmall;i<=front3numbig;i++)
		for(int j = last3numsmall ; j<=last3numbig ;j++)
			regionset.insert(i*1000+j);
}

void FleetSvc::parseAreaCondition( string & condition,DynamicFleet& df )
{
	Tokens areaidvec = StrSplit(condition,"|");
	char tmp[1024];
	for(int i = 0;i<(int)areaidvec.size();i++){
		int type,id;
		sscanf(areaidvec[i].c_str(),"%d-%d",&type,&id);
		if(type == 0)
		{
			sprintf(tmp, "%s %d", df.userid.c_str(), id);	//自定义区域id
			string areaid(tmp);
			TokenMapIter it =  m_mapUserid2DF.find(areaid);
			if(it!=m_mapUserid2DF.end())
			{
				it->second.push_back(df.userid + string(" ")+ df.categoryid);
			}
			else
			{
				Tokens a;
				sprintf(tmp, "%s %d", df.userid.c_str(), df.categoryid.c_str());
				a.push_back(tmp);
				m_mapUserid2DF.insert(TokenMap::value_type(areaid,a));
			}
		}
		else
		{
			sprintf(tmp, "%d", id);							//港区id
			const char* porig = g_PortSvc::instance()->m_alias.FindOrig(tmp);
			if (porig){
				id = atoi(porig);
				sprintf(tmp, "%d", id);
			}
			
			string areaid(tmp);
			TokenMapIter it =  m_mapPortid2DF.find(areaid);
			if(it!=m_mapPortid2DF.end())
			{
				it->second.push_back(df.userid + string(" ")+ df.categoryid);
			}
			else
			{
				Tokens a;
				sprintf(tmp, "%s %d", df.userid.c_str(), df.categoryid.c_str());
				a.push_back(tmp);
				m_mapPortid2DF.insert(TokenMap::value_type(areaid,a));
			}
		}
	}
}

void FleetSvc::calDynamicFleet2()
{
	SYSTEM_LOG("[FleetSvc::calDynamicFleet2] begin =======================");

	MapDFIter it =  m_mapDF.begin();
	while(it!=m_mapDF.end())
	{
		IntSet willarrivemmsi;
		DynamicFleet & df = it->second;
		for(int i = 0;i<(int)df.WillArrive.size();i++)
		{
			string onewillarriveport = df.WillArrive.at(i);
			int portid;
			int beginday;
			int endday;
			sscanf(onewillarriveport.c_str(),"%d-%d-%d",&portid,&beginday,&endday);

			ACE_Guard<ACE_Thread_Mutex> al(willarrive_cs);

			ArriveIter fit = m_mapWillArrive.find(portid);
			if (fit!=m_mapWillArrive.end())
			{
				for(int j = 0;j<(int)fit->second.size();j++)
				{
					WillArriveShip& was = fit->second.at(j);
					if(was.expatedate > time(NULL)+86400*beginday && was.expatedate < time(NULL)+86400*endday)
					{
						if(df.isAreaConEmpty){
							df.vecMmsi.push_back(was.mmsi);
						}else{
							willarrivemmsi.insert(was.mmsi);
						}
					}
				}
			}
		}
		if(!df.isAreaConEmpty && !df.isDestPortConEmpty)
		{
			vector<int>::iterator fit = df.vecMmsi.begin();
			while(fit!=df.vecMmsi.end())
			{
				if (willarrivemmsi.find(*fit)==willarrivemmsi.end())
				{
					fit = df.vecMmsi.erase(fit);
					continue;
				}
				fit++;
			}
		}
		it++;
	}
	SYSTEM_LOG("[FleetSvc::calDynamicFleet2] end =======================");
}

void FleetSvc::calDynamicFleet3()
{
	SYSTEM_LOG("[FleetSvc::calDynamicFleet3] begin =======================");

	char tmp[1024];

	MapDFIter it =  m_mapDF.begin();
	while(it!=m_mapDF.end())
	{
		DynamicFleet & df = it->second;
		for(int i = 0;i<(int)df.SecondCon.size();i++)
		{
			string _condition = df.SecondCon.at(i);
			Tokens convec = StrSplit(_condition,"-");
			if(convec.size() != 2) continue;
			int type = atoi(convec.front().c_str());
			string condition = convec.back();

			vector<int>::iterator vecit = df.vecMmsi.begin();
			while(vecit!=df.vecMmsi.end())
			{
				ShipData* pShip = g_VesselSvc::instance()->aisShip.FindShip(*vecit);
				if(!pShip) { vecit++; continue; }

				string shipcon;
				if(type == 0) 
				{
					sprintf(tmp, "%d", pShip->getType()/10);
					shipcon = tmp;
				}
				else if(type == 1)
				{
					shipcon = pShip->country;
					if(shipcon.empty())
						shipcon = "ALL";
				}
				else 
				{
					sprintf(tmp, "%d", pShip->getType()%10);
					shipcon = tmp;
				}

				if(shipcon !="ALL" && shipcon != condition)
				{
					vecit = df.vecMmsi.erase(vecit);
					continue;
				}
				vecit++;
			}
		}
		
		if(df.ThirdCon.size()!=2){it++; continue;}

		string thirdconstr = df.ThirdCon.front();
		int andor = atoi(df.ThirdCon.back().c_str());
		Tokens thirdconvec = StrSplit(thirdconstr,"@");

		if(thirdconvec.size()!=6){it++; continue;}

		vector<int>::iterator vecit = df.vecMmsi.begin();
		while(vecit!=df.vecMmsi.end())
		{
			ShipData* pShip = g_VesselSvc::instance()->aisShip.FindShip(*vecit);
			if(!pShip) {vecit++;continue;}

			//已经得到船的静态信息
			bool isStillAlive = true;
			double beginnum,endnum;

			sscanf(thirdconvec[0].c_str(),"%lf,%lf",&beginnum,&endnum);
			if( fabs(pShip->length) < 1e-3 || pShip->length < endnum + 1e-3 && pShip->length>beginnum - 1e-3)
				isStillAlive = true;
			else
				isStillAlive = false;

			sscanf(thirdconvec[1].c_str(),"%lf,%lf",&beginnum,&endnum);
			if( fabs(pShip->beam) < 1e-3 || pShip->beam < endnum + 1e-3 && pShip->beam > beginnum - 1e-3)
				isStillAlive = true;
			else{
				if(isStillAlive && andor/10000 == 0)
					isStillAlive = true;
				else
					isStillAlive = false;
			}

			sscanf(thirdconvec[2].c_str(),"%lf,%lf",&beginnum,&endnum);
			if( fabs(pShip->depth) < 1e-3 || pShip->depth < endnum + 1e-3 && pShip->depth > beginnum - 1e-3)
				isStillAlive = true;
			else{
				if(isStillAlive && (andor/1000)%10 == 0)
					isStillAlive = true;
				else
					isStillAlive = false;
			}

			sscanf(thirdconvec[3].c_str(),"%lf,%lf",&beginnum,&endnum);
			if( fabs(pShip->draft) < 1e-3|| pShip->draft < endnum + 1e-3 && pShip->draft > beginnum - 1e-3)
				isStillAlive = true;
			else{
				if(isStillAlive && (andor/100)%10 == 0)
					isStillAlive = true;
				else
					isStillAlive = false;
			}

			sscanf(thirdconvec[4].c_str(),"%lf,%lf",&beginnum,&endnum);
			if( fabs(pShip->speed) < 1e-3|| pShip->speed < endnum + 1e-3  && pShip->speed > beginnum - 1e-3)
				isStillAlive = true;
			else{
				if(isStillAlive && (andor/10)%10 == 0)
					isStillAlive = true;
				else
					isStillAlive = false;
			}

			sscanf(thirdconvec[5].c_str(),"%lf,%lf",&beginnum,&endnum);
			if( abs(pShip->dwt) < 1e-3|| pShip->dwt < endnum + 1e-3 && pShip->dwt > beginnum - 1e-3)
				isStillAlive = true;
			else{
				if(isStillAlive && andor%10 == 0)
					isStillAlive = true;
				else
					isStillAlive = false;
			}

			if(!isStillAlive) {
				vecit = df.vecMmsi.erase(vecit);continue;
			}
			vecit++;
		}
		it++;
	}
	SYSTEM_LOG("[FleetSvc::calDynamicFleet3] end =======================");
}

void FleetSvc::saveToMysql()
{
	SYSTEM_LOG("[FleetSvc::saveToMysql] begin =======================");

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;

	int nCount = 0, nCount2 = 0;
	MapDFIter it = m_mapDF.begin();
	string sql = "";
	while(it!=m_mapDF.end())
	{
		DynamicFleet& onefleet = it->second;

		SYSTEM_LOG("[FleetSvc::saveToMysql] uid:%s, cid:%s, count:%d =======================", onefleet.userid.c_str(), onefleet.categoryid.c_str(), (int)onefleet.vecMmsi.size());

		sql = "delete from boloomodb.t41_fleet_ship_dynamic where userid = '"+onefleet.userid + "' and categoryid ='" + onefleet.categoryid+"'";
		psql->Execute(sql.c_str());

		if(onefleet.vecMmsi.size() == 0) {it++;continue;}
		nCount2 = 0;
		sql = "insert ignore into boloomodb.t41_fleet_ship_dynamic (userid,shipid,categoryid,mmsi,shipname,update_dt) values ";
		for(int i = 0;i<(int)onefleet.vecMmsi.size();i++)
		{
			ShipData* pShip = g_VesselSvc::instance()->aisShip.FindShip(onefleet.vecMmsi[i]);
			if(!pShip)
				continue;

			string tttsql = FormatString(" ('%s','%s','%s','%d','%s','%s')",onefleet.userid.c_str(),pShip->shipid,onefleet.categoryid.c_str(),pShip->mmsi,pShip->name,gmt0now);//pShip->name
			if (nCount2>0)
				sql+=",";
			sql+=tttsql;

			nCount++;
			nCount2++;
			if(nCount2 >= 500)//500
			{
				psql->Execute(sql.c_str());

				nCount2 = 0;
				sql = "insert ignore into boloomodb.t41_fleet_ship_dynamic (userid,shipid,categoryid,mmsi,shipname,update_dt) values ";
			}
		}		
		it++;
		if(nCount2 > 0)
		{
			psql->Execute(sql.c_str());
		}
	}
	
	m_mapDF.clear();
	SYSTEM_LOG("[FleetSvc::saveToMysql] end =======================");
	RELEASE_MYSQL_NORETURN(psql);
}

//zhuxj
int	FleetSvc::_AddFleetWeb(const char *usrid, int mmsi, const char *shipid, const char *shipname, const char *alias, MySql *psql, std::stringstream& out)
{
	char gmt0now[20];
    GmtNow(gmt0now);
	char sql[1024] = "";
	sprintf (sql, "INSERT IGNORE INTO t41_fleet_category(fleet_id,categoryid,categoryname,update_dt,create_dt,monitor_flag,remark_num,trash_flag) VALUES ('%s','00','My Fleet','%s', '%s','0','0','0')", 
			usrid, gmt0now, gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	
    sprintf(sql, "replace into t41_fleet_ship(fleet_id,categoryid,shipid,mmsi,shipname,alias,monitor_flag,trash_flag,remark_num,valid_flag,update_dt,create_dt) values('%s','00','%s',%d,'%s','%s',0,0,0,'1','%s','%s')",
			usrid, shipid, mmsi, SqlReplace(string(shipname)).c_str(), SqlReplace(string(alias)).c_str(),gmt0now,gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
int	FleetSvc::_GetFleetStatisticWeb(const char *shipid, MySql *psql, std::stringstream& out)
{
	char sql[1024] = "";
	
    sprintf(sql, "SELECT fleet_id FROM t41_fleet_ship WHERE shipid = '%s' and trash_flag = '0'", shipid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	out << "{users:[";
	while (psql->NextRow())
	{
		char uid[32] = "";
		READMYSQL_STR(fleet_id, uid);
		if (cnt++)
			out << ",";
		out << "{uid:\"" << uid << "\"}";
	}

	out << "],cnt:" << cnt << "}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int	FleetSvc::_ConfirmFleet(const char *uid, const char *shipid, MySql *psql, std::stringstream& out)
{
	char sql[1024] = "";
	
    sprintf(sql, "SELECT COUNT(1) as num FROM t41_fleet_ship WHERE shipid = '%s' and fleet_id = '%s' and trash_flag = '0'", shipid, uid);
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	int num = 0;
	READMYSQL_INT(num, num, 0);

	if (num)
		out << "{\"eid\":0}";
	else
		out << "{\"eid\":100}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//zhuxj
//{uid:"",shipid:"",mmsi:123,shipname:"",shipalias:"",type:0/1/2}0添加船到默认船队/1从船队中删除船舶/2获取添加这艘船到船队的用户统计
int FleetSvc::FleetManagesWeb(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::FleetManagesWeb]bad format:", jsonString, 1);
    //string catId = root.getv("ci", "");
	string uid = root.getv("uid", "");
    string shipid = root.getv("shipid", "");
    int mmsi = root.getv("mmsi", 0);
    string shipname = root.getv("shipname", "");
    string shipalias = root.getv("shipalias", "");
    int type = root.getv("type", 0);

    MySql *psql = CREATE_MYSQL;
    switch (type)
    {
    case 0:
        return _AddFleetWeb(uid.c_str(),mmsi,shipid.c_str(),shipname.c_str(),shipalias.c_str(),psql,out);

    case 1:
        return _DelFleetVsl(uid.c_str(),mmsi,shipid.c_str(),psql,out);

    case 2:
        return _GetFleetStatisticWeb(shipid.c_str(),psql,out);  

	case 3:
		return _ConfirmFleet(uid.c_str(), shipid.c_str(), psql, out);
	}

    RELEASE_MYSQL_RETURN(psql, 2);
}

// {uid:"caiwj",ci1:"xxxx",ci2:"xxxxxxx",si:"S1314141"}
int FleetSvc::FleetShipMove(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::FleetShipMove]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string ci1 = root.getv("ci1", "");
	string ci2 = root.getv("ci2", "");
	string si = root.getv("si", "");

	if(uid.empty() || ci2.empty() || ci2.empty() || si.empty())
		return 1;

	char sql[1024];
	sprintf(sql, "UPDATE t41_fleet_ship SET categoryid = '%s' WHERE fleet_id = '%s' AND categoryid = '%s' AND shipid = '%s'", ci2.c_str(), uid.c_str(), ci1.c_str(), si.c_str());
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}
/////////////////////////////// 事件模板接口////////////////////////////////
// 获取所有事件模板
/*
{seq:”aa”,
templates:[ 单个订阅模版,,,]
}
{tid:””,
tname:””,
emailevent: [“eid|type|aid/pid|name”,…],
smsevent: [“eid|type|aid/pid|name”,…],
emailtime:”00:30|12:30|||||||||”,
smstime:” 00:30|12:30|||||||||”,
email:”chengzhl@boloomo.com||||”,
sms:”+86 2343242342||||”,
ships:”24234|shipname@@|”,
starttime:1232343243,
endtime:24242342343
}
*/
bool isNum(const string &i_strData)
{
	bool bRlt=true;
	for(int i=0;i<i_strData.length();i++)
	{
		if(i_strData.at(i)=='0')
			;
		else if(i_strData.at(i)=='1')
			;
		else if(i_strData.at(i)=='2')
			;
		else if(i_strData.at(i)=='3')
			;
		else if(i_strData.at(i)=='4')
			;
		else if(i_strData.at(i)=='5')
			;
		else if(i_strData.at(i)=='6')
			;
		else if(i_strData.at(i)=='7')
			;
		else if(i_strData.at(i)=='8')
			;
		else if(i_strData.at(i)=='9')
			;
		else	
			bRlt=false;
	}
	return bRlt;
}
int FleetSvc::GetAllEventTmp(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::GetAllEventTmp]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");
	string strUid = root.getv("uid", "");

	MySql *psql = CREATE_MYSQL;

	//获取所有模板类
	string csql="select TEMPLATE_ID,SUB_TYPE,TEMPLATE_NAME,VALID_STARTDT,VALID_ENDDT,BIND_SHIPS,SUB_BINDS,SUB_SENDTIME,SUB_EVENTS from aisdb.t41_aisevt_subscribe_template where USER_ID='"+strUid+"'";
	CHECK_MYSQL_STATUS(psql->Query(csql.c_str()), 3);

	if (psql->GetRowCount()==0)//
	{
//		char temp[512];
//		sprintf(temp,"ci:\"%s\",ts:%d,rn:%d,mf:%d,als:%d,ale:%d,em:\"%s\",tn:\"%s\"",_Category.catId,_Category.trash_flag,_Category.remark_num,_Category.auto_flag,_Category.sms,_Category.email,_Category.emailStr.c_str(),_Category.telnoStr.c_str());
		out<<"{\"seq\":\""<<strSeq.c_str()<<"\""
			<<",\"templates\":[]}";
	}
	else
	{
		ST_EVENTTMP stETmp;
		map<string,ST_EVENTTMP> mapETmp;
		char szTmp[6144]={'\0'};
		char nType;
		AisRealtime* pShipInfo;
		while(psql->NextRow())
		{					
			READMYSQL_STR(TEMPLATE_ID, stETmp.m_szTId);
			READMYSQL_STR(TEMPLATE_NAME, stETmp.m_szTName);
			READMYSQL_INT(VALID_STARTDT, stETmp.m_lStartTime,0); //有效期
			READMYSQL_INT(VALID_ENDDT, stETmp.m_lEndTime,0); // 有效期结束			
			READMYSQL_INT(SUB_TYPE, nType,0);// 1邮件 2短信
			if(nType==1)
			{				
				READMYSQL_STR(SUB_BINDS, szTmp);// 邮箱或手机号码列表
				stETmp.m_strEmail=szTmp;
				READMYSQL_STR(SUB_SENDTIME, szTmp);
				stETmp.m_strEmailTime=szTmp;
				READMYSQL_STR(SUB_EVENTS, szTmp);
				stETmp.m_strEmailEvent=szTmp;
			}
			else if(nType==2)
			{
				READMYSQL_STR(SUB_BINDS, szTmp);// 邮箱或手机号码列表
				stETmp.m_strSms=szTmp;
				READMYSQL_STR(SUB_SENDTIME, szTmp);
				stETmp.m_strSmsTime=szTmp;
				READMYSQL_STR(SUB_EVENTS, szTmp);
				stETmp.m_strSmsEvent=szTmp;
			}
			string strId=stETmp.m_szTId;
			if(mapETmp.find(strId)==mapETmp.end())
			{				
				READMYSQL_STR(BIND_SHIPS, szTmp);// 事件船舶
				string strShips=szTmp;			
				
				Tokens svrList;	
				if(strShips.find("|") != string::npos)
				{					 
					svrList = StrSplit(strShips, "|");
					for(int i=0; i<(int)svrList.size(); i++)
					{
						if(svrList[i].empty())
							continue;
						if(!isNum(svrList[i]))
							continue;
//						long lmmsi=atol(svrList[i].c_str());
						//						
						pShipInfo=g_VesselSvc::instance()->aisShip.FindAis(atoi(svrList[i].c_str()));
						if(pShipInfo==NULL)
						{							
							continue;
						}
						stETmp.m_strShips+=svrList[i];
						stETmp.m_strShips+="|";
						stETmp.m_strShips+=pShipInfo->name;
						stETmp.m_strShips+="@@";
					}
					if(stETmp.m_strShips.length()>3)
					{
						stETmp.m_strShips.erase(stETmp.m_strShips.length()-2,2);
					}
				}
				else
				{
					stETmp.m_strShips="";
				}
				mapETmp[strId]=stETmp;
			}
			else
			{
				if(nType==1)
				{
					mapETmp[strId].m_strEmail=stETmp.m_strEmail;
					mapETmp[strId].m_strEmailTime=stETmp.m_strEmailTime;
					mapETmp[strId].m_strEmailEvent=stETmp.m_strEmailEvent;
				}
				else if(nType==2)
				{
					mapETmp[strId].m_strSms=stETmp.m_strSms;
					mapETmp[strId].m_strSmsTime=stETmp.m_strSmsTime;
					mapETmp[strId].m_strSmsEvent=stETmp.m_strSmsEvent;
				}
			}
			
		}
		//封装返回串
		out<<"{\"seq\":\""<<strSeq.c_str()<<"\""
			<<",\"templates\":[";
		std::map<string,ST_EVENTTMP>::iterator it=mapETmp.begin();
		for (; it!=mapETmp.end(); it++)
		{
			ST_EVENTTMP stData=it->second;		
			if (it!=mapETmp.begin())
				out<<',';
			char temp[8192]={'\0'};
			sprintf(temp,\
				"tid:\"%s\",tname:\"%s\",emailevent:[%s],smsevent:[%s],emailtime:\"%s\",smstime:\"%s\",email:\"%s\",sms:\"%s\",ships:\"%s\",starttime:%d,endtime:%d",\
				stData.m_szTId,stData.m_szTName,stData.m_strEmailEvent.c_str(),stData.m_strSmsEvent.c_str(),stData.m_strEmailTime.c_str(),stData.m_strSmsTime.c_str(),\
				stData.m_strEmail.c_str(),stData.m_strSms.c_str(),stData.m_strShips.c_str(),stData.m_lStartTime,stData.m_lEndTime);
			out<<"{"<<temp<<"}";		 
		}
		out<<"]}";		
		mapETmp.clear();
	}
	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}
// 添加事件模板
/*
Mid:  sid:
{seq:”aa”,uid:”chengzhl”, 
tname:””,
emailevent: [“eid|type|aid/pid|name”,…],
smsevent: [“eid|type|aid/pid|name”,…],
emailtime:”00:30|12:30|||||||||”,
smstime:” 00:30|12:30|||||||||”,
email:”chengzhl@boloomo.com||||”,
sms:”+86 2343242342||||”,
ships:”24234|shipname|||”,
starttime:1232343243,
endtime:24242342343
}
Seq异步标志
Uid用户id
Tname 模版名称
Emailevent邮件订阅的事件
Smsevent短信订阅的事件
Emailtime邮件订阅的时间 00:30
Smstime短信订阅的时间
Email 收件邮箱 
Sms 收信号码
Mmsi适用的船舶mmsi
订阅开始utc时间，订阅结束utc时间

返回数据
{ seq: "ssssssss001",eid:0,tid=””} 
Seq异步标志
tid模版id
Eid 为0成功，其他均为失败
*/
int FleetSvc::AddEventTmp(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::AddEventTmp]bad format:", jsonString, 1);
//	FILE *pf;
	/*pf=fopen("/tmp/json1.log","wb");
	fwrite(jsonString,sizeof(char),strlen(jsonString),pf);
	fclose(pf);*/
	ST_EVENTTMP stETmp;
	string strSeq = root.getv("seq", "");
	string strUid = root.getv("uid", "");
	string strTname=root.getv("tname", "");
//	string strEmailevent=root.getv("emailevent", "");
//	string strSmsevent=root.getv("smsevent", "");
	string strEmailtime=root.getv("emailtime", "");
	string strSmstime=root.getv("smstime", "");
	string strEmail=root.getv("email", "");
	string strSms=root.getv("sms", "");
	string strShips=root.getv("ships", "");
	long lStarttime=root.getv("starttime",-1);
	long lendtime=root.getv("endtime",-1);
	//
	Json* pEmailEvent = root.get("emailevent");
	int len = 0;
	string strEmailevent="";
	for (int i = 0; i < pEmailEvent->size(); i++)
	{
		strEmailevent+="\"";
		strEmailevent+=pEmailEvent->getv(i, "");
		strEmailevent+="\"";
		strEmailevent+=",";
	}
	if(strEmailevent.length()>0)
		strEmailevent.erase(strEmailevent.length()-1,1);
	//
	Json* pSmsEvent = root.get("smsevent");
	len = 0;
	string strSmsevent="";
	for (int i = 0; i < pSmsEvent->size(); i++)
	{
		strSmsevent+="\"";
		strSmsevent+=pSmsEvent->getv(i, "");
		strSmsevent+="\"";
		strSmsevent+=",";
	}
	if(strSmsevent.length()>0)
		strSmsevent.erase(strSmsevent.length()-1,1);
	//
	string strTid=strUid;
	char szTmp[32];
	long ltime =time(0);
	sprintf(szTmp,"%d",ltime);
	strTid+=szTmp;//=+szTmp;//itoa(ltime,szTmp,1);
	//
	char gmt0now[20];
    GmtNow(gmt0now);

	char sql[4096];
	MySql *psql = CREATE_MYSQL;
	//插入email记录
	sprintf(sql,\
		"insert into aisdb.t41_aisevt_subscribe_template(USER_ID,TEMPLATE_ID,SUB_TYPE,TEMPLATE_NAME,VALID_STARTDT,VALID_ENDDT,BIND_SHIPS,SUB_BINDS,SUB_SENDTIME,SUB_EVENTS,UPDATE_DT,IS_VALID) \
		values ('%s','%s',%d,'%s',%d,%d,'%s','%s','%s','%s','%s',1)",strUid.c_str(),strTid.c_str(),1,strTname.c_str(),lStarttime,lendtime,strShips.c_str(),\
		strEmail.c_str(),strEmailtime.c_str(),strEmailevent.c_str(),gmt0now,1);
	/*pf=fopen("/tmp/sql1.log","wb");
	fwrite(sql,sizeof(char),strlen(jsonString),pf);
	fclose(pf);*/
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	//插入sms记录
	sprintf(sql,\
		"insert into aisdb.t41_aisevt_subscribe_template(USER_ID,TEMPLATE_ID,SUB_TYPE,TEMPLATE_NAME,VALID_STARTDT,VALID_ENDDT,BIND_SHIPS,SUB_BINDS,SUB_SENDTIME,SUB_EVENTS,UPDATE_DT,IS_VALID) \
		values ('%s','%s',%d,'%s',%d,%d,'%s','%s','%s','%s','%s',1)",strUid.c_str(),strTid.c_str(),2,strTname.c_str(),lStarttime,lendtime,strShips.c_str(),\
		strSms.c_str(),strSmstime.c_str(),strSmsevent.c_str(),gmt0now,1);
	/*pf=fopen("/tmp/sql2.log","wb");
	fwrite(sql,sizeof(char),strlen(jsonString),pf);
	fclose(pf);*/
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
 
	//封装返回串
	out<<"{\"seq\":\""<<strSeq.c_str()<<"\""
		<<",\"eid\":0,\"tid\":\""<<strTid.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}
// 删除事件模板
/*
Mid:  sid:
{seq:”aa”,uid:”chengzhl”,tid:”234”}
Seq异步标志
Uid用户id
Tid 模版id
返回数据
{ seq: "ssssssss001",eid:0} 
Seq异步标志
Eid 为0成功，其他均为失败
*/
int FleetSvc::DeleteEventTmp(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::DeleteEventTmp]bad format:", jsonString, 1);	  
	string strSeq = root.getv("seq", "");
	string strUid = root.getv("uid", "");
	string strTid = root.getv("tid", "");

	MySql *psql = CREATE_MYSQL;
	//插入email记录
	char* dyshpsql = "delete from aisdb.t41_aisevt_subscribe_template where USER_ID = '%s' and TEMPLATE_ID = '%s'";
	string t = FormatString(dyshpsql,strUid.c_str(),strTid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(t.c_str())>=0, 3);
 
	//封装返回串
	out<<"{\"seq\":\""<<strSeq.c_str()<<"\""
		<<",\"eid\":0}";

	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}
// 修改事件模板
/*
Mid:  sid:
{seq:”aa”,uid:”chengzhl”,
 tid:””,
tname:””,
emailevent: [“eid|type|aid/pid|name”,…],
smsevent: [“eid|type|aid/pid|name”,…],
emailtime:”00:30|12:30|||||||||”,
smstime:” 00:30|12:30|||||||||”,
email:”chengzhl@boloomo.com||||”,
sms:”+86 2343242342||||”,
ships:”24234|shipname|||”,
starttime:1232343243,
endtime:24242342343
}
*/
int FleetSvc::UpdateEventTmp(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[FleetSvc::UpdateEventTmp]bad format:", jsonString, 1);
	ST_EVENTTMP stETmp;
	string strSeq = root.getv("seq", "");
	string strUid = root.getv("uid", "");
	string strTid = root.getv("tid", "");
	string strTname=root.getv("tname", "");
//	string strEmailevent=root.getv("emailevent", "");
//	string strSmsevent=root.getv("smsevent", "");
	string strEmailtime=root.getv("emailtime", "");
	string strSmstime=root.getv("smstime", "");
	string strEmail=root.getv("email", "");
	string strSms=root.getv("sms", "");
	string strShips=root.getv("ships", "");
	long lStarttime=root.getv("starttime",-1);
	long lendtime=root.getv("endtime",-1);
	//
	Json* pEmailEvent = root.get("emailevent");
	int len = 0;
	string strEmailevent="";
	for (int i = 0; i < pEmailEvent->size(); i++)
	{
		strEmailevent+="\"";
		strEmailevent+=pEmailEvent->getv(i, "");
		strEmailevent+="\"";
		strEmailevent+=",";
	}
	if(strEmailevent.length()>0)
		strEmailevent.erase(strEmailevent.length()-1,1);
	//
	Json* pSmsEvent = root.get("smsevent");
	len = 0;
	string strSmsevent="";
	for (int i = 0; i < pSmsEvent->size(); i++)
	{
		strSmsevent+="\"";
		strSmsevent+=pSmsEvent->getv(i, "");
		strSmsevent+="\"";
		strSmsevent+=",";
	}
	if(strSmsevent.length()>0)
		strSmsevent.erase(strSmsevent.length()-1,1);
	char gmt0now[20];
    GmtNow(gmt0now);
	//
	char sql[4096];
	MySql *psql = CREATE_MYSQL;
	//修改email记录
	sprintf(sql,\
		"update aisdb.t41_aisevt_subscribe_template set TEMPLATE_NAME='%s',VALID_STARTDT=%d,VALID_ENDDT=%d,BIND_SHIPS='%s',\
		SUB_BINDS='%s',SUB_SENDTIME='%s',SUB_EVENTS='%s',UPDATE_DT='%s',IS_VALID=1 where USER_ID='%s' and TEMPLATE_ID='%s'and SUB_TYPE=1" ,\
		strTname.c_str(),lStarttime,lendtime,strShips.c_str(),strEmail.c_str(),strEmailtime.c_str(),strEmailevent.c_str(),gmt0now,strUid.c_str(),strTid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	//修改sms记录
	sprintf(sql,\
		"update aisdb.t41_aisevt_subscribe_template set TEMPLATE_NAME='%s',VALID_STARTDT=%d,VALID_ENDDT=%d,BIND_SHIPS='%s',\
		SUB_BINDS='%s',SUB_SENDTIME='%s',SUB_EVENTS='%s',UPDATE_DT='%s',IS_VALID=1 where USER_ID='%s' and TEMPLATE_ID='%s'and SUB_TYPE=2" ,\
		strTname.c_str(),lStarttime,lendtime,strShips.c_str(),strSms.c_str(),strSmstime.c_str(),strSmsevent.c_str(),gmt0now,strUid.c_str(),strTid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
 
	//封装返回串
	out<<"{\"seq\":\""<<strSeq.c_str()<<"\""
		<<",\"eid\":0}";

	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}


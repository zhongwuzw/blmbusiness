#include "blmcom_head.h"
#include "BerthSvc.h"
#include "UserRegSvc.h"
#include "MessageService.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include "NotifyService.h"
#include <iostream>
#include <fstream>

using namespace std;

IMPLEMENT_SERVICE_MAP(BerthSvc)

BerthSvc::BerthSvc()
{

}

BerthSvc::~BerthSvc()
{

}

bool BerthSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_BERTH, this))
        return false;

	//int interval = 86400;
	//int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
	//if(timerId <= 0)
	//	return false;

    SERVICE_MAP(SID_GET_PORT_BERTH_LIST,BerthSvc,getPortBerthList);
	SERVICE_MAP(SID_GET_BERTH_INFO,BerthSvc,getBerthInfo);
	SERVICE_MAP(SID_GET_BERTH_COMPINFO,BerthSvc,getBerthCompInfo);
	SERVICE_MAP(SID_GET_BERTH_SHIPINFO,BerthSvc,getBerthShipInfo);
	SERVICE_MAP(SID_GET_BERTH_MYWINFO,BerthSvc,getBerthMYWInfo);

	SERVICE_MAP(SID_GET_EQUIPMENTLIST,BerthSvc,getEquipmentList);
	SERVICE_MAP(SID_GET_STORAGELIST,BerthSvc,getStorageList);

	
	
	//SERVICE_MAP(SID_GET_BERTH_CAMERAINFO,BerthSvc,getBerthCameraInfo);
	SERVICE_MAP(SID_GET_COMP_MANAGER_INFO,BerthSvc,getBerthCompManagerInfo);
	SERVICE_MAP(SID_GET_TERMINAL_INFO,BerthSvc,getTerminalInfo);
	SERVICE_MAP(SID_GET_BERTH_STATICINFO,BerthSvc,getStatInfo);
	SERVICE_MAP(SID_NEW_BERTH_ITEM,BerthSvc,newBerthItem);
	SERVICE_MAP(SID_DEL_BERTH_ITEM,BerthSvc,delBerthItem);
	SERVICE_MAP(SID_GET_TERMINAL_LIST,BerthSvc,getTerminalList);
	SERVICE_MAP(SID_NEW_TERMINAL_ITEM,BerthSvc,newTerminalItem);
	SERVICE_MAP(SID_DEL_TERMINAL_ITEM,BerthSvc,delTerminalItem);
	SERVICE_MAP(SID_UDP_COMP_MANAGER_INFO,BerthSvc,udpOperatorsDetail);	

	SERVICE_MAP(SID_GET_BERTH_MOOR_SHIPINFO,BerthSvc,getMoorShipInfo);
	SERVICE_MAP(SID_GET_BERTH_CARGOINFO,BerthSvc,getCargoInfo);
	SERVICE_MAP(SID_UPD_SHIP_BASEINFO,BerthSvc,updShipBaseInfo);
	SERVICE_MAP(SID_UPD_INOUT_INFO,BerthSvc,updInOutInfo);	
	SERVICE_MAP(SID_GET_BERTH_INOUTSTAT,BerthSvc,getInOutInfo);
	SERVICE_MAP(SID_GET_BERTH_INOUTLIST,BerthSvc,getInOutList);
	SERVICE_MAP(SID_GET_UNDEAL_WARNING,BerthSvc,getUndealWarn);
	SERVICE_MAP(SID_EXCL_WARNING,BerthSvc,exclWarning);
	SERVICE_MAP(SID_GET_BERTH_STATICINDEX,BerthSvc,getStatIndex);
	SERVICE_MAP(SID_GET_BERTH_INOUTDETAIL,BerthSvc,getInOutDetail);
	SERVICE_MAP(SID_GET_SHIP_CONTACTINFO,BerthSvc,getContactInfo);
	SERVICE_MAP(SID_ADD_SHIP_CONTACTPERSON,BerthSvc,addShipContactPerson);
	SERVICE_MAP(SID_DEL_SHIP_CONTACTPERSON,BerthSvc,delShipContactPerson);

	SERVICE_MAP(SID_GET_USER_AUTHORITY,BerthSvc,getUserAuth);
	SERVICE_MAP(SID_NEW_BERTH_EQUIPMENT,BerthSvc,newBerthEquipment);
	SERVICE_MAP(SID_DEL_BERTH_EQUIPMENT,BerthSvc,delBerthEquipment);
	SERVICE_MAP(SID_NEW_TERMINAL_STORAGE,BerthSvc,newTerminalStorage);
	SERVICE_MAP(SID_DEL_TERMINAL_STORAGE,BerthSvc,delTerminalStorage);

	SERVICE_MAP(SID_DEL_THRUPT,BerthSvc,delThruptInfo);
	
	SERVICE_MAP(SID_GET_SHIPDETAIL,BerthSvc,getShipDetail);
	SERVICE_MAP(SID_UPD_SHIPDETAIL,BerthSvc,udpShipDetail);

	SERVICE_MAP(SID_GET_SYSTEM_USER,BerthSvc,getSystemUser);	
	SERVICE_MAP(SID_UPD_USER_INFO,BerthSvc,updUserInfo);
	SERVICE_MAP(SID_DEL_USER_INFO,BerthSvc,delUserInfo);
	SERVICE_MAP(SID_UPD_USER_PASSWORD,BerthSvc,updUserPassword);
	SERVICE_MAP(SID_CHECK_USER,BerthSvc,checkUser);



	SERVICE_MAP(SID_SYSTEM_AUTHROTY,BerthSvc,getSystemAuthrity);
	SERVICE_MAP(SID_SET_USER_AUTH,BerthSvc,setUserAuth);
	SERVICE_MAP(SID_GET_PORT_ORGAN,BerthSvc,getPortOrgan);
	SERVICE_MAP(SID_GET_CONCERN_SHIPS,BerthSvc,getConcernShips);
	SERVICE_MAP(SID_ADD_CONCERN_SHIPS,BerthSvc,addConcernShips);
	SERVICE_MAP(SID_DEL_CONCERN_SHIPS,BerthSvc,delConcernShips);


	SERVICE_MAP(SID_GET_COMP_MANAGER_LIST,BerthSvc,getCompanyList);
	SERVICE_MAP(SID_DEL_COMPANY,BerthSvc,delCompanyItem);
	SERVICE_MAP(SID_GET_ALL_ALERTS,BerthSvc,getAlertList);
	SERVICE_MAP(SID_GET_ALERT_USERS,BerthSvc,getAlertUsers);
	SERVICE_MAP(SID_SET_ALERT_USERS,BerthSvc,setAlertUsers);

	SERVICE_MAP(SID_GET_REMOVESHIP,BerthSvc,getRemoveShip);
	SERVICE_MAP(SID_DEL_REMOVESHIP,BerthSvc,delRemoveShip);
	SERVICE_MAP(SID_GET_LOCALSHIP,BerthSvc,getLocalShip);
	SERVICE_MAP(SID_GET_SHIPBYNAME,BerthSvc,searchShipByName);
	SERVICE_MAP(SID_SET_ALERT_LEVEL,BerthSvc,setAlertLevel);	
	

    DEBUG_LOG("[BerthSvc::Start] OK......................................");
	//std::stringstream out;
	//setAlertUsers("xinjingbingmo", " {objid:\"T12121224\",objname:\"码头名称\",portid:\"56646\",usrs:[{telno:\"13810653825\",usrname:\"tt\"},{telno:\"\",usrname:\"zhangmin\"},{telno:\"15311206727\",usrname:\"gongyupeng-1\"}]}", out);
	return true;
}

//zhuxj
int BerthSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{	
	time_t t = time(0);
	struct tm *ptr = gmtime(&t);
	if (ptr->tm_mday == 20)
	{
		SYSTEM_LOG("[BerthSvc::handle_timeout] begin deal warn table auto expired ==============");
		MySql* psql = CREATE_MYSQL;

		char gmt0now[20];
		GmtNow(gmt0now);

		char sql[1024];
		sprintf(sql, "UPDATE boloomodb.t41_berth_warning SET `level` = '2', `desc` = '%s', update_dt = '%s' WHERE `status` = '0' AND `level` <> '2'", 
			CodeConverter::Gb2312ToUtf8("即将逾期").c_str(), gmt0now,gmt0now);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 0);

		SYSTEM_LOG("[BerthSvc::handle_timeout] end deal warn table auto expired, count:%d ==============", psql->GetAffectedRows());
		RELEASE_MYSQL_RETURN(psql, 0);
	}
	return 0;
}

//{portid:"56646",harborid:["",""]}
int BerthSvc::getPortBerthList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getPortBerthList]bad format:", jsonString, 1);
	string portid = root.getv("portid", "");
	Json* harborid = root.get("harborid");
	char harborids[512] = {'\0'};
	int len = 0;
	for (int i = 0; i < harborid->size(); i++)
	{
		if (len)
			len += sprintf (harborids + len, ",");
		len += sprintf (harborids + len, "'%s'", harborid->getv(i, ""));
	}

	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	len = 0;
	len = sprintf (sql, "SELECT t1.Berth_id, t1.Name AS bnm, t1.ACT_CODE, t1.Terminal_id as dockid,t2.Name as dockname,t2.AreaName AS harborid, t1.coordinate, t3.NAME AS hnm, t1.Length, t1.TRESTLE_NUM, \
								t1.DWT_YEAR, t1.COMPLETE_DATE, t1.ACCEPT_DATE, t9.ISSURE_DT, t9.CERT_NO, t9.CERT_ID,t9.owner AS isname, t1.TRESTLE_MAX_LENGTH, t1.TRESTLE_MAX_WIDTH,\
								t1.Depth, t1.WIDTH, t1.VERTICAL_REFERENCE, t1.DWT, t1.Remark,t6.name AS OWNER, t7.name AS operation, t1.style_des,t11.STATUS \
								FROM t41_port_berth t1 LEFT JOIN t41_port_terminal t2 ON t1.Terminal_id = t2.Terminal_id\
								LEFT JOIN t41_port_harbour t3 ON t2.AreaName = t3.PORTAREA_ID\
								LEFT JOIN t41_port_service_contacts t5 ON t1.Berth_id = t5.EntityID \
								LEFT JOIN t41_company t6 ON t5.Company_Key = t6.Company_Key AND t5.ServiceType = '213'\
								LEFT JOIN t41_company t7 ON t5.Company_Key = t7.Company_Key AND t5.ServiceType = '203'\
								LEFT JOIN t41_port_certification t9 ON t1.Berth_id = t9.BERTH_ID \
								LEFT JOIN t41_pmonitor_berth_stat t11 ON t1.BERTH_ID = t11.BERTH_ID \
								WHERE t1.Portid = '%s' AND t1.Is_valid='1'", portid.c_str());
	if (harborid->size())
		len += sprintf (sql + len, " AND t1.HARBOUR_ID IN (%s)", harborids);

	sprintf (sql + len, " ORDER BY t1.Berth_id");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	char bid[100] = "";
	char bnm[100] = "";
	char dockid[100]="";
	char dockname[100]="";
	char bcode[32] = "";
	char hnm[100] = "";	
	char lbd[100] = "";
	double bcap = 0.0;
	char mainuse[1024] = "";
	double x = 0.0;
	double y = 0.0;
	char hid[100] = "";
	char _tcoor[1024] = "";
	time_t t = time(0);
	StrSet owner;					//拥有人/所有人std::set<string>
	StrSet operations;				//经营人(可能多个)std::set<string>
	char finishdate[32] = "";		//交工日期
	char acceptdate[32] = "";		//竣工验收时间
	char axdate[32] = "";			//岸线审批时间
	char axrefnum[64] = "";			//岸线审批文号
	char anunit[256] = "";			//岸线审批单位
	char axid[100] = "";			//岸线审批单位
	char structtp[32] = "";			//结构形式
	int trestle_num = 0;			//栈桥数量
	char trestle_lb[100] = "";		//栈桥最长*最宽 格式如:30*3
	int depassablty = 0;			//设计综合通过能力
	int curstatus = 0;
	char lon[32], lat[32];
	int cnt = -1;

	out << "{portid:\"" << portid << "\",harborid:\"" << harborid << "\",berths:[";
	while (psql->NextRow())
	{
		char _bid[100] = "";
		char _operation[100] = "";
		char _owner[100] = "";		
		READMYSQL_STR(Berth_id, _bid);
		READMYSQL_STR(OWNER, _owner);
		READMYSQL_STR(operation, _operation);
		if (!strlen(bid) || strcmp(_bid, bid))
		{
			if (cnt++ > 0)
				out << ",";
			if (cnt > 0)
			{
				out << "{bid:\"" << bid << "\",bnm:\"" << JsonReplace(bnm) << "\",berthcode:\"" << JsonReplace(bcode) << "\",hid:\"" << hid<< "\",dockid:\"" << dockid << "\",dockname:\"" << dockname << "\",x:\"" << lon << "\",y:\"" << lat << "\",hnm:\"" << hnm << "\",lbdphigh:\"" << lbd << "\",bcap:\"" << bcap << "\",mainuse:\"" << mainuse << "\",curstatus:\"" << curstatus 
					<< "\",owner:\"" << ToStr(owner) << "\",operations:\"" << ToStr(operations) << "\",structtp:\"" << structtp << "\",trestle_num:\"" << trestle_num << "\",trestle_lb:\"" << trestle_lb << "\",finishdate:\"" << finishdate << "\",acceptdate:\"" << acceptdate 
					<< "\",depassablty:\"" << depassablty << "\",anunit:\"" << anunit << "\",axdate:\"" << axdate<< "\",ax_id:\"" << axid << "\",axrefnum:\"" << axrefnum << "\"}";
				operations.clear();
				owner.clear();				
			}
			double length, width, depth, pmgc;
			x = 0.0;
			y = 0.0;
			strcpy(bid, _bid);
			if (strlen(_owner))
				owner.insert(_owner);
			if (strlen(_operation))
				operations.insert(_operation);

			READMYSQL_STR(dockid, dockid);
			READMYSQL_STR(dockname, dockname);
			READMYSQL_STR(bnm, bnm);
			READMYSQL_STR(ACT_CODE, bcode);
			READMYSQL_STR(harborid, hid);
			READMYSQL_STR(coordinate, _tcoor);
			Tokens xys = StrSplit(_tcoor, "|");
			int _tcnt = 0;
			for (unsigned int _i = 0; _i < xys.size(); _i++)
			{
				Tokens _xy = StrSplit(xys[_i], ",");
				if (_xy.size() == 2)
				{
					x += ACE_OS::atof(_xy[0].c_str());
					y += ACE_OS::atof(_xy[1].c_str());
					_tcnt++;
				}
			}
			if (_tcnt)
			{
				x /= _tcnt;
				y /= _tcnt;
			}
			
			sprintf (lon, "%.7lf", x);
			sprintf (lat, "%.7lf", y);
			READMYSQL_STR(hnm, hnm);
			READMYSQL_DOUBLE(Length, length, -1.0);
			READMYSQL_DOUBLE(Depth, depth, -1.0);
			READMYSQL_DOUBLE(WIDTH, width, -1.0);
			READMYSQL_DOUBLE(VERTICAL_REFERENCE, pmgc, -1.0);
			int len = 0;
			if (length > 0.0)
				len = sprintf (lbd, "%.1f", length);
			else
				len = sprintf (lbd, "%s", "N/A");

			if (width > 0.0 && depth > 0.0)
				len += sprintf (lbd + len, "*%.1f*%.1f", width, depth);
			else if (width > 0.0)
				len += sprintf (lbd + len, "*%.1f*N/A", width);
			else if (depth > 0.0)
				len += sprintf (lbd + len, "*N/A*%.1f", depth);
			else
				len += sprintf (lbd + len, "*%s*%s", "N/A", "N/A");

			if (pmgc > 0.0)
				sprintf (lbd + len, "*%.1f", pmgc);
			else
				sprintf (lbd + len, "*%s", "N/A");

			len = 0;
			READMYSQL_DOUBLE(TRESTLE_MAX_LENGTH, length, -1.0);
			READMYSQL_DOUBLE(TRESTLE_MAX_WIDTH, width, -1.0);
			if (length > 0.0)
				len += sprintf (trestle_lb + len, "%.1f", length);
			else
				len += sprintf (trestle_lb + len, "%s", "N/A");			
			if (width > 0.0)
				len += sprintf (trestle_lb + len, "*%.1f", width);
			else
				len += sprintf (trestle_lb + len, "*%s", "N/A");

			READMYSQL_DOUBLE(DWT, bcap, 0.0);
			READMYSQL_STR(Remark, mainuse);
	
			READMYSQL_STR(style_des, structtp);
			READMYSQL_INT(TRESTLE_NUM, trestle_num, 0);
			READMYSQL_INT(DWT_YEAR, depassablty, 0);
			READMYSQL_STR(COMPLETE_DATE, finishdate);
			READMYSQL_STR(ACCEPT_DATE, acceptdate);
			READMYSQL_STR(ISSURE_DT, axdate);
			READMYSQL_STR(CERT_NO, axrefnum);
			READMYSQL_STR(CERT_ID, axid);
			READMYSQL_STR(isname, anunit);
			READMYSQL_INT(STATUS, curstatus, 0);
		}
		else
		{
			if (strlen(_owner))
				owner.insert(_owner);
			if (strlen(_operation))
				operations.insert(_operation);
		}
	}
	if (cnt > 0)
		out << ",";
	if (cnt >= 0)
		out << "{bid:\"" << bid << "\",bnm:\"" << JsonReplace(bnm) << "\",berthcode:\"" << JsonReplace(bcode) << "\",hid:\"" << hid << "\",dockid:\"" << dockid << "\",dockname:\"" << dockname <<"\",x:\"" << lon << "\",y:\"" << lat << "\",hnm:\"" << hnm << "\",lbdphigh:\"" << lbd << "\",bcap:\"" << bcap << "\",mainuse:\"" << mainuse << "\",curstatus:\"" << curstatus 
			<< "\",owner:\"" << ToStr(owner) << "\",operations:\"" << ToStr(operations) << "\",structtp:\"" << structtp << "\",trestle_num:\"" << trestle_num << "\",trestle_lb:\"" << trestle_lb << "\",finishdate:\"" << finishdate << "\",acceptdate:\"" << acceptdate 
			<< "\",depassablty:\"" << depassablty << "\",anunit:\"" << anunit << "\",axdate:\"" << axdate<< "\",ax_id:\"" << axid << "\",axrefnum:\"" << axrefnum << "\"}";
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{bid:"b001"}
int BerthSvc::getBerthInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getBerthInfo]bad format:", jsonString, 1);
	string bid = root.getv("bid", "");
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.Berth_id, t1.Name AS bnm, t1.Terminal_id,  t2.Name AS docknm, t3.NAME AS harbornm, t2.LOCATION_DES, t1.style_des, t1.Remark,t1.DWT, t1.Length, t1.WIDTH, t1.Depth, \
						t1.TRESTLE_MAX_LENGTH, t1.TRESTLE_MAX_WIDTH, t1.TRESTLE_NUM, t1.VERTICAL_REFERENCE, t1.COMPLETE_DATE, t1.ACCEPT_DATE, t5.ISSURE_DT, t5.CERT_NO,t5.CERT_ID, t5.owner AS isName\
						FROM t41_port_berth  t1 LEFT JOIN t41_port_terminal t2 ON t1.Terminal_id = t2.Terminal_id\
						LEFT JOIN t41_port_harbour t3 ON t2.AreaName = t3.PORTAREA_ID \
						LEFT JOIN t41_port_certification t5 ON t1.Berth_id = t5.BERTH_ID \
					    WHERE t1.Berth_id = '%s'", bid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	char berthid[100] = "";
	char bnm[100] = "";
	char dockid[100] = "";		//所属码头ID
   	char docknm[256] = "";		//所属码头名称
    char harbornm[100] = "";	//所属港区名称
    char addr[128] = "";		//地址
    char structtp[32] = "";		//结构形式
    char use[1024] = "";		//主要用途
    char ddwt[32] = "";			//设计靠泊吨位
    char rdwt[32] = "";		//实际靠泊吨位
    char lbd[100] = "";			//泊位长*宽*前沿水深(格式如：30*11.4&1.1某项没有的话用N/A表示)
    char trestle_lb[100] = "";	//栈桥长*宽 格式如:30*3
    int trestle_num = 0;			//栈桥数量      
	double planehigh = 0;			//平面高程
    char finishdate[32] = "";	//交工日期
	char acceptdate[32] = "";	//竣工验收时间
	char axdate[32] = "";		//岸线审批时间
	char axrefnum[64] = "";		//岸线审批文号
	char anunit[256] = "";		//岸线审批单位
	char certid[32]=""; //岸线认证ID

	if (psql->NextRow())
	{
		double len, width, depth;
		int t_len = 0;
		READMYSQL_STR(Berth_id, berthid);
		READMYSQL_STR(bnm, bnm);		
		READMYSQL_STR(Terminal_id, dockid);
		READMYSQL_STR(docknm, docknm);
		READMYSQL_STR(harbornm, harbornm);		
		READMYSQL_STR(LOCATION_DES, addr);
		READMYSQL_STR(style_des, structtp);
		READMYSQL_STR(Remark, use);
		READMYSQL_STR(DWT, ddwt);

		READMYSQL_DOUBLE(Length, len, -1.0);
		if (len > 0.0)
			t_len += sprintf (lbd + t_len, "%.1f", len);
		else
			t_len += sprintf (lbd + t_len, "%s", "N/A");

		READMYSQL_DOUBLE(WIDTH, width, -1.0);	
		if (width > 0.0)
			t_len += sprintf (lbd + t_len, "*%.1f", width);
		else
			t_len += sprintf (lbd + t_len, "*%s", "N/A");

		READMYSQL_DOUBLE(Depth, depth, -1.0);
		if (depth > 0.0)
			t_len += sprintf (lbd + t_len, "*%.1f", depth);
		else
			t_len += sprintf (lbd + t_len, "*%s", "N/A");

		t_len = 0;
		READMYSQL_DOUBLE(TRESTLE_MAX_LENGTH, len, -1.0);
		if (len > 0.0)
			t_len += sprintf (trestle_lb + t_len, "%.1f", len);
		else
			t_len += sprintf (trestle_lb + t_len, "%s", "N/A");

		READMYSQL_DOUBLE(TRESTLE_MAX_WIDTH, width, -1.0);
		if (width > 0.0)
			t_len += sprintf (trestle_lb + t_len, "*%.1f", width);
		else
			t_len += sprintf (trestle_lb + t_len, "*%s", "N/A");

		READMYSQL_INT(TRESTLE_NUM, trestle_num, 0);
		READMYSQL_STR(COMPLETE_DATE, finishdate);
		READMYSQL_DOUBLE(VERTICAL_REFERENCE, planehigh, 0.0);

		READMYSQL_STR(ACCEPT_DATE, acceptdate);
		READMYSQL_STR(ISSURE_DT, axdate);
		READMYSQL_STR(CERT_NO, axrefnum);
		READMYSQL_STR(isName, anunit);
		READMYSQL_STR(CERT_ID, certid);
		
	}
	out << "{bid:\"" << berthid << "\",bnm:\"" << bnm << "\",dockid:\"" << dockid << "\",docknm:\"" << docknm << "\",harbornm:\"" << harbornm << "\",addr:\"" << addr << "\",structtp:\"" << structtp 
		<< "\",use:\"" << use << "\",ddwt:\"" << ddwt << "\",rdwt:\"" << rdwt << "\",lbd:\"" << lbd << "\",trestle_lb:\"" << trestle_lb << "\",trestle_num:\"" << trestle_num 
		<< "\",planehigh:\"" << planehigh << "\",finishdate:\"" << finishdate << "\",acceptdate:\"" << acceptdate << "\",ax_id:\"" << certid <<"\",axdate:\"" << axdate << "\",axrefnum:\"" << axrefnum << "\",anunit:\"" << anunit << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{bid:"b001"}
int BerthSvc::getBerthCompInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getBerthCompInfo]bad format:", jsonString, 1);
    string bid = root.getv("bid", "");
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.Company_key, t3.Name FROM t41_port_service_contacts t1 JOIN t41_port_berth t2 ON t1.EntityID = t2.Berth_id\
						LEFT JOIN t41_company t3 ON t1.Company_Key = t3.Company_Key WHERE t2.Berth_id = '%s' AND t1.ServiceType = '203'", bid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	

	int cnt = 0;
	out << "{bid:\"" << bid << "\",operations:[";
	while (psql->NextRow())
	{	
		char ck[100] = "";
		char cnm[256] = "";
		READMYSQL_STR(Company_key, ck);
		READMYSQL_STR(Name, cnm);
		if (cnt++)
			out << ",";
		out << "{cmpid:\"" << ck << "\",cmpnm:\"" << cnm << "\"}";
	}
	out << "],owner:[";

	sprintf (sql, "SELECT t1.Company_key, t3.Name FROM t41_port_service_contacts t1 JOIN t41_port_berth t2 ON t1.EntityID = t2.Berth_id\
						LEFT JOIN t41_company t3 ON t1.Company_Key = t3.Company_Key WHERE t2.Berth_id = '%s' AND t1.ServiceType = '213'", bid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	cnt = 0;
	while (psql->NextRow())
	{	
		char ck[100] = "";
		char cnm[256] = "";
		READMYSQL_STR(Company_key, ck);
		READMYSQL_STR(Name, cnm);
		if (cnt++)
			out << ",";
		out << "{cmpid:\"" << ck << "\",cmpnm:\"" << cnm << "\"}";
	}

	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{bid:"B111227384",tm:135342320}
int BerthSvc::getBerthShipInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getBerthShipInfo]bad format:", jsonString, 1);
    string bid = root.getv("bid", "");
	int tm = root.getv("tm", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.berthtm, t1.eleavetm, t1.status,t1.mmsi,t2.SHIPID, t2.NAME,t3.ShipId,t3.shipname AS shipname3 \
		FROM t41_pmonitor_berth_stat t1 LEFT JOIN t41_pmonitor_ship t3 ON T1.mmsi=t3.mmsi \
		LEFT JOIN t41_ship t2 ON t1.mmsi = t2.mmsi \
		WHERE BERTH_ID = '%s'", bid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int lt=0, bt=0, status=0;
	char shipid[100] = "";
	char name[255] = "";
	char shipid3[100]="";
	char shipname3[255]="";
	int mmsi=0;

	if (psql->NextRow())
	{
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(NAME, name);
		READMYSQL_STR(shipid3, shipid3);
		READMYSQL_STR(shipname3, shipname3);
		READMYSQL_INT(berthtm, bt, -1);
		READMYSQL_INT(eleavetm, lt, -1);
		READMYSQL_INT(status, status, 0);
		READMYSQL_INT(mmsi, mmsi, 0);
	}

	string shipidStr=shipid;
	string shipnameStr=name;
	if (strcmp(shipname3,"")!=0)
	{
		shipidStr=shipid3;
        shipnameStr=shipname3;
	}
  
	if (status==0)
		out << "{bid:\"" << bid << "\",tm:" << tm << ",status:0,shipinfo: {mmsi:\"0\",shipid:\"\",shipnm:\"\",berthtm:0,eleavetm:0}}";
	else
		out << "{bid:\"" << bid << "\",tm:" << tm << ",status:1,shipinfo: {mmsi:\""<<mmsi<<"\",shipid:\"" << shipidStr << "\",shipnm:\"" << shipnameStr << "\",berthtm:" << bt << ",eleavetm:" << lt << "}}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{bid:"b001"}
int BerthSvc::getBerthMYWInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getBerthMYWInfo]bad format:", jsonString, 1);
    string bid = root.getv("bid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT t1.EQUIPMENT_ID,t1.NO_DES, t1.TERMINAL_ID,t1.NAME AS eqnm, t1.TYPE_NAME, t1.MODEL, t1.CHECK_DT, t1.BUILDER,t1.CAPACITY \
				   FROM t41_port_equipment t1  WHERE t1.BERTH_ID = '%s' and IS_VALID=1", bid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "{bid:\"" << bid << "\",machine:[";


    char terminal_id[64]="";

	int index=0;
	while (psql->NextRow())
	{

		if (index==0)
		{
            READMYSQL_STR(TERMINAL_ID, terminal_id);
		}
		else
		{
			out << ",";
		}

		EquipmentInfo_B eqInfo;
		READMYSQL_STR(EQUIPMENT_ID, eqInfo.id);
		READMYSQL_STR(NO_DES, eqInfo.seq);
		READMYSQL_STR(eqnm, eqInfo.name);
		READMYSQL_STR(TYPE_NAME, eqInfo.type);
		READMYSQL_STR(MODEL, eqInfo.spec);
		READMYSQL_STR(CHECK_DT, eqInfo.validdt);
		READMYSQL_STR(BUILDER, eqInfo.producer);
		READMYSQL_STR(CAPACITY, eqInfo.capacity);
 
		out << "{ id:\""<<eqInfo.id<<"\",seq:\"" << eqInfo.seq << "\",name:\"" << eqInfo.name << "\",type:\"" << eqInfo.type << "\",spec:\"" << eqInfo.spec << "\",validdt:\"" << eqInfo.validdt << "\",producer:\"" << eqInfo.producer << "\",lcap:\"" << eqInfo.capacity <<"\"}";
		index++;
	}

	out << "]";


	if (string(terminal_id).empty())
	{
		sprintf (sql, "SELECT t1.TERMINAL_ID FROM t41_port_berth t1  WHERE t1.BERTH_ID = '%s'", bid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		if (psql->NextRow())
		{
			 READMYSQL_STR(TERMINAL_ID, terminal_id);
		}
	}



	sprintf (sql, "SELECT STORAGE_ID, STORAGE_TYPE, AREA, CAPACITY, CERTIFICATION,STORAGE_FLAG FROM t41_port_storage \
				   WHERE TERMINAL_ID = '%s' AND STORAGE_AFFILIATION=2 AND IS_VALID=1", terminal_id);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int flag=0;

	mWarehouseInfo_B mWhInfo_B;
	mStorageInfo_B mStInfo_B;

	while (psql->NextRow())
	{
		WarehouseInfo_B whInfo;
		StorageInfo_B stInfo;
		

		READMYSQL_INT(STORAGE_FLAG, flag,0);
		if (flag == 2) //堆场
		{
			READMYSQL_STR(STORAGE_ID, whInfo.id);
			READMYSQL_STR(AREA, whInfo.area);
			READMYSQL_STR(CAPACITY, whInfo.capacity);
			READMYSQL_STR(CERTIFICATION, whInfo.businesscard);
			READMYSQL_STR(STORAGE_TYPE, whInfo.type);
			mWhInfo_B[whInfo.id]=whInfo;

		}
		else if (flag==1) //仓库
		{
			READMYSQL_STR(STORAGE_ID, stInfo.id);
			READMYSQL_STR(AREA, stInfo.area);
			READMYSQL_STR(CAPACITY, stInfo.capacity);
			READMYSQL_STR(CERTIFICATION, stInfo.businesscard);
			READMYSQL_STR(STORAGE_TYPE, stInfo.type);
			mStInfo_B[stInfo.id]=stInfo;
		}		
	}

	out << ",yard:[";

	mWarehouseInfo_B::iterator wIter;
	mStorageInfo_B::iterator sIter;
	for (wIter = mWhInfo_B.begin(); wIter != mWhInfo_B.end(); wIter++)
	{
		if (wIter != mWhInfo_B.begin())
			out << ",";
		out<< "{ id:\""<<(*wIter).second.id<< "\",ytype:\"" << (*wIter).second.type << "\",area:\"" << (*wIter).second.area << "\",capacity:\"" << (*wIter).second.capacity << "\",businesscard:\"" << (*wIter).second.businesscard << "\"}";
	}
	out << "],warehourse:[";
	for (sIter = mStInfo_B.begin(); sIter != mStInfo_B.end(); sIter++)
	{
		if (sIter != mStInfo_B.begin())
			out << ",";
		out<< "{ id:\""<<(*sIter).second.id<< "\",wtype:\"" << (*sIter).second.type << "\",area:\"" << (*sIter).second.area << "\",capacity:\"" << (*sIter).second.capacity << "\",businesscard:\"" << (*sIter).second.businesscard << "\"}";
	}
	out << "]}";


	RELEASE_MYSQL_RETURN(psql, 0);
}


int BerthSvc::getEquipmentList(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[BerthSvc::getEquipmentList]bad format:", jsonString, 1);
	string portid = root.getv("portid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf (sql, "SELECT t1.EQUIPMENT_ID,t1.NO_DES, t1.Berth_id,t2.name as berthname,t1.NAME AS eqnm, t1.TYPE_NAME, t1.MODEL, t1.CHECK_DT, t1.BUILDER,t1.CAPACITY \
				  FROM t41_port_equipment t1 LEFT JOIN t41_port_berth t2 ON t1.Berth_id = t2.Berth_id\
				  WHERE t1.PORTID = '%s' and t1.IS_VALID=1", portid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";


	int index=0;
	while (psql->NextRow())
	{
		if (index>0)
		{
			out << ",";
		}

		EquipmentInfo_B eqInfo;
		READMYSQL_STR(EQUIPMENT_ID, eqInfo.id);
		READMYSQL_STR(NO_DES, eqInfo.seq);
		READMYSQL_STR(eqnm, eqInfo.name);
		READMYSQL_STR(TYPE_NAME, eqInfo.type);
		READMYSQL_STR(MODEL, eqInfo.spec);
		READMYSQL_STR(CHECK_DT, eqInfo.validdt);
		READMYSQL_STR(BUILDER, eqInfo.producer);
		READMYSQL_STR(CAPACITY, eqInfo.capacity);
		READMYSQL_STR(Berth_id, eqInfo.berthid);
		READMYSQL_STR(berthname, eqInfo.berthname);

		out << "{ id:\""<<eqInfo.id<<"\",seq:\"" << eqInfo.seq << "\",name:\"" << eqInfo.name <<"\",berthid:\"" << eqInfo.berthid <<"\",berth:\"" << eqInfo.berthname <<"\",type:\"" << eqInfo.type << "\",spec:\"" << eqInfo.spec << "\",validdt:\"" << eqInfo.validdt << "\",producer:\"" << eqInfo.producer << "\",lcap:\"" << eqInfo.capacity <<"\"}";
		index++;
	}

	out << "]";



	RELEASE_MYSQL_RETURN(psql, 0);
}
int BerthSvc::getStorageList(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[BerthSvc::getStorageList]bad format:", jsonString, 1);
	string portid = root.getv("portid", "");
	string type = root.getv("stroagetype", "");
	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf (sql, "SELECT T1.STORAGE_ID, T1.STORAGE_TYPE, T1.Terminal_id,T2.name as TerminalName,T1.AREA, T1.CAPACITY, T1.CERTIFICATION,T1.STORAGE_FLAG FROM t41_port_storage T1 \
				   LEFT JOIN t41_port_terminal t2 ON t1.Terminal_id = t2.Terminal_id\
				   WHERE T1.PORTID = '%s' AND T1.STORAGE_FLAG='%s' AND T1.IS_VALID=1", portid.c_str(),type.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	out << "[";

	int index=0;

	while (psql->NextRow())
	{

		if (index>0)
		{
			out << ",";
		}

		StorageInfo_B stInfo;

		READMYSQL_STR(STORAGE_ID, stInfo.id);
		READMYSQL_STR(AREA, stInfo.area);
		READMYSQL_STR(CAPACITY, stInfo.capacity);
		READMYSQL_STR(CERTIFICATION, stInfo.businesscard);
		READMYSQL_STR(STORAGE_TYPE, stInfo.type);
		READMYSQL_STR(Terminal_id, stInfo.terminalid);
		READMYSQL_STR(TerminalName, stInfo.terminalname);
	
	    out<< "{ id:\""<<stInfo.id<< "\",ytype:\"" << stInfo.type << "\",terminalid:\"" << stInfo.terminalid<<"\",terminal:\""<< stInfo.terminalname<< "\",area:\"" << stInfo.area << "\",capacity:\"" << stInfo.capacity << "\",businesscard:\"" << stInfo.businesscard << "\"}";
	
		index++;
	}

	out << "]";
	

	RELEASE_MYSQL_RETURN(psql, 0);
}

//int getBerthCameraInfo(const char* pUid, const char* jsonString, std::stringstream& out);

//{cmpid:"c001"}
int BerthSvc::getBerthCompManagerInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getBerthMYWInfo]bad format:", jsonString, 1);
    string cmpid = root.getv("cmpid", "");
	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	sprintf (sql, "SELECT  `NAME` AS cmpname, BUSI_CARD AS tcard, BUSI_ISS_UNIT AS issoffice, BUSI_ISS_DATE AS issdate, BUSI_VALID_DT AS validdt,\
					LEGAL_PERSON AS corp, LEGAL_ID_CARD AS idcard, ADDRESS AS waddr,REG_CAPITAL AS regcapital, INDUSTRY AS industry, SERVICE_TYPE,\
					OPER_AREA AS mareas, OPER_CARD AS mcard, OPER_ISS_ORG AS missunit, OPER_ISS_DATE AS missdate, OPER_ISS_VALID_DT AS mvaliddt, \
					CONTACT_NAME, CONTACT_TEL AS tel, CONTACT_MOBILE AS mobile, CONTACT_FAX AS fax, CONTACT_EMAIL AS email, CONTACT_TYPE  \
					FROM t41_berth_company WHERE COMPANY_ID = '%s' AND IS_VALID = '1'", cmpid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char cmpname[100] = "";
	char tcard[64] = ""; //工商证号
	char issoffice[100] = ""; //发证机关
	char issdate[32] = ""; //发证日期
	char validdt[32] = ""; //有效期
	char waddr[255] = ""; //办公地址
	char corp[64] = ""; //法人
	char idcard[64] = ""; //身份证号
	char industry[100] = ""; //行业
	char regcapital[64] = ""; //注册资金

	char name[32] = ""; //联系人姓名
	char type[128] = ""; //类型
	char tel[64] = ""; //电话
	char fax[64] = ""; //传真
	char mobile[64] = ""; //手机
	char email[256] = ""; //邮箱

	char mcard[64] = ""; //经营证号
	char missunit[64] = ""; //发证单位
	char missdate[32] = ""; //发证日期
	char mvaliddt[32] = ""; //有效期
	char mareas[64] = ""; //经营地域
	char mtype[64] = ""; //经营类型
	
	int cnt = 0;
	while (psql->NextRow())
	{
		READMYSQL_STR(cmpname, cmpname);
		READMYSQL_STR(issoffice, issoffice);
		READMYSQL_STR(issdate, issdate);
		READMYSQL_STR(tcard, tcard);
		READMYSQL_STR(validdt, validdt);
		READMYSQL_STR(corp, corp);
		READMYSQL_STR(idcard, idcard);
		READMYSQL_STR(regcapital, regcapital);
		READMYSQL_STR(CONTACT_NAME, name);
		READMYSQL_STR(tel, tel);
		READMYSQL_STR(mobile, mobile);
		READMYSQL_STR(fax, fax);
		READMYSQL_STR(mcard, mcard);
		READMYSQL_STR(missunit, missunit);
		READMYSQL_STR(missdate, missdate);
		READMYSQL_STR(mvaliddt, mvaliddt);		
		READMYSQL_STR(SERVICE_TYPE, mtype);
		READMYSQL_STR(waddr, waddr);	
		READMYSQL_STR(CONTACT_TYPE,type);
		READMYSQL_STR(mareas, mareas);
	}
	out << "{cmpid:\"" << cmpid << "\",cmpname:\"" << cmpname << "\",tcard:\"" << tcard << "\",issoffice:\"" << issoffice << "\",issdate:\"" << issdate 
		<< "\",validdt:\"" << validdt << "\",waddr:\"" << waddr << "\",corp:\"" << corp << "\",idcard:\"" << idcard << "\", industry:\"" << industry 
		<< "\", regcapital:\"" << regcapital << "\",name:\"" << name << "\",type:\"" << type << "\",tel:\"" << tel << "\",fax:\"" << fax << "\",mobile:\"" << mobile 
		<< "\", email:\"" << email << "\",mcard:\"" << mcard << "\",missunit:\"" << missunit << "\",missdate:\"" << missdate << "\",mvaliddt:\"" << mvaliddt 
		<< "\",mareas:\"" << mareas << "\",mtype:\"" << mtype << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{whid:""}
//{whname:"",whaddr:"",whlarea:"",whsarea:"",whllen:"",whworka:"",whremark:""}
int BerthSvc::getTerminalInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getTerminalInfo]bad format:", jsonString, 1);
    string whid = root.getv("whid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT NAME, LOCATION_DES, AREA AS whsarea, LAND_AREA AS whlarea, COASTLINE, Remark, WORK_AREA as whworka FROM t41_port_terminal WHERE Terminal_id = '%s'", whid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char whname[256] = "";			//码头名称
	char whaddr[128] = "";			//地址
	double whlarea = 0.0;			//陆域面积（平米）
	double whsarea = 0.0;			//水域面积（平米）
	double whllen = 0.0;			//岸线长度（米）
	double whworka = 0.0;			//办公生活面积（平米）
	char whremark[1024*10] = "";	//备注

	if (psql->NextRow())
	{
		READMYSQL_STR(NAME, whname);
		READMYSQL_STR(LOCATION_DES, whaddr);
		READMYSQL_STR(Remark, whremark);
		READMYSQL_DOUBLE(whsarea, whsarea, 0.0);
		READMYSQL_DOUBLE(whlarea, whlarea, 0.0);
		READMYSQL_DOUBLE(COASTLINE, whllen, 0.0);
		READMYSQL_DOUBLE(whworka, whworka, 0.0);
	}

	out << "{whid:\"" << whid << "\",whname:\"" << whname << "\",whaddr:\"" << whaddr << "\",whlarea:\"" << whlarea << "\",whsarea:\"" << whsarea << "\",whllen:\"" << whllen << "\",whworka:\"" << whworka << "\",whremark:\"" << whremark << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BerthSvc::getTerminalList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getPortBerthList]bad format:", jsonString, 1);
	string portid = root.getv("portid", "");
	Json* harborid = root.get("harborids");
	char harborids[512] = {'\0'};
	int len = 0;
	for (int i = 0; i < harborid->size(); i++)
	{
		if (len)
			len += sprintf (harborids + len, ",");
		len += sprintf (harborids + len, "'%s'", harborid->getv(i, ""));
	}

	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	len = 0;
	len = sprintf (sql, "SELECT terminal_id,NAME, LOCATION_DES, AreaName As harborid,AREA AS whsarea, LAND_AREA AS whlarea, COASTLINE, Remark, WORK_AREA as whworka FROM t41_port_terminal t1\
						 WHERE t1.Portid = '%s' AND is_valid='1'", portid.c_str());
	if (harborid->size())
		len += sprintf (sql + len, " AND t1.AreaName IN (%s)", harborids);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    char whid[128]="";
	char whname[256] = "";			//码头名称
	char whaddr[128] = "";			//地址
	char areaid[128]="";          //港区ID
	double whlarea = 0.0;			//陆域面积（平米）
	double whsarea = 0.0;			//水域面积（平米）
	double whllen = 0.0;			//岸线长度（米）
	double whworka = 0.0;			//办公生活面积（平米）
	char whremark[1024*10] = "";	//备注

	out << "{wharfs:[";
	
	int index=0;
	while (psql->NextRow())
	{
		if (index>0)
		{
            out <<",";
		}
		
		READMYSQL_STR(terminal_id, whid);
		READMYSQL_STR(NAME, whname);
		READMYSQL_STR(LOCATION_DES, whaddr);
		READMYSQL_STR(harborid, areaid);
		READMYSQL_STR(Remark, whremark);
		READMYSQL_DOUBLE(whsarea, whsarea, 0.0);
		READMYSQL_DOUBLE(whlarea, whlarea, 0.0);
		READMYSQL_DOUBLE(COASTLINE, whllen, 0.0);
		READMYSQL_DOUBLE(whworka, whworka, 0.0);
		index++;
		out << "{whid:\"" << whid << "\",whname:\"" << whname <<"\",harborid:\"" << areaid<< "\",whaddr:\"" << whaddr << "\",whlarea:\"" << whlarea << "\",whsarea:\"" << whsarea << "\",whllen:\"" << whllen << "\",whworka:\"" << whworka << "\",whremark:\"" << whremark << "\"}";
	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}


int BerthSvc::newTerminalItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::updInOutInfo]bad format:", jsonString, 1);
	MySql* psql = CREATE_MYSQL;

	char gmt0now[20];
	GmtNow(gmt0now);
	uint64 timeusec = GmtNowUsec();

	string flag=root.getv("whflag", "");
	string terminalid=root.getv("whid", "");
	string portid = root.getv("portid", "");
	string harborid = root.getv("harborid", "");
	string whaddr = root.getv("whaddr", ""); //地址
    string docknm = root.getv("whname", ""); //码头名称
	double whlarea = atof(root.getv("whlarea", "")); //路域面积
	double whsarea = atof(root.getv("whsarea", "")); //水域面积
	double whllen = atof(root.getv("whllen", ""));  //岸线长度
	double whworka = atof(root.getv("whworka", "")); //办公生活面积
	string remark = root.getv("whremark", "");//备注
	


	if (flag=="0")//新增
	{
		char dockid[32] = "";
		sprintf (dockid, "T%lld", timeusec);


		char sql[1024 * 2] = "";

		sprintf (sql, "INSERT INTO t41_port_terminal (terminal_id, portid, AreaName,Name,LOCATION_DES, LAND_AREA, Area, COASTLINE, WORK_AREA, Remark,last_upd_dt,last_upd_user,Is_Valid) \
					  VALUES ('%s','%s', '%s', '%s','%s',%.1f, %.1f, %.1f, %.1f, '%s','%s','%s','1')", 
					  dockid, portid.c_str(), harborid.c_str(), docknm.c_str(), whaddr.c_str(), whlarea, whsarea, whllen,whworka ,remark.c_str(),gmt0now,pUid
					  );

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else if (flag=="1"&&!terminalid.empty())//修改
	{
		char sql[1024 * 2] = "";
            
		sprintf (sql, "update t41_port_terminal set name='%s',AreaName='%s',location_des='%s', land_area=%.1f, area=%.1f, COASTLINE=%.1f, WORK_AREA=%.1f, Remark='%s',last_upd_dt='%s',last_upd_user='%s'  \
					  where terminal_id='%s'",
					  docknm.c_str(),harborid.c_str(),whaddr.c_str(), whlarea, whsarea, whllen, whworka ,remark.c_str(),gmt0now,pUid,terminalid.c_str()
					  );

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	
	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}

int BerthSvc::delTerminalItem(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[BerthSvc::getTerminalInfo]bad format:", jsonString, 1);
	string whid = root.getv("whid", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_port_terminal SET is_valid='0',last_upd_user='%s',last_upd_dt='%s' WHERE terminal_id = '%s'",pUid,gmt0now,whid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{cflag:"",cmpid:"",cmpname:"",tcard:"",issoffice:"",issdate:" ",validdt:"0",waddr:"",corp:"",idcard:"", industry:"", 
//regcapital:"",name:"",type:"",tel:"",fax:"",mobile:"", email:"",mcard:"",missunit:"", missdate:"", mvaliddt :"", mareas:"",mtype:"" }
int BerthSvc::udpOperatorsDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::udpOperatorsDetail]bad format:", jsonString, 1);
	MySql* psql = CREATE_MYSQL;
	
	int cflag = atoi(root.getv("cflag", ""));
	string cmpid = root.getv("cmpid", "");
	string cmpname = root.getv("cmpname", "");
	string tcard = root.getv("tcard", "");
	string issoffice = root.getv("issoffice", "");
	string issdate = root.getv("issdate", "");
	string validdt = root.getv("validdt", "");
	string waddr = root.getv("waddr", "");
	string corp = root.getv("corp", "");
	string idcard = root.getv("idcard", "");
	string industry = root.getv("industry", "");
	string regcapital = root.getv("regcapital", "");
	string name = root.getv("name", "");

	string type = root.getv("type", "");
	string tel = root.getv("tel", "");
	string fax = root.getv("fax", "");
	string mobile = root.getv("mobile", "");
	string email = root.getv("email", "");
	string mcard = root.getv("mcard", "");
	string missunit = root.getv("missunit", "");
	string missdate = root.getv("missdate", "");
	string mvaliddt = root.getv("mvaliddt", "");
	string mareas = root.getv("mareas", "");
	string mtype = root.getv("mtype", "");
	string portid = root.getv("portid", "");

	time_t t = time(0);
	struct tm *ptr = gmtime(&t);
	char sql[1024] = "";
	char gmt0now[20];
	GmtNow(gmt0now);


	if (cflag==0)
	{

		uint64 timeusec = GmtNowUsec();
		char _cmpid[64] = "";
		sprintf (_cmpid, "C%lld", timeusec);

		/*uint64 timeusec = GmtNowUsec();
		sprintf (sql, "SELECT COUNT(1) AS num FROM t41_berth_company WHERE COMPANY_ID LIKE 'C%d%d%d%%'", ptr->tm_year+1900, ptr->tm_mon+1, ptr->tm_mday);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		int num = 0;

		if (psql->NextRow())
		{
			READMYSQL_INT(num, num, 0);
		}
		num++;	*/	
		
		//sprintf (_cmpid, "C%d%d%d%d", ptr->tm_year, ptr->tm_mon+1, ptr->tm_mday, num);
		sprintf (sql, "INSERT INTO t41_berth_company (COMPANY_ID, `NAME`, BUSI_CARD, BUSI_ISS_UNIT, BUSI_ISS_DATE, BUSI_VALID_DT, LEGAL_PERSON, LEGAL_ID_CARD,\
						ADDRESS, REG_CAPITAL, INDUSTRY, SERVICE_TYPE, OPER_AREA, OPER_CARD, OPER_ISS_ORG, OPER_ISS_DATE, OPER_ISS_VALID_DT, CONTACT_NAME, \
						CONTACT_TEL, CONTACT_MOBILE, CONTACT_FAX, CONTACT_EMAIL, CONTACT_TYPE, UPDATE_DT, IS_VALID,PORT_ID) VALUES \
						('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1','%s')", 
						_cmpid, cmpname.c_str(), tcard.c_str(), issoffice.c_str(), issdate.c_str(), validdt.c_str(), corp.c_str(), idcard.c_str(), waddr.c_str(),
						regcapital.c_str(), industry.c_str(), mtype.c_str(), mareas.c_str(), mcard.c_str(),missunit.c_str(), missdate.c_str(),
						mvaliddt.c_str(), name.c_str(), tel.c_str(), mobile.c_str(), fax.c_str(), email.c_str(), type.c_str(), gmt0now,portid.c_str());
	}
	else if (cflag==1)
	{
		sprintf (sql, "UPDATE t41_berth_company SET `NAME` = '%s', BUSI_CARD = '%s', BUSI_ISS_UNIT = '%s', BUSI_ISS_DATE = '%s', BUSI_VALID_DT = '%s',\
						LEGAL_PERSON = '%s', LEGAL_ID_CARD = '%s', ADDRESS = '%s', REG_CAPITAL = '%s', INDUSTRY = '%s', SERVICE_TYPE = '%s', \
						OPER_AREA = '%s', OPER_CARD = '%s', OPER_ISS_ORG = '%s', OPER_ISS_DATE = '%s', OPER_ISS_VALID_DT = '%s', CONTACT_NAME = '%s', \
						CONTACT_TEL = '%s', CONTACT_MOBILE = '%s', CONTACT_FAX = '%s', CONTACT_EMAIL = '%s', CONTACT_TYPE = '%s', UPDATE_DT = '%s', \
						IS_VALID = '1' WHERE COMPANY_ID = '%s'", cmpname.c_str(), tcard.c_str(), issoffice.c_str(), issdate.c_str(), validdt.c_str(), 
						corp.c_str(), idcard.c_str(), waddr.c_str(), regcapital.c_str(), industry.c_str(), mtype.c_str(), mareas.c_str(), mcard.c_str(), 
						missunit.c_str(), missdate.c_str(), mvaliddt.c_str(), name.c_str(), tel.c_str(), mobile.c_str(), fax.c_str(), 
						email.c_str(), type.c_str(), gmt0now, cmpid.c_str());
	}
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BerthSvc::newBerthItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::updInOutInfo]bad format:", jsonString, 1);
	MySql* psql = CREATE_MYSQL;

	char gmt0now[20];
	GmtNow(gmt0now);
	uint64 timeusec = GmtNowUsec();

	string pflag=root.getv("pflag", "");
	string bid=root.getv("bid", "");
	string terminalid=root.getv("dockid", "");
	string harborid = root.getv("harborid", "");
	string portid = root.getv("portid", "");
	string berthcode = root.getv("berthcode", "");
	string berthname = root.getv("berthname", "");
	string xpos = root.getv("xpos", "");
	string ypos = root.getv("ypos", "");
	string coordinate=xpos+","+ypos;

	string structtp = root.getv("structuretp", "");
	string use = root.getv("use", "");
	string l = root.getv("length", "");
	string b = root.getv("broad", "");
	string d = root.getv("deep", "");

	string ownerid=root.getv("ownerid","");
	string operationid=root.getv("operatorid","");

	string owner=root.getv("owner","");
	string operations=root.getv("operations","");


	string planehigh = root.getv("planehigh", "");
	string designdwt = root.getv("designdwt", "");
	string depassablty = root.getv("depassablty", "");


	string t_num = root.getv("trestle_num", "");
	string t_length = root.getv("trestle_length", "");
	string t_broad = root.getv("trestle_broad", "");

	string finishdate = root.getv("finishdate", "");
	string acceptdate = root.getv("acceptdate", "");
    

	string ax_danwei = root.getv("axapprovedanwei", "");
	string ax_date = root.getv("axapprovedate", "");
	string ax_refer = root.getv("axapproverefer", "");
	string bcode = root.getv("berthcode", "");
	string ax_id= root.getv("ax_id","");
	

   if (pflag=="0")//新增
   {
	   char berthid[32] = "";
	   sprintf (berthid, "B%lld", timeusec);

	   char sql[1024 * 10] = "";

	   sprintf (sql, "INSERT INTO t41_port_berth (Berth_id,Terminal_id,Harbour_id,portid,Name,coordinate,style_des, Remark,Length,Width, Depth,\
					 vertical_reference,dwt,dwt_year,TRESTLE_NUM,TRESTLE_MAX_LENGTH,TRESTLE_MAX_WIDTH,complete_date,accept_date,last_upd_dt,last_upd_user,Is_Valid,ACT_CODE) \
					 VALUES ('%s','%s', '%s', '%s','%s', '%s', '%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','1','%s')", 
					 berthid, terminalid.c_str(), harborid.c_str(), portid.c_str(), berthname.c_str(), coordinate.c_str(), structtp.c_str(),use.c_str(),l.c_str(),b.c_str(),d.c_str(),
					 planehigh.c_str(),designdwt.c_str() ,depassablty.c_str(),t_num.c_str(),t_length.c_str(),t_broad.c_str(),finishdate.c_str(),acceptdate.c_str(),gmt0now,pUid,bcode.c_str()
				  );

	   CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);



	   char certid[32] = "";
	   sprintf (certid, "PC%lld", timeusec);
	   char sql1[1024] = "";

	   sprintf (sql1, "INSERT INTO t41_port_certification (cert_id,owner,issure_dt,cert_type,cert_no,berth_id,terminal_id,update_dt)\
					  VALUES ('%s','%s', '%s', '1','%s', '%s', '%s','%s')", 
					  certid, ax_danwei.c_str(), ax_date.c_str(), ax_refer.c_str(), berthid,terminalid.c_str(),gmt0now
					  );

	   CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);


       bid=berthid;
   }
   else if (pflag=="1"&&!bid.empty())
   {
        

	   char sql[1024 * 4] = "";

	   sprintf (sql, "update t41_port_berth set Terminal_id='%s',harbour_id='%s',Name='%s',\
		               coordinate='%s',style_des='%s',Remark='%s',Length='%s',Width='%s',Depth='%s',\
		               vertical_reference='%s',dwt='%s',dwt_year='%s',TRESTLE_NUM='%s',TRESTLE_MAX_LENGTH='%s',\
		               TRESTLE_MAX_WIDTH='%s',complete_date='%s',accept_date='%s',last_upd_dt='%s',last_upd_user='%s',ACT_CODE='%s' \
	                   where Berth_id='%s'", 
					   terminalid.c_str(), harborid.c_str(),berthname.c_str(), coordinate.c_str(), structtp.c_str(),use.c_str(),l.c_str(),b.c_str(),d.c_str(),
					   planehigh.c_str(),designdwt.c_str() ,depassablty.c_str(),t_num.c_str(),t_length.c_str(),
					   t_broad.c_str(),finishdate.c_str(),acceptdate.c_str(),gmt0now,pUid,bcode.c_str(),bid.c_str()
				  );

	   CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	   if (!ax_id.empty())
	   {

		   char sql1[1024] = "";

		   sprintf (sql1, "update t41_port_certification set owner='%s',issure_dt='%s',cert_type='%s',\
			               berth_id='%s',terminal_id='%s',update_dt='%s' where cert_id='%s'", 
						   ax_danwei.c_str(), ax_date.c_str(), ax_refer.c_str(), bid.c_str(),terminalid.c_str(),gmt0now,ax_id.c_str()
						  );

		   CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);
            
	   }
	   else
	   {
		   char certid[32] = "";
		   sprintf (certid, "PC%lld", timeusec);
		   char sql1[1024] = "";

		   sprintf (sql1, "INSERT INTO t41_port_certification (cert_id,owner,issure_dt,cert_type,cert_no,berth_id,terminal_id,update_dt)\
						  VALUES ('%s','%s', '%s', '1','%s', '%s', '%s','%s')", 
						  certid, ax_danwei.c_str(), ax_date.c_str(), ax_refer.c_str(), bid.c_str(),terminalid.c_str(),gmt0now
						  );

		   CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);
	   }

   }
   

   if (!ownerid.empty())
   {

	   char sql2[256] = "";
	   sprintf (sql2, "delete from t41_port_service_contacts where EntityID='%s' and serviceType=213",bid.c_str());

	   CHECK_MYSQL_STATUS(psql->Execute(sql2)>=0, 3);


	   char sql3[1024] = "";

	   sprintf (sql3, "INSERT INTO t41_port_service_contacts (EntityID,PortID,ServiceType,Company_Key)\
					     VALUES ('%s','%s', '213', '%s')", 
					    bid.c_str(), portid.c_str(), ownerid.c_str()
					  );
        CHECK_MYSQL_STATUS(psql->Execute(sql3)>=0, 3);

   }
	
   if (!operationid.empty())
   {
	   char sql2[256] = "";
	   sprintf (sql2, "delete from t41_port_service_contacts where EntityID='%s' and serviceType=203",bid.c_str());

	   CHECK_MYSQL_STATUS(psql->Execute(sql2)>=0, 3);


	   char sql3[1024] = "";

	   sprintf (sql3, "INSERT INTO t41_port_service_contacts (EntityID,PortID,ServiceType,Company_Key)\
					  VALUES ('%s','%s', '203', '%s')", 
					  bid.c_str(), portid.c_str(), operationid.c_str()
					  );
	   CHECK_MYSQL_STATUS(psql->Execute(sql3)>=0, 3);
   }

    //if (!owner.empty()||!operations.empty())
    //{
    //    
    //}
    



	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
int BerthSvc::delBerthItem(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[BerthSvc::getTerminalInfo]bad format:", jsonString, 1);
	string berthid = root.getv("bid", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_port_berth SET is_valid='0',last_upd_user='%s',last_upd_dt='%s' WHERE berth_id = '%s'",pUid,gmt0now,berthid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BerthSvc::newBerthEquipment(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::updInOutInfo]bad format:", jsonString, 1);
	MySql* psql = CREATE_MYSQL;

	char gmt0now[20];
	GmtNow(gmt0now);
	uint64 timeusec = GmtNowUsec();

	
	string flag=root.getv("mflag", "");//标志
	string equipmentid=root.getv("equipid", "");//ID
	string berthid = root.getv("berthid", "");//泊位ID
	string terminalid = root.getv("terminalid", "");//码头ID
	string portid = root.getv("portid", "");//码头ID
	string seq = root.getv("seq", ""); //编号
	string name  =  root.getv("name", ""); //设备名称
	string type = root.getv("type", ""); //机械类型	
	string spec = root.getv("spec", ""); //规格
	string capacity = root.getv("capacity", "");  //负荷能力	
	string validdt = root.getv("validdt", ""); //有效期
	string producer = root.getv("producer", "");//生产商


	if (flag=="0") //添加
	{
		char equipid[32] = "";
		sprintf (equipid, "E%lld", timeusec);

		char sql[2048] = "";
		sprintf (sql, "INSERT INTO t41_port_equipment (equipment_id,no_des,berth_id,terminal_id,portid,equip_affiliation,name,type_name,model,capacity,check_dt,builder,update_dt,is_valid) \
					  VALUES ('%s','%s','%s', '%s', '%s','1','%s', '%s','%s', '%s', '%s','%s','%s','1')", 
					  equipid, seq.c_str(),berthid.c_str(), terminalid.c_str(),portid.c_str(), name.c_str(), type.c_str(),spec.c_str(), capacity.c_str(),validdt.c_str(),producer.c_str(),gmt0now
					  );

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else if (flag=="1"&&!equipmentid.empty())
	{

		char sql[2048] = "";
		sprintf (sql, "update t41_port_equipment set no_des='%s',berth_id='%s',terminal_id='%s',name='%s',type_name='%s',\
			            model='%s',capacity='%s',check_dt='%s',builder='%s',update_dt='%s' \
		                where equipment_id='%s'", 
					   seq.c_str(),berthid.c_str(), terminalid.c_str(), name.c_str(), type.c_str(),
					   spec.c_str(), capacity.c_str(),validdt.c_str(),producer.c_str(),gmt0now,equipmentid.c_str()
					  );

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);


}
int BerthSvc::delBerthEquipment(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[BerthSvc::getTerminalInfo]bad format:", jsonString, 1);
	string equipid = root.getv("equipid", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_port_equipment SET is_valid='0',update_dt='%s' WHERE equipment_id = '%s'",gmt0now,equipid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}

int BerthSvc::newTerminalStorage(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::updInOutInfo]bad format:", jsonString, 1);
	MySql* psql = CREATE_MYSQL;

	char gmt0now[20];
	GmtNow(gmt0now);
	uint64 timeusec = GmtNowUsec();

	string flag=root.getv("sflag", "");//仓储类型
	string storage_id=root.getv("storageid", "");//仓储类型

	int storage = root.getv("storage", 0);//仓储类型
	string terminalid = root.getv("terminalid", "");//码头ID
	string portid = root.getv("portid", "");//港口ID
	string type = root.getv("type", ""); //堆场类型/仓库类型
	string area  =  root.getv("area", ""); //堆场面积/仓库面积
	string capacity = root.getv("capacity", ""); //堆存能力/仓库容积	
	string businesscard = root.getv("businesscard", ""); //运营证号


	if (flag=="0")
	{
		char storageid[32] = "";
		sprintf (storageid, "PS%lld", timeusec);

		char sql[1024] = "";
		sprintf (sql, "INSERT INTO t41_port_storage (storage_id,terminal_id,portid,storage_affiliation,area,capacity,certification,storage_type,storage_flag,update_dt,is_valid) \
					  VALUES ('%s','%s','%s','2','%s', '%s','%s', '%s', '%d','%s','1')", 
					  storageid, terminalid.c_str(),portid.c_str(), area.c_str(), capacity.c_str(), businesscard.c_str(),type.c_str(),storage,gmt0now
					  );

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else if (flag=="1"&&!storage_id.empty())
	{

		char sql[1024] = "";
		sprintf (sql, "update t41_port_storage set terminal_id='%s',area='%s',capacity='%s',certification='%s',storage_type='%s',update_dt='%s' \
		               where storage_id='%s'",
					   terminalid.c_str(),area.c_str(), capacity.c_str(), businesscard.c_str(),type.c_str(),gmt0now,storage_id.c_str()
					  );

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}



	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
int BerthSvc::delTerminalStorage(const char* pUid, const char* jsonString, std::stringstream& out)
{
	
	JSON_PARSE_RETURN("[BerthSvc::getTerminalInfo]bad format:", jsonString, 1);
	string storageid = root.getv("storageid", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_port_storage SET is_valid='0',update_dt='%s' WHERE storage_id = '%s'",gmt0now,storageid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}

//{portid:"",harbors:["",""]}
int BerthSvc::getStatInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getInOutDetail]bad format:", jsonString, 1);
    string portid = root.getv("portid", "");
	Json* harbor = root.get("harbors");

	char harbors[1024] = "";
	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	int len = 0;
	if (harbor)
	{
		for (int i = 0; i < harbor->size(); i++)
		{
			if (len)
				len += sprintf (harbors + len, ",");
			string _tmp = harbor->getv(i, "");
			if (_tmp.length())
				len += sprintf (harbors + len, "'%s'", _tmp.c_str());
		}
	}

	len = sprintf (sql, "SELECT t1.BERTH_ID, t2.STATUS FROM t41_port_berth t1 LEFT JOIN t41_pmonitor_berth_stat t2 ON t1.BERTH_ID = t2.BERTH_ID AND t1.PORTID=t2.PORT_ID \
							 WHERE t1.Portid = '%s' AND t1.is_valid=1", portid.c_str());
	if (strlen(harbors))
		len += sprintf (sql + len, " AND t1.HARBOUR_ID IN (%s)", harbors);	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int busy = 0;
	int free = 0;
	int status = 0;

	while (psql->NextRow())
	{
		READMYSQL_INT(STATUS, status, 0);
		if (status)
			busy++;
		else
			free++;
	}

	out << "{busy:" << busy << ",free:" << free << "}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{bid:"",type:1} 1:预警,2:吞吐流水
int BerthSvc::getMoorShipInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getMoorShipInfo]bad format:", jsonString, 1);
    string bid = root.getv("bid", "");
	int type = root.getv("type", 1);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	if (type == 1)
		sprintf (sql, "SELECT t1.shipname as shipname_en,t1.to_portid, t1.to_port, t1.berth_id, t1.berth_time,t1.leave_time,t1.ship_id As Shipid,t1.mmsi,t2.shipname_cn as shipname,t2.IMO,t2.REG_PORTID, t2.REG_PORT,t2.DWT,t2.OPERATOR,\
		               t2.CARD_NO, t3.name AS berthname FROM boloomodb.t41_berth_warning t1 LEFT JOIN boloomodb.t41_pmonitor_ship t2 ON t1.ship_id =  t2.shipid\
		               LEFT JOIN boloomodb.t41_port_berth t3 ON t1.berth_id = t3.berth_id\
					   WHERE t1.warn_id = '%s'", bid.c_str());
	else
		sprintf (sql, "SELECT t1.shipname as shipname_en,t1.to_portid, t1.to_port, t1.FROM_PORTID, t1.FROM_PORT, t1.LAYTIME_SPAN, t1.berth_id, t1.ARRIVE_DT, t1.LEAVE_DT,t1.Shipid,t1.mmsi,t2.shipname_cn as shipname,\
		                t1.TRADE_FLAG,t1.LOAD_FLAG, t2.IMO, t2.REG_PORTID, t2.REG_PORT, t2.DWT, t2.OPERATOR, t2.CARD_NO,t3.name AS berthname \
		                FROM boloomodb.t41_berth_flow_info t1 LEFT JOIN boloomodb.t41_pmonitor_ship t2 ON t1.shipid =  t2.shipid\
		                LEFT JOIN boloomodb.t41_port_berth t3 ON t1.berth_id = t3.berth_id\
						WHERE t1.FLOW_ID = '%s'", bid.c_str());
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char shipid[32] = "";
	char shipname[64]= "";
	char shipname_en[64]="";
	int mmsi = 0;
	int imo = 0;
	char regportid[64] = "";		//船籍港ID
	char regport[64] = "";		//船籍港
	double dwt = 0;
	char _operator[64] = "";		//经营人
	char opercard[64] = "";		//营运证号

	int sportid = 0;			//起始港ID
	char sportnm[64] = "";		//起始港名称
	int dportid = 0;			//目的港ID
	char dportnm[64] = "";		//目的港
	int berthstatus = 0;		//靠泊状态 1已靠泊 0计划靠泊
	char berthid[32] = "";		//泊位ID
	char berthname[128] = "";		//泊位名称
	int berthtime = 0;			//靠泊时间
	int leavetime = 0;			//离泊时间
	int loadspan = 0;			//装卸时长

	int tradeflag=0;
	int loadflag=1;

	if (psql->NextRow())
	{
		READMYSQL_STR(Shipid, shipid);
		READMYSQL_STR(shipname, shipname);
		READMYSQL_STR(shipname_en, shipname_en);
		READMYSQL_INT(mmsi, mmsi, 0);
		READMYSQL_INT(IMO, imo, 0);
		/*if (!imo)
			READMYSQL_INT(t4Imo, imo, 0);
		if (!imo)
			READMYSQL_INT(t5Imo, imo, 0);*/

		READMYSQL_STR(REG_PORTID, regportid);
		READMYSQL_STR(REG_PORT, regport);
		READMYSQL_INT(DWT, dwt, 0);
		READMYSQL_STR(OPERATOR, _operator);
		READMYSQL_STR(CARD_NO, opercard);
		
		if (type == 2)
		{
			READMYSQL_INT(FROM_PORTID, sportid, 0);
			READMYSQL_STR(FROM_PORT, sportnm);
			READMYSQL_INT(LAYTIME_SPAN, loadspan, 0);
			READMYSQL_INT(ARRIVE_DT, berthtime, 0);
			READMYSQL_INT(LEAVE_DT, leavetime, 0);
			READMYSQL_INT(TRADE_FLAG, tradeflag, 0);
			READMYSQL_INT(LOAD_FLAG, loadflag, 1);
				
		}
		else
		{
			READMYSQL_INT(berth_time, berthtime, 0);
			READMYSQL_INT(leave_time, leavetime, 0);
		}
		READMYSQL_INT(to_portid, dportid, 0);
		READMYSQL_STR(to_port, dportnm);
		READMYSQL_STR(berth_id, berthid);
		READMYSQL_STR(berthname, berthname);
	}

	out << "{shipinfo:{shipid:\"" << shipid << "\",shipname_en:\""<<shipname_en<<"\",shipname:\""<<shipname<<"\",mmsi:\"" << mmsi << "\",imo:\"" << imo << "\",regportid:\"" << regportid << "\",regport:\"" 
		<< regport << "\",dwt:" << dwt << ", operator:\"" << _operator << "\", opercard:\"" << opercard << "\"},berthinfo:{ sportid:\"" << sportid 
		<< "\",sportnm:\"" << sportnm << "\",dportid:\"" << dportid << "\",dportnm:\"" << dportnm << "\",berthstatus:" << berthstatus << ",berthid:\"" 
		<< berthid << "\",berthname:\"" << berthname << "\", berthtime:" << berthtime << ",leavetime:" << leavetime << ",trade:" << tradeflag <<",loadflag:" <<loadflag<<",loadspan:\"" << loadspan << "\"}}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{flowid:""}
int BerthSvc::getCargoInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getCargoInfo]bad format:", jsonString, 1);
    string flowid = root.getv("flowid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT WAYBILL_ID, CARGO_TYPE1, CARGO_TYPE2, CARGO_NUM, TOTAL_WEIGHT, OWNER, FREIGHT, TURNOVER, CARGOID\
					FROM t41_berth_flow_cargo WHERE FLOW_ID = '%s'", flowid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char cargoid[32];		//货物ID
	char waybill[32];			//运单号
	int type1;				//货物类型 0普通货物 1集装箱 2客(滚)船
	char type2[16];			//货物子类型 根据type1
	int amount;				//数量
	double weight;			//重量
	char owner[64];			//货主
	double charge;			//运费
	double turnover;		//周转量

	int cnt = 0;
	out << "{flowid:\"" << flowid << "\",cargo:[";
	while (psql->NextRow())
	{
		READMYSQL_STR(WAYBILL_ID, waybill);
		READMYSQL_INT(CARGO_TYPE1, type1, 0);
		READMYSQL_STR(CARGO_TYPE2, type2);
		READMYSQL_INT(CARGO_NUM, amount, 0);
		READMYSQL_DOUBLE(TOTAL_WEIGHT, weight, 0.0);
		READMYSQL_DOUBLE(FREIGHT, charge, 0.0);
		READMYSQL_DOUBLE(TURNOVER, turnover, 0.0);
		READMYSQL_STR(OWNER, owner);
		READMYSQL_STR(CARGOID, cargoid);
		if (cnt++)
			out << ",";
		out << "{cargoid:\"" << cargoid << "\",waybill:\"" << waybill << "\",type1:\"" << type1 << "\",type2:\"" << type2 << "\",amount:" << amount << ",weight:\"" << weight << "\",owner:\"" << owner << "\",charge:\"" << charge << "\",turnover:\"" << turnover << "\"}";
	}

	out <<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"",mmsi:"",imo:"",regportid:"",regport:"",dwt:0, operator:"", opercard:""}
int BerthSvc::updShipBaseInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::updShipBaseInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string mmsi =  root.getv("mmsi", "");
	string imo = root.getv("imo", "");
	string shipname_en = root.getv("shipname_en", "");
	string shipname_cn = root.getv("shipname", "");
	string regportid = root.getv("regportid", "");
	string regport = root.getv("regport", "");
	int dwt = root.getv("dwt", 0);
	string _operator = root.getv("operator", "");
	string opercard = root.getv("opercard", "");

	MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";

	sprintf(sql, "select count(1) as count from t41_pmonitor_ship where shipid = '%s'",shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	if(atoi(psql->GetField("count"))>0)
	{
		sprintf(sql, "update t41_pmonitor_ship set imo='%s',mmsi='%s',shipname='%s',shipname_cn='%s',REG_PORTID='%s',REG_PORT='%s',dwt='%d',operator='%s',CARD_NO='%s' where shipid = '%s'", 
			imo.c_str(), mmsi.c_str(),shipname_en.c_str(),shipname_cn.c_str(), regportid.c_str(), regport.c_str(), dwt, _operator.c_str(), opercard.c_str(),shipid.c_str());
		
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else
	{
		sprintf (sql, "insert into t41_pmonitor_ship (shipid, imo, mmsi,shipname,shipname_cn, REG_PORTID, REG_PORT, dwt, operator, CARD_NO) VALUES ('%s','%s','%s','%s','%s','%s','%s','%d','%s','%s')", 
			shipid.c_str(), imo.c_str(), mmsi.c_str(),shipname_en.c_str(),shipname_cn.c_str(), regportid.c_str(), regport.c_str(), dwt, _operator.c_str(), opercard.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}


	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{bid:"",type:1,
//berthinfo:{shipid:"",shipnm:"",mmsi:"",sportid:"",sportnm:"",dportid:"",dportnm:"",berthid:"",berthname:"",berthtime:1302123457,leavetime:1302123457,berthspan:"",loadspan:""},
//cargo:[{cargoid:"",waybill:"",type1:"",type2:"",amount:124,weight:"",owner:"",charge:"",turnover:""}...]}
int BerthSvc::updInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::updInOutInfo]bad format:", jsonString, 1);
	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 10] = "";

	char gmt0now[20];
	GmtNow(gmt0now);
	uint64 timeusec = GmtNowUsec();

    string bid = root.getv("bid", "");
	int type = root.getv("type", 0);
	Json* berthinfo = root.get("berthinfo");
	string shipid = berthinfo->getv("shipid", "");
	string shipnm = berthinfo->getv("shipnm", "");
	string mmsi = berthinfo->getv("mmsi", "");
	string sportid = berthinfo->getv("sportid", "");
	string sportnm = berthinfo->getv("sportnm", "");
	string dportid = berthinfo->getv("dportid", "");
	string dportnm = berthinfo->getv("dportnm", "");
	string berthid = berthinfo->getv("berthid", "");
	string berthname = berthinfo->getv("berthname", "");
	int berthtime = berthinfo->getv("berthtime", 0);
	int leavetime = berthinfo->getv("leavetime", 0);
	int lspan = 0;
	if (berthtime && leavetime)
		lspan = leavetime - berthtime;
	string berthspan = berthinfo->getv("berthspan", "");
	string loadspan = berthinfo->getv("loadspan", "");
    int tradeflag=berthinfo->getv("trade", 0);
	int loadflag=berthinfo->getv("loadflag", 0);

	double totalweight=0.0;

	if (type==1&&!bid.empty())//处理预警
	{
		sprintf (sql, "select count(*) as num from t41_berth_warning where warn_id='%s' and status=0",bid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		int number=0;
        if (psql->NextRow())
		{
			READMYSQL_INT(num, number, 0);
		}
        //如果已处理，直接返回，防止重复处理
		if (number<=0)
		{
			out << "{\"eid\":\"0\"}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}
	}


	string cargotype="";

	char flowid[32] = "";
	if (type == 2)
		sprintf (flowid, "%s", bid.c_str());
	else
		sprintf (flowid, "%lld", timeusec);


	char sqldel[256];
	sprintf (sqldel, "delete from t41_berth_flow_cargo where flow_id='%s'",flowid);
	CHECK_MYSQL_STATUS(psql->Execute(sqldel)>=0, 3);
	
	Json* cargo = root.get("cargo");

	int _cnt = 0;
	int len = sprintf (sql, "INSERT INTO t41_berth_flow_cargo (FLOW_ID, WAYBILL_ID, CARGO_TYPE1, CARGO_TYPE2, CARGO_NAME, CARGO_NUM, LOAD_FLAG, TOTAL_WEIGHT, OWNER, FREIGHT, UPDATE_DT, CARGOID, TURNOVER) VALUES");
	for (int i = 0; i < cargo->size(); i++)
	{
		Json* _cargo = cargo->get(i);
		string cargoid = _cargo->getv("cargoid", "");
		string waybill = _cargo->getv("waybill", "");
		string type1 = _cargo->getv("type1", "");
		string type2 = _cargo->getv("type2", "");
		int amount = _cargo->getv("amount", 0);
		string weight = _cargo->getv("weight", "");
		string owner = _cargo->getv("owner", "");
		string charge = _cargo->getv("charge", "");
		string turnover = _cargo->getv("turnover", "");

		totalweight+=atof(weight.c_str());
		cargotype=type1;
		//cargonm, load_flag,
		if (_cnt++)
			len += sprintf (sql + len, ",");
		len += sprintf (sql + len, "('%s', '%s', '%s', '%s', '', '%d', '0', '%s', '%s', '%s', '%s', '%s', '%s')", 
			flowid, waybill.c_str(), type1.c_str(), type2.c_str(), amount, weight.c_str(), owner.c_str(), charge.c_str(), gmt0now, cargoid.c_str(), turnover.c_str());
	}

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);



	sprintf (sql, "REPLACE INTO t41_berth_flow_info (FLOW_ID, SHIPID, MMSI, SHIPNAME, BERTH_ID, FROM_PORTID, FROM_PORT, TO_PORTID, TO_PORT, DISTANCE, ARRIVE_DT, LEAVE_DT, LAYTIME_SPAN, OPERATOR_ID, OPERATOR, UPDATE_DT,CARGO_TYPE,TOTAL_WEIGHT,TRADE_FLAG,LOAD_FLAG) \
				VALUES ('%s','%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '0.0', '%d', '%d', '%s', '%s', '', '%s','%s','%.1f',%d,%d)", 
				flowid, shipid.c_str(), mmsi.c_str(), shipnm.c_str(), berthid.c_str(), sportid.c_str(), sportnm.c_str(), dportid.c_str(), 
				dportnm.c_str(), berthtime, leavetime, loadspan.c_str(), pUid, gmt0now,cargotype.c_str(),totalweight,tradeflag,loadflag);

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	if (type == 1&&!bid.empty())
	{

		sprintf (sql, "UPDATE t41_berth_warning SET `status` = 1,update_dt='%s' WHERE warn_id = '%s'", gmt0now,bid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	

	
	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{portid:"",harbors:["",""],starttm:135442320,endtm:135442320}
int BerthSvc::getInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getInOutInfo]bad format:", jsonString, 1);
    string portid = root.getv("portid", "");
	Json* harbor = root.get("harbors");
	int starttm = root.getv("starttm", 0)-8*3600;
	int endtm = root.getv("endtm", 0)-8*3600;
	char harbors[1024] = "";
	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	int prestarttm=starttm-15*24*3600;//往前推15天
	int len = 0;
	if (harbor)
	{
		for (int i = 0; i < harbor->size(); i++)
		{
			if (len)
				len += sprintf (harbors + len, ",");
			string _tmp = harbor->getv(i, "");
			if (_tmp.length())
				len += sprintf (harbors + len, "'%s'", _tmp.c_str());
		}
	}

	len = sprintf (sql, "SELECT t2.status,t2.berth_time,t2.leave_time,UNIX_TIMESTAMP(t2.UPDATE_DT) as updtm FROM t41_port_berth t1, t41_berth_warning t2 \
							WHERE t1.Portid = '%s' and t1.Berth_id = t2.Berth_id", portid.c_str());
	if (strlen(harbors))
		len += sprintf (sql + len, " AND t1.HARBOUR_ID IN (%s)", harbors);
	sprintf (sql + len, " AND UNIX_TIMESTAMP(t2.UPDATE_DT)>='%d' AND UNIX_TIMESTAMP(t2.UPDATE_DT)<='%d'", prestarttm, endtm);
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int unhandled = 0;	//未填写
	int handled = 0;	//已填写
	int timeout = 0;	//已逾期
	int exclude =0;
	int status;
	int berthtime=0;
	int leavetime=0;
	int updtime=0;

	while (psql->NextRow())
	{
		READMYSQL_INT(status, status, 0);
		READMYSQL_INT(berth_time, berthtime, 0);
		READMYSQL_INT(leave_time, leavetime, 0);
		READMYSQL_INT(updtm, updtime, 0);

		if (status==0)
		{
		   if (leavetime>0&&leavetime<starttm)
		   {
			   timeout++;
		   }
		   else
		   {
		        unhandled++;
		   }
		}	
		else if (status == 1)
		{
			if (updtime>=starttm)
			{
				handled++;
			}
		}
			
		else if (status == 2)
		{
			 if (updtime>=starttm)
			 {
				 exclude++;
			 } 
		}
			
	}

	out << "{unhandled:" << unhandled << ",handled:" << handled << ",timeout:" << timeout <<",exclude:" << exclude << "}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{portid:"",harbors:["",""],starttm:135442320,endtm:135442320}
int BerthSvc::getInOutList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getInOutList]bad format:", jsonString, 1);
    string portid = root.getv("portid", "");
	Json* harbor = root.get("harbors");
	int starttm = root.getv("starttm", 0)-8*3600;
	int endtm = root.getv("endtm", 0)-8*3600;

	int prestarttm=starttm-15*24*3600;//往前推一个月

	char harbors[1024] = "";
	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	int len = 0;
	if (harbor)
	{
		for (int i = 0; i < harbor->size(); i++)
		{
			if (len)
				len += sprintf (harbors + len, ",");
			string _tmp = harbor->getv(i, "");
			if (_tmp.length())
				len += sprintf (harbors + len, "'%s'", _tmp.c_str());
		}
	}

	len = sprintf (sql, "SELECT t2.warn_id, t2.level, t2.mmsi, t2.berth_id, t2.leave_time, t2.berth_time, t1.Name AS berthnm, t2.ship_id AS SHIPID, t2.shipname AS shipname,UNIX_TIMESTAMP(t2.UPDATE_DT) as updtm,t2.to_port, t2.status \
							FROM t41_port_berth t1,t41_berth_warning t2 WHERE t1.Portid = '%s' and t1.Berth_id = t2.Berth_id and t2.status!=1", portid.c_str());
	if (strlen(harbors))
		len += sprintf (sql + len, " AND t1.HARBOUR_ID IN (%s)", harbors);
	sprintf (sql + len, " AND UNIX_TIMESTAMP(t2.UPDATE_DT)>='%d' AND UNIX_TIMESTAMP(t2.UPDATE_DT)<='%d'", prestarttm, endtm);
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	out << "[";

	char bid[32] = "";		//预警ID或吞吐流水ID
	int level;			//预警级别,对于非 待填写状态，level为-1
	int status;			//0待填写 1已填写 2已逾期 3已排除
	char shipid[64] = "";	//船舶id
	char shipnm[128] = "";	//船舶名称,
	int mmsi;
	char berthid[64] = "";	//停靠泊位ID
	char berthnm[128] = "";	//泊位名称
	int arrivetm;		//到港时间
	float portspan;		//在港时长(小时)
	char startport[64] = "";	//始发港
	char destport[64] = "";	//目的港
	int cargotype = 0;		//货物类型
	double weight = 0;		//吞吐量
	char opid[64] = "";		//经办人ID
	char opnm[128] = "";		//经办人
    int updtm=0;
	int cnt = 0;
	while (psql->NextRow())
	{

		int _tleave = 0;
		READMYSQL_INT(berth_time, arrivetm, 0);
		READMYSQL_INT(leave_time, _tleave, 0);
        READMYSQL_INT(status, status, 0);
		READMYSQL_INT(updtm, updtm, 0);

		if (status==0)
		{
			if (_tleave>0&&_tleave<starttm)
			{
				status=3; //已逾期
			}
		}	
		else if (status == 2) //已排除
		{
			if (updtm<starttm)
			{
				continue;
			} 
		}

		READMYSQL_STR(warn_id, bid);
		READMYSQL_INT(level, level, 0);
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(shipname, shipnm);
		READMYSQL_INT(mmsi, mmsi, 0);
		READMYSQL_STR(berth_id, berthid);
		READMYSQL_STR(to_port, destport);
		READMYSQL_STR(berthnm, berthnm);
		
		if (_tleave && arrivetm)
			portspan = (float(_tleave - arrivetm))/3600;
		else
			portspan = 0.0;
		if (cnt++)
			out << ",";
		out << "{bid:\"" << bid << "\",level:\"" << level << "\",status:\"" << status << "\",shipid:\"" << shipid << "\",shipnm:\"" << shipnm 
			<< "\",mmsi:\"" << mmsi << "\",berthid:\"" << berthid << "\",berthnm:\"" << berthnm << "\",arrivetm:" << arrivetm << ",portspan:\"" 
			<< portspan << "\",startport:\"\",destport:\""<<destport<<"\",cargotype:\"\",weight:\"\",opid:\"\",opnm:\"\"}";
	}

	len = sprintf (sql, "SELECT t2.FLOW_ID, t2.SHIPID, t2.SHIPNAME, t2.MMSI, t2.BERTH_ID, t1.Name AS bnm, t2.ARRIVE_DT, t2.LEAVE_DT, t2.TO_PORT, t2.FROM_PORT, t3.name AS OPERATOR,\
						t2.OPERATOR_ID, t2.TOTAL_WEIGHT, t2.CARGO_TYPE FROM t41_port_berth t1,t41_berth_flow_info t2, t41_pmonitor_user t3 \
						WHERE t1.Portid = '%s' and t1.Berth_id = t2.BERTH_ID and t2.OPERATOR_ID=t3.USER_ID", portid.c_str());
	if (strlen(harbors))
		len += sprintf (sql + len, " AND t1.HARBOUR_ID IN (%s)", harbors);
	sprintf (sql + len, " AND UNIX_TIMESTAMP(t2.UPDATE_DT)>='%d' AND UNIX_TIMESTAMP(t2.UPDATE_DT)<='%d'", starttm, endtm);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while (psql->NextRow())
	{
		READMYSQL_STR(FLOW_ID, bid);
		level = -1;
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(SHIPNAME, shipnm);
		READMYSQL_INT(MMSI, mmsi, 0);
		READMYSQL_STR(BERTH_ID, berthid);
		READMYSQL_STR(bnm, berthnm);
		READMYSQL_INT(ARRIVE_DT, arrivetm, 0);
		int _tleave;
		READMYSQL_INT(LEAVE_DT, _tleave, 0);
		if (arrivetm && _tleave)
			portspan = (float(_tleave - arrivetm))/3600;
		else
			portspan = 0.0;
		READMYSQL_STR(TO_PORT, destport);
		READMYSQL_STR(FROM_PORT, startport);
		READMYSQL_INT(CARGO_TYPE, cargotype, 0);
		READMYSQL_DOUBLE(TOTAL_WEIGHT, weight, 0.0);
		READMYSQL_STR(OPERATOR_ID, opid);
		READMYSQL_STR(OPERATOR, opnm);
		if (cnt++)
			out << ",";
		out << "{bid:\"" << bid << "\",level:\"" << level << "\",status:\"1"<< "\",shipid:\"" << shipid << "\",shipnm:\"" << shipnm 
			<< "\",mmsi:\"" << mmsi << "\",berthid:\"" << berthid << "\",berthnm:\"" << berthnm << "\",arrivetm:" << arrivetm << ",portspan:\"" 
			<< portspan << "\",startport:\"" << startport << "\",destport:\"" << destport << "\",cargotype:\"" << cargotype << "\",weight:\""
			<< weight << "\",opid:\"" << opid << "\",opnm:\"" << opnm << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{portid:"",harbors:["",""],starttm:135442320,endtm:135442320}
int BerthSvc::getUndealWarn(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getUndealWarn]bad format:", jsonString, 1);
    string portid = root.getv("portid", "");
	Json* harbor = root.get("harbors");
	int starttm = root.getv("starttm", 0)-8*3600;
	int endtm = root.getv("endtm", 0)-8*3600;
	char harbors[1024] = "";
	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	int len = 0;
	if (harbor)
	{
		for (int i = 0; i < harbor->size(); i++)
		{
			if (len)
				len += sprintf (harbors + len, ",");
			string _tmp = harbor->getv(i, "");
			if (_tmp.length())
				len += sprintf (harbors + len, "'%s'", _tmp.c_str());
		}
	}

	len = sprintf (sql, "SELECT t2.warn_id, t2.level, t2.mmsi,t2.ship_id, t2.shipname,t2.berth_id, t2.longitude, t2.latitude, t2.leave_time, t2.berth_time, t1.Name AS berthnm \
		             FROM t41_port_berth t1,t41_berth_warning t2  WHERE t1.Portid = '%s' and t1.Berth_id = t2.Berth_id ", portid.c_str());
	if (strlen(harbors))
		len += sprintf (sql + len, " AND t1.HARBOUR_ID IN (%s)", harbors);
	sprintf (sql + len, " AND UNIX_TIMESTAMP(t2.UPDATE_DT)>='%d' AND UNIX_TIMESTAMP(t2.UPDATE_DT)<='%d' AND t2.status = '0'", starttm, endtm);
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char alertid[32];		//预警ID
	int level;				//预警级别
	char shipid[64];		//船舶id
	char shipnm[64];		//船舶名称,
	int mmsi;
	char berthid[64];		//停靠泊位ID
	char berthnm[64];		//泊位名称
	double xpos;			//经度
	double ypos;			//纬度
	int bertm;				//靠泊时间
	int leavetm;			//离泊时间

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(warn_id, alertid);
		READMYSQL_STR(berth_id, berthid);
		READMYSQL_STR(berthnm, berthnm);
		READMYSQL_INT(level, level, 0);
		READMYSQL_INT(mmsi, mmsi, 0);
		READMYSQL_DOUBLE(longitude, xpos, -1.0);
		READMYSQL_DOUBLE(latitude, ypos, -1.0);
		READMYSQL_INT(leave_time, leavetm, 0);
		READMYSQL_INT(berth_time, bertm, 0);
		READMYSQL_STR(ship_id, shipid);
		READMYSQL_STR(shipname, shipnm);
		if (cnt++)
			out << ",";
		out << "{alertid:\"" << alertid << "\",level:\"" << level << "\",shipid:\"" << shipid << "\",shipnm:\"" << shipnm << "\",mmsi:\"" << mmsi 
			<< "\",berthid:\"" << berthid << "\",berthnm:\"" << berthnm << "\",xpos:\"" << xpos << "\",ypos:\"" << ypos << "\",bertm:" << bertm 
			<< ",leavetm:" << leavetm << "}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{alertid:"",bremove:0,harborid:""}
int BerthSvc::exclWarning(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::exclWarning]bad format:", jsonString, 1);
    string alertid = root.getv("alertid", "");
	int bremove = root.getv("bremove",  0);
	string harborid = root.getv("harborid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "UPDATE t41_berth_warning SET `status` = 2 ,update_dt='%s' WHERE warn_id = '%s'",gmt0now,alertid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	
	if (bremove)
	{
		sprintf (sql, "SELECT ship_id, shipname, mmsi FROM t41_berth_warning  WHERE warn_id = '%s'", alertid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		char shipid[256] = "";	
		char name[128] = "";
		int mmsi = 0;

		if (psql->NextRow())
		{
			READMYSQL_STR(ship_id, shipid);
			READMYSQL_STR(shipname, name);
			READMYSQL_INT(mmsi, mmsi, 0);
		}

		sprintf (sql, "INSERT INTO t41_pmonitor_ship_remove (SHIPID, SHIPNAME, HARBOUR_ID, OPERATOR, IS_VALID, UPDATE_DT, mmsi) VALUES ('%s','%s','%s','%s','%d','%s', '%d')",
			shipid, name, harborid.c_str(), pUid, bremove, gmt0now, mmsi);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	}
	

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BerthSvc::getStatIndex(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getStatIndex]bad format:", jsonString, 1);
    string portid = root.getv("portid", "");
	Json* harbor = root.get("harbors");
	int starttm = root.getv("starttm", 0);
	int endtm = root.getv("endtm", 0);
	char harbors[1024] = "";
	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	int len = 0;
	if (harbor)
	{
		for (int i = 0; i < harbor->size(); i++)
		{
			if (len)
				len += sprintf (harbors + len, ",");
			string _tmp = harbor->getv(i, "");
			if (_tmp.length())
				len += sprintf (harbors + len, "'%s'", _tmp.c_str());
		}
	}

	len = sprintf (sql, "SELECT t2.FLOW_ID, t3.TURNOVER, t3.TOTAL_WEIGHT \
		                 FROM t41_port_berth t1,t41_berth_flow_info t2,t41_berth_flow_cargo t3\
		                 WHERE t1.Portid = '%s' AND t1.Berth_id = t2.BERTH_ID  AND t2.FLOW_ID = t3.FLOW_ID", portid.c_str());
	if (strlen(harbors))
		len += sprintf (sql + len, " AND t1.HARBOUR_ID IN (%s)", harbors);
	sprintf (sql + len, " AND t2.LEAVE_DT>=%d AND t2.LEAVE_DT<=%d ORDER BY t2.FLOW_ID", starttm, endtm);
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	double tw = 0.0;
	double cw = 0.0;
	int cnt = 0;
	char _tflowid[32] = "";


	while (psql->NextRow())
	{
		double _tw = 0.0;
		double _cw = 0.0;
		char flowid[32] = "";
		READMYSQL_STR(FLOW_ID, flowid);
		if (strcmp(flowid, _tflowid))
		{
			strcpy(_tflowid, flowid);
			cnt++;
		}
		READMYSQL_DOUBLE(TURNOVER, _cw, 0.0);
		READMYSQL_DOUBLE(TOTAL_WEIGHT, _tw, 0.0);
		tw += _tw;
		cw += _cw;
	}

	char twStr[32]="";
	sprintf(twStr,"%.0f",tw);

	char cwStr[32]="";
	sprintf(cwStr,"%.0f",cw);

	out << "{entryships:" << cnt << ",throughput:\"" << twStr << "\",circleflow:\"" << cwStr << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{portid:"",harbors:["",""],starttm:135442320,endtm:135442320}
int BerthSvc::getInOutDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getInOutDetail]bad format:", jsonString, 1);
    string portid = root.getv("portid", "");
	Json* harbor = root.get("harbors");
	int starttm = root.getv("starttm", 0);
	int endtm = root.getv("endtm", 0);
	char harbors[1024] = "";
	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	int len = 0;
	if (harbor)
	{
		for (int i = 0; i < harbor->size(); i++)
		{
			if (len)
				len += sprintf (harbors + len, ",");
			string _tmp = harbor->getv(i, "");
			if (_tmp.length())
				len += sprintf (harbors + len, "'%s'", _tmp.c_str());
		}
	}

	len = sprintf (sql, "SELECT t2.FLOW_ID, t2.SHIPID, t2.SHIPNAME, t2.MMSI, t3.SHIPNAME_CN, t3.REG_PORT, t3.DWT, t3.CARD_NO, t2.BERTH_ID, t1.Name AS bnm,t1.harbour_id,t1.terminal_id,\
		t2.ARRIVE_DT, t2.LEAVE_DT, t2.LAYTIME_SPAN, t2.TRADE_FLAG,t2.LOAD_FLAG, t2.TO_PORT, t2.FROM_PORT, t2.DISTANCE, t2.OPERATOR,\
		t2.OPERATOR_ID,t5.NAME AS OPERATOR, t4.TOTAL_WEIGHT, t4.CARGOID, t4.WAYBILL_ID, t4.CARGO_TYPE1, t4.CARGO_TYPE2, t4.OWNER,t4.FREIGHT, t4.TURNOVER \
		FROM t41_port_berth t1,t41_berth_flow_cargo t4,t41_berth_flow_info t2 LEFT JOIN t41_pmonitor_ship t3 ON t2.SHIPID = t3.SHIPID \
		LEFT JOIN t41_pmonitor_user t5 ON T2.OPERATOR_ID=t5.USER_ID \
		WHERE t1.Portid = '%s' and t1.Berth_id = t2.BERTH_ID and t2.FLOW_ID = t4.FLOW_ID", portid.c_str());
	if (strlen(harbors))
		len += sprintf (sql + len, " AND t1.HARBOUR_ID IN (%s)", harbors);
	sprintf (sql + len, " AND t2.LEAVE_DT>=%d AND t2.LEAVE_DT<=%d ORDER BY t2.FLOW_ID", starttm, endtm);
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	
	std::map<std::string,_ThruptFlowInfo> mapThruFlow;

	
	while (psql->NextRow())
	{

         _ThruptFlowInfo FlowInfoItem;

		 READMYSQL_STR(FLOW_ID, FlowInfoItem.flowid);
         
		 if (mapThruFlow.find(FlowInfoItem.flowid)==mapThruFlow.end())
		 {
            
			 READMYSQL_STR(SHIPID, FlowInfoItem.shipid);
			 READMYSQL_STR(SHIPNAME, FlowInfoItem.shipnm);
			 READMYSQL_STR(SHIPNAME_CN, FlowInfoItem.shipchnm);
			 READMYSQL_INT(MMSI, FlowInfoItem.mmsi, 0);
			 READMYSQL_STR(REG_PORT, FlowInfoItem.regport);
			 READMYSQL_DOUBLE(DWT, FlowInfoItem.dwt, 0.0);
			 READMYSQL_STR(OPER, FlowInfoItem.op);
			 READMYSQL_STR(CARD_NO, FlowInfoItem.opcd);
			 READMYSQL_STR(BERTH_ID, FlowInfoItem.bid);
			 READMYSQL_STR(bnm, FlowInfoItem.bnm);
			
			 READMYSQL_STR(terminal_id, FlowInfoItem.tid);
			 READMYSQL_STR(harbour_id, FlowInfoItem.hid);
			 READMYSQL_INT(ARRIVE_DT, FlowInfoItem.arrivetm, 0);
			 READMYSQL_INT(LOAD_FLAG, FlowInfoItem.loadflag, 1);
			 int _tleave = 0;
			 READMYSQL_INT(LEAVE_DT, FlowInfoItem.leavetm, 0);
			 _tleave=FlowInfoItem.leavetm;
			 if (_tleave && FlowInfoItem.arrivetm)
				 FlowInfoItem.pspan = ((float)(_tleave - FlowInfoItem.arrivetm))/3600;
			 else
				 FlowInfoItem.pspan = 0.0;
			 READMYSQL_DOUBLE(LAYTIME_SPAN, FlowInfoItem.lpan, 0.0);
			 READMYSQL_INT(TRADE_FLAG, FlowInfoItem.trade, 0);
			 READMYSQL_STR(TO_PORT, FlowInfoItem.dport);
			 READMYSQL_STR(FROM_PORT, FlowInfoItem.sport);
			 double _tdist;
			 READMYSQL_DOUBLE(DISTANCE, _tdist, 0.0);
			 READMYSQL_STR(OPERATOR, FlowInfoItem.opnm);
			 READMYSQL_STR(OPERATOR_ID, FlowInfoItem.opid);
			

			 _CargoInfo CargoItem;
			 READMYSQL_DOUBLE(TOTAL_WEIGHT, CargoItem.tw, 0.0);
			 READMYSQL_DOUBLE(TURNOVER, CargoItem.cw, 0.0);

			 READMYSQL_STR(WAYBILL_ID, CargoItem.billway);
			 READMYSQL_STR(CARGOID, CargoItem.cid);
			 READMYSQL_INT(CARGO_TYPE1, CargoItem.tp1, 0);
			 READMYSQL_STR(CARGO_TYPE2, CargoItem.tp2);			
			 READMYSQL_STR(OWNER, CargoItem.own);
			 READMYSQL_INT(LOAD_FLAG, CargoItem.load, 0);
			 READMYSQL_DOUBLE(FREIGHT, CargoItem.fr, 0.0);
             
			 FlowInfoItem.cargoList.push_back(CargoItem);
			 mapThruFlow[FlowInfoItem.flowid]=FlowInfoItem;
			 
		 }
		 else
		 {   
			
			 _CargoInfo CargoItem;
			 READMYSQL_DOUBLE(TOTAL_WEIGHT, CargoItem.tw, 0.0);
			 READMYSQL_DOUBLE(TURNOVER, CargoItem.cw, 0.0);

			 READMYSQL_STR(WAYBILL_ID, CargoItem.billway);
			 READMYSQL_STR(CARGOID, CargoItem.cid);
			 READMYSQL_INT(CARGO_TYPE1, CargoItem.tp1, 0);
			 READMYSQL_STR(CARGO_TYPE2, CargoItem.tp2);			
			 READMYSQL_STR(OWNER, CargoItem.own);
			 READMYSQL_INT(LOAD_FLAG, CargoItem.load, 1);
			 READMYSQL_DOUBLE(FREIGHT, CargoItem.fr, 0.0);
			 mapThruFlow[FlowInfoItem.flowid].cargoList.push_back(CargoItem);
			 
		 }


	}



	out << "{info:[";

	
	std::map<std::string,_ThruptFlowInfo>::iterator it=mapThruFlow.begin();

	int index=0;
	for (;it!=mapThruFlow.end();it++)
	{
		if (index>0)
		{
			out << ",";
		}
         _ThruptFlowInfo ThruptItem=it->second;

		 out << "{flowid:\"" << ThruptItem.flowid << "\",shipid:\"" << ThruptItem.shipid << "\",shipnm:\"" << ThruptItem.shipnm << "\",shipchnm:\"" << ThruptItem.shipchnm << "\",mmsi:\"" << ThruptItem.mmsi << "\",regport:\"" 
			 << ThruptItem.regport << "\",dwt:" << ThruptItem.dwt << ",op:\"" << ThruptItem.op << "\",opcd:\"" << ThruptItem.opcd<<"\",harborid:\""<<ThruptItem.hid<<"\",terminalid:\""<<ThruptItem.tid<< "\",bid:\"" << ThruptItem.bid << "\",bnm:\"" << ThruptItem.bnm 
			 << "\",arrivetm:" << ThruptItem.arrivetm <<",leavetm:" << ThruptItem.leavetm << ",pspan:\"" << ThruptItem.pspan << "\", lpan:\"" << ThruptItem.lpan<< "\",load:\"" << ThruptItem.loadflag << "\",trade:\"" << ThruptItem.trade << "\",sport:\"" << ThruptItem.sport 
			 << "\",dport:\"" << ThruptItem.dport << "\",opid:\"" << ThruptItem.opid << "\",opnm:\"" << ThruptItem.opnm << "\",cargo:[";

		 double _tw=0.0,_cw=0.0;

		 for (int i=0;i<(int)ThruptItem.cargoList.size();i++)
		 {
			 _CargoInfo CargoItem=ThruptItem.cargoList[i];

			 if (i>0)
			 {
                out << ",";
			 }
             out<<"{cid:\"" << CargoItem.cid << "\",billway:\"" << CargoItem.billway << "\",tp1:\"" << CargoItem.tp1 << "\",tp2:\"" << CargoItem.tp2 << "\",load:\"" << CargoItem.load ;
             out<< "\",own:\"" << CargoItem.own << "\",fr:\"" << CargoItem.fr << "\",tw:\"" << CargoItem.tw << "\",cw:\"" << CargoItem.cw << "\"}";
             _tw+=CargoItem.tw;
             _cw+=CargoItem.cw;
		 }

          out <<"],tw:\"" << _tw << "\",cw:\"" << _cw << "\"}";
 
		 index++;
	}
   

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:""}
int BerthSvc::getContactInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getContactInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT RID, NAME, TYPE, TELNO FROM t41_pmonitor_ship_relation WHERE shipid = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char rid[32] = "";			//记录ID
	char name[64] = "";			//联系人名称
	char type[32] = "";			//类型
	char telno[32] = "";		//电话号码

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(RID, rid);
		READMYSQL_STR(NAME, name);
		READMYSQL_STR(TYPE, type);
		READMYSQL_STR(TELNO, telno);
		if (cnt++)
			out << ",";
		out << "{rid:\"" << rid << "\",name:\"" << name << "\",type:\"" << type << "\",telno:\"" << telno << "\"}";
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"", name:"",type:"",telno:""}
int BerthSvc::addShipContactPerson(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::addShipContactPerson]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string name = root.getv("name", "");
	string type = root.getv("type", "");
	string telno = root.getv("telno", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	uint64 timeusec = GmtNowUsec();

	sprintf (sql, "INSERT INTO t41_pmonitor_ship_relation (RID, SHIPID, NAME, TYPE, TELNO) VALUES ('R%lld','%s','%s','%s','%s')", 
		timeusec, shipid.c_str(), name.c_str(), type.c_str(), telno.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"", rid:""}
int BerthSvc::delShipContactPerson(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getTerminalInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string rid = root.getv("rid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "DELETE FROM t41_pmonitor_ship_relation WHERE shipid = '%s' AND rid = '%s'", shipid.c_str(), rid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{}
int BerthSvc::getUserAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t2.NAME AS organ_nm,t1.name, t1.ORGAN_ID AS organ_id, t1.RANK, t1.ROLE,t1.duty,t1.telephone,t1.mobile,t3.HARBOUR_ID, t3.name AS harbornm, t4.password \
						FROM t41_pmonitor_user t1 LEFT JOIN t41_pmonitor_organ t2 ON t1.ORGAN_ID = t2.ORGAN_ID\
						LEFT JOIN t41_pmonitor_rel_organ_harbour t3 ON t1.ORGAN_ID = t3.ORGAN_ID \
						LEFT JOIN t00_user t4 ON t1.USER_ID=t4.user_id \
						WHERE t1.USER_ID = '%s' AND t1.IS_VALID >= 1 AND t3.IS_VALID = '1'", pUid);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char organ_nm[64] = "";		//所属机构名称
	char organ_id[64] = "";		//所属机构ID 
	int rank = 0;					//级别
	char role[128];					//权限
	char harborid[32] = "";	//服务港区ID
	char harbornm[64] = "";		//服务港区名称
	char telephone[32]="";
	char mobile[32]="";
	char duty[128]="";
	char password[64]="";
	char name[32]="";
	

	int cnt = 0;
	while (psql->NextRow())
	{
		if (!cnt)
		{
			READMYSQL_STR(organ_nm, organ_nm);
			READMYSQL_STR(organ_id, organ_id);
			READMYSQL_INT(RANK, rank, 0);
			READMYSQL_STR(ROLE, role);
			READMYSQL_STR(telephone, telephone);
			READMYSQL_STR(mobile, mobile);
			READMYSQL_STR(duty, duty);
			READMYSQL_STR(name, name);
			READMYSQL_STR(password, password);

			out << "{organ_id:\"" << organ_id << "\",organ_nm:\"" << organ_nm<< "\",name:\"" << name<< "\",pwd:\"" << password << "\",rank:\"" << rank << "\",role:\"" << role<<"\",duty:\"" << duty << "\",telno:\"" << telephone << "\",mobile:\"" << mobile <<"\",harbors:[";
		}		
		READMYSQL_STR(HARBOUR_ID, harborid);
		READMYSQL_STR(harbornm, harbornm);
		if (cnt++)
			out << ",";
		out << "{harborid:\"" << harborid << "\",harbornm:\"" << harbornm << "\"}";
	}
	if (cnt)
		out << "]}";
	else
		out <<"{organ_id:\"\",organ_nm:\"\",rank:\"\",role:\"\",harbors:[]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//{portid:""}
int BerthSvc::getSystemUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getSystemUser]bad format:", jsonString, 1);

	string portid = root.getv("portid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT a.USER_ID, a.NAME, a.ORGAN_ID, a.ROLE, a.DUTY, a.TELEPHONE, a.MOBILE, b.password,\
		      c.name as organname  FROM t00_user b,t41_pmonitor_user a left join t41_pmonitor_organ c on a.organ_id=c.organ_id \
			  WHERE a.PORTID = '%s' and a.IS_VALID='1' and a.user_id = b.user_id order by rank", portid.c_str());
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		char usrid[32];
		char pwd[32];
		char name[64];
		char organid[32];
		char organname[64];
		char auth[64];
		char duty[128];
		char telno[32];
		char mobile[32];

		READMYSQL_STR(USER_ID, usrid);
		READMYSQL_STR(NAME, name);
		READMYSQL_STR(ORGAN_ID, organid);
		READMYSQL_STR(organname, organname);
		READMYSQL_STR(ROLE, auth);
		READMYSQL_STR(DUTY, duty);
		READMYSQL_STR(TELEPHONE, telno);
		READMYSQL_STR(MOBILE, mobile);
		READMYSQL_STR(password, pwd);

		if (cnt++)
			out << ",";
		out << "{usrid:\"" << usrid << "\",pwd:\"" << pwd << "\",name:\"" << name << "\",organid:\"" << organid 
			<< "\",organname:\"" << organname << "\",auth:\"" << auth << "\",duty:\"" << duty << "\",telno:\"" << telno << "\",mobile:\"" << mobile << "\"}";
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{flag:0,usrid:"",pwd:"",name:"",organid:"",duty:"",telno:"",mobile:""}
int BerthSvc::updUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::updUserInfo]bad format:", jsonString, 1);
	int flag = root.getv("flag", 0);
    string usrid = root.getv("usrid", "");
	string pwd = root.getv("pwd", "");
	string name = root.getv("name", "");
	string organid = root.getv("organid", "");
	string duty = root.getv("duty", "");
	string telno = root.getv("telno", "");
	string mobile = root.getv("mobile", "");
	string portid = root.getv("portid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char gmt0now[20];
	GmtNow(gmt0now);

	if (flag)
		sprintf (sql, "UPDATE t00_user SET REALNAME = '%s',  NICKNAME = '%s' WHERE USER_ID = '%s'", name.c_str(), name.c_str(), usrid.c_str());
	else
		sprintf (sql, "INSERT INTO t00_user (USER_ID, `PASSWORD`, REALNAME, NICKNAME, MOBILE, TELNO, IS_FLAG, MOBILE_VERIFIED, ACTIVATE_DT, CREATIONDATE, USERTYPE)\
				  VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s', '21')", usrid.c_str(), pwd.c_str(), name.c_str(), name.c_str(), mobile.c_str(), telno.c_str(), gmt0now, gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	if (flag)
		sprintf (sql, "UPDATE t41_pmonitor_user SET `name` = '%s', ORGAN_ID = '%s', DUTY = '%s', TELEPHONE = '%s', MOBILE = '%s' WHERE user_id = '%s'",
				name.c_str(), organid.c_str(), duty.c_str(), telno.c_str(), mobile.c_str(), usrid.c_str());
	else
		sprintf (sql, "INSERT INTO t41_pmonitor_user (USER_ID, `NAME`, ORGAN_ID, DUTY, TELEPHONE, MOBILE, IS_VALID, UPDATE_DT,PORTID,RANK) VALUES\
					  ('%s', '%s', '%s', '%s', '%s', '%s', '1', '%s','%s',100)", usrid.c_str(), name.c_str(), organid.c_str(), duty.c_str(), telno.c_str(), mobile.c_str(), gmt0now,portid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{userid:""}
int BerthSvc::delUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::delUserInfo]bad format:", jsonString, 1);
    string userid = root.getv("userid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "DELETE FROM t41_pmonitor_user WHERE user_id = '%s'", userid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf (sql, "DELETE FROM t00_user WHERE user_id = '%s'", userid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{userid:"",newpwd:""}
int BerthSvc::updUserPassword(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::updUserPassword]bad format:", jsonString, 1);
    string userid = root.getv("userid", "");
	string newpwd = root.getv("newpwd", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "UPDATE t00_user SET `password` = '%s' WHERE user_id = '%s'", newpwd.c_str(), userid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{userid:""}
int BerthSvc::checkUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::checkUser]bad format:", jsonString, 1);
    string userid = root.getv("userid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT COUNT(1) AS num FROM t00_user b WHERE b.user_id = '%s'", userid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int num = 0;
	if (psql->NextRow())
	{
		READMYSQL_INT(num, num, 0);
	}
	if (num)
		num = 1;

	out << "{userid:\"" << userid << "\",exist:" << num << "}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{}
int BerthSvc::getSystemAuthrity(const char* pUid, const char* jsonString, std::stringstream& out)
{
	MySql* psql = CREATE_MYSQL;
	
	CHECK_MYSQL_STATUS(psql->Query("SELECT AUTH_ID, `NAME`, REMARK FROM t41_pmonitor_auth WHERE IS_VALID = '1'"), 3);

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		int authid;
		char authname[32];
		char desc[128];
		READMYSQL_INT(AUTH_ID, authid, 0);
		READMYSQL_STR(NAME, authname);
		READMYSQL_STR(REMARK, desc);

		if (cnt++)
			out << ",";
		out << "{authid:\"" << authid << "\",authname:\"" << authname << "\",des:\"" << desc << "\"}";
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{userid:"",auth:""}
int BerthSvc::setUserAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::setUserAuth]bad format:", jsonString, 1);

	string userid = root.getv("userid", "");
	string auth = root.getv("auth", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_pmonitor_user SET ROLE = '%s' WHERE USER_ID = '%s'", auth.c_str(), userid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{portid:""}
int BerthSvc::getPortOrgan(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getPortOrgan]bad format:", jsonString, 1);

	string portid = root.getv("portid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT ORGAN_ID, `NAME`, LEVEL FROM t41_pmonitor_organ WHERE PORTID = '%s'", portid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		char organid[32];
		char organname[64];
		int level;
		READMYSQL_STR(ORGAN_ID, organid);
		READMYSQL_STR(NAME, organname);
		READMYSQL_INT(LEVEL, level, 0);
		if (cnt++)
			out << ",";
		out << "{organid:\"" << organid << "\",organname:\"" << organname << "\",level:\"" << level << "\"}";
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//
int BerthSvc::getConcernShips(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getConcernShips]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT a.SHIPID, a.MMSI, a.SHIPNAME,a.UPDATE_DT,b.SHIPNAME_CN, b.DWT, b.REG_PORT, b.SHIPTYPE \
			FROM t41_pmonitor_ship_attention a LEFT JOIN t41_pmonitor_ship b ON a.SHIPID = b.SHIPID WHERE a.user_id = '%s'", pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		char shipid[32];
		char shipnm_e[64];
		char shipnm_c[64];
		char tp[32];
		char portid[32];
		char mmsi[16];
		char dwt[20];
		char updt[24];
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(SHIPNAME, shipnm_e);
		READMYSQL_STR(SHIPNAME_CN, shipnm_c);
		READMYSQL_STR(SHIPTYPE, tp);
		READMYSQL_STR(DWT, dwt);
		READMYSQL_STR(MMSI, mmsi);
		READMYSQL_STR(REG_PORT, portid);
		READMYSQL_STR(UPDATE_DT, updt);
		
		if (cnt++)
			out << ",";
		out << "{shipid:\"" << shipid << "\",mmsi:\"" << mmsi << "\",nm_en:\"" << shipnm_e << "\",nm_cn:\"" << shipnm_c 
			<< "\",type:\"" << tp << "\",updt:\"" << updt << "\",rport:\"" << portid << "\",dwt:\"" << dwt << "\"}";
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"",mmsi:"",shipnm:""}
int BerthSvc::addConcernShips(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::addConcernShips]bad format:", jsonString, 1);

	string shipid = root.getv("shipid", "");
	string mmsi = root.getv("mmsi", "");
	string shipnm = root.getv("shipnm", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "INSERT INTO t41_pmonitor_ship_attention (USER_ID, SHIPID, MMSI, SHIPNAME, MONITOR_FLAG, IS_VALID, UPDATE_DT) VALUES\
				  ('%s', '%s', '%s', '%s', '0', '1', '%s')", pUid, shipid.c_str(), mmsi.c_str(), shipnm.c_str(), gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"",mmsi:""}
int BerthSvc::delConcernShips(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::delConcernShips]bad format:", jsonString, 1);

	string shipid = root.getv("shipid", "");
	string mmsi = root.getv("mmsi", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "DELETE FROM t41_pmonitor_ship_attention WHERE user_id = '%s' AND shipid = '%s'", pUid, shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}



int BerthSvc::getCompanyList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getCompanyList]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;


	string portid = root.getv("portid", "");
	string cmpyname = root.getv("cmpyname", "");



	char sql[1024 * 2] = "";


	int len = 0;
	len=sprintf (sql, "select t1.company_id,t1.name,t1.address,t1.legal_person,t1.service_type,t1.contact_name from t41_berth_company t1 \
	                   where t1.port_id='%s' AND t1.is_valid='1'",portid.c_str());
	if (!cmpyname.empty())
		len += sprintf (sql + len, " AND t1.name like '%%%s%%'", cmpyname.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char cmpid[128]=""; 
	char cmpname[256] = "";			
	char address[128] = "";	    //地址
	char legalperson[128]="";   //法人
	char svrtype[1024] = "";	//经营类型
	char contactname[64] = "";	//联系人名称

	out << "[";

	int index=0;
	while (psql->NextRow())
	{
		if (index>0)
		{
			out <<",";
		}

		READMYSQL_STR(company_id, cmpid);
		READMYSQL_STR(name, cmpname);
		READMYSQL_STR(address, address);
		READMYSQL_STR(legal_person, legalperson);
		READMYSQL_STR(service_type, svrtype);
		READMYSQL_STR(contact_name, contactname);
		
		out << "{cmpid:\"" << cmpid << "\",cmpname:\"" << cmpname <<"\",waddr:\"" << address<< "\",corp:\"" << legalperson << "\",mtype:\"" << svrtype << "\",name:\"" << contactname<<"\"}";
	    index++;
	}

	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}
int BerthSvc::delCompanyItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::delCompanyItem]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string companyid = root.getv("cmpid", "");
	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_berth_company SET is_valid='0',update_dt='%s' WHERE company_id = '%s'",gmt0now,companyid.c_str());
	
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BerthSvc::getAlertList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getAlertList]bad format:", jsonString, 1);

	string portid = root.getv("portid", "");
	Json* harbor = root.get("harbors");
	int starttm = root.getv("starttm", 0)-8*3600;
	int endtm = root.getv("endtm", 0)-8*3600;
	char harbors[1024] = "";
	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	int len = 0;
	if (harbor)
	{
		for (int i = 0; i < harbor->size(); i++)
		{
			if (len)
				len += sprintf (harbors + len, ",");
			string _tmp = harbor->getv(i, "");
			if (_tmp.length())
				len += sprintf (harbors + len, "'%s'", _tmp.c_str());
		}
	}

	len = sprintf (sql, "SELECT t2.warn_id,t2.status,t2.level,t2.desc,t2.mmsi,t2.ship_id, t2.shipname,t2.berth_id,UNIX_TIMESTAMP(t2.update_dt) as alert_time,t2.leave_time, t2.berth_time, t1.Name AS berthnm \
						FROM t41_port_berth t1,t41_berth_warning t2  WHERE t1.Portid = '%s' and t1.Berth_id = t2.Berth_id ", portid.c_str());
	if (strlen(harbors))
		len += sprintf (sql + len, " AND t1.HARBOUR_ID IN (%s)", harbors);
	sprintf (sql + len, " AND UNIX_TIMESTAMP(t2.UPDATE_DT)>='%d' AND UNIX_TIMESTAMP(t2.UPDATE_DT)<='%d'", starttm, endtm);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char alertid[32];		//预警ID
	int status;
	int level;				//预警级别
	char shipid[64];		//船舶id
	char shipnm[64];		//船舶名称,
	int mmsi;
	char berthid[64];		//停靠泊位ID
	char berthnm[64];		//泊位名称
    int alerttm;
	int bertm;				//靠泊时间
	int leavetm;			//离泊时间
	char desc[512];

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(warn_id, alertid);
		READMYSQL_STR(berth_id, berthid);
		READMYSQL_STR(berthnm, berthnm);
		READMYSQL_INT(level, level, 0);
		READMYSQL_INT(mmsi, mmsi, 0);
		READMYSQL_INT(leave_time, leavetm, 0);
		READMYSQL_INT(berth_time, bertm, 0);
		READMYSQL_INT(alert_time, alerttm, 0);
		alerttm+=8*3600;//中国本地时间
		READMYSQL_INT(status, status, -1);
		READMYSQL_STR(ship_id, shipid);
		READMYSQL_STR(shipname, shipnm);
        READMYSQL_STR(desc, desc);
		
		if (cnt++)
			out << ",";
		out << "{alertid:\"" << alertid << "\",level:\"" << level<< "\",desc:\"" << desc << "\",shipid:\"" << shipid << "\",shipnm:\"" << shipnm << "\",mmsi:\"" << mmsi 
			<< "\",berthid:\"" << berthid << "\",berthnm:\"" << berthnm << "\",status:" << status << ",alerttm:" << alerttm << ",bertm:" << bertm 
			<< ",leavetm:" << leavetm << "}";
	}
	out << "]";

   	RELEASE_MYSQL_RETURN(psql, 0);
}

//{portid:""}
int BerthSvc::getAlertUsers(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getAlertUsers]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string portid = root.getv("portid", "");

	char sql[1024] = "";
	sprintf (sql, "SELECT OBJ_ID, OBJ_NAME, MOBILE, USERNAME, OBJ_TYPE FROM t41_berth_warning_sms_rel WHERE PORTID = '%s' ORDER BY OBJ_ID", portid.c_str());
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	map<string,_AlertUserInfo> mapAlertSms;

	char objid[32]="";
	while (psql->NextRow())
	{
		_UsrTelInfo usrTelInfo;
        
		READMYSQL_STR(MOBILE, usrTelInfo.mobile);
		READMYSQL_STR(USERNAME, usrTelInfo.usrname);
		READMYSQL_STR(OBJ_ID, objid);

		string objIdStr=string(objid);

		if (mapAlertSms.find(objIdStr)==mapAlertSms.end())
		{
			_AlertUserInfo AlertObj;
			READMYSQL_STR(OBJ_ID, AlertObj.objid);
			READMYSQL_STR(OBJ_NAME, AlertObj.objname);
			READMYSQL_INT(OBJ_TYPE, AlertObj.objtype, 2);
			AlertObj.vecTelUsrs.push_back(usrTelInfo);
            mapAlertSms[objIdStr]=AlertObj;
		}
		else
		{
             mapAlertSms[objIdStr].vecTelUsrs.push_back(usrTelInfo);
		}

	  }
      
	

	  out << "[";

	
	  int i=0,j=0;

	  map<string,_AlertUserInfo>::iterator it=mapAlertSms.begin();
	  for (;it!=mapAlertSms.end();it++)
	  {

		  if (i>0)
		  {
			   out <<",";
		  }

		  _AlertUserInfo AlertObj=it->second;

		  out << "{objid:\"" << AlertObj.objid << "\",objname:\"" << AlertObj.objname << "\",objtype:\"" << AlertObj.objtype << "\", usrs:[";


		  for(j=0;j<(int)AlertObj.vecTelUsrs.size();j++)
		  {
			  if (j>0)
			  {
				  out <<",";
			  }

             _UsrTelInfo usrTelObj=AlertObj.vecTelUsrs[j];

			 out << "{telno:\"" << usrTelObj.mobile << "\",usrname:\"" << usrTelObj.usrname << "\"}";
              
		  }


          out << "]}";

		  i++;
	  }
	
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{objid:"",objname:"",objtype:"",portid:"",usrs:[{telno:"",usrname:""},{telno:"",usrname:""}]}
int BerthSvc::setAlertUsers(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::setAlertUsers]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string objid = root.getv("objid", "");
	string objname = root.getv("objname", "");
	string portid = root.getv("portid", "");
	string objtype = root.getv("objtype", "");

	Json* usrs = root.get("usrs");
	
	char sql[1024 * 10] = "";
	
	sprintf (sql, "DELETE FROM t41_berth_warning_sms_rel WHERE OBJ_ID = '%s'", objid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	if (usrs&&usrs->size()>0)
	{
		char gmt0now[20];
		GmtNow(gmt0now);
		int len = sprintf (sql, "REPLACE INTO t41_berth_warning_sms_rel (OBJ_ID, MOBILE, OBJ_NAME, USERNAME, PORTID, IS_VALID, UPDATE_DT, OBJ_TYPE) VALUES ");
		for (int i = 0; i < usrs->size(); i++)
		{
			Json* user = usrs->get(i);
			string telno = user->getv("telno","");
			string usrname = user->getv("usrname", "");

			len += sprintf (sql + len, i?",('%s', '%s', '%s', '%s', '%s', '1', '%s', '%s')":"('%s', '%s', '%s', '%s', '%s', '1', '%s', '%s')",
				objid.c_str(), telno.c_str(), objname.c_str(), usrname.c_str(), portid.c_str(), gmt0now, objtype.c_str());
		}

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	}

	out << "{\"eid\":\"0\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"s001",
//shipname:"",shipport:"",shipicode:"",shiprmark:"",totalt:"",nett:"",shiptype:"",finishdate:"",
//factorynm:"", operations:"", owner:"",
//totallen:"",shiplen:"",shipwide:"",mdepth:"", ldraught:"",rcargocap:"", sailareas:"", material:"",
//memodel:"",metype:"", ratepower:"", 
//loadline:"", mvaliddt:"",nativeport:}
int BerthSvc::getShipDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getShipDetail]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string shipid = root.getv("shipid", "");

	char sql[1024] = "";

	sprintf (sql, "SELECT SHIPNAME_CN, REG_PORT, SHIP_CODE, SHIP_REGCODE, GROSS_TON,\
					NET_TON, SHIPTYPE, FINISH_DT, BUILDER, OPERATOR, `OWNER`, To_TALLEN,\
					LOA, BEAM, DEPTH, DRAFT, DWT_REF, SAILAREA, MATERIAL, ENG_MODEL, \
					ENG_TYPE, ENG_POWER, DWT_LINE, CARD_VALID_DT, is_local\
					FROM t41_pmonitor_ship WHERE SHIPID = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char shipname[100] = "";
	char shipport[100] = "";
	char shipicode[100] = "";
	char shiprmark[100] = "";
	char totalt[100] = "";
	char nett[100] = "";
	char shiptype[100] = "";
	char finishdate[100] = "";
	char factorynm[100] = "";
	char operations[100] = "";
	char owner[100] = "";
	char totallen[100] = "";
	char shiplen[100] = "";
	char shipwide[100] = "";
	char mdepth[100] = "";
	char ldraught[100] = "";
	char rcargocap[100] = "";
	char sailareas[64] = "";
	char material[100] = "";
	char memodel[100] = "";
	char metype[100] = "";
	char ratepower[100] = "";
	char loadline[100] = "";
	char mvaliddt[100] = "";
	int nativeport = 0;
	if (psql->NextRow())
	{
		READMYSQL_STR(SHIPNAME_CN, shipname);
		READMYSQL_STR(REG_PORT, shipport);
		READMYSQL_STR(SHIP_CODE, shipicode);
		READMYSQL_STR(SHIP_REGCODE, shiprmark);
		READMYSQL_STR(GROSS_TON, totalt);
		READMYSQL_STR(NET_TON, nett);
		READMYSQL_STR(SHIPTYPE, shiptype);
		READMYSQL_STR(FINISH_DT, finishdate);
		READMYSQL_STR(BUILDER, factorynm);
		READMYSQL_STR(OPERATOR, operations);
		READMYSQL_STR(OWNER, owner);
		READMYSQL_STR(To_TALLEN, totallen);
		READMYSQL_STR(LOA, shiplen);
		READMYSQL_STR(BEAM, shipwide);
		READMYSQL_STR(DEPTH, mdepth);
		READMYSQL_STR(DRAFT, ldraught);
		READMYSQL_STR(DWT_REF, rcargocap);
		READMYSQL_STR(SAILAREA, sailareas);
		READMYSQL_STR(MATERIAL, material);
		READMYSQL_STR(ENG_MODEL, memodel);
		READMYSQL_STR(ENG_TYPE, metype);
		READMYSQL_STR(ENG_POWER, ratepower);
		READMYSQL_STR(DWT_LINE, loadline);
		READMYSQL_STR(CARD_VALID_DT, mvaliddt);
		READMYSQL_INT(is_local, nativeport, 0);
	}

	out << "{shipid:\"" << shipid << "\",shipname:\"" << shipname << "\",shipport:\"" << shipport << "\",shipicode:\"" << shipicode 
		<< "\",shiprmark:\"" << shiprmark << "\",totalt:\"" << totalt << "\",nett:\"" << nett << "\",shiptype:\"" << shiptype 
		<< "\",finishdate:\"" << finishdate << "\",factorynm:\"" << factorynm << "\", operations:\"" << operations << "\", owner:\"" 
		<< owner << "\",totallen:\"" << totallen << "\",shiplen:\"" << shiplen << "\",shipwide:\"" << shipwide << "\",mdepth:\"" 
		<< mdepth << "\", ldraught:\"" << ldraught << "\",rcargocap:\"" << rcargocap << "\", sailareas:\"" << sailareas << "\", material:\"" 
		<< material << "\",memodel:\"" << memodel << "\",metype:\"" << metype << "\", ratepower:\"" << ratepower << "\",loadline:\"" 
		<< loadline << "\", mvaliddt:\"" << mvaliddt << "\",nativeport:" << nativeport << "}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"s001",
//shipname:"",shipport:"",shipicode:"",shiprmark:"",totalt:"",nett:"",shiptype:"",finishdate:"",
//factorynm:"", operations:"", owner:"",
//totallen:"",shiplen:"",shipwide:"",mdepth:"", ldraught:"",rcargocap:"", sailareas:"", material:"",
//memodel:"",metype:"", ratepower:"", 
//loadline:"", mvaliddt:"",nationport:} 
int BerthSvc::udpShipDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::udpShipDetail]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;
	char sql[1024*10] = "";

	string shipid = root.getv("shipid", "");
	string IMO = root.getv("imo", "");
	string MMSI = root.getv("mmsi", "");
	string shipname_en = root.getv("shipname_en", "");
	string shipname = root.getv("shipname", "");
	string shipport = root.getv("shipport", "");
	string shipicode = root.getv("shipicode", "");
	string shiprmark = root.getv("shiprmark", "");
	string totalt = root.getv("totalt", "");
	string nett = root.getv("nett", "");
	string shiptype = root.getv("shiptype", "");
	string finishdate = root.getv("finishdate", "");
	string factorynm = root.getv("factorynm", "");
	string operations = root.getv("operations", "");
	string owner = root.getv("owner", "");
	string totallen = root.getv("totallen", "");
	string shiplen = root.getv("shiplen", "");
	string shipwide = root.getv("shipwide", "");
	string mdepth = root.getv("mdepth", "");
	string ldraught = root.getv("ldraught", "");
	string rcargocap = root.getv("rcargocap", "");
	string sailareas = root.getv("sailareas", "");
	string material = root.getv("material", "");
	string memodel = root.getv("memodel", "");
	string metype = root.getv("metype", "");
	string ratepower = root.getv("ratepower", "");
	string loadline = root.getv("loadline", "");
	string mvaliddt = root.getv("mvaliddt", "");
	int nationport = root.getv("nationport", 0);

	sprintf (sql, "REPLACE INTO t41_pmonitor_ship (SHIPID,IMO,MMSI,SHIPNAME, SHIPNAME_CN, REG_PORT, SHIP_CODE, SHIP_REGCODE, GROSS_TON, NET_TON, SHIPTYPE, FINISH_DT, BUILDER, \
					OPERATOR, `OWNER`, To_TALLEN, LOA, BEAM, DEPTH, DRAFT, DWT_REF, SAILAREA, MATERIAL, ENG_MODEL, ENG_TYPE, ENG_POWER, DWT_LINE, CARD_VALID_DT, IS_LOCAL) VALUES \
					('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%d')", shipid.c_str(), 
					IMO.c_str(),MMSI.c_str(),shipname_en.c_str(),shipname.c_str(), shipport.c_str(), shipicode.c_str(), shiprmark.c_str(), totalt.c_str(), nett.c_str(),
					shiptype.c_str(), finishdate.c_str(), factorynm.c_str(), operations.c_str(), owner.c_str(), totallen.c_str(), shiplen.c_str(),
					shipwide.c_str(), mdepth.c_str(), ldraught.c_str(), rcargocap.c_str(), sailareas.c_str(), material.c_str(), 
					memodel.c_str(), metype.c_str(), ratepower.c_str(), loadline.c_str(), mvaliddt.c_str(), nationport);

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{portid:""}
int BerthSvc::getRemoveShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	//JSON_PARSE_RETURN("[BerthSvc::getRemoveShip]bad format:", jsonString, 1);

	//string pid = root.getv("portid", "");
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT SHIPID, SHIPNAME, mmsi, OPERATOR, UNIX_TIMESTAMP(UPDATE_DT) AS upddt FROM boloomodb.t41_pmonitor_ship_remove");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";
	int cnt = 0;
	while (psql->NextRow())
	{
		char shipid[32];
		char shipnm[64];
		char opernm[64];
		int mmsi;		
		int upddt;
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(SHIPNAME, shipnm);
		READMYSQL_STR(OPERATOR, opernm);
		READMYSQL_INT(mmsi, mmsi, 0);
		READMYSQL_INT(upddt, upddt, 0);
		if (cnt++)
			out << ",";
		out << "{shipid:\"" << shipid << "\",shipnm:\"" << shipnm << "\",mmsi:\"" << mmsi << "\", operatornm:\"" << opernm << "\",updateTm:" << upddt << "}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:""}
int BerthSvc::delRemoveShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::delRemoveShip]bad format:", jsonString, 1);
	string shipid = root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "DELETE FROM boloomodb.t41_pmonitor_ship_remove WHERE SHIPID = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BerthSvc::getLocalShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::getLocalShip]bad format:", jsonString, 1);

	string pid = root.getv("portid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT SHIPID, SHIPNAME,SHIPNAME_CN, OPERATOR, MMSI FROM boloomodb.t41_pmonitor_ship WHERE IS_LOCAL = '1' ", pid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out.str("");
	out << "[";
	int cnt = 0;
	while (psql->NextRow())
	{
		char shipid[32];
		char shipnm[64];
		char shipnm_cn[64];
		char opernm[64];
		char mmsi[20];
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(SHIPNAME, shipnm);
		READMYSQL_STR(SHIPNAME_CN, shipnm_cn);
		READMYSQL_STR(OPERATOR, opernm);
		READMYSQL_STR(MMSI, mmsi);
		if (cnt++)
			out << ",";
		out << "{shipid:\"" << shipid << "\",shipnm:\"" << shipnm<< "\",shipnm_cn:\"" << shipnm_cn << "\",mmsi:\"" << mmsi << "\", operatornm:\"" << opernm << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BerthSvc::searchShipByName(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[BerthSvc::searchShipByName]bad format:", jsonString, 1);

	string shipnm = root.getv("shipnm", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT SHIPID, SHIPNAME,SHIPNAME_CN, OPERATOR,CARD_NO,MMSI,REG_PORTID,REG_PORT FROM boloomodb.t41_pmonitor_ship WHERE SHIPNAME_CN like '%%%s%%' or SHIPNAME like '%%%s%%'", shipnm.c_str(),shipnm.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out.str("");
	out << "[";
	int cnt = 0;
	while (psql->NextRow())
	{
		char shipid[32];
		char shipnm[64];
		char shipnm_cn[64];
		char opernm[256];
		char mmsi[20];
		char regportid[64];
		char regport[128];
		char card_no[64];

		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(SHIPNAME, shipnm);
		READMYSQL_STR(SHIPNAME_CN, shipnm_cn);
		READMYSQL_STR(OPERATOR, opernm);
		READMYSQL_STR(MMSI, mmsi);
		READMYSQL_STR(CARD_NO, card_no);
		READMYSQL_STR(REG_PORTID, regportid);
		READMYSQL_STR(REG_PORT, regport);

		if (cnt++)
			out << ",";
		out << "{shipid:\"" << shipid << "\",nm_en:\"" << shipnm<< "\",nm_cn:\"" << shipnm_cn << "\",mmsi:\"" << mmsi<< "\", regptid:\"" << regportid<< "\", regport:\"" << regport<< "\", opercard:\"" << card_no << "\", operator:\"" << opernm << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
      


}

//{alertid:""}
int BerthSvc::setAlertLevel(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::setAlertLevel]bad format:", jsonString, 1);

	string alertid = root.getv("alertid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE boloomodb.t41_berth_warning SET LEVEL = '1' WHERE warn_id = '%s'", alertid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BerthSvc::delThruptInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[BerthSvc::delThruptInfo]bad format:", jsonString, 1);
	string flowid = root.getv("flowid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "DELETE FROM boloomodb.t41_berth_flow_info WHERE FLOW_ID = '%s'", flowid.c_str());
	
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}


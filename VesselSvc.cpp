#include "blmcom_head.h"
#include "VesselSvc.h"
#include "CompanySvc.h"
#include "UserRegSvc.h"
#include "PortSvc.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "ObjectPool.h"
#include "kSQL.h"
#include "Pinyin4j.h"
#include "json.h"
#include <math.h>

using namespace std;

IMPLEMENT_SERVICE_MAP(VesselSvc)

VesselSvc::VesselSvc() : m_lastTelCodeTime(0)
{
}

VesselSvc::~VesselSvc()
{
}

bool VesselSvc::Start()
{
    int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday()+ACE_Time_Value(100), ACE_Time_Value(100));
    if(timerId <= 0)
        return false;

	aisShip.Refresh();

    if(!g_MessageService::instance()->RegisterCmd(MID_VESSEL, this))
        return false;

    if(!g_UserRegSvc::instance()->Start())
        return false;

    SERVICE_MAP(SID_USEREXISTS,VesselSvc,OnUserEmailExists);
    SERVICE_MAP(SID_REGISTER,VesselSvc,OnRegisterUser);
    SERVICE_MAP(SID_ACTIVATE,VesselSvc,OnActivateUser);
    SERVICE_MAP(SID_REQACTIVATION,VesselSvc,OnSendActivation);
    SERVICE_MAP(SID_REGISTER_INFO,VesselSvc,OnRegisterInfo);
	SERVICE_MAP(SID_MOBILE_BOUND,VesselSvc,OnMobileBound);
    SERVICE_MAP(SID_USERPWD,VesselSvc,OnUserEmailPwd);
	SERVICE_MAP(SID_RESET_PWD,VesselSvc,OnUserResetPwd);

    SERVICE_MAP(SID_VESSELSEARCH,VesselSvc,OnVesselDirMsg);
    SERVICE_MAP(SID_VESSELCONTACT,VesselSvc,OnReqVesselContact);
    SERVICE_MAP(SID_VESSELIDBYMMSI,VesselSvc,OnReqVesselIdByMMSI);
    SERVICE_MAP(SID_VESSEL_BASE,VesselSvc,OnReqVslBaseInfo);
    SERVICE_MAP(SID_VESSEL_HISTORY,VesselSvc,OnReqVslHisDetail);
    SERVICE_MAP(SID_VESSEL_LOGO,VesselSvc,OnReqVslLogos);
    SERVICE_MAP(SID_VESSEL_CONTACT,VesselSvc,OnReqVslCmpyContact);
    SERVICE_MAP(SID_VESSEL_COMM,VesselSvc,OnReqVslCommunication);
    SERVICE_MAP(SID_VESSEL_DIMENSON,VesselSvc,OnReqVslSpecDetail);
    SERVICE_MAP(SID_VESSEL_MACHINE,VesselSvc,OnReqVslMachineryDetail);
    SERVICE_MAP(SID_VESSEL_CAPACITY,VesselSvc,OnReqVslCapacityDetail);
    SERVICE_MAP(SID_VESSEL_CERT,VesselSvc,OnReqVslCertDetail);
    SERVICE_MAP(SID_VESSEL_CREW,VesselSvc,OnReqVslCrewDetail);
    SERVICE_MAP(SID_VESSEL_EVENTS,VesselSvc,OnReqVslEventList);
    SERVICE_MAP(SID_VESSEL_GZ_COUNT,VesselSvc,OnReqAttenVslUserCount);
    SERVICE_MAP(SID_VESSEL_GZ_LIST,VesselSvc,OnReqAttenVslUserList);
	SERVICE_MAP(SID_VESSELSEARCH2,VesselSvc,OnVesselDirMsg2);
	SERVICE_MAP(SID_CCOMPANY_INFO,VesselSvc,getCCompanyInfo);
	SERVICE_MAP(SID_SAME_COMP_SHIP,VesselSvc,getSameCompShip);
	SERVICE_MAP(SID_BASE_SHIP_INFO,VesselSvc,getBaseShipInfo);
	SERVICE_MAP(SID_GET_MMSI,VesselSvc,getMmsi);
	SERVICE_MAP(SID_VESSELSEARCH3,VesselSvc,OnVesselDirMsg3);
	SERVICE_MAP(SID_JUDGE_SHIPID,VesselSvc,judgeShipid);
	SERVICE_MAP(SID_IMO_MMSI,VesselSvc,imoGetMmsi);

	SERVICE_MAP(SID_VESSELSEARCH4,VesselSvc,OnVesselDirMsg4);
	SERVICE_MAP(SID_VESSEL_BH,VesselSvc,getVesselVeryDetails);
	
    DEBUG_LOG("[VesselSvc::Start] OK......................................");

	/*const char* jsonString = "{text:\"鍜岄『\",pagecur:1,pagemax:200,details:true,filter:false}";
	std::stringstream out;
	OnVesselDirMsg("caiwj", jsonString, out);*/
	//DEBUG_LOG("[VesselSvc::Test] %s", out.str().c_str());

    return true;
}

int VesselSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	time_t tNow = time(NULL);

	SYSTEM_LOG("[VesselSvc::handle_timeout] begin ========================= ");

	// 脟氓鲁媒鹿媒脝脷脢脰禄煤录陇禄卯脗毛(1800)
	if(tNow - m_lastTelCodeTime >= 1800)
		g_UserRegSvc::instance()->ClearExpired();
	else
		m_lastTelCodeTime = (long)tNow;


	// 脣垄脨脗AIS潞脥麓卢虏掳脢媒戮脻(100)
	aisShip.Refresh();

	SYSTEM_LOG("[VesselSvc::handle_timeout] end ========================= ");
    return 0;
}

int VesselSvc::OnUserEmailExists(const char* pUid, const char* jsonString, std::stringstream& out)
{
    return g_UserRegSvc::instance()->userEmailExists(jsonString, out);
}

int VesselSvc::OnRegisterUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
    return g_UserRegSvc::instance()->registerUser(jsonString, out);
}

int VesselSvc::OnActivateUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
    return g_UserRegSvc::instance()->activateUser(jsonString, out);
}

int VesselSvc::OnSendActivation(const char* pUid, const char* jsonString, std::stringstream& out)
{
    return g_UserRegSvc::instance()->sendActivation(jsonString, out);
}

int VesselSvc::OnRegisterInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    return g_UserRegSvc::instance()->registerInfo(jsonString, out);
}

int VesselSvc::OnMobileBound(const char* pUid, const char* jsonString, std::stringstream& out)
{
	return g_UserRegSvc::instance()->mobileBound(jsonString, out);
}


int VesselSvc::OnUserEmailPwd(const char* pUid, const char* jsonString, std::stringstream& out)
{
    return g_UserRegSvc::instance()->userEmailPwd(jsonString, out);
}

int VesselSvc::OnUserResetPwd(const char* pUid, const char* jsonString, std::stringstream& out)
{
	return g_UserRegSvc::instance()->userResetPwd(jsonString, out);
}


//麓卢虏掳禄霉卤戮脨脜脧垄
int  VesselSvc::OnReqVslBaseInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslBaseInfo]bad format:", jsonString, 1);
	string id = root.getv("id", "");

	char sql[1024];
	MySql *psql = CREATE_MYSQL;

	sprintf(sql, "SELECT T1.NAME,T1.SPEED,T1.IMO,T1.CALLSIGN,T1.MMSI,T1.COUNTRY_CODE,T1.PORTID,T2.NAME AS PORTNAME,"\
		"T1.SHIPID,T1.SHIPTYPE_KEY,T1.BUILT,T3.BEAMMOULDED AS MAXBEAM,T3.LOA,T3.HEIGHT,T3.DEPTH,T3.DRAFT,T4.DWT"\
		" FROM (T41_SHIP T1 LEFT OUTER JOIN T41_SHIP_DIMENSION T3 ON T1.SHIPID = T3.SHIPID) LEFT OUTER JOIN T41_SHIP_TONNAGE T4 ON T1.SHIPID = T4.SHIPID LEFT OUTER JOIN T41_PORT T2 ON T1.PORTID=T2.PORTID WHERE T1.SHIPID ='%s'", id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<<'{';
	if(psql->NextRow())
	{
		const char* name=psql->GetField("NAME");
		const char* imo=psql->GetField("IMO");
		const char* mmsi=psql->GetField("MMSI");
		const char* callsign=psql->GetField("CALLSIGN");
		const char* shiptypekey=NOTNULL(psql->GetField("SHIPTYPE_KEY"));
		const char* iso3=psql->GetField("COUNTRY_CODE");
		const char* bspeed=psql->GetField("SPEED");
		const char* portid=psql->GetField("PORTID");
		const char* portname=psql->GetField("PORTNAME");
		const char* loa=psql->GetField("LOA");
		const char* maxbeam=psql->GetField("MAXBEAM");
		const char* depth=psql->GetField("DEPTH");
		const char* draft=psql->GetField("DRAFT");
		const char* dwt=psql->GetField("DWT");
		const char* buidt=psql->GetField("BUILT");

		string mmsiStr = string(NOTNULL(mmsi));

		out<<"rs:1,"<<"id:\""<<id<<"\"";
		out<<",name:\""<<NOTNULL(name)<<"\"";
		out<<",imo:\""<<NOTNULL(imo)<<"\"";
		out<<",mmsi:\""<<NOTNULL(mmsi)<<"\"";
		out<<",callsign:\""<<NOTNULL(callsign)<<"\"";
		out<<",typekey:\""<<shiptypekey<<"\"";
		out<<",typename:\"\"";
		out<<",typekey1:\""<<shiptypekey[0]<<"\"";
		out<<",iso3:\""<<NOTNULL(iso3)<<"\"";

		if(iso3)
		{
			const char* country = NULL;
			_Country* pCountry = g_PortSvc::instance()->m_country.FindCountry(iso3);
			if(pCountry)
				country = pCountry->nameen;
			out<<",ctry:\""<<NOTNULL(country)<<"\"";
		}
		else
		{
			out<<",ctry:\"\"";
		}

		out<<",speed:\""<<NOTNULL(bspeed)<<"\"";
		out<<",regportid:\""<<NOTNULL(portid)<<"\"";
		out<<",regport:\""<<NOTNULL(portname)<<"\"";
		out<<",length:\""<<NOTNULL(loa)<<"\"";
		out<<",breadth:\""<<NOTNULL(maxbeam)<<"\"";
		out<<",depth:\""<<NOTNULL(depth)<<"\"";
		out<<",draft:\""<<NOTNULL(draft)<<"\"";
		out<<",dwt:\""<<NOTNULL(dwt)<<"\"";
		out<<",built:\""<<NOTNULL(buidt)<<"\"";
		out<<",his:[";

		string hisSql = "SELECT ID,NAME,CHG,CHGCODE FROM T41_SHIP_HISTORY WHERE SHIPID = '"+id+"'"+" ORDER BY CHG DESC";
		CHECK_MYSQL_STATUS(psql->Query(hisSql.c_str()), 3);
		
		int total = 0;
		while(psql->NextRow())
		{
			const char* changeCode = psql->GetField("CHGCODE");
			const char* id=psql->GetField("ID");
			const char* name = psql->GetField("NAME");
			const char* year=psql->GetField("CHG");

			int code10=strtol(changeCode,NULL,2);	//露镁陆酶脰脝脳陋禄炉
			if ((code10&0x39f)||code10==0)			//鹿媒脗脣碌么manager脫毛owner
			{
				if(total != 0)
					out<< ',';
				total++;

				out<<'['<<FormatString("\"%s\"",NOTNULL(id))<<','<<FormatString("\"%s\"",NOTNULL(name))<<','<<FormatString("\"%s\"",NOTNULL(year))<<']';
			}
		}
		out<<"],engine:[";
		sprintf(sql, "SELECT MODEL,BUILDER,NUMBER,CYLINDER,RPM,STROKE,BORE,POWERKW,POWERHP FROM T41_SHIP_ENGINE WHERE SHIPID='%s'",id.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		total = 0;
		while(psql->NextRow())
		{
			if(total != 0)
				out << ",";
			total++;

			const char* _model=psql->GetField("MODEL");
			const char* _builder=psql->GetField("BUILDER");
			const char* _number=psql->GetField("NUMBER");
			const char* _cylinder=psql->GetField("CYLINDER");
			const char* _rpm=psql->GetField("RPM");
			const char* _stroke=psql->GetField("STROKE");
			const char* _bore=psql->GetField("BORE");
			const char* _powerkw=psql->GetField("POWERKW");
			const char* _powerhp=psql->GetField("POWERHP");

			out<<'{';
			out<<"model:\""<<NOTNULL(_model)<<"\"";
			out<<",builder:\""<<NOTNULL(_builder)<<"\"";
			out<<",number:"<<(_number?_number:"0");
			out<<",cylinders:\""<<NOTNULL(_cylinder)<<"\"";
			out<<",RPM:\""<<NOTNULL(_rpm)<<"\"";
			out<<",stroke:\""<<NOTNULL(_stroke)<<"\"";
			out<<",bore:\""<<NOTNULL(_bore)<<"\"";
			out<<",pwKW:"<<(_powerkw?_powerkw:"0");
			out<<",pwHP:"<<(_powerhp?_powerhp:"0");
			out<<'}';
			
		}
		out<<"]}";

		char gmt0now[20];
		GmtNow(gmt0now);

		sprintf(sql, "select count(1) as count from t41_shipview_user where user_id = '%s' and shipid = '%s'", pUid, id.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

		if(atoi(psql->GetField("count"))>0)
		{
			sprintf(sql, "update t41_shipview_user set lastdt='%s' where user_id = '%s' and shipid = '%s'", gmt0now, pUid, id.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
		else
		{
			sprintf(sql, "insert into t41_shipview_user(user_id,shipid,mmsi,lastdt) values('%s','%s','%s','%s')", pUid, id.c_str(), mmsiStr.c_str(), gmt0now);
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}
	else
	{
		out<<"rs:0}";
	}

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVslHisDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslHisDetail]bad format:", jsonString, 1);
	string vid = root.getv("vid", "");
	string tid = root.getv("tid", "");

	char sql[1024];
    MySql *psql = CREATE_MYSQL;
	sprintf(sql, "SELECT T.NAME,T.FLAG,T.IMO,T.RPORT,T.MMSI,T.CALLSIGN,T.CHGCODE,T.DWT FROM T41_SHIP_HISTORY T"\
		" WHERE T.ID='%s' AND T.SHIPID='%s'",tid.c_str(),vid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<<'{';
	if (psql->NextRow())
	{
		const char* name=psql->GetField("NAME");
		const char* imo=psql->GetField("IMO");
		const char* mmsi=psql->GetField("MMSI");
		const char* callsign=psql->GetField("CALLSIGN");
		const char* dwt=psql->GetField("DWT");
		const char* flag=psql->GetField("FLAG");
		const char* code=psql->GetField("CHGCODE");
		const char* rport=psql->GetField("RPORT");

		out<<"name:\""<<NOTNULL(name)<<"\"";
		out<<",imo:\""<<NOTNULL(imo)<<"\"";
		out<<",mmsi:\""<<NOTNULL(mmsi)<<"\"";
		out<<",callsign:\""<<NOTNULL(callsign)<<"\"";
		out<<",dwt:\""<<NOTNULL(dwt)<<"\"";
		out<<",flag:\""<<NOTNULL(flag)<<"\"";
		out<<",code:\""<<NOTNULL(code)<<"\"";
		out<<",rport:\""<<NOTNULL(rport)<<"\"";
	}
	out<<"}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVslLogos(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslLogos]bad format:", jsonString, 1);
	string id = root.getv("id", "");
	Tokens excludeList;
	
	int size=root["filter"]->size();
	for (int i=0; i<size; i++)
	{
		excludeList.push_back( root["filter"]->getv(i, ""));
	}

    MySql *psql = CREATE_MYSQL;
	string sql = FormatString("SELECT PICKEY,PICTURE FROM T41_SHIP_PICTURES WHERE SHIPID='%s'",id.c_str());
	for (int j=0; j<(int)excludeList.size(); j++)
	{
		sql += FormatString(" AND PICKEY!='%s'",excludeList[j].c_str()) ;
	}
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);

	Tokens picDataList;
	int total = 0;
	out<<"{vid:\""<<id<<"\",pics:[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* pickey = psql->GetField("PICKEY");
		int piclen;
		char* picdata = (char*)psql->GetField("PICTURE",&piclen);

		out<<"{pid:\""<<pickey<<"\",len:"<<piclen<<"}";
		picDataList.push_back(string(picdata,piclen));
	}

	out<<"]}";
	for (int k=0; k<(int)picDataList.size(); k++)
	{
		out<<picDataList[k];
	}

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVslCommunication(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslCommunication]bad format:", jsonString, 1);
	string id = root.getv("id", "");

    char sql[1024];
    MySql *psql = CREATE_MYSQL;
	sprintf(sql, "SELECT T.TERMINAL_NO,T.SYSTEM_TYPE,T1.DESC_EN as SYS_NAME,T.USE_TYPE_CD,T2.DESC_EN AS USE_NAME FROM T41_SHIP_COMMUNICATION T,t91_code T1,t91_code T2 WHERE T.SHIPID='%s' AND T.SYSTEM_TYPE=T1.KEY1 AND T.USE_TYPE_CD=T2.KEY1 AND T1.CLASS_CODE = 7 AND T2.CLASS_CODE = 8",id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	vector<CommuInfo> _VoiceList;
	vector<CommuInfo> _FaxList;
	vector<CommuInfo> _TelexList;
	vector<CommuInfo> _DataList;
	while(psql->NextRow())
	{
		CommuInfo _ComInfo;
		READMYSQL_STR(TERMINAL_NO, _ComInfo.number)
		READMYSQL_STR(SYS_NAME, _ComInfo.sysType)
		READMYSQL_STR(USE_NAME, _ComInfo.useType)
		READMYSQL_STR(USE_TYPE_CD,_ComInfo.sysTypecd)
		if (_ComInfo.sysTypecd[0]=='1')
		{
			_VoiceList.push_back(_ComInfo);
		}
		else if (_ComInfo.sysTypecd[0]=='3')
		{
			_FaxList.push_back(_ComInfo);
		}
		else if (_ComInfo.sysTypecd[0]=='4')
		{
			_TelexList.push_back(_ComInfo);
		}
		else if (_ComInfo.sysTypecd[0]=='2')
		{
			_DataList.push_back(_ComInfo);
		}
	}
    
	int size;
	CommuInfo _CurrInfo;
	//脨麓Voice
	out<<"{voice:[";
	size=_VoiceList.size();
	for (int i=0; i<size; i++)
	{
		if(i != 0)
			out<<",";
		_CurrInfo=_VoiceList[i];
		out<<FormatString("[\"%s\",\"%s\",\"%s\"]",_CurrInfo.number,_CurrInfo.sysType,_CurrInfo.useType);
	}
	out<<"],";

	//脨麓Fax
	out<<"fax:[";
	size=_FaxList.size();
	for (int i=0; i<size; i++)
	{
		if(i != 0)
			out<<",";
		_CurrInfo=_FaxList[i];
		out<<FormatString("[\"%s\",\"%s\",\"%s\"]",_CurrInfo.number,_CurrInfo.sysType,_CurrInfo.useType);
	}
	out<<"],";

	//脨麓Telex
	out<<"telex:[";
	size=_TelexList.size();
	for (int i=0; i<size; i++)
	{
		if(i != 0)
			out<<",";
		_CurrInfo=_TelexList[i];
		out<<FormatString("[\"%s\",\"%s\",\"%s\"]",_CurrInfo.number,_CurrInfo.sysType,_CurrInfo.useType);
	}
	out<<"],";

	//脨麓Data
	out<<"data:[";
	size=_DataList.size();
	for (int i=0; i<size; i++)
	{
		if(i != 0)
			out<<",";
		_CurrInfo=_DataList[i];
		out<<FormatString("[\"%s\",\"%s\",\"%s\"]",_CurrInfo.number,_CurrInfo.sysType,_CurrInfo.useType);
	}
	out<<"]}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVslCmpyContact(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslCmpyContact]bad format:", jsonString, 1);
	string id = root.getv("id", "");

    MySql *psql = CREATE_MYSQL;
    
	//脠隆碌卤脟掳鹿芦脣戮ID
	Tokens CmpyList;
	string sql = "SELECT T.SHIPID,T.OWNER,T.MANAGER,T.BUILDER FROM T41_SHIP T WHERE T.SHIPID = '"+id+"'";
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);

	if (psql->NextRow())
	{
		const char* owner=psql->GetField("OWNER");
		const char* manager=psql->GetField("MANAGER");
		const char* builder=psql->GetField("BUILDER");
		CmpyList.push_back(NOTNULL(owner));
		CmpyList.push_back(NOTNULL(manager));
		CmpyList.push_back(NOTNULL(builder));
	}
	else
	{
		CmpyList.push_back("");
		CmpyList.push_back("");
		CmpyList.push_back("");
	} 

	std::map<string,CompanyInfo> CmpyMap;

	//脠隆碌卤脟掳鹿芦脣戮脙没鲁脝
	string tempIn;
	for (size_t i=0; i<(int)CmpyList.size(); i++)
	{
		if(!CmpyList[i].empty())
			tempIn+="'"+CmpyList[i]+"',";
	}
	if (tempIn!="")
	{
		tempIn=tempIn.substr(0,tempIn.length()-1);
		tempIn="("+tempIn+")";
		sql = "SELECT COMPANY_KEY,NAME,Address,CountryName FROM T41_COMPANY WHERE COMPANY_KEY IN "+tempIn;
		CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);

		while(psql->NextRow())
		{
			CompanyInfo cmpyData;
			READMYSQL_STR(COMPANY_KEY, cmpyData.cmpyid);
			READMYSQL_STR(NAME, cmpyData.cmpyname);
			READMYSQL_STR(Address, cmpyData.cmpyaddr);
			READMYSQL_STR(CountryName, cmpyData.cmpycry);
			CmpyMap[cmpyData.cmpyid]=cmpyData;
		}
	}
	else
	{
	}

	//脠隆麓卢虏掳鹿芦脣戮脌煤脢路
	sql = FormatString("select t1.owner,t2.name,t1.chg,t1.chgcode from t41_ship_history t1,t41_company t2"\
						" where t1.shipid='%s' AND SUBSTRING(t1.chgcode,4,1)='1'and t1.owner=t2.company_key", id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);

	vector<CompanyHis> _OwnerHisList;
	while(psql->NextRow())
	{
		CompanyHis _ComHisInfo;
		READMYSQL_STR(owner, _ComHisInfo.cmpyid);
		READMYSQL_STR(name, _ComHisInfo.cmpyname);
		READMYSQL_STR(chgcode, _ComHisInfo.code);
		READMYSQL_STR(chg, _ComHisInfo.year);
		_OwnerHisList.push_back(_ComHisInfo);
	}

	sql = FormatString("select t1.manager,t2.name,t1.chg,t1.chgcode from t41_ship_history t1,t41_company t2"\
						" where t1.shipid='%s' AND SUBSTRING(t1.chgcode,5,1)='1'and t1.manager=t2.company_key",id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);

	vector<CompanyHis> _ManagerHisList;
	while(psql->NextRow())
	{
		CompanyHis _ComHisInfo;
		READMYSQL_STR(manager, _ComHisInfo.cmpyid);
		READMYSQL_STR(name, _ComHisInfo.cmpyname);
		READMYSQL_STR(chgcode, _ComHisInfo.code);
		READMYSQL_STR(chg, _ComHisInfo.year);
		_ManagerHisList.push_back(_ComHisInfo);
	}

	//路芒脳掳脢媒戮脻
	out<<"[";
	for (size_t k=0; k<(int)CmpyList.size(); k++)
	{
		if(k != 0)
			out<< ',';

		string sType;
		if (k==0)
		{
			sType="OWNER";
		}
		else if (k==1)
		{
			sType="MANAGER";
		}
		else if (k==2)
		{
			sType="BUILDER";
		}
		out<<'{';
		string cmpyKey=CmpyList[k];
		if (cmpyKey!=""&&CmpyMap.find(cmpyKey)!=CmpyMap.end())
		{
			CompanyInfo& _CurrCmpyData=CmpyMap[cmpyKey];
			out<<FormatString("id:\"%s\",name:\"%s\",type:\"%s\",cry:\"%s\",addr:\"%s\"", _CurrCmpyData.cmpyid,_CurrCmpyData.cmpyname,sType.c_str(),_CurrCmpyData.cmpycry,_CurrCmpyData.cmpyaddr);
		}
		else
		{
			out<<FormatString("id:\"%s\",name:\"%s\",type:\"%s\",cry:\"%s\",addr:\"%s\"", "","","","","");
		}
		out<<",his:[";
		int index;
		if (k==0)    //OWNER脌煤脢路
		{
			index=0;
			for(int m=0; m<(int)_OwnerHisList.size(); m++)
			{
				CompanyHis& m_cmpHis=_OwnerHisList[m];
				if (index>0)
				{
					out<< ',';
				}
				out<<FormatString("[\"%s\",\"%s\",\"%s\"]",m_cmpHis.cmpyid,m_cmpHis.cmpyname,m_cmpHis.year);
				index++;
			}
		}
		else if (k==1) //MANAGER脌煤脢路
		{
			index=0;
			for(int n=0; n<(int)_ManagerHisList.size(); n++)
			{
				CompanyHis& m_cmpHis=_ManagerHisList[n];
				if (index>0)
				{
					out<< ',';
				}
				out<<FormatString("[\"%s\",\"%s\",\"%s\"]",m_cmpHis.cmpyid,m_cmpHis.cmpyname,m_cmpHis.year);
				index++;
			}
		}
		out<<"]}";
	}
	out<<"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVslSpecDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslSpecDetail]bad format:", jsonString, 1);
	string id = root.getv("id", "");

	char sql[1024];
    MySql *psql = CREATE_MYSQL;
	sprintf(sql, "SELECT LOA,LBP,BEAMMOULDED,DEPTH,DRAFT,FREEBORAD FROM T41_SHIP_DIMENSION WHERE SHIPID='%s'",id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<<'{';
	if(psql->NextRow())
	{
		const char* _loa=psql->GetField("LOA");
		const char* _lbp=psql->GetField("LBP");
		const char* _beam=psql->GetField("BEAMMOULDED");
		const char* _depth=psql->GetField("DEPTH");
		const char* _draft=psql->GetField("DRAFT");
		const char* _freeboard=psql->GetField("FREEBORAD");

		out<<FormatString("LOA:\"%s\",LBP:\"%s\",beam:\"%s\",depth:\"%s\",draft:\"%s\",freeboard:\"%s\",",
			NOTNULL(_loa),NOTNULL(_lbp),NOTNULL(_beam),NOTNULL(_depth),NOTNULL(_draft),NOTNULL(_freeboard));
	}
	else
	{
		out<<"LOA:\"\",LBP:\"\",beam:\"\",depth:\"\",draft:\"\",freeboard:\"\",";
	}
	
	sprintf(sql, "SELECT MATERIALTYPE,HULLNO,BULKHEADS,HULLDES,DECKS,DBLHULL,DBLBOTTOM,DBLSIDESKINS,BULBOWSBOW FROM T41_SHIP_HULL WHERE SHIPID='%s'",id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
	{
		const char* _material=psql->GetField("MATERIALTYPE");
		const char* _hullno=psql->GetField("HULLNO");
		const char* _bulkhead=psql->GetField("BULKHEADS");
		const char* _hulldes=psql->GetField("HULLDES");
		const char* _decks=psql->GetField("DECKS");
		const char* _DBLH=psql->GetField("DBLHULL");
		const char* _DBLB=psql->GetField("DBLBOTTOM");
		const char* _DBLS=psql->GetField("DBLSIDESKINS");
		const char* _bulbowsbow=psql->GetField("BULBOWSBOW");

		out<<FormatString("marterial:\"%s\",hullno:\"%s\",bulkhead:\"%s\",hulldes:\"%s\",decks:\"%s\",DBLH:\"%s\",DBLB:\"%s\",DBLS:\"%s\",bulbowsb:\"%s\",",
			NOTNULL(_material),NOTNULL(_hullno),NOTNULL(_bulkhead),NOTNULL(_hulldes),NOTNULL(_decks),
			(_DBLH?_DBLH:"0"),(_DBLB?_DBLB:"0"),(_DBLS?_DBLS:"0"),(_bulbowsbow?_bulbowsbow:"0"));
	}
	else
	{
		out<<"marterial:\"\",hullno:\"\",bulkhead:\"\",hulldes:\"\",decks:\"\",DBLH:\"0\",DBLB:\"0\",DBLS:\"0\",bulbowsb:\"0\",";
	}

	sprintf(sql, "SELECT GROSSTON,NETTON,DWT FROM T41_SHIP_TONNAGE WHERE SHIPID='%s'",id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{
		const char* _grosston=psql->GetField("GROSSTON");
		const char* _netton=psql->GetField("NETTON");
		const char* _dwt=psql->GetField("DWT");
		out<<FormatString("gton:\"%s\",nton:\"%s\",dwt:\"%s\"", NOTNULL(_grosston),NOTNULL(_netton),NOTNULL(_dwt));
	}
	else
	{
		out<<"gton:\"\",nton:\"\",dwt:\"\"";
	}
	out<<"}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVslMachineryDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslMachineryDetail]bad format:", jsonString, 1);
	string id = root.getv("id", "");

	char sql[1024];
	MySql *psql = CREATE_MYSQL;

	sprintf(sql, "SELECT BOILER,GENERATOR,RADIO,NAVI,RESCUE,MOORING,PROPELLER,REFRIGERATING,OTHER FROM T41_SHIP_MACHINERY WHERE SHIPID='%s'",id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<<'{';
	if(psql->NextRow())
	{
		const char* _boiler=psql->GetField("BOILER");
		const char* _generator=psql->GetField("GENERATOR");
		const char* _radio=psql->GetField("RADIO");
		const char* _navi=psql->GetField("NAVI");
		const char* _rescue=psql->GetField("RESCUE");
		const char* _mooring=psql->GetField("MOORING");
		const char* _propeller=psql->GetField("PROPELLER");
		const char* _refrigering=psql->GetField("REFRIGERATING");
		const char* _other=psql->GetField("OTHER");

		out << FormatString("boiler:\"%s\",generator:\"%s\",radio:\"%s\",navigation:\"%s\",rescue:\"%s\",mooring:\"%s\",propeller:\"%s\",refrigerating:\"%s\",other:\"%s\",",
			NOTNULL(_boiler),NOTNULL(_generator),NOTNULL(_radio),NOTNULL(_navi),NOTNULL(_rescue),NOTNULL(_mooring),NOTNULL(_propeller),NOTNULL(_refrigering),NOTNULL(_other));
	}
	else
	{
		out << "boiler:\"\",generator:\"\",radio:\"\",navigation:\"\",rescue:\"\",mooring:\"\",propeller:\"\",refrigerating:\"\",other:\"\",";
	}

	sprintf(sql, "SELECT NAME,BUILDER,NUMBER,CAPACITY FROM T41_SHIP_LIFT WHERE SHIPID='%s'",id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out<<"lift:[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* _name = psql->GetField("NAME");
		const char* _builder = psql->GetField("BUILDER");
		const char* _number = psql->GetField("NUMBER");
		const char* _capacity = psql->GetField("CAPACITY");

		out<<FormatString("{name:\"%s\",builder:\"%s\",number:\"%s\",capacity:\"%s\"}", NOTNULL(_name),NOTNULL(_builder),NOTNULL(_number),NOTNULL(_capacity));
	}
    out<<"]}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVslCapacityDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslCapacityDetail]bad format:", jsonString, 1);

    MySql *psql = CREATE_MYSQL;

	const char* id = root.getv("id", "");
	out<<'{';

	string Drybuf=FormatString("SELECT BALE,GRAIN,TEU,REEFER_CAPACITY,RAMPSINTERNAL,HATCHES,HATCHESSIZEMAX,DRYDES FROM T41_SHIP_DRY WHERE SHIPID='%s'",id);
	CHECK_MYSQL_STATUS(psql->Query(Drybuf.c_str()), 3);
	out<<"dry:{";
	if(psql->NextRow())
	{
		const char* _bale=psql->GetField("BALE");
		const char* _grain=psql->GetField("GRAIN");
		const char* _teu=psql->GetField("TEU");
		const char* _reefer=psql->GetField("REEFER_CAPACITY");
		const char* _rampsinternal=psql->GetField("RAMPSINTERNAL");
		const char* _hatches=psql->GetField("HATCHES");
		const char* _hatchessizemax=psql->GetField("HATCHESSIZEMAX");
		const char* _des=psql->GetField("DRYDES");
		out<<FormatString("bale:\"\",grain:\"\",TEU:\"\",reefer:\"\",rampsinternal:\"\",hatches:\"\",hmaxsize:\"\",desc:\"\"",
			NOTNULL(_bale),NOTNULL(_grain),NOTNULL(_teu),NOTNULL(_reefer),NOTNULL(_rampsinternal),NOTNULL(_hatches),NOTNULL(_hatchessizemax),NOTNULL(_des));
	}
	else
	{
		out<<"bale:\"\",grain:\"\",TEU:\"\",reefer:\"\",rampsinternal:\"\",hatches:\"\",hmaxsize:\"\",desc:\"\"";
	}
	out<<"},";

	string Liquidbuf=FormatString("SELECT LIQUID,TANKS,PUMP,LIQUIDDES FROM T41_SHIP_LIQUID WHERE SHIPID='%s'",id);
	CHECK_MYSQL_STATUS(psql->Query(Liquidbuf.c_str()), 3);
	out<<"liquid:{";
	if(psql->NextRow())
	{
		const char* _liquid=psql->GetField("LIQUID");
		const char* _tanks=psql->GetField("TANKS");
		const char* _pump=psql->GetField("PUMP");
		const char* _liquiddes=psql->GetField("LIQUIDDES");
		out<<FormatString("liquid:\"%s\",tanks:\"%s\",pump:\"%s\",des:\"%s\"",
			NOTNULL(_liquid),NOTNULL(_tanks),NOTNULL(_pump),NOTNULL(_liquiddes));
	}
	else
	{
		out<<"liquid:\"\",tanks:\"\",pump:\"\",des:\"\"";
	}
	out<<"},";

	string Consumpbuf=FormatString("SELECT FOCAPACITY,DOCAPACITY,FWCAPACITY,IFOBALLAST FROM T41_SHIP_CONSUMPTION WHERE SHIPID='%s'",id);
	CHECK_MYSQL_STATUS(psql->Query(Consumpbuf.c_str()), 3);
	out<<"consump:{";
	if(psql->NextRow())
	{
		const char* _FO=psql->GetField("FOCAPACITY");
		const char* _DO=psql->GetField("DOCAPACITY");
		const char* _FW=psql->GetField("FWCAPACITY");
		const char* _ballast=psql->GetField("IFOBALLAST");
		out<<FormatString("FO:\"%s\",DO:\"%s\",FW:\"%s\",ballast:\"%s\"",
			NOTNULL(_FO),NOTNULL(_DO),NOTNULL(_FW),NOTNULL(_ballast));
	}
	else
	{
		out<<"FO:\"\",DO:\"\",FW:\"\",ballast:\"\"";
	}
	out<<"}}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVslCertDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslCertDetail]bad format:", jsonString, 1);
	const char* id = root.getv("id", "");

    MySql *psql = CREATE_MYSQL;
	string reqbuf=FormatString("SELECT CERTNAME,CERTORG,ISSUEDDATE,EXPIRYDATE FROM T41_SHIP_CERTINFO WHERE SHIPID='%s'",id);
	CHECK_MYSQL_STATUS(psql->Query(reqbuf.c_str()), 3);

	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out<<",";
		total++;

		const char* _certname = psql->GetField("CERTNAME");
		const char* _certorgan = psql->GetField("CERTORG");
		const char* _issuedate = psql->GetField("ISSUEDDATE");
		const char* _expiredate = psql->GetField("EXPIRYDATE");
		out << FormatString("{certification:\"%s\",certtify:\"%s\",IDate:\"%s\",EDate:\"%s\"}",
							NOTNULL(_certname),NOTNULL(_certorgan),NOTNULL(_issuedate),NOTNULL(_expiredate));
	}
	out<<"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVslCrewDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    return 1;
}

int  VesselSvc::OnReqVslEventList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVslEventList]bad format:", jsonString, 1);

	char sql[1024];
    MySql *psql = CREATE_MYSQL;
    
	const char* shipid = root.getv("shipid", "");
	long startDt = root.getv("startdt", 0);
	long endDt = root.getv("enddt", 0);
	if (startDt==0||endDt==0)
	{
		RELEASE_MYSQL_RETURN(psql, 1);
	}

	sprintf(sql, "select TranType,OldVal,NewVal,UpdDt from t41_ship_chg_his where ShipId='%s' and UpdDt>=%d and UpdDt<=%d", shipid, startDt, endDt);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total=0;
	out<<"{shipchg:[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;
		const char* TranType=psql->GetField("TranType");
		const char* OldVal=psql->GetField("OldVal");
		const char* NewVal=psql->GetField("NewVal");
		const char* UpdStr=psql->GetField("UpdDt");
		out<<FormatString("{oldval:\"%s\",newval:\"%s\",type:\"%s\",dt:\"%s\"}", NOTNULL(OldVal), NOTNULL(NewVal), NOTNULL(TranType), UpdStr);
	}
	out<<"],fltevt:[]}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqAttenVslUserCount(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqAttenVslUserCount]bad format:", jsonString, 1);
    MySql *psql = CREATE_MYSQL;

	const char* id = root.getv("id", "");
	const char* usrtype = root.getv("usrtype", "");
     
	int fltCount=0;
	int ViewCount=0;
	string reqbuf0;
	if (strcmp(usrtype,"21")==0||strcmp(usrtype,"1")==0||strcmp(usrtype,"3")==0||strcmp(usrtype,"")==0)
	{
		reqbuf0=FormatString("select count(t1.fleet_id) as UserCount from t41_fleet_ship t1,t00_user t2 where t1.shipid='%s' and t1.trash_flag='0' and t1.valid_flag='1' and t1.fleet_id=t2.user_id and t2.usertype!=50",id);
	}
	else if (strcmp(usrtype,"50")==0)
	{
		reqbuf0=FormatString("select count(t1.fleet_id) as UserCount from t41_fleet_ship t1,t00_user t2 where t1.shipid='%s' and t1.trash_flag='0' and t1.valid_flag='1' and t1.fleet_id=t2.user_id and (t2.usertype=50 or t2.usertype=51)",id);
	}
	else
	{
		reqbuf0=FormatString("select count(*) as UserCount from t41_fleet_ship where shipid='%s' and trash_flag='0' and valid_flag='1'",id);
	}
	CHECK_MYSQL_STATUS(psql->Query(reqbuf0.c_str())&&psql->NextRow(), 3);
	READMYSQL_INT(UserCount,fltCount,0);

	string reqbuf;
	if (strcmp(usrtype,"21")==0||strcmp(usrtype,"1")==0||strcmp(usrtype,"3")==0||strcmp(usrtype,"")==0)
	{
		reqbuf=FormatString("select count(t1.user_id) as shipViewCount from t41_shipview_user t1,t00_user t2 where t1.shipid='%s' and t1.user_id=t2.user_id and t2.usertype!=50",id);
	}
	else if (strcmp(usrtype,"50")==0)
	{
		reqbuf=FormatString("select count(t1.user_id) as shipViewCount from t41_shipview_user t1,t00_user t2 where t1.shipid='%s' and t1.user_id=t2.user_id and (t2.usertype=50 or t2.usertype=51)",id);
	}
	else
	{
		reqbuf=FormatString("select count(*) as shipViewCount from t41_shipview_user where shipid='%s'",id);
	}
	CHECK_MYSQL_STATUS(psql->Query(reqbuf.c_str())&&psql->NextRow(), 3);
	READMYSQL_INT(shipViewCount,ViewCount,0)

	//录脫脡脧脟脳脟茅掳忙碌脛脫脙禄搂(路脟鲁陇陆颅脫脙禄搂)
	int familyCount = 0;
	if(strcmp(usrtype,"50")!=0)
	{
		reqbuf=FormatString("select count(*) as num from t00_user_conship where shipid='%s' and is_flag = '1'", id);
		CHECK_MYSQL_STATUS(psql->Query(reqbuf.c_str())&&psql->NextRow(), 3);
		READMYSQL_INT(num, familyCount, 0);
	}

	int count=max(fltCount,ViewCount)+familyCount;
	out<<FormatString("{id:\"%s\",num:%d}",NOTNULL(id),count);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqAttenVslUserList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqAttenVslUserList]bad format:", jsonString, 1);
    MySql *psql = CREATE_MYSQL;

	const char* id = root.getv("id", "");
	const char* usrtype = root.getv("usrtype", "");

	string fleetBuf;
	if (strcmp(usrtype,"21")==0||strcmp(usrtype,"1")==0||strcmp(usrtype,"3")==0||strcmp(usrtype,"")==0||strcmp(usrtype, "10")==0)
	{
		fleetBuf=FormatString("select t1.fleet_id, t2.usertype, t2.nickname, t2.mobile, t2.country, t1.create_dt as dt from t41_fleet_ship t1,t00_user t2 where t1.shipid='%s' and t1.trash_flag='0' and t1.valid_flag='1' and t1.fleet_id=t2.user_id and t2.usertype!=50",id);
	}
	else if (strcmp(usrtype,"50")==0)
	{
		fleetBuf=FormatString("select t1.fleet_id, t2.usertype, t2.nickname, t2.mobile, t2.country, t1.create_dt as dt from t41_fleet_ship t1,t00_user t2 where t1.shipid='%s' and t1.trash_flag='0' and t1.valid_flag='1' and t1.fleet_id=t2.user_id and (t2.usertype=50 or t2.usertype=51)",id);
	}
	else
	{
		fleetBuf=FormatString("select t1.fleet_id, t2.usertype, t2.nickname, t2.mobile, t2.country, t1.create_dt as dt from t41_fleet_ship t1,t00_user t2 where t1.shipid='%s' and t1.trash_flag='0' and t1.valid_flag='1' and t1.fleet_id=t2.user_id", id);
	}
	CHECK_MYSQL_STATUS(psql->Query(fleetBuf.c_str()), 3);

	set<string> userSet;
	map<string,VslUserInfo> FltUsrMap;
	while(psql->NextRow())
	{
		VslUserInfo FltUsrInfo;
		READMYSQL_STR(fleet_id, FltUsrInfo.usrid)
		READMYSQL_STR(nickname, FltUsrInfo.usrname)
		READMYSQL_INT(usertype, FltUsrInfo.type, 21)
		READMYSQL_STR(dt, FltUsrInfo.updt)
		READMYSQL_STR(country, FltUsrInfo.country);
		READMYSQL_STR(mobile, FltUsrInfo.mobile);
		FltUsrMap[string(FltUsrInfo.usrid)]=FltUsrInfo;
	}
	string VslViewBuf;
	if (strcmp(usrtype,"21")==0||strcmp(usrtype,"1")==0||strcmp(usrtype,"3")==0||strcmp(usrtype,"")==0||strcmp(usrtype, "10")==0)
	{
		VslViewBuf=FormatString("select t1.user_id, t2.usertype, t2.nickname, t2.mobile, t2.country, t1.lastdt as lastdt from t41_shipview_user t1,t00_user t2 where t1.shipid='%s' and t1.user_id=t2.user_id and t2.usertype!=50",id);
	}
	else if (strcmp(usrtype,"50")==0)
	{
		VslViewBuf=FormatString("select t1.user_id, t2.usertype, t2.nickname, t2.mobile, t2.country, t1.lastdt as lastdt from t41_shipview_user t1,t00_user t2 where t1.shipid='%s' and t1.user_id=t2.user_id and (t2.usertype=50 or t2.usertype=51)",id);
	}
	else
	{
		VslViewBuf=FormatString("select t1.user_id, t2.usertype, t2.nickname, t2.mobile, t2.country, t1.lastdt as lastdt from t41_shipview_user t1,t00_user t2 where t1.shipid='%s' and t1.user_id=t2.user_id",id);
	}
	CHECK_MYSQL_STATUS(psql->Query(VslViewBuf.c_str()), 3);

	int total=0;
	out<<"[";
	while(psql->NextRow())
	{
		const char* UserId=psql->GetField("user_id");
		const char* UserType=psql->GetField("usertype");
		const char* NkName=psql->GetField("nickname");
		const char* LastDt=psql->GetField("lastdt");
		const char* Country=psql->GetField("country");
		const char* pMobile = psql->GetField("mobile");

		if(userSet.find(UserId) != userSet.end())
			continue;
		userSet.insert(UserId);
		
		int flag=1;
		string lfdt;
		string useridStr=string(UserId);
		if (FltUsrMap.find(useridStr)!=FltUsrMap.end())
		{
			flag=3;
			VslUserInfo FltUserItem=FltUsrMap[useridStr];
			lfdt = FltUserItem.updt;
		}
		
		if(total != 0)
			out<<",";
		total++;
		out<<FormatString("{uid:\"%s\",nick:\"%s\",iso3:\"%s\",telno:\"%s\",utype:\"%s\",lfdt:\"%s\",lvdt:\"%s\",flag:%d}",NOTNULL(UserId), NOTNULL(NkName), NOTNULL(Country), NOTNULL(pMobile), NOTNULL(UserType), NOTNULL(LastDt), lfdt.c_str(), flag);
	}

	//录脫脡脧脟脳脟茅掳忙脫脙禄搂拢篓路脟鲁陇陆颅脫脙禄搂拢漏
	if(strcmp(usrtype,"50")!=0)
	{
		string familyBuf = FormatString("SELECT t1.user_id, t2.usertype, t2.nickname, t2.mobile, t2.country, t1.last_upd_dt AS lastdt FROM t00_user_conship t1,t00_user t2\
			WHERE t1.USER_ID = t2.USER_ID AND t1.SHIPID = '%s' AND t1.is_flag = '1'",id);
		CHECK_MYSQL_STATUS(psql->Query(familyBuf.c_str()), 3);

		while(psql->NextRow())
		{
			const char* UserId=psql->GetField("user_id");
			const char* UserType=psql->GetField("usertype");
			const char* NkName=psql->GetField("nickname");
			const char* LastDt=psql->GetField("lastdt");
			const char* Country=psql->GetField("country");
			const char* pMobile = psql->GetField("mobile");

			if(userSet.find(UserId) != userSet.end())
				continue;
			userSet.insert(UserId);

			if(total != 0)
				out<<",";
			total++;
			out<<FormatString("{uid:\"%s\",nick:\"%s\",iso3:\"%s\",telno:\"%s\",utype:\"%s\",lfdt:\"%s\",lvdt:\"\",flag:0}",NOTNULL(UserId), NOTNULL(NkName), NOTNULL(Country), NOTNULL(pMobile), NOTNULL(UserType), NOTNULL(LastDt));
		}
	}

	map<string,VslUserInfo>::iterator it=FltUsrMap.begin();
	for (; it!=FltUsrMap.end(); it++)
	{
		VslUserInfo CurUserItem=it->second;

		if(userSet.find(CurUserItem.usrid) != userSet.end())
			continue;
		userSet.insert(CurUserItem.usrid);

		if (total != 0)
			out<<",";
		total++;
		out<<FormatString("{uid:\"%s\",nick:\"%s\",iso3:\"%s\",telno:\"%s\",utype:\"%d\",lfdt:\"%s\",lvdt:\"\",flag:2}",
							CurUserItem.usrid,CurUserItem.usrname,CurUserItem.country,CurUserItem.mobile,CurUserItem.type,CurUserItem.updt);
	}
	out<<"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

extern void delete_non_digitAndChar(char buff[], bool toLower, bool keepSpace);

//{text:"",pagecur:1,pagesize:10,type:0,state:0,bdetails:false,filter:false,country:"",built:,spd:,spdop:,dwt:,dwtop:}
int VesselSvc::OnVesselDirMsg(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnVesselDirMsg]bad format:", jsonString, 1);
	
	const char* text = root.getv("text", "");
	int pagecur = root.getv("pagecur", 0);
	int pagesize = root.getv("pagemax", 0);
    bool bdetails = root.getv("details", false);

	ShipCondition shipFilter;
	shipFilter.filter = root.getv("filter", false);

	if (shipFilter.filter )
	{
		shipFilter.type =  root.getv("type", 0);
		shipFilter.state = root.getv("state", 0);
		string country = root.getv("country", "");	
		strcpy(shipFilter.country,country.c_str());

		shipFilter.built_l = root.getv("built", 0);
		shipFilter.built_u = root.getv("built", 0);
		shipFilter.speed_l = root.getv("spd", 0.0);
		shipFilter.speed_u = root.getv("spd", 0.0);
		shipFilter.speedopt = root.getv("spdop", 0);
		shipFilter.dwt = root.getv("dwt", 0);
		shipFilter.dwtopt = root.getv("dwtop", 0);
	}

	if(strlen(text)< 3 || pagecur <= 0 || pagesize <= 0)
		return 1;

	int n = 0;
	char **pytxt = g_Pinyin4j::instance()->FindPinYin(CodeConverter::Utf8ToUnicode(text).c_str(), n);

	AISLIST aisList;
	for (int i=0;i<n;i++)
	{
		char *textval=pytxt[i];
		delete_non_digitAndChar(textval, true, false);
		if(strlen(textval)<3)
			continue;
		aisShip.AdvSearchWithCondition(aisList,textval,&shipFilter);
	}
	

	if (bdetails)
	{
		if(aisList.empty()) //如果实时表中没有，从t41_ship表中查
		{
             
			SHIPLIST shipList;
			for (int k=0;k<n;k++)
			{
				char *textval=pytxt[k];
				delete_non_digitAndChar(textval, true, false);
				if(strlen(textval)<3)
					continue;
				aisShip.AdvSearchWithCondition(shipList,textval,NULL);
			}

			if (shipList.empty())
			{
				out<<FormatString("{pagecur:%d,pagesize:%d,amount:%d,totals:0,tk:0,details:[]}", pagecur, pagesize, (int)shipList.size());
			}
			else
			{

				out<<FormatString("{pagecur:%d,pagesize:%d,amount:%u,totals:%u,tk:0,details:[", pagecur, pagesize,shipList.size(), shipList.size());

				for(int i=(pagecur-1)*pagesize, j=0; i<(int)shipList.size()&&j<pagesize; i++, j++)
				{
					ShipData *pShip = &shipList[i];
					
					if(j>0)
						out<< ',';
					out<<FormatString("{imo:\"%d\",id:\"%s\",callsign:\"%s\",country:\"%s\",type:\"%d\",name:\"%s\"",
						pShip->imo, pShip->shipid, pShip->callsign,pShip->country,99, pShip->name);
					out<<FormatString(",mmsi:\"%d\",new_type:%d,state:%d,lasttm:%u,dwt:%d,built:%d,spd:%.1f}",
						pShip->mmsi, 0, 2, 0,pShip->dwt,pShip->built,pShip->speed);
				}
                out<<"]}";

			}

			return 0;
		}
		out<<FormatString("{pagecur:%d,pagesize:%d,amount:%u,totals:%u,tk:0,details:[", pagecur, pagesize,aisList.size(), aisList.size());

		for(int i=(pagecur-1)*pagesize, j=0; i<(int)aisList.size()&&j<pagesize; i++, j++)
		{
			AisRealtime *pAis = &aisList[i];
			ShipData* pShip = aisShip.FindShip(pAis->mmsi);

			char mmsiStr[20]="";
            if (!pShip)
            {
				snprintf(mmsiStr,sizeof(mmsiStr),"%d",pAis->mmsi);
            }

			if(j>0)
				out<< ',';
			out<<FormatString("{imo:\"%d\",id:\"%s\",callsign:\"%s\",country:\"%s\",type:\"%d\",name:\"%s\"",
				pAis->imo, (pShip?pShip->shipid:mmsiStr), pAis->callsign, (pShip?pShip->country:""),pAis->ship_type, pAis->name);
			out<<FormatString(",mmsi:\"%d\",new_type:%d,state:%d,lasttm:%u,dwt:%d,built:%d,spd:%.1f}",
				pAis->mmsi, pAis->ship_type, pAis->getState(), pAis->time,(pShip?pShip->dwt:0),(pShip?pShip->built:0),(pShip?pShip->speed:0));
		}

		out<<"]}";
	}
	else
	{
          StrSet newtype;
		  StrSet countrys;
		  StrSet builts;
		  StrSet states;

          for (int i=0;i<aisList.size();i++)
          {
			  AisRealtime *pAis = &aisList[i];
			  ShipData* pShip = aisShip.FindShip(pAis->mmsi);

			  char shiptp[20];
			  snprintf(shiptp,sizeof(shiptp),"%d",pAis->ship_type);
              newtype.insert(string(shiptp));


			  char statstr[20];
			  snprintf(statstr,sizeof(statstr),"%d",pAis->getState());
			  states.insert(string(statstr));

			  if (pShip)
			  {
				  char built[20];
				  snprintf(built,sizeof(built),"%d",pShip->built);

				  builts.insert(string(built));
			      countrys.insert(pShip->country);
			  }
			  else
			  {
                  countrys.insert("");
			  }

          }

		  out<<FormatString("{newtype:[%s],built:[%s],state:[%s],ctry:[",ToStr(newtype, ",").c_str(),ToStr(builts, ",").c_str(),ToStr(states, ",").c_str());

		  StrSet::iterator itc=countrys.begin();
		  for (;itc!=countrys.end();itc++)
		  {
			  if (itc==countrys.begin())
			  {
				  out<<"\""<<*itc<<"\"";
			  }
			  else
			  {
                  out<<",\""<<*itc<<"\"";
			  }

		  }

		  out<<"]}";
	}

	FreeArray(pytxt, n);
	

	return 0;
}

#define PageSize 10
int  VesselSvc::OnVesselDirMsg2(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnVesselDirMsg2]bad format:", jsonString, 1);

	MySql *psql = CREATE_MYSQL;

    ShipData *p;
    SHIPLIST res;

    int mmsi= atoi(root.getv("mmsi", ""));
    const char *imo = root.getv("imo", "");
    const char *nameString = root.getv("name", "");
    const char *callsignString = root.getv("callsign", "");
    int dwt = atoi(root.getv("dwt", ""));
    short build = atoi(root.getv("build", ""));
    const char *country = root.getv("flag", "");
    int pageno = root.getv("pageno", 0);
    int pagesize = root.getv("pagesize", 0);
    const char *url = root.getv("url", "");
    int resPCnt = 0;

    //脤铆录脫脜脨露脧脢脟路帽脦陋脥酶脪鲁脳垄虏谩脫脙禄搂
    char sql[1024] = "";
    sprintf (sql, "SELECT COUNT(1) as Num FROM t00_website_user WHERE website = \"%s\"", url);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    int num = 0;
	if (psql ->NextRow())
	{
		READMYSQL_INT(Num, num, 0);
	}
    num = (num?1:num);

    if (!num)
    {
        out<<FormatString("{\"status\":%d,\"pn\":0,\"ss\":[]}", num);
        RELEASE_MYSQL_RETURN(psql, 0);
    }
    else
    {
        sprintf (sql, "UPDATE t00_website_user SET usenum=usenum+1 WHERE website = \"%s\"", url);
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }

    if (mmsi != 0)
    {
        if(p = aisShip.FindShip(mmsi))
		{
			res.push_back(*p);
			resPCnt = 1;
		}
    }
    if (!resPCnt && strlen(imo) > 0)
    {
        p = aisShip.FindShip_imo(atoi(imo));
        if (p && p ->mmsi > 99999999 && p ->mmsi < 1000000000)
        {
            res.push_back(*p);
            resPCnt = 1;
        }
    }
    if(!resPCnt)
    {
        if (strlen(nameString) > 0)
        {
			resPCnt = aisShip.Search_NameCallsign(res, nameString);
        }
        else if (strlen(callsignString) > 0)
        {
            resPCnt = aisShip.Search_NameCallsign(res, callsignString);
        }
       
        if (dwt > 0)
        {
            SHIPLIST::iterator lIter;
            for (lIter = res.begin(); lIter != res.end();)
            {
                if ((*lIter).dwt != dwt)
                {
                    lIter = res.erase(lIter);
                    resPCnt--;
                }
                else lIter++;
            }
        }
        if (build > 0)
        {
            SHIPLIST::iterator lIter;
            for (lIter = res.begin(); lIter != res.end();)
            {
                if ((*lIter).built != build)
                {
                    lIter = res.erase(lIter);
                    resPCnt--;
                }
                else lIter++;
            }
        }
        if (strlen(country) > 0)
        {
            SHIPLIST::iterator lIter;
            for (lIter = res.begin(); lIter != res.end();)
            {
                if (strcmp((*lIter).country,country))
                {
                    lIter = res.erase(lIter);
                    resPCnt--;
                }
                else lIter++;
            }
        }
    }
    if(res.empty())
    {
        out<<FormatString("{\"status\":%d,\"pn\":0,\"ss\":[]}", num);
        RELEASE_MYSQL_RETURN(psql, 0);
    }

    resPCnt = (int)ceil((double)res.size()/pagesize);
    if (pageno > resPCnt) pageno = resPCnt;
    out<<FormatString("{\"status\":%d,\"pn\":%d,\"ss\":[", num, resPCnt);
	SHIPLIST::iterator mIter = res.begin();
    int cntI = 0;
    int totalCnt = (pageno-1)*pagesize;
    while (cntI++ < totalCnt && mIter != res.end()) mIter++;
    for(cntI = 0; mIter != res.end() && cntI < pagesize; mIter++, cntI++)
    {
        ShipData* pShip = &(*mIter);
        AisRealtime* pReal = aisShip.FindAis(pShip->mmsi);
        struct tm* local;
		time_t Eta_Unixtime = (pReal ? Eta2Unixtime(pReal->eta) : 0);
        local = localtime(&Eta_Unixtime);
        char etaTounixtime[20] = "";
        sprintf(etaTounixtime, "%02d-%02d %02d:%02d", (local->tm_mon+1), local->tm_mday, local->tm_hour, local->tm_min);

        //脜脜鲁媒脢媒戮脻脰脨脳脰路没麓庐鲁枚脧脰隆卤碌脛脟茅驴枚
        SqlReplace(pShip->name);
        SqlReplace(pShip->imo);
        SqlReplace(pShip->callsign);
        SqlReplace(pShip->country);
        if(pReal) SqlReplace(pReal->dest);
        if(cntI)
			out << ",";

		char _length[20] = "";
		char _beam[20] = "";
		char _draft[20] = "";

		if (pShip->length > 0.0)
			sprintf (_length, "%.2f", pShip->length);
		else
			sprintf (_length, "%d", (pReal ? pReal->length : 0));
		if (pShip->beam > 0.0)
			sprintf (_beam, "%.2f", pShip->beam);
		else
			sprintf (_beam, "%d", (pReal ? pReal->width : 0));
		
		if (pReal && pReal->draught > 0)
			sprintf (_draft, "%d", (pReal ? pReal->draught : 0));
		else
			sprintf (_draft, "%.2f", pShip->draft*10);
		
		out<<FormatString("{\"sid\":\"%s\",\"name\":\"%s\",\"imo\":\"%s\",\"mmsi\":\"%d\",\"callsign\":\"%s\",\"dwt\":\"%d\",\"build\":\"%d\",\"flag\":\"%s\",\"lon\":\"%f\",\"lat\":\"%f\",\"st\":\"%d\",\"sp\":\"%d\",\"head\":\"%d\",\"track\":\"%d\",\"dest\":\"%s\",\"eta\":\"%s\",\"ld\":\"%s\",\"type\":\"%1d\",\"length\":\"%s\",\"beam\":\"%s\",\"draft\":\"%s\"}",
			pShip->shipid, pShip->name, pShip->imo, pShip->mmsi, pShip->callsign, pShip->dwt, pShip->built, pShip->country, (pReal?pReal->lon:0), (pReal?pReal->lat:0), (pReal?pReal->nav_status:0), (pReal?pReal->sog:0), (pReal?pReal->true_head:0), (pReal?pReal->cog:0), (pReal?pReal->dest:""), etaTounixtime, (pReal?TimeToTimestampStr(pReal->time).c_str():""), CAisShip::GetType(pShip, pReal), _length, _beam, _draft);
    }
    out<<"]}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int  VesselSvc::OnReqVesselContact(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVesselContact]bad format:", jsonString, 1);
    string id = root.getv("id", "");

	char buff[1024];
	char sql[1024];
    MySql *psql = CREATE_MYSQL;
    
	Tokens CmpyList;
	sprintf(sql, "SELECT T.SHIPID,T.OWNER,T.MANAGER,T.BUILDER FROM T41_SHIP T WHERE T.SHIPID = '%s'", id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{
		const char *owner=psql->GetField("OWNER");
		const char *manager=psql->GetField("MANAGER");
		const char *builder=psql->GetField("BUILDER");
		CmpyList.push_back(NOTNULL(owner));
		CmpyList.push_back(NOTNULL(manager));
		CmpyList.push_back(NOTNULL(builder));
	}
	else
	{
		CmpyList.push_back("");
		CmpyList.push_back("");
		CmpyList.push_back("");
	}
	
	string tempIn;
	for (size_t i=0; i<CmpyList.size(); i++)
	{
		tempIn+="'"+CmpyList[i]+"',";
	}
	if (tempIn!="")
	{
		tempIn=tempIn.substr(0,tempIn.length()-1);
	}
	tempIn="("+tempIn+")";

	std::map<string,CmpyData> CmpyMap;
	if(tempIn != "()")
	{
		string Csql="SELECT COMPANY_KEY,NAME,ADDRESS,TELNO,FAX,EMAIL,WEBSITE,COUNTRYNAME FROM T41_COMPANY WHERE COMPANY_KEY IN "+tempIn;
		CHECK_MYSQL_STATUS(psql->Query(Csql.c_str()), 3);

		while(psql->NextRow())
		{
			CmpyData cmpyData;
			cmpyData.cmypid = NOTNULL(psql->GetField("COMPANY_KEY"));
			cmpyData.name = NOTNULL(psql->GetField("NAME"));
			cmpyData.address = NOTNULL(psql->GetField("ADDRESS"));
			cmpyData.tel = NOTNULL(psql->GetField("TELNO"));
			cmpyData.fax = NOTNULL(psql->GetField("FAX"));
			cmpyData.email = NOTNULL(psql->GetField("EMAIL"));
			cmpyData.homepage = NOTNULL(psql->GetField("WEBSITE"));
			cmpyData.country = NOTNULL(psql->GetField("COUNTRYNAME"));
			CmpyMap[cmpyData.cmypid]=cmpyData;
		}
	}
	
	out<<"[";
	for (size_t k=0; k<(int)CmpyList.size(); k++)
	{
		string sType;
		if (k==0)
		{
			sType="OWNER";
		}
		else if (k==1)
		{
			sType="MANAGER";
		}
		else if (k==2)
		{
			sType="BUILDER";
		}
		out<<'{';
		string cmpyKey=CmpyList[k];
		if (CmpyMap.find(cmpyKey)!=CmpyMap.end())
		{
			CmpyData _CurrCmpyData=CmpyMap[cmpyKey];
			sprintf(buff, "id:\"%s\",name:\"%s\",type:\"%s\",tel:\"%s\",fax:\"%s\",email:\"%s\",address:\"%s\",country:\"%s\",homepage:\"%s\"",
				_CurrCmpyData.cmypid.c_str(),_CurrCmpyData.name.c_str(),sType.c_str(),_CurrCmpyData.tel.c_str(),_CurrCmpyData.fax.c_str(),_CurrCmpyData.email.c_str(),_CurrCmpyData.address.c_str(),_CurrCmpyData.country.c_str(),_CurrCmpyData.homepage.c_str());
			out << buff;
		}
		else
		{
			sprintf(buff, "id:\"%s\",name:\"%s\",type:\"%s\",tel:\"%s\",fax:\"%s\",email:\"%s\",address:\"%s\",country:\"%s\",homepage:\"%s\"",
				"","","","","","","","","");
			out << buff;
		}
		out<<'}';
		if(k < 2)
			out<<',';
	}
	out<<"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int VesselSvc::OnReqVesselIdByMMSI(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VesselSvc::OnReqVesselIdByMMSI]bad format:", jsonString, 1);
    const char *_mmsi= root.getv("mmsi", "");
	ShipData* pShip = aisShip.FindShip(atoi(_mmsi));
	if(!pShip)
		out << "{id:\"\",name:\"\"}";
	else
		out << FormatString("{id:\"%s\",name:\"%s\"}", pShip->shipid, pShip->name);

    return 0;
}

//zhuxj
//{sid:"",type:0/1/2}
int VesselSvc::getCCompanyInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[VesselSvc::getCCompanyInfo]bad format:", jsonString, 1);
	string sid = root.getv("sid", "");
	int type = root.getv("type", 0);

	if (type > 2 || type < 0)
		return 1;

	string t_s[] = {"OWNER", "MANAGER", "builder"};

	MySql *psql = CREATE_MYSQL;
	char sql[1024] = "";
	string name = "";
	string addr = "";
	string fax = "";
	string telno = "";
	string cn = "";
	string email = "";
	string servType = "";
	sprintf (sql, "SELECT t2.Name, t2.Address, t2.Fax, t2.TelNo, t2.CountryName, t2.Email FROM t41_company t2, t41_ship t1 WHERE t1.%s = t2.company_key AND t1.shipid = '%s'", t_s[type].c_str(), sid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{		
		name = NOTNULL(psql->GetField("Name"));
		addr = NOTNULL(psql->GetField("Address"));
		fax = NOTNULL(psql->GetField("Fax"));
		telno = NOTNULL(psql->GetField("TelNo"));
		cn = NOTNULL(psql->GetField("CountryName"));
		email = NOTNULL(psql->GetField("Email"));
	}
	out << "{name:\"" << name << "\",addr:\"" << addr << "\",fax:\"" << fax << "\",telno:\"" << telno << "\",cn:\"" << cn << "\",email:\"" << email << "\",servType:\"" << t_s[type] << "\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{sid:"",type:0/1/2}
int VesselSvc::getSameCompShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[VesselSvc::getSameCompShip]bad format:", jsonString, 1);
	string sid = root.getv("sid", "");
	int type = root.getv("type", 0);

	if (type > 2 || type < 0)
		return 1;

	MySql *psql = CREATE_MYSQL;
	char sql[1024] = "";
	string eq_s[3] = {"a.OWNER = b.OWNER", "a.MANAGER = b.MANAGER", "a.BUILDER = b.BUILDER"};

	sprintf (sql, "SELECT b.SHIPID, b.NAME, b.BUILT, b.CALLSIGN, b.mmsi, b.IMO, b.COUNTRY_CODE, b.PORTID, b.SPEED, c.dwt \
					FROM t41_ship a, t41_ship b JOIN t41_ship_tonnage c ON b.shipid = c.shipid WHERE a.shipid = '%s'\
					AND %s", sid.c_str(), eq_s[type].c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		string shipid = NOTNULL(psql->GetField("SHIPID"));
		string name = NOTNULL(psql->GetField("NAME"));
		string built = NOTNULL(psql->GetField("BUILT"));
		string callsign = NOTNULL(psql->GetField("CALLSIGN"));
		int mmsi = atoi(NOTNULL(psql->GetField("mmsi")));
		int imo = atoi(NOTNULL(psql->GetField("IMO")));
		string cny = NOTNULL(psql->GetField("COUNTRY_CODE"));
		string portid = NOTNULL(psql->GetField("PORTID"));
		double speed = atof(NOTNULL(psql->GetField("SPEED")));
		int dwt = atoi(NOTNULL(psql->GetField("dwt")));

		time_t t = time(0);
		struct tm *ptr = localtime(&t);
		int age = -1;
		if (built.length() >= 4)
			age = ptr->tm_year + 1900 - atoi(built.substr(0,4).c_str());

		if (cnt++)
			out << ",";
		out << "{shipid:\"" << shipid << "\",name:\"" << name << "\",age:" << age << ",callsign:\"" << callsign << "\",mmsi:" << mmsi << ",imo:" << imo << ",cny:\"" << cny << "\",portid:\"" << portid << "\",speed:" << speed << ",dwt:" << dwt << "}";
	}
	out << "]";
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{sid:"",mmsi:""}
int VesselSvc::getBaseShipInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[VesselSvc::getBaseShipInfo]bad format:", jsonString, 1);
	string sid = root.getv("sid", "");
	string mmsi = root.getv("mmsi", "");

	MySql *psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT t1.NAME, t1.BUILT, t1.CALLSIGN, t1.mmsi, t1.IMO, t1.PORTID, t1.SPEED, t1.COUNTRY_CODE, t2.DWT \
					FROM t41_ship t1 LEFT JOIN t41_ship_tonnage t2 ON t1.shipid = t2.shipid WHERE t1.shipid = '%s' or t1.mmsi = '%s'", sid.c_str(), mmsi.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{
		string name = NOTNULL(psql->GetField("NAME"));
		string built = NOTNULL(psql->GetField("BUILT"));
		string callsign = NOTNULL(psql->GetField("CALLSIGN"));
		int mmsi = atoi(NOTNULL(psql->GetField("mmsi")));
		int imo = atoi(NOTNULL(psql->GetField("IMO")));
		string portid = NOTNULL(psql->GetField("PORTID"));
		double speed = atof(NOTNULL(psql->GetField("SPEED")));
		string cny = NOTNULL(psql->GetField("COUNTRY_CODE"));
		int dwt = atoi(NOTNULL(psql->GetField("dwt")));

		time_t t = time(0);
		struct tm *ptr = localtime(&t);
		int age = -1;
		if (built.length() >= 4)
			age = ptr->tm_year + 1900 - atoi(built.substr(0,4).c_str());
		out << "{name:\"" << name << "\",age:" << age << ",callsign:\"" << callsign << "\",mmsi:" << mmsi << ",imo:" << imo << ",portid:\"" << portid << "\",speed:" << speed  << ",cny:\"" << cny << "\",dwt:" << dwt << "}";
	}
	else if (mmsi.length() == 9)
	{
		sprintf (sql, "SELECT mmsi, imo, callsign, NAME FROM aisdb.t41_ais_ship_realtime WHERE mmsi = '%s'", mmsi.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if (psql->NextRow())
		{
			string name = NOTNULL(psql->GetField("NAME"));
			string callsign = NOTNULL(psql->GetField("CALLSIGN"));
			int mmsi = atoi(NOTNULL(psql->GetField("mmsi")));
			int imo = atoi(NOTNULL(psql->GetField("IMO")));
			out << "{name:\"" << name << "\",age:-1,callsign:\"" << callsign << "\",mmsi:" << mmsi << ",imo:" << imo << ",portid:\"\",speed:0,cny:\"\",dwt:0}";
		}
	}
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{sid:""}
int VesselSvc::getMmsi(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[VesselSvc::getMmsi]bad format:", jsonString, 1);
	string sid = root.getv("sid", "");
	MySql *psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT mmsi	FROM t41_ship WHERE shipid = '%s'", sid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{
		string mmsi = NOTNULL(psql->GetField("mmsi"));
		out << "{mmsi:" << mmsi << "}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}


int VesselSvc::OnVesselDirMsg3_4(const char* pUid, const char* jsonString, std::stringstream& out,bool bonlycnss)
{
	JSON_PARSE_RETURN("[VesselSvc::OnVesselDirMsg3]bad format:", jsonString, 1);


	ShipCondition shipFilter;

	string text = root.getv("text", "");
	shipFilter.type = atoi(root.getv("type", ""));
	shipFilter.state = atoi(root.getv("state", ""));
	string country = root.getv("flag", "");	
	strcpy(shipFilter.country,country.c_str());
	shipFilter.built_l = root.getv("built_l", 0);
	shipFilter.built_u = root.getv("built_u", 0);
	shipFilter.speed_l = root.getv("spd_l", 0.0);
	shipFilter.speed_u = root.getv("spd_u", 0.0);
	int pn = root.getv("pn", 0);
	int psize = root.getv("psize", 0);
	int timezone = root.getv("timezone", 0);
	int lang = root.getv("lang", 0);

	if (text.empty())
		return 1;


	char _text[512];
	strcpy(_text, text.c_str());
	delete_non_digitAndChar(_text, true, false);

	SHIPLIST shipList;

	//陆芦脰脨脦脛脳陋鲁脡脝麓脪么
	int n = 0; //露脿脪么脳脰
	char **pytxt = g_Pinyin4j::instance()->FindPinYin(CodeConverter::Utf8ToUnicode(_text).c_str(), n);

	for (int i=0;i<n;i++)
	{
		char *textval=pytxt[i];
		aisShip.AdvSearchWithCondition(shipList,textval,&shipFilter,bonlycnss);
	}

	if(shipList.empty())
	{
		out << "{\"total\":0,\"pn\":0,\"types\":\"\",\"states\":\"\",\"cnys\":\"\",\"builts\":\"\",\"ss\":[]}";	//FormatString(
		return 0;
	}

	StrSet types;
	StrSet states;
	StrSet cnys;
	StrSet builts;

	ShipData shipData;
	for (int j=0;j<shipList.size();j++)
	{
		shipData=shipList[j];
		if (shipData.built>0)
		{
			char built[20];
			snprintf(built,sizeof(built),"%d",shipData.built);
			builts.insert(built);
		}
		cnys.insert(shipData.country);

		if (shipData.mmsi>0)
		{
			AisRealtime* pReal =aisShip.FindAis(shipData.mmsi);
			if (pReal)
			{
				char shiptype[6];
				snprintf(shiptype,sizeof(shiptype),"%d",pReal->ship_type);
				types.insert(shiptype);

				char state[6];
				snprintf(state,sizeof(state),"%d",pReal->getState());
				states.insert(state);
			}
			else
			{
				states.insert("3");
			}
		}
		else
		{
			states.insert("3");
		}

	}

	int t_pn = (int)ceil(shipList.size()*1.0/psize);

	out << FormatString("{\"total\":%d,\"pn\":%d,\"types\":\"%s\",\"states\":\"%s\",\"cnys\":\"%s\",\"builts\":\"%s\",\"ss\":[", shipList.size(), t_pn, ToStr(types, "|").c_str(), ToStr(states, "|").c_str(), ToStr(cnys, "|").c_str(), ToStr(builts, "|").c_str());


	string desc_en[9] = {
		"%1 %2 changed her status to under way at %5&%4,%7 nm %8 degrees of port %9",
		"%1 %2 anchored at %5&%4,%7 nm %8 degrees of port %9",
		"%1 %2 changed her status to not under command at %5&%4,%7 nm %8 degrees of port %9",
		"%1 %2 changed her status to Restricted Manoeuvrability at %5&%4,%7 nm %8 degrees of port %9",
		"%1 %2 changed her status to Constrained by her draught at %5&%4,%7 nm %8 degrees of port %9",
		"%1 %2 moored  at %5&%4,%7 nm %8 degrees of port %9",
		"%1 %2 Aground at %5&%4,%7 nm %8 degrees of port %9",
		"%1 %2 engaged in fishing at %5&%4,%7 nm %8 degrees of port %9",
		"%1 %2 changed her status to under way sailing at %5&%4,%7 nm %8 degrees of port %9"
	};
	string desc_cn[9] = {
		"%1 %2在%9港%8度方向%7海里处(%5&%4)改变为风帆动力航行状态",
		"%1 %2在%9港%8度方向%7海里处(%5&%4)锚泊",
		"%1 %2在%9港%8度方向%7海里处(%5&%4)改为未受指令状态",
		"%1 %2在%9港%8度方向%7海里处(%5&%4)改变为机动受限状态",
		"%1 %2在%9港%8度方向%7海里处(%5&%4)改变为吃水受限状态",
		"%1 %2在%9港%8度方向%7海里处(%5&%4)系泊",
		"%1 %2在%9港%8度方向%7海里处(%5&%4)搁浅",
		"%1 %2在%9港%8度方向%7海里处(%5&%4)捕捞作业",
		"%1 %2在%9港%8度方向%7海里处(%5&%4)改变为风帆动力航行状态"
	};

	//脟贸鲁枚脢盲鲁枚麓卢虏掳脨脜脧垄碌脛脦禄脰脙
	int begin=(pn-1)*psize;
	int end=pn*psize;
	if (pn*psize>shipList.size())
	{
		end=shipList.size();
	}


	int index=0;
	DEBUG_LOG("[VesselSvc::OnVesselDirMsg3]shipInfoMap.size:%u, begin:%d,end:%d", shipList.size(), begin,end);
	for (int i=begin;i<end;i++)
	{
		ShipData* pdata = &shipList[i];
		AisRealtime* pReal =aisShip.FindAis(pdata->mmsi);


		string desc="";
		long time=0;
		double logitue=0.0;
		double latitude=0.0;
		if (pReal)
		{
			time=pReal->time;
			logitue=pReal->lon;
			latitude=pReal->lat;

			int n_state = pReal->nav_status;
			if (n_state > 8 || n_state < 0)
				n_state = 0;

			desc = lang?CodeConverter::Gb2312ToUtf8(desc_cn[n_state].c_str()):desc_en[n_state];
			time_t paratime = int(pReal->time + timezone*3600);
			struct tm *converttm;
			converttm = gmtime(&paratime);
			char tmptime[20];
			snprintf(tmptime,sizeof(tmptime),"%4d-%02d-%02d %02d:%02d:%02d",converttm->tm_year + 1900,converttm->tm_mon + 1,
				converttm->tm_mday,converttm->tm_hour,converttm->tm_min,converttm->tm_sec);

			desc = StrReplace(desc,"%2", tmptime);
			desc = StrReplace(desc,"%1", strlen(pdata->name)<3 ? IntToStr(pdata->mmsi) : pdata->name);
			string tempLonLat = "";
			if(pReal->lon >= 0.0)
			{
				tempLonLat = DoubleToStr(ABS(pReal->lon));
				tempLonLat += "E";
				desc = StrReplace(desc,"%4", tempLonLat);
			}
			else
			{
				tempLonLat = DoubleToStr(ABS(pReal->lon));
				tempLonLat += "W";
				desc = StrReplace(desc,"%4", tempLonLat);
			}

			if(pReal->lat >= 0.0)
			{
				tempLonLat = DoubleToStr(ABS(pReal->lat));
				tempLonLat += "N";
				desc = StrReplace(desc,"%5", tempLonLat);
			}
			else
			{
				tempLonLat = DoubleToStr(ABS(pReal->lat));
				tempLonLat += "S";
				desc = StrReplace(desc,"%5", tempLonLat);
			}

			//碌脙碌陆脝盲脦禄脰脙脙猫脢枚脨脜脧垄
			double dist, degree;
			PortInfo* pNearPort = g_PortSvc::instance()->FindNearestPort(pReal->lon, pReal->lat, dist, degree, true);
			desc = StrReplace(desc, "%9", lang?pNearPort->portnamecn:pNearPort->portname);
			desc = StrReplace(desc, "%8", DoubleToStr(degree));
			desc = StrReplace(desc, "%7", DoubleToStr(dist));
		}

		if (index>0)
			out << ",";
		out << FormatString("{\"shipid\":\"%s\",\"shipname\":\"%s\",\"loa\":\"%.2f\",\"beam\":\"%.2f\",\"time\":\"%d\",\"lon\":%.3f,\"lat\":%.3f,\"callsign\":\"%s\",\"imo\":\"%s\",\"mmsi\":\"%d\",\"event_desc\":\"%s\",\"cny\":\"%s\"}", 
			pdata->shipid, pdata->name,pdata->length,pdata->beam,time, logitue, latitude, pdata->callsign, pdata->imo, pdata->mmsi, desc.c_str(), pdata->country);

		index++;

	}
	out<<"]}";

	return 0;

}
//zhuxj
//{text:"",type:,state:,flag:"",built_l:,built_u:,spd_l:,spd_u:,pn:,psize:,timezone:,lang:0/1}lang:1 cn,0 en  mmsi:,imo:"", name:"",callsign:""
int VesselSvc::OnVesselDirMsg3(const char* pUid, const char* jsonString, std::stringstream& out)
{
    return OnVesselDirMsg3_4(pUid,jsonString,out,false);
}

int VesselSvc::OnVesselDirMsg4(const char* pUid, const char* jsonString, std::stringstream& out)
{
    return OnVesselDirMsg3_4(pUid,jsonString,out,true);
}

//{shipid:""}
int VesselSvc::judgeShipid(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[VesselSvc::judgeShipid]bad format:", jsonString, 1);
	string shipid = root.getv("shipid", "");
	
	MySql *psql = CREATE_MYSQL;
	char sql[1024] = "";
	int num = 0;
	sprintf (sql, "SELECT COUNT(1) AS num FROM t20_camera_ship_cj WHERE SHIPID = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{		
		READMYSQL_INT(num, num, 0);
	}
	if (num) num = 1;
	out << "{intflag:" << num << "}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//{imo:"12,23"}
int VesselSvc::imoGetMmsi(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[VesselSvc::imoGetMmsi]bad format:", jsonString, 1);
	string imos = root.getv("imo", "");
	Tokens imoVec = StrSplit(imos, ",");
	MySql *psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT imo, mmsi FROM boloomodb.t41_ship WHERE imo IN ('%s')", ToStr(imoVec, "','").c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";
	int cnt = 0;
	while (psql->NextRow())
	{		
		int imo, mmsi;
		READMYSQL_INT(imo, imo, 0);
		READMYSQL_INT(mmsi, mmsi, 0);
		if (cnt++)
			out << ",";
		out << "{imo:" << imo << ",mmsi:" << mmsi << "}";
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int VesselSvc::getVesselVeryDetails(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[VesselSvc::getVesselVeryDetails]bad format:", jsonString, 1);
	int mmsi = root.getv("mmsi", 0);
	if(mmsi <= 0) return 1;

	MySql *psql = CREATE_MYSQL;
	char sql[10240] = "";
	sprintf (sql, "SELECT \
		t1.SHIPID, t1.IMO, t1.NAME, t1.COUNTRY_CODE, t1.ClassKey, t1.CALLSIGN, t1.mmsi, t1.SHIPTYPE_KEY,\
		t2.LOA, t2.MAXBEAM, t2.BEAMMOULDED, t2.DRAFT,\
		t3.DWT, t3.LIGHTDISPLACEMENT, t3.LOADEDDISPLACEMENT, t3.GROSSTON, t3.NETTON,\
		t4.HullDes, t4.HullNo,\
		t5.Name AS manager_comp, t5.IMO AS manager_imo, t5.CountryName AS manager_iso3, t5.Address AS manager_addr,\
		t6.Name AS owner_comp, t6.IMO AS owner_imo, t6.CountryName AS owner_iso3, t6.Address AS owner_addr,\
		ta.mmsi, ta.longitude, ta.latitude, ta.sog, ta.true_head, ta.rot, ta.nav_status, ta.dest, ta.eta, ta.draught, ta.pos_type,\
		GROUP_CONCAT(t7.Terminal_No) AS Inmarsat\
		FROM \
		(SELECT mmsi, longitude, latitude, sog, true_head, rot, nav_status, dest, eta, draught, pos_type FROM aisdb.t41_ais_ship_realtime WHERE mmsi = %d) ta\
		LEFT JOIN t41_ship t1 ON ta.mmsi = t1.mmsi\
		LEFT JOIN t41_ship_dimension t2 ON t1.SHIPID = t2.SHIPID\
		LEFT JOIN t41_ship_tonnage t3 ON t1.SHIPID = t3.SHIPID\
		LEFT JOIN t41_ship_hull t4 ON t1.SHIPID = t4.SHIPID\
		LEFT JOIN t41_company t5 ON t1.MANAGER = t5.company_key\
		LEFT JOIN t41_company t6 ON t1.OWNER = t6.company_key\
		LEFT JOIN t41_ship_communication t7 ON t1.SHIPID = t7.shipid\
		GROUP BY t1.Shipid", mmsi);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{		
		out << FormatString("{shipid:\"%s\",imo:\"%s\",name:\"%s\",country:\"%s\",class:\"%s\",callsign:\"%s\",mmsi:\"%s\",type:\"%s\",loa:\"%s\",maxbeam:\"%s\",beammoulded:\"%s\",draft:\"%s\",dwt:\"%s\",lightdisplayment:\"%s\",loaddisplayment:\"%s\",\
							grosston:\"%s\",netton:\"%s\",hulldes:\"%s\",hullno:\"%s\",manager_comp:\"%s\",manager_imo:\"%s\",manager_iso3:\"%s\",manager_addr:\"%s\",\
							owner_comp:\"%s\",owner_imo:\"%s\",owner_iso3:\"%s\",owner_addr:\"%s\",\
							longitude:\"%s\",latitude:\"%s\",sog:\"%s\",true_head:\"%s\",rot:\"%s\",nav_status:\"%s\",dest:\"%s\",eta:\"%s\",draught:\"%s\",pos_type:\"%s\",inmarsat:\"%s\"}",
							NOTNULL(psql->GetField("SHIPID")), NOTNULL(psql->GetField("IMO")), NOTNULL(psql->GetField("NAME")), NOTNULL(psql->GetField("COUNTRY_CODE")),
							NOTNULL(psql->GetField("ClassKey")), NOTNULL(psql->GetField("CALLSIGN")), NOTNULL(psql->GetField("mmsi")),NOTNULL(psql->GetField("SHIPTYPE_KEY")), 
							NOTNULL(psql->GetField("LOA")), NOTNULL(psql->GetField("MAXBEAM")), NOTNULL(psql->GetField("BEAMMOULDED")), NOTNULL(psql->GetField("DRAFT")),
							NOTNULL(psql->GetField("DWT")), NOTNULL(psql->GetField("LIGHTDISPLACEMENT")), NOTNULL(psql->GetField("LOADEDDISPLACEMENT")), NOTNULL(psql->GetField("GROSSTON")), NOTNULL(psql->GetField("NETTON")),
							NOTNULL(psql->GetField("HullDes")), NOTNULL(psql->GetField("HullNo")),
							NOTNULL(psql->GetField("manager_comp")), NOTNULL(psql->GetField("manager_imo")), NOTNULL(psql->GetField("manager_iso3")), NOTNULL(psql->GetField("manager_addr")),
							NOTNULL(psql->GetField("owner_comp")), NOTNULL(psql->GetField("owner_imo")), NOTNULL(psql->GetField("owner_iso3")), NOTNULL(psql->GetField("owner_addr")),
							NOTNULL(psql->GetField("longitude")), NOTNULL(psql->GetField("latitude")), NOTNULL(psql->GetField("sog")), NOTNULL(psql->GetField("true_head")),
							NOTNULL(psql->GetField("rot")), NOTNULL(psql->GetField("nav_status")), NOTNULL(psql->GetField("dest")), NOTNULL(psql->GetField("eta")), NOTNULL(psql->GetField("draught")), NOTNULL(psql->GetField("pos_type")), 
							NOTNULL(psql->GetField("Inmarsat")));
	}
	else
	{
		out << "{eid:2,msg:\"not found this record\"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

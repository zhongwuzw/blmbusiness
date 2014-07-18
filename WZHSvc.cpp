#include "blmcom_head.h"
#include "WZHSvc.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"
#define NOTNULL_0(p)			((p) ? (p) : "0")

CPortAliasWZH::CPortAliasWZH()
{
}

CPortAliasWZH::~CPortAliasWZH()
{
}

bool CPortAliasWZH::LoadAll()
{
	SYSTEM_LOG("[PortSvc::loadPortAlias] begin ======================== ");

	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query("select POINT_ID,ORIG_POINT_ID from T14_PORT_ALIAS"), false);

	const char *pv, *pe;
	while(psql->NextRow())
	{
		pv = psql->GetField("POINT_ID");
		pe = psql->GetField("ORIG_POINT_ID");

		if (!pe)
			pe = pv;
		portalias_table[pv] = pe;
	}

	SYSTEM_LOG("[PortSvc::loadPortAlias] end, total:%d ======================== ", (int)portalias_table.size());
	RELEASE_MYSQL_RETURN(psql, true);
}

char* CPortAliasWZH::FindOrig(const char *aliasID)
{
	PORTALIAS_TABLE::iterator p = portalias_table.find(aliasID);
	if (p != portalias_table.end())
		return (char*)p->second.c_str();
	return NULL;
}

IMPLEMENT_SERVICE_MAP(WZHSvc)

WZHSvc::WZHSvc()
{
}

WZHSvc::~WZHSvc()
{

}

bool WZHSvc::Start()
{
	if(!m_alias.LoadAll())
		return false;

	if(!g_MessageService::instance()->RegisterCmd(MID_WZH, this))
		return false;

	SERVICE_MAP(SID_WZH_PORTINFO,WZHSvc,getPortInfo);
	SERVICE_MAP(SID_WZH_PORTSETDANGER,WZHSvc,setPortDanager);
	SERVICE_MAP(SID_WZH_PORTDYNAMIC,WZHSvc,getPortDynamic);
	SERVICE_MAP(SID_WZH_PORTALARMLIST,WZHSvc,getPortAlarmList);
	SERVICE_MAP(SID_WZH_GETSTOWS,WZHSvc,getStows);
	SERVICE_MAP(SID_WZH_UPDATESTOW,WZHSvc,updateStow);
	SERVICE_MAP(SID_WZH_DELETESTOW,WZHSvc,deleteStow);
	

	DEBUG_LOG("[WZHSvc::Start] OK......................................");
	return true;
}

int WZHSvc::getPortInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WZHSvc::getPortInfo]bad format:", jsonString, 1);
	string vid = root.getv("versionid", "");
	string portid = root.getv("portid", "");
	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;

	char sql[1024];
	char* sql_base = "SELECT T.TIMEZONE,T.CHARTNO,T.ZONEMOVE,T.LATITUDE,"\
		"T.LONGITUDE,T.PORTID,T.NAME,T.ISO3,"\
		"T.COMPANY_KEY,T1.NAME COMPNAY_NAME,T2.IS_DANGER FROM T41_PORT T "\
		"LEFT OUTER JOIN t41_company T1 ON T.COMPANY_KEY = T1.COMPANY_KEY "\
		"LEFT OUTER JOIN T41_WZH_PORT T2 ON T.PortID = T2.PORTID "\
		"WHERE T.PortID = '%s';";

	sprintf(sql, sql_base, portid.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql) && psql->NextRow(), 3);

	const char* timezone	= psql->GetField("TIMEZONE");
	const char* chartno		= psql->GetField("CHARTNO");
	const char* lat			= NOTNULL(psql->GetField("LATITUDE"));
	const char* lon			= NOTNULL(psql->GetField("LONGITUDE"));
	const char* id			= psql->GetField("PORTID");
	const char* name		= NOTNULL(psql->GetField("NAME"));
	const char* iso3		= psql->GetField("ISO3");
	const char* compid		= psql->GetField("COMPANY_KEY");
	const char* compna		= NOTNULL(psql->GetField("COMPNAY_NAME"));
	const char* is_danger   = NOTNULL_0(psql->GetField("IS_DANGER"));

	string szLat = JsonReplace(lat);
	string szLon = JsonReplace(lon);
	string szName = JsonReplace(name);
	string szComp = JsonReplace(compna);

	out<<"{\"portid\":\""<<NOTNULL(id)<<'\"'
		<<",\"name\":\""<<szName<<'\"'
		<<",\"iso\":\""<<NOTNULL(iso3)<<'\"'
		<<",\"tz\":\""<<NOTNULL(timezone)<<'\"'
		<<",\"cno\":\""<<NOTNULL(chartno)<<'\"'
		<<",\"lat\":\""<<szLat<<'\"'
		<<",\"lon\":\""<<szLon<<'\"'
		<<",\"compid\":\""<<NOTNULL(compid)<<'\"'
		<<",\"office\":\""<<szComp<<'\"'
		<<",\"danger\":"<<is_danger<<"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int WZHSvc::setPortDanager(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WZHSvc::setPortDanager]bad format:", jsonString, 1);
	string portid = root.getv("portid", "");
	int isdanger = root.getv("danger", 0);
	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;

	char sql[1024];
	char* sql_base = "REPLACE INTO T41_WZH_PORT(PORTID,IS_DANGER,LAST_OP_DT) "\
		"VALUES('%s','%d',NOW());";
	sprintf(sql, sql_base, portid.c_str(),isdanger);
	MySql* psql = CREATE_MYSQL;
	psql->Execute(sql);
	out << FormatString("{eid:0,portid:\"%s\",danger:\"%d\"}", portid.c_str(),isdanger);
	RELEASE_MYSQL_RETURN(psql, 0);
}

int WZHSvc::getPortDynamic(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WZHSvc::getPortDynamic]bad format:", jsonString, 1);
	string portid = root.getv("portid", "");
	char* orgportid = m_alias.FindOrig(portid.c_str());
	int startdt = root.getv("startdt", 0);
	int enddt = root.getv("enddt", 0);
	if(startdt > enddt) return 1;
	if (orgportid)
		portid = orgportid;

	//todo 
	char sql[1024];
	char* sql_base = "REPLACE INTO T41_WZH_PORT(PORTID,IS_DANGER,LAST_OP_DT) "\
		"VALUES('%s','%d',NOW());";
	//sprintf(sql, sql_base, portid.c_str(),isdanger);
	MySql* psql = CREATE_MYSQL;
	//psql->Execute(sql);

	out << FormatString("{eid:0,portid:\"%s\",xibo:%d,maobo:%d,yudao:%d,ligang:%d,yujing:%d}", 
		portid.c_str(),0,0,0,0,0);
	RELEASE_MYSQL_RETURN(psql, 0);
}

// {seq:¡±¡±,Alerts:
// [
// {alertid:¡±¡±,alertime:1404887105,shipid:¡±¡±,shipname:¡±¡±,wharf:¡±¡±,wharfmin:¡±¡±,riskrank:1},
// {...}
// ]
// }
int WZHSvc::getPortAlarmList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WZHSvc::setPortDangetPortAlarmListager]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string portid = root.getv("portid", "");
	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;

	//todo
	char sql[1024];
	char* sql_base = "REPLACE INTO T41_WZH_PORT(PORTID,IS_DANGER,LAST_OP_DT) "\
		"VALUES('%s','%d',NOW());";
	MySql* psql = CREATE_MYSQL;


	out<<"{seq:\"%s\",Alerts:[";
	for (int i=0;i<20;i++)
	{
		if (i>0) out<<",";
		out<<FormatString("{alertid:\"ALERTID_%d\",alerttime:%ld,shipid:\"S%d\",shipname:\"Name%d\",wharf:\"Berth%d\",wharfmin\"%d\",riskrank:%d}",
			i+1,1405067169-i*10,i+1,i+1,i+1,i+1,(i+1)%4);
	}
	out<<"]}";
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

int WZHSvc::getStows(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WZHSvc::getStows]bad format:", jsonString, 1);
	string vid = root.getv("versionid", "");
	string shipid = root.getv("shipid", "");

	char sql[1024];
	char* sql_base = "SELECT ID, SHIPID, SRC_PORTID, DEST_PORTID ,MANAGER, OWNER, STOW_DT, NUM FROM t41_wzh_stowaway"\
		" WHERE SHIPID='%s' ORDER BY STOW_DT DESC";
	sprintf(sql,sql_base,shipid.c_str());

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << FormatString("{shipid:\"%s\",stowaway:[", shipid.c_str());
	bool bfirst = true;
	while(psql->NextRow())
	{
		if (bfirst)
		{
			bfirst = false;	
		}
		else
		{
			out<<",";
		}

		out<<FormatString("{recordid:\"%s\",source:\"%s\",dest:\"%s\",manager:\"%s\",owner:\"%s\",date:%s,number:%s}",
			psql->GetField("ID"),
			psql->GetField("SRC_PORTID"),
			psql->GetField("DEST_PORTID"),
			psql->GetField("MANAGER"),
			psql->GetField("OWNER"),
			psql->GetField("STOW_DT"),
			psql->GetField("NUM"));
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);

}

int WZHSvc::updateStow(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WZHSvc::updateStow]bad format:", jsonString, 1);
	string vid = root.getv("versionid", "");
	string shipid = root.getv("shipid", "");
	string rid = root.getv("recordid", "");
	string src = root.getv("source", "");
	string dest = root.getv("dest", "");
	string manager = root.getv("manager", "");
	string owner = root.getv("owner", "");
	int ndate = root.getv("date", 0);
	int num = root.getv("number", 0);
	int nflag = root.getv("bflag",0);
	if (nflag == 0)
		rid = getStowID();

	char sql[1024*2];
	char *sql_base = "replace into  t41_wzh_stowaway(ID, SHIPID, SRC_PORTID, DEST_PORTID, MANAGER, OWNER, STOW_DT, NUM) values "\
		"('%s','%s','%s','%s','%s','%s',%d,%d);";
	sprintf(sql,sql_base,
		rid.c_str(),
		shipid.c_str(),
		src.c_str(),
		dest.c_str(),
		manager.c_str(),
		owner.c_str(),
		ndate,
		num);

	MySql *psql = CREATE_MYSQL;
	if(psql->Execute(sql) >= 0)
		out << FormatString("{eid:0,shipid:\"%s\",recordid:\"%s\"}",shipid.c_str(),rid.c_str());
	else
		out << FormatString("{eid:1,shipid:\"\",recordid:\"\"}");

	RELEASE_MYSQL_RETURN(psql,0);

}

int WZHSvc::deleteStow(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WZHSvc::deleteStow]bad format:", jsonString, 1);
	
	string shipid = root.getv("shipid", "");
	string rid = root.getv("recordid", "");
	

	char sql[1024];
	char *sql_base = "delete from t41_wzh_stowaway where ID = '%s' and SHIPID = '%s'";
	sprintf(sql,sql_base,
		rid.c_str(),
		shipid.c_str());

	MySql *psql = CREATE_MYSQL;
	if(psql->Execute(sql) >= 0)
		out << FormatString("{eid:0,shipid:\"%s\",recordid:\"%s\"}",shipid.c_str(),rid.c_str());
	else
		out << FormatString("{eid:1,shipid:\"%s\",recordid:\"%s\"}",shipid.c_str(),rid.c_str());

	RELEASE_MYSQL_RETURN(psql,0);
}

std::string WZHSvc::getStowID()
{
	static int _id = 0;
	if(_id == 0) {
		const char* sql = "select max(id+0) as maxid from t41_wzh_stowaway";
		MySql* psql = CREATE_MYSQL;
		if(psql->Query(sql) && psql->NextRow())
			_id = atoi(NOTNULL(psql->GetField("maxid")))+1;
		else
			_id = 1;
		RELEASE_MYSQL_NORETURN(psql);
	} else {
		_id++;
	}
	return FormatString("%d", _id);
}
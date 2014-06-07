#include "blmcom_head.h"
#include "EquipmentSvc.h"
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

#define MAX_DIFF 0xfffffffe

IMPLEMENT_SERVICE_MAP(EquipmentSvc)

EquipmentSvc::EquipmentSvc()
{

}

EquipmentSvc::~EquipmentSvc()
{

}

bool EquipmentSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_EQUIPMENT, this))
        return false;

	int interval = 60;
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
	if(timerId <= 0)
		return false;

    SERVICE_MAP(SID_GET_EQUIMENT_CATEGORYINFO,EquipmentSvc,getCategoryInfo);
	SERVICE_MAP(SID_UPD_EQUIMENT_CATEGORYINFO,EquipmentSvc,updCategoryInfo);
	SERVICE_MAP(SID_ADD_EQUIMENT_CATEGORYINFO,EquipmentSvc,addCategoryInfo);
	SERVICE_MAP(SID_DEL_EQUIMENT_CATEGORYINFO,EquipmentSvc,delCategoryInfo);
	SERVICE_MAP(SID_GET_EQUIMENT_LIST,EquipmentSvc,getEquimentList);
	SERVICE_MAP(SID_UPD_EQUIMENT_INFO,EquipmentSvc,updEquimentInfo);
	SERVICE_MAP(SID_ADD_EQUIMENT_INFO,EquipmentSvc,addEquimentInfo);
	SERVICE_MAP(SID_DEL_EQUIMENT_INFO,EquipmentSvc,delEquimentInfo);
	SERVICE_MAP(SID_GET_EQUIMENT_DATALIST,EquipmentSvc,getDataList);
	SERVICE_MAP(SID_UPD_EQUIMENT_DATAINFO,EquipmentSvc,updDataInfo);
	SERVICE_MAP(SID_ADD_EQUIMENT_DATAINFO,EquipmentSvc,addDataInfo);
	SERVICE_MAP(SID_DEL_EQUIMENT_DATAINFO,EquipmentSvc,delDataInfo);
	SERVICE_MAP(SID_GET_EQUIMENT_INOUTNFO,EquipmentSvc,getInOutInfo);
	SERVICE_MAP(SID_INSERT_EQUIMENT_INOUTINFO,EquipmentSvc,insertInOutInfo);
	SERVICE_MAP(SID_GET_EQUIMENT_MEMORY,EquipmentSvc,getEquimentMemory);
	SERVICE_MAP(SID_INSERT_EQUIMENT_MEMORY,EquipmentSvc,insertEquimentMemory);
	SERVICE_MAP(SID_UPD_EQUIMENT_MEMORY,EquipmentSvc,updEquimentMemory);
	SERVICE_MAP(SID_GET_EQUIMENT_SAMECOMPSHIP,EquipmentSvc,getSameCompShip);
	SERVICE_MAP(SID_GET_EQUIMENT_STANDARD,EquipmentSvc,getEquimentStandard);
	SERVICE_MAP(SID_INSERT_EQUIMENT_STANDARD,EquipmentSvc,insertEquimentStandard);
	SERVICE_MAP(SID_APP_AUTHORITY,EquipmentSvc,ApplyEquipmentSvcAuthority);
	SERVICE_MAP(SID_DEAL_USERAPPLY,EquipmentSvc,DealWithUserApplication);
	SERVICE_MAP(SID_GET_AUTHORITY_LIST,EquipmentSvc,getEquipmentAuth);
	SERVICE_MAP(SID_DEL_AUTHORITY,EquipmentSvc,delEquipmentAuth);
	SERVICE_MAP(SID_GET_HISTORY_SP,EquipmentSvc,getHistorySp);
	SERVICE_MAP(SID_UPD_EQUIPMENT_WH,EquipmentSvc,updEquipmentWh);
	SERVICE_MAP(SID_UDP_EQUIPMENT_PICT,EquipmentSvc,updEqPict);
	SERVICE_MAP(SID_GET_EQUIPMENT_PICT,EquipmentSvc,getEqPict);
	SERVICE_MAP(SID_DEL_EQUIPMENT_PICT,EquipmentSvc,delEqPict);


    DEBUG_LOG("[EquipmentSvc::Start] OK......................................");
	//std::stringstream out;
	//delEqPict("libh", "{eqid:\"EQ00031\",picd:\"00001261.png\"}",out);
	return true;
}

//{shipid}
//[{cid:"ML001",cnm:"主机系统",lvl:"1",pcid:"0"},{}...]
int EquipmentSvc::getCategoryInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getCategoryInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.CATEGORYID, t1.CATEGORY_NAME, t1.CATEGORY_LEVEL, t1.PARENT_CATEGORYID \
					FROM t41_equipment_category t1 JOIN t41_seipc_company t2 ON t1.COMPANY_KEY = t2.COMPANY_KEY WHERE t2.SHIPID = '%s' ORDER BY t1.CATEGORY_LEVEL", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	char cid[100] = "";
	char cnm[100] = "";
	int lvl = -1;
	char pcid[100] = "";
	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(CATEGORYID, cid);
		READMYSQL_STR(CATEGORY_NAME, cnm);
		READMYSQL_INT(CATEGORY_LEVEL, lvl, -1);
		READMYSQL_STR(PARENT_CATEGORYID, pcid);
		if (cnt++)
			out << ",";
		out << "{cid:\"" << cid << "\",cnm:\"" << JsonReplace(cnm) << "\",lvl:\"" << lvl << "\",pcid:\"" << pcid << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",cid:"ML001",cnm:"主机系统"}
int EquipmentSvc::updCategoryInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::updCategoryInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string cid = root.getv("cid", "");
	string cnm = root.getv("cnm", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT COMPANY_KEY FROM t41_seipc_company WHERE shipid = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char comp_id[100] = "";
	if (psql->NextRow())
	{		
		READMYSQL_STR(COMPANY_KEY, comp_id);
	}
	else
		RELEASE_MYSQL_RETURN(psql, 1);

	//CodeConverter::Gb2312ToUtf8(cnm.c_str()).c_str()
	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "UPDATE t41_equipment_category SET CATEGORY_NAME = '%s', LAST_UPD_DT = '%s', LAST_UPD_USER = '%s' WHERE COMPANY_KEY = '%s' AND  CATEGORYID = '%s'", SqlReplace(cnm).c_str(), gmt0now, pUid, comp_id, cid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{eid:\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",cnm:"主机系统",lvl:"3",pcid:"ML008"}
int EquipmentSvc::addCategoryInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::addCategoryInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string cnm = root.getv("cnm", "");
	string lvl = root.getv("lvl", "");
	string pcid = root.getv("pcid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT MAX(CATEGORYID) as cid, t2.COMPANY_KEY FROM t41_equipment_category t1 RIGHT JOIN t41_seipc_company t2 ON t1.COMPANY_KEY = t2.COMPANY_KEY \
					WHERE t2.SHIPID = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char comp_id[100] = "";
	char cid[100] = "";
	if (psql->NextRow())
	{
		READMYSQL_STR(cid, cid);
		READMYSQL_STR(COMPANY_KEY, comp_id);
	}

	if (!strlen(comp_id))
	{
		out << "{cid:\"\"}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	int id_num = 1;
	if (strlen(cid))
	{
		char _tmp[100] = "";
		sscanf(cid, "ML%s", _tmp);
		id_num = ACE_OS::atoi(_tmp) + 1;
	}
	
	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (cid, "ML%05d", id_num);
	sprintf (sql, "INSERT INTO t41_equipment_category (COMPANY_KEY, CATEGORYID, CATEGORY_NAME, CATEGORY_LEVEL, PARENT_CATEGORYID, LAST_UPD_DT, LAST_UPD_USER)\
				  VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s')", comp_id, cid, SqlReplace(cnm).c_str(), lvl.c_str(), pcid.c_str(), gmt0now, pUid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{cid:\"" << cid << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",cid:"ML022"}
int EquipmentSvc::delCategoryInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::delCategoryInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string cid = root.getv("cid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT COMPANY_KEY FROM t41_seipc_company WHERE shipid = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char comp_id[100] = "";
	if (psql->NextRow())
	{		
		READMYSQL_STR(COMPANY_KEY, comp_id);
	}
	else
		RELEASE_MYSQL_RETURN(psql, 1);

	sprintf (sql, "SELECT EQUIPID FROM t41_equipment t1 JOIN t41_equipment_category t2 ON t1.CATEGORYID = t2.CATEGORYID \
						WHERE t2.COMPANY_KEY = '%s' AND t2.CATEGORYID = '%s'", comp_id, cid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char eqid[100] = "";
	if (psql->NextRow())
	{		
		READMYSQL_STR(EQUIPID, eqid);
	}

	sprintf (sql, "SELECT COUNT(1) AS num FROM t41_equipment_category WHERE COMPANY_KEY = '%s' AND PARENT_CATEGORYID = '%s'", comp_id, cid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	int num = 0;
	READMYSQL_INT(num, num, 0);
	if (strlen(eqid) || num)
	{
		out << "{eid:\"100\"}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	sprintf (sql, "DELETE FROM t41_equipment_category WHERE COMPANY_KEY = '%s' AND CATEGORYID = '%s'", comp_id, cid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{eid:\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",cid:"ML022"}
//{cid:"ML022",eqid:""，eqnm:"高压油泵",mtp:"机器型号",spe:"规格",unit:"kg",sp:"23",inv:"23",DESC:"描述信息",qnum:"本季度流水量"}
int EquipmentSvc::getEquimentList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getEquimentList]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string cid = root.getv("cid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	time_t _t = time(0);
	struct tm *ptr = localtime(&_t);
	int year = ptr->tm_year + 1900;
	int smonth = ptr->tm_mon/3*3+1;
	int emonth = ptr->tm_mon/3*3+3;

	int len = sprintf (sql, "SELECT t1.CATEGORYID, t1.EQUIPID, t1.EQUIPNAME, t1.MACHINE_TYPE, t1.SPECIFICATIONS, t1.UNIT, t1.DESC, t2.SURPLUS, t3.INVENTORY, t5.COMPANY_KEY, t5.CREATE_DT, t6.DATASET_NAME, t2.WAREHOUSE AS CKS, t3.WAREHOUSEID AS CKI\
								FROM t41_equipment t1 LEFT JOIN t41_equipment_surplus t2 ON t1.SHIPID = t2.SHIPID AND t1.EQUIPID = t2.EQUIPID \
								LEFT JOIN t41_equipment_inventory t3 ON t1.SHIPID = t3.SHIPID AND t1.EQUIPID = t3.EQUIPID \
								LEFT JOIN t41_seipc_company t4 ON t1.SHIPID = t4.SHIPID\
								LEFT JOIN t41_equipment_transaction t5 ON t1.EQUIPID = t5.EQUIPID AND t4.COMPANY_KEY = t5.COMPANY_KEY AND t1.SHIPID = t5.SHIPID\
								LEFT JOIN t41_equipment_dataset t6 ON t1.UNIT = t6.DATASETID AND t4.COMPANY_KEY = t6.COMPANY_KEY\
								WHERE t1.SHIPID = '%s' AND ((YEAR(t5.OP_TIME) = '%d' AND MONTH(t5.OP_TIME) >=%d AND MONTH(t5.OP_TIME) <= %d) OR t5.OP_TIME IS NULL)", shipid.c_str(), year, smonth, emonth);
	if (cid.length())
		sprintf (sql + len, " AND t1.CATEGORYID = '%s'", cid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	EquipmentInfoMap equipInfos;
	EquipmentInfoMap::iterator iter;
	while (psql->NextRow())
	{
		EquipmentInfo equip;
		char _key[128] = "";
		char comp_id[64] = "";
		char c_dt[64] = "";
		double _sp;
		double _inv;
		char cks[64];
		char cki[64];

		READMYSQL_STR(EQUIPID, equip.eqid);
		READMYSQL_STR(COMPANY_KEY, comp_id);
		sprintf (_key, "%s@%s", equip.eqid, comp_id);

		READMYSQL_DOUBLE(SURPLUS, _sp, 0.0);
		READMYSQL_DOUBLE(INVENTORY, _inv, 0.0);
		READMYSQL_STR(CREATE_DT, c_dt);
		READMYSQL_STR(CKS, cks);
		READMYSQL_STR(CKI, cki);

		iter = equipInfos.find(_key);
		if (iter == equipInfos.end())
		{
			READMYSQL_STR(CATEGORYID, equip.cid);
			READMYSQL_STR(EQUIPNAME, equip.eqnm);
			READMYSQL_STR(MACHINE_TYPE, equip.mtp);
			READMYSQL_STR(SPECIFICATIONS, equip.spe);
			READMYSQL_STR(UNIT, equip.unit);			
			READMYSQL_STR(DESC, equip.desc);			
			READMYSQL_STR(CREATE_DT, c_dt);
			READMYSQL_STR(DATASET_NAME, equip.unm);
			if (strlen(c_dt))
			{
				equip.qnum = 1;
				equip.c_time.insert(string(c_dt));
			}
			else
				equip.qnum = 0;
			if (strlen(cks))
			{
				equip.cks.insert(string(cks));
				equip.sp = _sp;
			}
			else
				equip.sp = 0.0;
			if (strlen(cki))
			{
				equip.cki.insert(string(cki));
				equip.inv  = _inv;
			}
			else
				equip.inv = 0.0;
			equipInfos.insert(make_pair(_key, equip));
		}
		else
		{
			if (strlen(c_dt) && (*iter).second.c_time.find(string(c_dt)) == (*iter).second.c_time.end())
			{
				(*iter).second.qnum += (strlen(c_dt) > 0);
				(*iter).second.c_time.insert(string(c_dt));
			}

			if (strlen(cks) && (*iter).second.cks.find(string(cks)) == (*iter).second.cks.end())
			{
				(*iter).second.cks.insert(string(cks));
				(*iter).second.sp += _sp;
			}

			if (strlen(cki) && (*iter).second.cki.find(string(cki)) == (*iter).second.cki.end())
			{
				(*iter).second.cki.insert(string(cki));
				(*iter).second.inv  += _inv;
			}
		}
	}
	out << "[";
	for (iter = equipInfos.begin(); iter != equipInfos.end(); iter++)
	{
		if (iter != equipInfos.begin())
			out << ",";
		out << "{cid:\"" << (*iter).second.cid << "\",eqid:\"" << (*iter).second.eqid << "\",eqnm:\"" << JsonReplace((*iter).second.eqnm) << "\",mtp:\"" << JsonReplace((*iter).second.mtp) 
			<< "\",spe:\"" << JsonReplace((*iter).second.spe) << "\",unit:\"" << (*iter).second.unit << "\",unm:\"" << JsonReplace((*iter).second.unm) << "\",sp:\"" << (*iter).second.sp << "\",inv:\""
			<< (*iter).second.inv << "\",DESC:\"" << JsonReplace((*iter).second.desc) << "\",qnum:\"" << (*iter).second.qnum << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",eqid:"",eqnm:"高压油泵",mtp:"机器型号",spe:"规格",unit:"kg",desc:"描述信息"}
int EquipmentSvc::updEquimentInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::updEquimentInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string eqid = root.getv("eqid", "");
	string eqnm = root.getv("eqnm", "");
	string mtp = root.getv("mtp", "");
	string spe = root.getv("spe", "");	
	string unit = root.getv("unit", "");
	string desc = root.getv("desc", "");
	string cid = root.getv("cid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_equipment SET CATEGORYID = '%s', EQUIPNAME = '%s', MACHINE_TYPE = '%s', SPECIFICATIONS = '%s', UNIT = '%s', `DESC` = '%s' WHERE SHIPID = '%s' AND EQUIPID = '%s'",
					cid.c_str(), SqlReplace(eqnm).c_str(), SqlReplace(mtp).c_str(), SqlReplace(spe).c_str(), unit.c_str(), SqlReplace(desc).c_str(), shipid.c_str(), eqid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{eid:\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",cid:"",eqnm:"高压油泵",mtp:"机器型号",spe:"规格",unit:"kg",desc:"描述信息"}
int EquipmentSvc::addEquimentInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::addEquimentInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string cid = root.getv("cid", "");
	string eqnm = root.getv("eqnm", "");
	string mtp = root.getv("mtp", "");
	string spe = root.getv("spe", "");
	string unit = root.getv("unit", "");
	string desc = root.getv("desc", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT MAX(EQUIPID) AS eqid FROM t41_equipment WHERE shipid = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	char eqid[100] = "";
	if (psql->NextRow())
	{
		READMYSQL_STR(eqid, eqid);
	}
	int _id_num = 1;
	if (strlen(eqid))
	{
		char _tmp[100] = "";
		sscanf (eqid, "EQ%s", _tmp);
		_id_num = ACE_OS::atoi(_tmp) + 1;
	}
	sprintf (eqid, "EQ%05d", _id_num);

	sprintf (sql, "INSERT INTO t41_equipment (SHIPID, EQUIPID, CATEGORYID, EQUIPNAME, MACHINE_TYPE, SPECIFICATIONS, UNIT, `DESC`)\
					VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')", shipid.c_str(), eqid, cid.c_str(), SqlReplace(eqnm).c_str(), SqlReplace(mtp).c_str(), SqlReplace(spe).c_str(), unit.c_str(), SqlReplace(desc).c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{eqid:\"" << eqid << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{shipid:"1234",eqid:""}
int EquipmentSvc::delEquimentInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::delEquimentInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string eqid = root.getv("eqid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT COUNT(1) as num FROM t41_equipment_transaction WHERE SHIPID = '%s' AND EQUIPID = '%s'", shipid.c_str(), eqid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int num = 0;
	if (psql->NextRow())
	{
		READMYSQL_INT(num, num, 0);
	}
	if (num)
	{
		out << "{eid:\"100\"}";
	}
	else
	{
		sprintf (sql, "DELETE FROM t41_equipment WHERE SHIPID = '%s' AND EQUIPID = '%s'", shipid.c_str(), eqid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out << "{eid:\"0\"}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:""}
//{did:"0",tp:"3",nm:"王五",desc:"船长"}
int EquipmentSvc::getDataList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getDataList]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.DATASETID, t1.DATASET_TYPE, t1.DATASET_NAME, t1.DESC \
					FROM t41_equipment_dataset t1 JOIN t41_seipc_company t2 ON t1.COMPANY_KEY = t2.COMPANY_KEY WHERE t2.SHIPID = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";
	int cnt = 0;
	while (psql->NextRow())
	{
		char did[32];
		int dtp;
		char dname[32];
		char desc[1024];
		READMYSQL_STR(DATASETID, did);
		READMYSQL_INT(DATASET_TYPE, dtp, -1);
		READMYSQL_STR(DATASET_NAME, dname);
		READMYSQL_STR(DESC, desc);
		if (cnt++)
			out << ",";
		out << "{did:\"" << did << "\",tp:\"" << dtp << "\",nm:\"" << JsonReplace(dname) << "\",desc:\"" << JsonReplace(desc) << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",did:"0",nm:"王五",desc:"船长"}
int EquipmentSvc::updDataInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::updDataInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string did = root.getv("did", "");
	string nm = root.getv("nm", "");
	string desc = root.getv("desc", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT COMPANY_KEY FROM t41_seipc_company WHERE shipid = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char comp_id[64] = "";
	if (psql->NextRow())
	{
		READMYSQL_STR(COMPANY_KEY, comp_id);
	}
	if (strlen(comp_id))
	{
		sprintf (sql, "UPDATE t41_equipment_dataset SET DATASET_NAME = '%s', `DESC` = '%s' WHERE COMPANY_KEY = '%s' AND DATASETID = '%s'", SqlReplace(nm).c_str(), SqlReplace(desc).c_str(), comp_id, did.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out << "{eid:\"0\"}";
	}
	else
		out << "{eid:\"100\"}";
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",tp:"1",nm:"王五",desc:"船长"} 1:仓库  2:单位   3:职员
int EquipmentSvc::addDataInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::addDataInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string tp = root.getv("tp", "");
	string nm = root.getv("nm", "");
	string desc = root.getv("desc", "");

	int tp_i = tp[0] - '0' - 1;
	if (tp_i > 2 || tp_i < 0)
		return 1;

	string data_id[] = {"CK", "DW", "ZY"};
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT COMPANY_KEY FROM t41_seipc_company WHERE SHIPID = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	char comp_id[100] = "";
	if (psql->NextRow())
	{
		READMYSQL_STR(COMPANY_KEY, comp_id);
	}

	if (!strlen(comp_id))
	{
		out << "{did:\"\"}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	sprintf (sql, "SELECT MAX(DATASETID) AS did FROM t41_equipment_dataset WHERE COMPANY_KEY = '%s' AND DATASET_TYPE = '%s'", comp_id, tp.c_str());
	char did[100] = "";
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		READMYSQL_STR(did, did);
	}

	int _id_num = 1;
	if (strlen(did))
	{
		char _tmp[10] = "";
		if (tp_i == 0)
			sscanf(did, "CK%s", _tmp);
		if (tp_i == 1)
			sscanf(did, "DW%s", _tmp);
		if (tp_i == 2)
			sscanf(did, "ZY%s", _tmp);
		_id_num = ACE_OS::atoi(_tmp) + 1;
	}
	sprintf (did, "%s%03d", data_id[tp_i].c_str(), _id_num);
	sprintf (sql, "INSERT INTO t41_equipment_dataset (COMPANY_KEY, DATASETID, DATASET_TYPE, DATASET_NAME, `DESC`)\
					VALUES ('%s', '%s', '%s', '%s', '%s')", comp_id, did, tp.c_str(), SqlReplace(nm).c_str(), SqlReplace(desc).c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{did:\"" << did << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",did:"ZY002"}
int EquipmentSvc::delDataInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::delDataInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string did = root.getv("did", "");
	int tp = (did[0] == 'Z')?2:(did[0] == 'C')?0:1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT COMPANY_KEY FROM t41_seipc_company WHERE SHIPID = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char comp_id[100] = "";
	if (psql->NextRow())
	{
		READMYSQL_STR(COMPANY_KEY, comp_id);
	}
	if (!strlen(comp_id))
		RELEASE_MYSQL_RETURN(psql, 1);

	int num = 0;
	if (tp == 0)
	{
		int _num;
		sprintf (sql, "SELECT COUNT(1) AS num FROM t41_equipment_transaction WHERE shipid = '%s' AND COMPANY_KEY = '%s' AND WAREHOUSEID = '%s'", shipid.c_str(), comp_id, did.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);		
		READMYSQL_INT(num, _num, 0);
		num += _num;

		sprintf (sql, "SELECT COUNT(1) AS num FROM t41_equipment_inventory WHERE SHIPID = '%s' AND WAREHOUSEID = '%s'", shipid.c_str(), did.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);		
		READMYSQL_INT(num, _num, 0);
		num += _num;

		sprintf (sql, "SELECT COUNT(1) AS num FROM t41_equipment_surplus WHERE SHIPID = '%s' AND WAREHOUSE = '%s'", shipid.c_str(), did.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);		
		READMYSQL_INT(num, _num, 0);
		num += _num;
	}
	else if (tp == 1)
	{
		sprintf (sql, "SELECT COUNT(1) as num FROM t41_equipment WHERE UNIT = '%s' AND shipid = '%s'", did.c_str(), shipid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);		
		READMYSQL_INT(num, num, 0);
	}
	else if (tp == 2)
	{
		sprintf (sql, "SELECT COUNT(1) AS num FROM t41_equipment_transaction WHERE shipid = '%s' AND COMPANY_KEY = '%s' AND OP_USER = '%s'", shipid.c_str(), comp_id, did.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);		
		READMYSQL_INT(num, num, 0);
	}

	if (num)
	{
		out << "{eid:\"100\"}";
	}
	else 
	{
		sprintf (sql, "DELETE FROM t41_equipment_dataset WHERE COMPANY_KEY = '%s' AND DATASETID = '%s' AND DATASET_TYPE = '%d'", comp_id, did.c_str(), tp+1);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out << "{eid:\"0\"}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",eqid:"ZY002",stime:"YYYY-MM-DD",etime:"YYYY-MM-DD",tp:"0"}
//[{shipid:"1234",eqid:"ZY002",tm:"",tp:"",eqnm:"",dhh:"",mafr:"",srcdest:"",qt:"",gg:"",unit:"",pe:"",wh:"",uid:"",DESC:"",whnm:"",unm:""},{}...]
int EquipmentSvc::getInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getInOutInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string eqid = root.getv("eqid", "");
	string stime = root.getv("stime", "");
	string etime = root.getv("etime", "");
	string tp = root.getv("tp", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024 * 2] = "";
	int len = sprintf (sql, "SELECT t3.EQUIPNAME, t3.SPECIFICATIONS, t3.UNIT, t1.EQUIPID, t1.OP_TYPE, UNIX_TIMESTAMP(t1.OP_TIME) AS OP_TIME, UNIX_TIMESTAMP(t1.CREATE_DT) as crdt, t1.ORDERID, t1.MANUFACTURER, t1.SOURCE_OR_DEST, t1.QUANTITY, t1.COMPANY_KEY, t1.PRICE, \
					t1.WAREHOUSEID, t1.DESC, t1.OP_USER, t4.DATASET_NAME AS ZYNM, t2.DATASET_NAME AS CKNM FROM t41_equipment_transaction t1 \
					LEFT JOIN t41_equipment_dataset t2 ON t1.WAREHOUSEID = t2.DATASETID AND t1.COMPANY_KEY = t2.COMPANY_KEY\
					LEFT JOIN t41_equipment t3 ON t1.SHIPID = t3.SHIPID AND t1.EQUIPID = t3.EQUIPID\
					LEFT JOIN t41_equipment_dataset t4 ON t1.OP_USER = t4.DATASETID AND t1.COMPANY_KEY = t4.COMPANY_KEY\
					WHERE t1.SHIPID = '%s'\
					AND UNIX_TIMESTAMP(t1.OP_TIME) >= '%s' AND UNIX_TIMESTAMP(t1.OP_TIME) <= '%s'",
					shipid.c_str(), stime.c_str(), etime.c_str());
	if (tp[0] == '1' || tp[0] == '2')
		len += sprintf (sql + len, " AND t1.OP_TYPE = '%s' ", tp.c_str());
	if (eqid.length())
		len += sprintf (sql + len, " AND t1.EQUIPID = '%s'", eqid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		char tp_io[10];
		char _eqid[100];
		char cmpid[64];
		char tm[64];
		char crdt[64];
		char eqnm[100];
		char dhh[100];
		char mafr[100];
		char srcdest[128];
		char qt[100];
		char gg[100];
		char unit[10];
		char pe[100];
		char wh[10];
		char uid[10];
		char desc[1024 * 10];
		char whnm[100];
		char unm[100];

		READMYSQL_STR(EQUIPID, _eqid);
		READMYSQL_STR(OP_TYPE, tp_io);
		READMYSQL_STR(COMPANY_KEY, cmpid);
		READMYSQL_STR(OP_TIME, tm);
		READMYSQL_STR(crdt, crdt);
		READMYSQL_STR(EQUIPNAME, eqnm);
		READMYSQL_STR(ORDERID, dhh);
		READMYSQL_STR(MANUFACTURER, mafr);
		READMYSQL_STR(SOURCE_OR_DEST, srcdest);
		READMYSQL_STR(QUANTITY, qt);		
		READMYSQL_STR(SPECIFICATIONS, gg);
		READMYSQL_STR(UNIT, unit);
		READMYSQL_STR(PRICE, pe);
		READMYSQL_STR(WAREHOUSEID, wh);
		READMYSQL_STR(OP_USER, uid);
		READMYSQL_STR(DESC, desc);
		READMYSQL_STR(CKNM, whnm);
		READMYSQL_STR(ZYNM, unm);

		if (cnt++)
			out << ",";
		out << "{shipid:\"" << shipid << "\",eqid:\"" << _eqid << "\",cmpid:\"" << cmpid << "\",tm:\"" << tm << "\",crdt:\"" << crdt << "\",tp:\"" << tp_io << "\",eqnm:\"" << JsonReplace(eqnm) << "\",dhh:\""
			<< dhh << "\",mafr:\"" << mafr << "\",srcdest:\"" << srcdest << "\",qt:\"" << qt << "\",gg:\"" << JsonReplace(gg) << "\",unit:\"" << unit 
			<< "\",pe:\"" << pe << "\",wh:\"" << wh << "\",uid:\"" << uid << "\",DESC:\"" << JsonReplace(desc) << "\",whnm:\"" << JsonReplace(whnm) << "\",unm:\"" << JsonReplace(unm) << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",tm:"",tp:"",eqid:"",dhh:"",mafr:"",srcdest:"",qt:"",unit:"",pe:"",wh:"",desc:"",user:""}
int EquipmentSvc::insertInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::insertInOutInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string tm = root.getv("tm", "");
	string tp = root.getv("tp", "");
	string eqid = root.getv("eqid", "");
	string dhh = root.getv("dhh", "");
	string mafr = root.getv("mafr", "");
	string srcdest = root.getv("srcdest", "");
	string qt = root.getv("qt", "");
	string pe = root.getv("pe", "");
	string wh = root.getv("wh", "");
	string desc = root.getv("desc", "");
	string user = root.getv("user", "");

	MySql* psql = CREATE_MYSQL;
	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024] = "";
	sprintf (sql, "SELECT COMPANY_KEY FROM t41_seipc_company WHERE SHIPID = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char comp_id[100] = "";
	if (psql->NextRow())
	{
		READMYSQL_STR(COMPANY_KEY, comp_id);
	}

	sprintf (sql, "SELECT SURPLUS FROM t41_equipment_surplus WHERE SHIPID = '%s' AND EQUIPID = '%s' AND WAREHOUSE = '%s'", shipid.c_str(), eqid.c_str(), wh.c_str());
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	double _sp = -1.0;
	if (psql->NextRow())
	{
		READMYSQL_DOUBLE(SURPLUS, _sp, -1.0);
	}
		
	if (strlen(comp_id) == 0 || (tp[0] == '2' && _sp < ACE_OS::atoi(qt.c_str())))
	{
		out << "{eid:\"100\"}";		
	}
	else 
	{
		sprintf (sql, "INSERT INTO t41_equipment_transaction (COMPANY_KEY, SHIPID, EQUIPID, CREATE_DT, OP_TIME, OP_TYPE, ORDERID, MANUFACTURER, SOURCE_OR_DEST, QUANTITY, WAREHOUSEID, PRICE, OP_USER, `DESC`)\
						VALUES ('%s', '%s', '%s', '%s', FROM_UNIXTIME('%s'), '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
						comp_id, shipid.c_str(), eqid.c_str(), gmt0now, tm.c_str(), tp.c_str(), dhh.c_str(), mafr.c_str(), srcdest.c_str(), qt.c_str(), wh.c_str(), pe.c_str(), user.c_str(), SqlReplace(desc).c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		sprintf (sql, "SELECT EQUIPNAME, CATEGORYID FROM t41_equipment WHERE SHIPID = '%s' AND EQUIPID = '%s' ", shipid.c_str(), eqid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		char eqnm[64] = "";
		char cid[64] = "";
		if (psql->NextRow())
		{
			READMYSQL_STR(EQUIPNAME, eqnm);
			READMYSQL_STR(CATEGORYID, cid);
		}

		if (_sp < 0.0)
		{			
			sprintf (sql, "INSERT INTO t41_equipment_surplus (SHIPID, EQUIPID, CATEGORYID, EQUIPNAME, WAREHOUSE, SURPLUS)VALUES ('%s', '%s', '%s', '%s', '%s', '%s')",
							shipid.c_str(), eqid.c_str(), cid, eqnm, wh.c_str(), qt.c_str());
		}
		else
			sprintf (sql, "UPDATE t41_equipment_surplus SET SURPLUS = SURPLUS %s %s WHERE SHIPID = '%s' AND EQUIPID = '%s' AND WAREHOUSE = '%s'", tp[0] == '1'?"+":"-", qt.c_str(), shipid.c_str(), eqid.c_str(), wh.c_str());
			
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		sprintf (sql, "SELECT SURPLUS FROM t41_equipment_surplus WHERE SHIPID = '%s' AND EQUIPID = '%s'", shipid.c_str(), eqid.c_str());
	
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		_sp = 0.0;
		while (psql->NextRow())
		{
			double t_sp = 0.0;
			READMYSQL_DOUBLE(SURPLUS, t_sp, 0.0);
			_sp += t_sp;
		}
		sprintf (sql, "INSERT IGNORE INTO t41_equipment_surplus_history (SHIPID, EQUIP_ID, CATEGORY_ID, INCREASE, DECREASE, SURPLUS, CREATE_DT) VALUES \
					  ('%s', '%s', '%s', '%s', '%s', '%f', '%s')", shipid.c_str(), eqid.c_str(), cid, tp[0] == '1'?qt.c_str():"0.0", tp[0] == '2'?qt.c_str():"0.0", _sp, gmt0now);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out << "{eid:\"0\"}";
	}	
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",eqid:""}
//[{eqid:"",ctid:"",eqnm:"123",wh:"cangku1",whnm:"cangku1",sp:"2",inv:"3",invt:"132343223"},{}...]
int EquipmentSvc::getEquimentMemory(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getEquimentMemory]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string eqid = root.getv("eqid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.CATEGORYID, t1.WAREHOUSEID, t1.INVENTORY, UNIX_TIMESTAMP(t1.INVENTORY_TIME) AS INVENTORY_TIME, t4.EQUIPNAME, t2.SURPLUS, t3.DATASET_NAME\
						FROM t41_equipment_inventory t1 LEFT JOIN t41_equipment_surplus t2 ON t1.SHIPID = t2.SHIPID AND t1.EQUIPID = t2.EQUIPID AND t1.CATEGORYID = t2.CATEGORYID AND t1.WAREHOUSEID = t2.WAREHOUSE\
						LEFT JOIN t41_equipment_dataset t3 ON t1.WAREHOUSEID = t3.DATASETID \
						LEFT JOIN t41_equipment t4 ON t1.SHIPID = t4.SHIPID AND t1.EQUIPID = t4.EQUIPID\
						WHERE t1.shipid = '%s' AND t1.EQUIPID = '%s'", shipid.c_str(), eqid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		char ctid[100];
		char eqnm[100];
		char wh[100];
		char whnm[100];
		double sp;
		double inv;
		char invt[100];

		READMYSQL_STR(CATEGORYID, ctid);
		READMYSQL_STR(EQUIPNAME, eqnm);
		READMYSQL_STR(WAREHOUSEID, wh);
		READMYSQL_DOUBLE(SURPLUS, sp, 0.0);
		READMYSQL_STR(DATASET_NAME, whnm);
		READMYSQL_DOUBLE(INVENTORY, inv, 0.0);
		READMYSQL_STR(INVENTORY_TIME, invt);

		if(cnt++)
			out << ",";
		out << "{eqid:\"" << eqid << "\",ctid:\""<< ctid << "\",eqnm:\"" << JsonReplace(eqnm) << "\",wh:\"" << wh << "\",whnm:\"" << JsonReplace(whnm) << "\",sp:\"" << sp 
			<< "\",inv:\"" << inv << "\",invt:\"" << invt << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",eqid:"",whid:"CK001",inv:"3"}
int EquipmentSvc::insertEquimentMemory(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::insertEquimentMemory]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string eqid = root.getv("eqid", "");
	string whid = root.getv("whid", "");
	string inv = root.getv("inv", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT CATEGORYID, EQUIPNAME FROM t41_equipment WHERE shipid = '%s' AND EQUIPID = '%s'", shipid.c_str(), eqid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char ctid[100] = "";
	char eqnm[100] = "";
	if (psql->NextRow())
	{
		READMYSQL_STR(CATEGORYID, ctid);
		READMYSQL_STR(EQUIPNAME, eqnm);
	}

	char gmt0now[20];
	GmtNow(gmt0now);

	if (strlen(ctid))
	{
		sprintf (sql, "INSERT INTO t41_equipment_inventory (SHIPID, EQUIPID, CATEGORYID, WAREHOUSEID, INVENTORY, INVENTORY_TIME, INVENTORY_USER)\
					VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s')", shipid.c_str(), eqid.c_str(), ctid, whid.c_str(), inv.c_str(), gmt0now, pUid);
		
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		sprintf (sql, "INSERT INTO t41_equipment_surplus (SHIPID, EQUIPID, CATEGORYID, EQUIPNAME, WAREHOUSE, SURPLUS)\
							VALUES ('%s', '%s', '%s', '%s', '%s', '%s')", shipid.c_str(), eqid.c_str(), ctid, SqlReplace(eqnm).c_str(), whid.c_str(), inv.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out << "{eid:\"0\"}";
	}
	else
	{
		out << "{eid:\"100\"}";
	}
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"1234",cid:"MU001",eqid:"",whid:"CK001",inv:"3"}
int EquipmentSvc::updEquimentMemory(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::updEquimentMemory]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");
	string cid = root.getv("cid", "");
	string eqid = root.getv("eqid", "");
	string whid = root.getv("whid", "");
	string inv = root.getv("inv", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "UPDATE t41_equipment_inventory SET INVENTORY = '%s', INVENTORY_TIME= '%s', INVENTORY_USER = '%s'\
					WHERE SHIPID = '%s' AND EQUIPID = '%s' AND CATEGORYID = '%s' AND WAREHOUSEID = '%s'", inv.c_str(), gmt0now, pUid, shipid.c_str(), eqid.c_str(), cid.c_str(), whid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	
	sprintf (sql, "UPDATE t41_equipment_surplus SET SURPLUS = '%s' \
					WHERE SHIPID = '%s' AND EQUIPID = '%s' AND CATEGORYID = '%s' AND WAREHOUSE = '%s'", inv.c_str(), shipid.c_str(), eqid.c_str(), cid.c_str(), whid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{eid:\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:""}
//[{ck:""},{cnm:""},{shipid:"123",nm:"cosco1",mi:"123456789"},{}...]
int EquipmentSvc::getSameCompShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getSameCompShip]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t4.COMPANY_KEY, t4.SHIPID, t4.MMSI, t2.name AS cnm, t3.name AS snm\
						FROM t41_seipc_company t1 JOIN t41_seipc_company t4 ON t1.COMPANY_KEY = t4.COMPANY_KEY\
						LEFT JOIN t41_company t2 ON t1.COMPANY_KEY = t2.COMPANY_KEY\
						LEFT JOIN t41_ship t3 ON t4.shipid = t3.shipid WHERE t1.shipid = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	char comp_id[100];
	char cnm[100];
	out << "[";
	while (psql->NextRow())
	{
		if(!cnt++)
		{
			READMYSQL_STR(COMPANY_KEY, comp_id);
			READMYSQL_STR(cnm, cnm);
			out << "{ck:\"" << comp_id << "\"},{cnm:\"" << JsonReplace(cnm) << "\"}";
		}
		char ship_id[100];
		char snm[100];
		int mmsi;
		READMYSQL_STR(SHIPID, ship_id);
		READMYSQL_STR(snm, snm);
		READMYSQL_INT(MMSI, mmsi, 0);
		out << ",{shipid:\"" << ship_id << "\",nm:\"" << JsonReplace(snm) << "\",mi:\"" << mmsi << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:""}
//[{tp:"1",nm:"03234"},{}...]
int EquipmentSvc::getEquimentStandard(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getEquimentStandard]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.INPUT_DATASET_TYPE, t1.INPUT_DATASET_NAME FROM t41_equipment_inputdataset t1 LEFT JOIN t41_seipc_company t2 ON t1.COMPANY_KEY = t2.COMPANY_KEY\
					WHERE t2.SHIPID = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		char tp[100] = "";
		char tpnm[100] = "";
		READMYSQL_STR(INPUT_DATASET_TYPE, tp);
		READMYSQL_STR(INPUT_DATASET_NAME, tpnm);
		if (cnt++)
			out << ",";
		out << "{tp:\"" << tp << "\",nm:\"" << JsonReplace(tpnm) << "\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{ck:"",tp:"1",nm:"03234"}
int EquipmentSvc::insertEquimentStandard(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::insertEquimentStandard]bad format:", jsonString, 1);
    string ck = root.getv("ck", "");
	string tp = root.getv("tp", "");
	string nm = root.getv("nm", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "REPLACE INTO t41_equipment_inputdataset(COMPANY_KEY, INPUT_DATASET_TYPE, INPUT_DATASET_NAME) VALUES ('%s', '%s', '%s')", ck.c_str(), tp.c_str(), SqlReplace(nm).c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{eid:\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:"",shipid:""}
int EquipmentSvc::ApplyEquipmentSvcAuthority(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[EquipmentSvc::ApplyEquipmentSvcAuthority]bad format:", jsonString, 1);
    string uid = root.getv("uid", "");
    string shipid = root.getv("shipid", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
	MySql* psql = CREATE_MYSQL;

	sprintf(sql, "INSERT INTO  t41_authen_apply (user_id, BUSTYPE, BUSID, accept, apply_dt, update_dt) VALUES ( '%s', '2', '%s', 0, '%s', '%s')", uid.c_str(), shipid.c_str() ,gmt0now, gmt0now);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:"",shipid:"",accept:"1/0",rw:\"%s\"}
int EquipmentSvc::DealWithUserApplication(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[EquipmentSvc::DealWithUserApplication]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string shipid = root.getv("shipid", "");
	int accept = atoi(root.getv("accept", ""));
	string rw = root.getv("rw", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "SELECT count(1) as num FROM t41_seipc_user t1, t41_seipc_connection t2 \
					WHERE t1.seipc_id = t2.seipc_id AND t1.is_valid = '1' AND t2.IS_VALID = '1' AND t2.ship_id = '%s' AND t1.user_id = '%s'", shipid.c_str(), pUid);
    CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	int num = 0;
    READMYSQL_INT(num, num, 0);
	if (!num)
	{
		out << "{eid:\"100\"}";
	}
	else
	{
		sprintf(sql, "UPDATE t41_authen_apply SET ACCEPT='%d', UPDATE_DT='%s', AUTHTYPE = '%s' WHERE user_id='%s' AND BUSID='%s' AND BUSTYPE = '2'", (accept==0? 2:1), gmt0now, rw.c_str(), uid.c_str(), shipid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out << "{eid:\"0\"}";
	}
	
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:"jiangtao",shipid:"S1000019"}
int EquipmentSvc::getEquipmentAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getEquipmentAuth]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string shipid = root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

    sprintf(sql, "SELECT count(1) as num FROM t41_seipc_user t1, t41_seipc_connection t2 \
					WHERE t1.seipc_id = t2.seipc_id AND t1.is_valid = '1' AND t2.IS_VALID = '1' AND t2.ship_id = '%s' AND t1.user_id = '%s'", shipid.c_str(), uid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	int num = 0;
    READMYSQL_INT(num, num, 0);
	
	if (num)
	{
		sprintf (sql, "SELECT t1.user_id, t1.accept, t1.AUTHTYPE, UNIX_TIMESTAMP(t1.UPDATE_DT) AS updt, t2.nickname \
							FROM t41_authen_apply AS t1 LEFT JOIN t00_user AS t2 ON t1.user_id = t2.user_id WHERE BUSID = '%s' AND t1.BUSTYPE = '2'", shipid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		int cnt = 0;
		out << "{appstate:\"-1\",rw:\"1\",appliers:[";
		while (psql->NextRow())
		{
			char user_id[64];
			char nickname[255];
			int tm;
			int status;
			int rw = 0;

			READMYSQL_STR(user_id, user_id);
			if (!strcmp(user_id, uid.c_str()))
				continue;
			READMYSQL_INT(updt, tm, 0);
			READMYSQL_INT(accept, status, -1);
			READMYSQL_INT(AUTHTYPE, rw, 0);
			READMYSQL_STR(nickname, nickname);
			if (cnt++)
				out << ",";
			out << "{usrId:\"" << user_id << "\",usrnk:\"" << JsonReplace(nickname) << "\",state:\"" << status << "\",updt:" << tm << ",rw:\"" << rw << "\"}";
		}
		out << "]}";
	}
	else
	{
		sprintf (sql, "SELECT ACCEPT, AUTHTYPE FROM t41_authen_apply WHERE USER_ID = '%s' AND BUSID = '%s' AND BUSTYPE = '2' ORDER BY UPDATE_DT DESC", uid.c_str(), shipid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		int flg = -1;
		int rw = -1;
		if (psql->NextRow())
		{
			READMYSQL_INT(ACCEPT, flg, -1);
			READMYSQL_INT(AUTHTYPE, rw, 0);
		}
		out << "{appstate:\"" << flg << "\",rw:\"" << rw << "\",appliers:[]}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}

// {ownerid:\"%s\",uid:\"%s\",shipid:\"%s\"}
int EquipmentSvc::delEquipmentAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::delEquipmentAuth]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
	string shipid = root.getv("shipid", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "SELECT count(1) as num FROM t41_seipc_user t1, t41_seipc_connection t2 \
					WHERE t1.seipc_id = t2.seipc_id AND t1.is_valid = '1' AND t2.IS_VALID = '1' AND t2.ship_id = '%s' AND t1.user_id = '%s'", shipid.c_str(), pUid);
    CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	int num = 0;
    READMYSQL_INT(num, num, 0);
	if (!num)
	{
		out << "{eid:\"100\"}";
	}
	else
	{
		sprintf(sql, "DELETE FROM t41_authen_apply WHERE user_id='%s' AND BUSID='%s' AND BUSTYPE = '2'", uid.c_str(), shipid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out << "{eid:\"0\"}";
	}
	
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:"",starttm:,endtm:}
//[{eqid:"",eqnm:"高压油泵",mtp:"机器型号",dhh:"",mafr:"",spe:"",sp0:"",sp1:"",incr:"",decr:"",remark:""},...]
int EquipmentSvc::getHistorySp(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getHistorySp]bad format:", jsonString, 1);

	string shipid = root.getv("shipid", "");
	int starttm = root.getv("starttm", 0);
	int endtm = root.getv("endtm", 0);

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
	
	EquipmentHisInfoMap m_eqHis;
	EquipmentHisInfoMap::iterator iter;

    sprintf(sql, "SELECT EQUIPID, EQUIPNAME, MACHINE_TYPE, SPECIFICATIONS, `DESC` FROM t41_equipment  WHERE SHIPID = '%s'", shipid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	

	while (psql->NextRow())
	{
		EquipmentHisInfo eqHis;
		READMYSQL_STR(EQUIPID, eqHis.eqid);
		if ((iter = m_eqHis.find(eqHis.eqid)) == m_eqHis.end())
		{
			READMYSQL_STR(EQUIPID, eqHis.eqid);
			READMYSQL_STR(EQUIPNAME, eqHis.eqnm);
			READMYSQL_STR(MACHINE_TYPE, eqHis.mtp);
			READMYSQL_STR(SPECIFICATIONS, eqHis.spe);
			READMYSQL_STR(DESC, eqHis.remark);
			eqHis.mafr[0] = '\0';
			eqHis.dhh[0] = '\0';
			eqHis.incr = 0.0;
			eqHis.decr = 0.0;
			eqHis.sp0 = 0.0;
			eqHis.sp1 = 0.0;
			m_eqHis.insert(make_pair(eqHis.eqid, eqHis));
		}
		else
		{
			DEBUG_LOG("[EquipmentSvc::getHistorySp] t41_equipment exp");
			RELEASE_MYSQL_RETURN(psql, 3);
		}
	}

	sprintf (sql, "SELECT EQUIPID, ORDERID, MANUFACTURER FROM t41_equipment_transaction WHERE SHIPID = '%s' \
				AND UNIX_TIMESTAMP(OP_TIME) >= '%d' AND UNIX_TIMESTAMP(OP_TIME) <= '%d' ORDER BY OP_TIME DESC ", shipid.c_str(), starttm, endtm);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while (psql->NextRow())
	{
		char eqid[64];
		READMYSQL_STR(EQUIPID, eqid);
		if ((iter = m_eqHis.find(eqid)) == m_eqHis.end())
		{
			DEBUG_LOG("[EquipmentSvc::getHistorySp] t41_equipment_transaction exp");
			RELEASE_MYSQL_RETURN(psql, 3);
		}
		else if (!strlen((*iter).second.dhh) && !strlen((*iter).second.mafr))
		{
			READMYSQL_STR(ORDERID, (*iter).second.dhh);
			READMYSQL_STR(MANUFACTURER, (*iter).second.mafr);
		}
	}

	sprintf (sql, "SELECT EQUIP_ID, SURPLUS, INCREASE, DECREASE, UNIX_TIMESTAMP(CREATE_DT) as cdt FROM t41_equipment_surplus_history  \
					WHERE UNIX_TIMESTAMP(CREATE_DT) >= '%d' AND UNIX_TIMESTAMP(CREATE_DT) <= '%d' AND SHIPID = '%s'", starttm, endtm, shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while (psql->NextRow())
	{
		char eqid[64];
		int cdt = 0;
		int dif_cdt0 = -1;
		int dif_cdt1 = -1;
		READMYSQL_STR(EQUIP_ID, eqid);
		READMYSQL_INT(cdt, cdt, 0);
		if ((iter = m_eqHis.find(eqid)) == m_eqHis.end())
		{
			DEBUG_LOG("[EquipmentSvc::getHistorySp] t41_equipment_surplus_history exp");
			RELEASE_MYSQL_RETURN(psql, 3);
		}
		else
		{			
			if (dif_cdt0 == -1 || dif_cdt0 > cdt - starttm)
			{
				READMYSQL_DOUBLE(SURPLUS, (*iter).second.sp0, 0.0);
				dif_cdt0 = cdt - starttm;
			}
			if (dif_cdt1 == -1 || dif_cdt1 > endtm - cdt)
			{
				READMYSQL_DOUBLE(SURPLUS, (*iter).second.sp1, 0.0);
				dif_cdt1 = endtm - cdt;
			}
			double _incr, _decr;
			READMYSQL_DOUBLE(INCREASE, _incr, 0.0);
			READMYSQL_DOUBLE(DECREASE, _decr, 0.0);
			(*iter).second.incr += _incr;
			(*iter).second.decr += _decr;
		}
	}

	out << "[";
	for (iter = m_eqHis.begin(); iter != m_eqHis.end(); iter++)
	{
		if (iter != m_eqHis.begin())
			out << ",";
		out << "{eqid:\"" << (*iter).second.eqid << "\",eqnm:\"" << JsonReplace((*iter).second.eqnm) << "\",mtp:\"" << JsonReplace((*iter).second.mtp) 
			<< "\",dhh:\"" << (*iter).second.dhh << "\",mafr:\"" << (*iter).second.mafr << "\",spe:\"" << JsonReplace((*iter).second.spe) 
			<< "\",sp0:\"" << (*iter).second.sp0 << "\",sp1:\"" << (*iter).second.sp1 << "\",incr:\"" << (*iter).second.incr 
			<< "\",decr:\"" << (*iter).second.decr << "\",remark:\"" << JsonReplace((*iter).second.remark) << "\"}";
	}
	out << "]";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int EquipmentSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	time_t t = time(0);
	struct tm *ptr = localtime(&t);
	if (ptr->tm_hour == 23 && ptr->tm_min == 59)
	{
		SYSTEM_LOG("[EquipmentSvc::handle_timeout] begin deal surplus time:%d==============", (int)t);
		MySql* psql = CREATE_MYSQL;
#ifdef WIN32
		Sleep(60);
#else
		sleep(60);
#endif
		int t0 = (int)t - (ptr->tm_sec + ptr->tm_min*60 + ptr->tm_hour*3600);
		int t1 = (int)t - ptr->tm_sec + 59;

		char sql[1024 * 100];

		SurplusHisInfoMap m_spHis;
		SurplusHisInfoMap::iterator iter;		

		sprintf (sql, "SELECT SHIPID, EQUIPID, CATEGORYID, SURPLUS FROM t41_equipment_surplus");
		CHECK_MYSQL_STATUS(psql->Query(sql), 0);
		while (psql->NextRow())
		{
			SurplusHistory spHis;
			char key[100] = "";
			READMYSQL_STR(SHIPID, spHis.shipid);
			READMYSQL_STR(EQUIPID, spHis.eqid);			
			if ((iter = m_spHis.find(key)) == m_spHis.end())
			{
				READMYSQL_DOUBLE(SURPLUS, spHis.surplus, 0.0);
				READMYSQL_STR(CATEGORYID, spHis.cid);
				spHis.incr = 0.0;
				spHis.decr = 0.0;
				m_spHis.insert(make_pair(key, spHis));
			}
			else
			{
				READMYSQL_DOUBLE(SURPLUS, spHis.surplus, 0.0); //累计各个仓库的余量
				(*iter).second.surplus += spHis.surplus;
			}
		}

		//sprintf(sql, "SELECT SHIPID, EQUIPID, OP_TYPE, QUANTITY FROM t41_equipment_transaction t\
		//				WHERE UNIX_TIMESTAMP(OP_TIME) >= '%d' AND UNIX_TIMESTAMP(OP_TIME) <= '%d' ORDER BY OP_TIME DESC", t0, t1);
		//CHECK_MYSQL_STATUS(psql->Query(sql), 0);

		//while (psql->NextRow())
		//{
		//	SurplusHistory spHis;
		//	char key[100] = "";
		//	double quantity = 0.0;
		//	int opt = -1;

		//	READMYSQL_STR(SHIPID, spHis.shipid);
		//	READMYSQL_STR(EQUIPID, spHis.eqid);
		//	READMYSQL_DOUBLE(QUANTITY, quantity, 0.0);
		//	READMYSQL_INT(OP_TYPE, opt, 1);//1:in 2:out

		//	sprintf (key, "%s@%s", spHis.shipid, spHis.eqid);
		//	if ((iter = m_spHis.find(key)) == m_spHis.end())
		//	{
		//		SYSTEM_LOG("[EquipmentSvc::handle_timeout] exp: key:%s ==============", key);
		//	}
		//	else
		//	{
		//		if (opt == 1)
		//			(*iter).second.incr += quantity;
		//		else
		//			(*iter).second.decr += quantity;
		//	}
		//}
		
		int len = sprintf (sql, "INSERT IGNORE INTO t41_equipment_surplus_history (SHIPID, EQUIP_ID, CATEGORY_ID, INCREASE, DECREASE, SURPLUS, CREATE_DT) VALUES ");
		for (iter = m_spHis.begin(); iter != m_spHis.end(); iter++)
		{
			if (iter != m_spHis.begin())
				len += sprintf (sql + len, ",");
			len += sprintf (sql + len, "('%s', '%s', '%s', '%f', '%f', '%f', FROM_UNIXTIME('%d'))", 
				(*iter).second.shipid, (*iter).second.eqid, (*iter).second.cid, (*iter).second.incr, (*iter).second.decr, (*iter).second.surplus, t1);
		}
		
		if (m_spHis.size())
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 0);
		
		SYSTEM_LOG("[EquipmentSvc::handle_timeout] end deal surplus ==============");
		RELEASE_MYSQL_RETURN(psql, 0);
	}
	return 0;
}

//{comp:"",shipid:"",eqid:"",crdt:"",WarehouseID:""}
int EquipmentSvc::updEquipmentWh(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::updEquipmentWh]bad format:", jsonString, 1);

    string comp = root.getv("comp", "");
	string shipid = root.getv("shipid", "");
	string eqid = root.getv("eqid", "");
	string crdt = root.getv("crdt", "");
	string WarehouseID = root.getv("WarehouseID", "");

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "UPDATE t41_equipment_transaction SET WAREHOUSEID = '%s' WHERE COMPANY_KEY = '%s' AND SHIPID = '%s' AND EQUIPID = '%s' AND UNIX_TIMESTAMP(CREATE_DT) = '%s'", 
					WarehouseID.c_str(), comp.c_str(), shipid.c_str(), eqid.c_str(), crdt.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{eid:\"0\"}";
	
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{eqid: "EQ00078",picd:"00001190.jpg"}
int EquipmentSvc::updEqPict(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getEqPict]bad format:", jsonString, 1);

    string eqid = root.getv("eqid", "");
	string picd = root.getv("picd", "");
	Tokens picids = StrSplit(picd, "|");
	picd = ToStr(picids, ";");

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "SELECT PICTUREID FROM t41_equipment WHERE EQUIPID = '%s'", eqid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char oldpicid[1024] = "";
	while (psql->NextRow() && !strlen(oldpicid))
	{
		READMYSQL_STR(PICTUREID, oldpicid);
	}
	if (strlen(oldpicid))
		sprintf (sql, "UPDATE t41_equipment SET PICTUREID = '%s;%s' WHERE EQUIPID = '%s'", picd.c_str(), oldpicid, eqid.c_str());
	else
		sprintf (sql, "REPLACE INTO t41_equipment (SHIPID, EQUIPID, PICTUREID) VALUES ('', '%s', '%s')", eqid.c_str(), picd.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{eid:\"0\"}";
	
    RELEASE_MYSQL_RETURN(psql, 0);
}

int EquipmentSvc::getEqPict(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::getEqPict]bad format:", jsonString, 1);

    string eqid = root.getv("eqid", "");

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "SELECT PICTUREID FROM t41_equipment WHERE EQUIPID = '%s'", eqid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	char picid[1024] = "";
	while (psql->NextRow() && !strlen(picid))
	{
		READMYSQL_STR(PICTUREID, picid);
	}

	Tokens picids = StrSplit(picid, ";");
	string picd = ToStr(picids, "|");
	out << "{picid:\"" << picd << "\"}";
	
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{eqid: "",picd:""}
int EquipmentSvc::delEqPict(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[EquipmentSvc::delEqPict]bad format:", jsonString, 1);

    string eqid = root.getv("eqid", "");
	string picd = root.getv("picd", "");

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
	sprintf(sql, "SELECT PICTUREID FROM t41_equipment WHERE EQUIPID = '%s' AND PICTUREID LIKE '%%%s%%'", eqid.c_str(), picd.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char oldpicid[1024] = "";
	while (psql->NextRow() && !strlen(oldpicid))
	{
		READMYSQL_STR(PICTUREID, oldpicid);
	}

	Tokens picids = StrSplit(string(oldpicid), ";");
	for (Tokens::iterator it = picids.begin(); it != picids.end(); it++)
	{
		if ((*it) == picd)
		{
			picids.erase(it);
			break;
		}
	}

	sprintf (sql, "UPDATE t41_equipment SET PICTUREID = '%s' WHERE EQUIPID = '%s'", ToStr(picids, ";").c_str(), eqid.c_str());	
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		
    RELEASE_MYSQL_RETURN(psql, 0);
}

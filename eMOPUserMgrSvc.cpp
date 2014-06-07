#include "blmcom_head.h"
#include "eMOPUserMgrSvc.h"
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

IMPLEMENT_SERVICE_MAP(eMOPUserMgrSvc)

eMOPUserMgrSvc::eMOPUserMgrSvc()
{

}

eMOPUserMgrSvc::~eMOPUserMgrSvc()
{

}

bool eMOPUserMgrSvc::Start()
{
	if(!g_MessageService::instance()->RegisterCmd(MID_EMOP_USERMGR, this))
		return false;

	int interval = 60;
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
	if(timerId <= 0)
		return false;

	SERVICE_MAP(0x01,eMOPUserMgrSvc,GetAllUserGeneralInfo_C);
	SERVICE_MAP(0x02,eMOPUserMgrSvc,GetGetAllRoles);
	SERVICE_MAP(0x03,eMOPUserMgrSvc,GetRoleAuthorities);
	SERVICE_MAP(0x04,eMOPUserMgrSvc,SetUserRoleName);
	SERVICE_MAP(0x05,eMOPUserMgrSvc,NewUserRole);
	SERVICE_MAP(0x06,eMOPUserMgrSvc,DelUserRole);
	SERVICE_MAP(0x07,eMOPUserMgrSvc,ModifyRoleAuthorities);
	SERVICE_MAP(0x08,eMOPUserMgrSvc,NewDpt);
	SERVICE_MAP(0x09,eMOPUserMgrSvc,UpdateDpt);

	SERVICE_MAP(0x0a,eMOPUserMgrSvc,NewDptShip);
	SERVICE_MAP(0x0b,eMOPUserMgrSvc,DelDptShip);
	SERVICE_MAP(0x0c,eMOPUserMgrSvc,DelDpt);
	SERVICE_MAP(0x0d,eMOPUserMgrSvc,NewUser);
	SERVICE_MAP(0x0e,eMOPUserMgrSvc,DelUser);
	SERVICE_MAP(0x0f,eMOPUserMgrSvc,ModifyUserDpt);
	SERVICE_MAP(0x10,eMOPUserMgrSvc,GetUserInfo);
	SERVICE_MAP(0x11,eMOPUserMgrSvc,AddUserShip);
	SERVICE_MAP(0x12,eMOPUserMgrSvc,DelUserShip);
	SERVICE_MAP(0x13,eMOPUserMgrSvc,GetAllUserShips);
	SERVICE_MAP(0x14,eMOPUserMgrSvc,ResetPwd);
	SERVICE_MAP(0x15,eMOPUserMgrSvc,EnableUser);
	SERVICE_MAP(0x16,eMOPUserMgrSvc,SearchUser);
	SERVICE_MAP(0x17,eMOPUserMgrSvc,SearchShipDpt);
	SERVICE_MAP(0x18,eMOPUserMgrSvc,GetAllUserGeneralInfo_S);
	SERVICE_MAP(0x19,eMOPUserMgrSvc,GetUserRoles);
	SERVICE_MAP(0x20,eMOPUserMgrSvc,GetUserDpt);
	SERVICE_MAP(0x21,eMOPUserMgrSvc,GetCompanyDpt);
	SERVICE_MAP(0x22,eMOPUserMgrSvc,GetDptUsers);
	SERVICE_MAP(0x23,eMOPUserMgrSvc,GetRoleUsers);
	SERVICE_MAP(0x24,eMOPUserMgrSvc,UpdateUserState);
	SERVICE_MAP(0x25,eMOPUserMgrSvc,AddOrDelUserRole);
	SERVICE_MAP(0x26,eMOPUserMgrSvc,AddOrDelUserShip);



	//船舶资料部分
	SERVICE_MAP(0xf1,eMOPUserMgrSvc,GetShipDetailInfo);
	SERVICE_MAP(0xf2,eMOPUserMgrSvc,GetShipHistoryList);
	SERVICE_MAP(0xf3,eMOPUserMgrSvc,GetShipCommunicationList);
	SERVICE_MAP(0xf4,eMOPUserMgrSvc,GetShipEquipmentList);
	//
	SERVICE_MAP(0xf5,eMOPUserMgrSvc,GetShipBaseData);
	SERVICE_MAP(0xf6,eMOPUserMgrSvc,GetShipDetail1);
	SERVICE_MAP(0xf7,eMOPUserMgrSvc,GetShipDetail2);
	SERVICE_MAP(0xf8,eMOPUserMgrSvc,GetShipDetail3);
	SERVICE_MAP(0xf9,eMOPUserMgrSvc,GetShipEquipments);

	DEBUG_LOG("[eMOPUserMgrSvc::Start] OK......................................");
	//std::stringstream out;
	//delEqPict("libh", "{eqid:\"EQ00031\",picd:\"00001261.png\"}",out);
	return true;
}
//
int eMOPUserMgrSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{

	return 0;
}
// 根据userid获取公司id
int eMOPUserMgrSvc::GetCorpInfo(string &o_strCorpid,string &o_strCorpName,string &i_strUid)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t2.COMPANY_KEY,t2.NAME FROM blm_emop_etl.t50_emop_users t1 LEFT JOIN blm_emop_etl.t41_company t2 ON t1.COMPANY_KEY=t2.COMPANY_KEY WHERE t1.USER_ID='%s'",i_strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char id[100] = "";
	char na[100] = "";
	if(psql->NextRow())
	{
		READMYSQL_STR(COMPANY_KEY, id);
		READMYSQL_STR(NAME, na);
		o_strCorpid=id;
		o_strCorpName=na;
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 获取公司所有部门[{na:"",dk:"",dt:"",users:[]],dfleet:[],cfleet[]}
int eMOPUserMgrSvc::GetCorpDpt(std::stringstream& out,string &i_strCorpId,bool i_bShip)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select DEPARTMENT_CODE,NAME_CN,SHIP_FLAG from blm_emop_etl.t50_emop_department_code where REGISTED_OBJECT_ID='%s'",i_strCorpId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[100] = "";
	char name[100]="";
	int iFlag=-1;
	int cnt = 0;
	string strKey;
	while (psql->NextRow())
	{
		READMYSQL_STR(DEPARTMENT_CODE, id);
		READMYSQL_STR(NAME_CN, name);
		READMYSQL_INT(SHIP_FLAG, iFlag,-1);
		if (cnt++)
			out << ",";
		out << "{na:\"" << name << "\",dk:\"" << id<<"\",dt:"<<iFlag<<",users:[";
		strKey=id;
		GeDptUser(out,strKey,i_bShip);
		out<<"]";
		if(!i_bShip)
		{
			out<<",dfleet:[";
			GetDptShips(out,strKey);
			out<<"]";
		}
		out<<"}";
	}	
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 获取部门所有用户{uid:"",ut:"",ships:[{sn:
int eMOPUserMgrSvc::GeDptUser(std::stringstream& out,string &i_strDptId,bool i_bShip)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select USER_ID,VALID_FLAG from blm_emop_etl.t50_emop_users where DEPARTMENT_CODE='%s'",i_strDptId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[100] = "";
	int iFlag=-1;
	int cnt = 0;
	string strKey;
	while (psql->NextRow())
	{
		READMYSQL_STR(USER_ID, id);
		READMYSQL_INT(VALID_FLAG, iFlag,-1);
		if (cnt++)
			out << ",";
		out << "{uid:\"" << id<<"\",ut:"<<iFlag;
		if(!i_bShip)
		{
			out<<",ships:[";
			strKey=id;
			GetUserShips(out,strKey);
			out<<"]";
		}
		out<<"}";		
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 获取用户所有船舶{sn:"",sid:""}
int eMOPUserMgrSvc::GetUserShips(std::stringstream& out,string &i_strUId)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select t2.SHIPID,t2.NAME_CN,t2.NAME_EN from blm_emop_etl.t50_emop_registered_user_fleet t1 left join blm_emop_etl.t41_emop_ship t2 on t1.SHIPID=t2.SHIPID where t1.USER_ID='%s'",i_strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[100] = "";
	char cn[100] = "";
	char en[100] = "";
	int cnt = 0;
	string strTmp;
	while (psql->NextRow())
	{
		READMYSQL_STR(SHIPID, id);
		READMYSQL_STR(NAME_CN, cn);
		READMYSQL_STR(NAME_EN, en);
		strTmp=cn;
		if(strTmp.empty())
			strTmp=en;
		if (cnt++)
			out << ",";
		out << "{sn:\"" << strTmp.c_str()<<"\",sid:\""<<id<<"\"}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 获取部门船舶
int eMOPUserMgrSvc::GetDptShips(std::stringstream& out,string &i_strDptId)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t2.SHIPID,t2.NAME_CN,t2.NAME_EN FROM blm_emop_etl.t50_emop_depart_authorities t1 LEFT JOIN blm_emop_etl.t41_emop_ship t2 ON t1.SHIPID=t2.SHIPID WHERE t1.DEPARTMENT_CODE='%s'",i_strDptId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[100] = "";
	char cn[100] = "";
	char en[100] = "";
	int cnt = 0;
	string strTmp;
	while (psql->NextRow())
	{
		READMYSQL_STR(SHIPID, id);
		READMYSQL_STR(NAME_CN, cn);
		READMYSQL_STR(NAME_EN, en);
		strTmp=cn;
		if(strTmp.empty())
			strTmp=en;
		if (cnt++)
			out << ",";
		out << "{sn:\"" << strTmp.c_str()<<"\",sid:\""<<id<<"\"}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 获取公司船舶
int eMOPUserMgrSvc::GetCorpShips(std::stringstream& out,string &i_strCorpId)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t2.SHIPID,t2.NAME_CN,t2.NAME_EN FROM blm_emop_etl.t50_emop_registed_fleet t1 LEFT JOIN blm_emop_etl.t41_emop_ship t2 ON t1.SHIPID=t2.SHIPID WHERE t1.REGISTED_COMP_ID='%s'",i_strCorpId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[100] = "";
	char cn[100] = "";
	char en[100] = "";
	int cnt = 0;
	string strTmp;
	while (psql->NextRow())
	{
		READMYSQL_STR(SHIPID, id);
		READMYSQL_STR(NAME_CN, cn);
		READMYSQL_STR(NAME_EN, en);
		strTmp=cn;
		if(strTmp.empty())
			strTmp=en;
		if (cnt++)
			out << ",";
		out << "{sn:\"" << strTmp.c_str()<<"\",sid:\""<<id<<"\"}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6501  (mt=0x65,st=0x01)
//{seq:”s001”,uid: "administrator"}
//{seq:””,com:"",ck:"",depts:[{na:"",dk:"",dt:"",users:[{uid:"",ut:"",ships:[{sn:"",sid:""},{ }]}],dfleet:[{sn:"",sid:""},{sn:"",sid:\"s002\"}]},{na:"",dk:"",dt:"",users:[{uid:\"用户3\",ut:\"1\"},{uid:\"用户4\",ut:\"0\"}]}],cfleet:[{sn:\"船舶1\",sid:\"s001\"},{sn:\"船舶2\",sid:\"s002\"}]}
int eMOPUserMgrSvc::GetAllUserGeneralInfo_C(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetAllUserGeneralInfo_C]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strUId=root.getv("uid", "");
	string strCorpId,strCorpName;
	// 根据userid获取公司id
	GetCorpInfo(strCorpId,strCorpName,strUId);
	out<<"{seq:\""<<strSeq.c_str()<<"\",com:\""<<strCorpName.c_str()<<"\",ck:\""<<strCorpId.c_str()<<"\",depts:[";
	// 获取公司所有部门
	GetCorpDpt(out,strCorpId);
	out<<"]";
	// 获取公司船舶
	out<<",cfleet:[";
	GetCorpShips(out,strCorpId);
	out<<"]}";

	return 0;
}
//
//CmdId:0x6502  (mt=0x65,st=0x02)
//{seq:”s002”,cskey: "S001"}
//{seq:”s002”,roles:[{na:”administrator”,rk:”101”,rt:”0”},{..},..]}
int eMOPUserMgrSvc::GetGetAllRoles(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetGetAllRoles]bad format:", jsonString, 1);
	string strCskey = root.getv("cskey", "");	
	string strSeq= root.getv("seq", "");
	out<< "{seq:\""<<strSeq.c_str()<<"\",roles:[";	
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"select * from blm_emop_etl.t50_emop_user_role where REGISTED_COMP_ID='%s'",strCskey.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char namecn[128]="";
	char rk[100] = "";
	short rt=-1;
	int cnt = 0;
	while(psql->NextRow())
	{
		READMYSQL_STR(ROLE_ID, rk);
		READMYSQL_STR(NAME_CN, namecn);
		READMYSQL_INT(BOARD_FLAG,rt,-1);
		if (cnt++)
			out << ",";
		out << "{na:\"" << namecn << "\",rk:\"" << rk<<"\",rt:"<<rt<<"}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6503  (mt=0x65,st=0x03)
//{uid: "administrator",rkey:”101”}
//{auth:”eMOP”,”key:”001”,st:”1”,pm”:[{na:””,key:””,st:1,cm:[{na:””,key:””,st:1,rm:[{na:””,key:””,st:1},{..}..]},{..}..]},{..}..]}
int eMOPUserMgrSvc::GetRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetRoleAuthorities]bad format:", jsonString, 1);
	string strRkey= root.getv("rkey", "");
	string strSeq= root.getv("seq", "");
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";	
	out<< "{seq:\""<<strSeq.c_str()<<"\",auth:\"eMOP\",key:\"001\""<<",pm:[";
	sprintf (sql, "select * from blm_emop_etl.t50_emop_mod_code");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char na[128]="";
	char key[100] = "";
	short st=0;
	short rst=0;
	int cnt = 0;
	string strKey;
	vector <KeyValue>vecT1;
	KeyValue kv;
	while(psql->NextRow())
	{
		READMYSQL_STR(MAIN_MODULE_ID, key);
		READMYSQL_STR(NAME_CN, na);
		kv.m_strKey=key;
		kv.m_strValue=na;
		vecT1.push_back(kv);
		/*if (cnt++)
		out << ",";
		out << "{na:\"" << na << "\",key:\"" << key<<"\",cm:[";
		strKey=key;
		GetSubData(out,st,strKey,strRkey);
		if(st==1)
		rst=1;
		out<<"]";
		out <<",st:"<<st<<"}";*/
	}
	for(int i=0;i<vecT1.size();i++)
	{
		if (cnt++)
			out << ",";
		out << "{na:\"" << vecT1.at(i).m_strValue.c_str() << "\",key:\"" << vecT1.at(i).m_strKey.c_str()<<"\",cm:[";
		//		strKey=key;
		GetSubData(out,st,vecT1.at(i).m_strKey,strRkey,psql);
		if(st==1)
			rst=1;
		out<<"]";
		out <<",st:"<<st<<"}";
	}
	out<<"]";
	out <<",st:"<<rst<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
int eMOPUserMgrSvc::GetSubData(std::stringstream& out,short &o_nSt,string &i_strKey,string &i_strRoleid,MySql* psql )
{
	//	MySql* psql=CREATE_MYSQL;	
	char sql[1024] = "";	
	sprintf (sql, "select * from blm_emop_etl.t50_emop_sub_mod_code where MAIN_MODULE_ID='%s'",i_strKey.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char na[128]="";
	char key[100] = "";	
	string strKey;
	int cnt = 0;
	vector <KeyValue>vecT1;
	KeyValue kv;
	while(psql->NextRow())
	{
		READMYSQL_STR(SUB_MODULE_CODE, key);
		READMYSQL_STR(NAME_CN, na);
		kv.m_strKey=key;
		kv.m_strValue=na;
		vecT1.push_back(kv);
		/*if (cnt++)
		out << ",";
		out << "{na:\"" << na << "\",key:\"" << key<<"\",rm:[";
		strKey=key;
		GetFuncData(out,o_nSt,strKey,i_strRoleid,psql);
		out<<"]";
		out<<",st:"<<o_nSt<<"}";*/

	}
	for(int i=0;i<vecT1.size();i++)
	{
		if (cnt++)
			out << ",";
		out << "{na:\"" << vecT1.at(i).m_strValue.c_str()  << "\",key:\"" << vecT1.at(i).m_strKey.c_str() <<"\",rm:[";
		//		strKey=key;
		GetFuncData(out,o_nSt,vecT1.at(i).m_strKey,i_strRoleid,psql);
		out<<"]";
		out<<",st:"<<o_nSt<<"}";
	}

	//	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
int eMOPUserMgrSvc::GetFuncData(std::stringstream& out,short &o_nSt,string &i_strKey,string &i_strRoleid,MySql* psql)
{
	//	MySql* psql=CREATE_MYSQL;
	//	MySql* pTmp=CREATE_MYSQL;
	char sql[1024] = "";	
	sprintf (sql, "select * from blm_emop_etl.t50_emop_mod_func where SUB_MODULE_CODE='%s'",i_strKey.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char na[128]="";
	char key[100] = "";
	short st=0;
	int cnt = 0;		
	vector <KeyValue>vecT1;
	KeyValue kv;
	while(psql->NextRow())
	{
		READMYSQL_STR(EMOP_FUNCTION_CODE, key);
		READMYSQL_STR(NAME_CN, na);
		kv.m_strKey=key;
		kv.m_strValue=na;
		vecT1.push_back(kv);
		/*sprintf (sql, "select * from blm_emop_etl.t50_emop_role_authorities where ROLE_ID='%s' and EMOP_FUNCTION_CODE='%s'",i_strRoleid.c_str(),key);
		CHECK_MYSQL_STATUS(pTmp->Query(sql), 3);
		if(pTmp->GetRowCount()>0)
		{
		st=1;
		o_nSt=1;
		}
		else
		st=0;
		if (cnt++)
		out << ",";
		out << "{na:\"" << na << "\",key:\"" << key<<"\",st:"<<st<<"}";*/
	}
	for(int i=0;i<vecT1.size();i++)
	{
		sprintf (sql, "select * from blm_emop_etl.t50_emop_role_authorities where ROLE_ID='%s' and EMOP_FUNCTION_CODE='%s'",i_strRoleid.c_str(),vecT1.at(i).m_strKey.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->GetRowCount()>0)
		{
			st=1;
			o_nSt=1;
		}
		else
			st=0;
		if (cnt++)
			out << ",";
		out << "{na:\"" << vecT1.at(i).m_strValue.c_str() << "\",key:\"" << vecT1.at(i).m_strKey.c_str()<<"\",st:"<<st<<"}";
	}
	//	RELEASE_MYSQL_RETURN(pTmp, 0);
	//	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//
// CmdId:0x6504  (mt=0x65,st=0x04)
// {seq:”s004”,rkey:”r001”,rn:”大管轮”,rtype:0} 
// {seq:”s004”,eid：0}
int eMOPUserMgrSvc::SetUserRoleName(const char* pUid, const char* jsonString, std::stringstream& out)
{// 船和模板是否分开
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::SetUserRoleName]bad format:", jsonString, 1);
	string strRk= root.getv("rkey", "");
	string strRn= root.getv("rn", "");
	string strSeq= root.getv("seq", "");
	int roletype= root.getv("rtype", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE blm_emop_etl.t50_emop_user_role SET NAME_CN = '%s',BOARD_FLAG=%d WHERE ROLE_ID = '%s'",\
		strRn.c_str(), roletype,strRk.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//CmdId:0x6505  (mt=0x65,st=0x05)
//{seq:”s005”,uid:”admin”,rn:”轮机长”,cskey:”S001”,rtype:0}
//{seq:”s005”,eid:0,rkey:”r001”}
int eMOPUserMgrSvc::NewUserRole(const char* pUid, const char* jsonString, std::stringstream& out)
{ //
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::NewUserRole]bad format:", jsonString, 1);
	string strRn = root.getv("rn", "");
	string strCskey = root.getv("ckey", "");
	int iType=root.getv("rtype", -1);
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 

	string strRoleId="ROLE";
	char szTmp[32];
	long ltime =time(0);
	sprintf(szTmp,"%d",ltime);
	strRoleId+=szTmp;
	sprintf (sql, "insert into blm_emop_etl.t50_emop_user_role(ROLE_ID,NAME_CN, NAME_EN,BOARD_FLAG,REGISTED_COMP_ID) values ('%s','%s','%s','%d','%s')",\
		strRoleId.c_str(),strRn.c_str(), strRn.c_str(), iType,strCskey.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	/*sprintf (sql, "insert into blm_emop_etl.t50_emop_user_authorities(ROLE_ID,USER_ID) values ('%s','%s')",\
	strRoleId.c_str(),strUId.c_str());	 	
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);*/
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",rkey:\""<<strRoleId<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6506  (mt=0x65,st=0x06)
//{seq:”s006”,uid:”admin”,rkey:”r001”}
//{seq:”s006”,eid:0,code:0}
int eMOPUserMgrSvc::DelUserRole(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::DelUserRole]bad format:", jsonString, 1);
	string strRkey = root.getv("rkey", "");
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select * from blm_emop_etl.t50_emop_user_authorities where ROLE_ID='%s'",strRkey.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->GetRowCount()>0)//如果该角色下有用户不能删除
	{
		out<< "{eid:-1,"<<"seq:\""<<strSeq.c_str()<<"\",code:1"<<"}";
	}
	else
	{
		sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_user_role WHERE ROLE_ID = '%s'", strRkey.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_user_authorities WHERE ROLE_ID = '%s'", strRkey.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:0"<<"}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6507  (mt=0x65,st=0x07)
//{seq:”s007”,uid:””,rk:”r001”,sm:”sm01,sm02,sm03”}
//{seq:”s007”,eid:0}
int eMOPUserMgrSvc::ModifyRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out)
{// 
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::ModifyRoleAuthorities]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strRk= root.getv("rk", "");
	string strSm= root.getv("sm", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_role_authorities WHERE ROLE_ID = '%s'", strRk.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	Tokens svrList;	
	if(strSm.find(",") != string::npos)
	{					 
		svrList = StrSplit(strSm, ",");
		for(int i=0; i<(int)svrList.size(); i++)
		{
			sprintf (sql, "insert into blm_emop_etl.t50_emop_role_authorities(ROLE_ID,EMOP_FUNCTION_CODE) values ('%s','%s')", strRk.c_str(),svrList[i].c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}
	else
	{
		sprintf (sql, "insert into blm_emop_etl.t50_emop_role_authorities(ROLE_ID,EMOP_FUNCTION_CODE) values ('%s','%s')", strRk.c_str(),strSm.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6508 (mt=0x65,st=0x08)
//{seq:”s008”,cskey:”C0904201”,did:"",dept:”新部门N”,type:”0”,dtype:”0”}
//{seq:”s008”,eid:0,key:”d001”}
int eMOPUserMgrSvc::NewDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{ 
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::NewDpt]bad format:", jsonString, 1);
	string strParentDptId= root.getv("did", "");
	string strDpt = root.getv("dept", "");
	string strCskey = root.getv("cskey", "");//公司/船舶key
	int iType=root.getv("type", -1);//（一般部门）；1（特殊部门）
	int iDType=root.getv("dtype", -1);//岸端，1船端
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 

	string strDptId="DPT";
	char szTmp[32];
	long ltime =time(0);
	sprintf(szTmp,"%d",ltime);
	strDptId+=szTmp;
	sprintf (sql,\
		"insert into blm_emop_etl.t50_emop_department_code(DEPARTMENT_CODE,PARENT_DEPT_CODE,REGISTED_OBJECT_ID,NAME_CN,BOARD_FLAG,SHIP_FLAG) values ('%s','%s','%s','%s','%d','%d')",\
		strDptId.c_str(),strParentDptId.c_str(),strCskey.c_str(), strDpt.c_str(), iDType,iType);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",key:\""<<strDptId.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6509  (mt=0x65,st=0x09)
//{seq:”s009”,dkey:”d201”,dept:”修改部门名称”,type:”0”}
//{seq:”s009”,eid:0}
int eMOPUserMgrSvc::UpdateDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::UpdateDpt]bad format:", jsonString, 1);
	string strDpt = root.getv("dept", "");
	string strDkey = root.getv("dkey", "");//公司/船舶key
	int iType=root.getv("type", -1);//（一般部门）；1（特殊部门）
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE blm_emop_etl.t50_emop_department_code SET NAME_CN = '%s',SHIP_FLAG='%d' WHERE DEPARTMENT_CODE = '%s'",\
		strDpt.c_str(), iType,strDkey.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x650a  (mt=0x65,st=0x0a)
//{seq:”s010”,dkey:”d0904201”,shipid:”S001,S002,S003”}
//{seq:”s010”,eid:0}
int eMOPUserMgrSvc::NewDptShip(const char* pUid, const char* jsonString, std::stringstream& out)
{ 
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::NewDptShip]bad format:", jsonString, 1);
	string strCskey = root.getv("dkey", "");//部门id
	string strShipId = root.getv("shipid", "");//
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 

	string strDptId="DPT";
	char szTmp[32];
	long ltime =time(0);
	sprintf(szTmp,"%d",ltime);
	strDptId+=szTmp;
	sprintf (sql,\
		"insert into blm_emop_etl.t50_emop_depart_authorities(DEPARTMENT_CODE,SHIPID) values ('%s','%s')",\
		strCskey.c_str(),strShipId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);	

}

//CmdId:0x650b  (mt=0x65,st=0x0b)
//{seq:”s011”,dkey:”d0904201”,shipid:”S001”}
//{seq:”s011”,eid:0}
int eMOPUserMgrSvc::DelDptShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::DelDptShip]bad format:", jsonString, 1);
	string strDkey = root.getv("dkey", "");
	string strShipid= root.getv("shipid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_depart_authorities WHERE DEPARTMENT_CODE='%s' and SHIPID='%s'", strDkey.c_str(),strShipid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x650c  (mt=0x65,st=0x0c)
//{seq:”s012”,cskey:”c0003”,dkey:”ce323”}
//{seq:”s012”,eid:0,code:1}
int eMOPUserMgrSvc::DelDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::DelDptShip]bad format:", jsonString, 1);
	//	string strCskey = root.getv("cskey", "");
	string strDkey = root.getv("dkey", "");//部门key
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select * from blm_emop_etl.t50_emop_users where DEPARTMENT_CODE='%s'",strDkey.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->GetRowCount()>0)//如果该部门下有用户不能删除
	{
		out<< "{eid:-1,"<<"seq:\""<<strSeq.c_str()<<"\",code:1"<<"}";
	}
	else
	{
		sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_department_code WHERE DEPARTMENT_CODE = '%s'", strDkey.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:0"<<"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x650d  (mt=0x65,st=0x0d)
//{seq:”s013”,uid:””,rn:”blm”,pwd:””,tel:””,fax:””,mobile:””,email:””,st:123,et:2343,dept:””role:””,ships:””,operation:0}
//{seq:”s013”,eid:0};0添加成功
int eMOPUserMgrSvc::NewUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::NewUserRole]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");//
	string strCorpId = root.getv("compkey", "");
	string strRn = root.getv("rn", "");
	string strPwd = root.getv("pwd", "");
	string strTel = root.getv("tel", "");
	string strFax = root.getv("fax", "");
	string strMobile = root.getv("mobile", "");
	string strEmail = root.getv("email", "");
	string strSt = root.getv("st", "");
	string strEt = root.getv("et", "");
	string strDpt = root.getv("dept", "");//部门（key）
	string strRole = root.getv("role", "");//角色（key）  以‘，’号分割
	string strShip = root.getv("ships", "");//船舶（key） 以‘，’号分割
	int iOpt = root.getv("operation", -1);//Operation:0添加，1修改
	int iType=root.getv("type", -1);	
	string strSeq= root.getv("seq", "");
	//
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_user_authorities WHERE USER_ID = '%s'", strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	Tokens svrList;	
	if(strRole.find(",") != string::npos)
	{					 
		svrList = StrSplit(strRole, ",");
		for(int i=0; i<(int)svrList.size(); i++)
		{
			sprintf (sql, "insert into blm_emop_etl.t50_emop_user_authorities(USER_ID,ROLE_ID) values ('%s','%s')",\
				strUId.c_str(),svrList[i].c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}
	else if(!strRole.empty())
	{
		sprintf (sql, "insert into blm_emop_etl.t50_emop_user_authorities(USER_ID,ROLE_ID) values ('%s','%s')",\
			strUId.c_str(),strRole.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_registered_user_fleet WHERE USER_ID = '%s'", strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	if(strShip.find(",") != string::npos)
	{					 
		svrList = StrSplit(strShip, ",");
		for(int i=0; i<(int)svrList.size(); i++)
		{
			sprintf (sql, "insert into blm_emop_etl.t50_emop_registered_user_fleet(USER_ID,SHIPID) values ('%s','%s')",\
				strUId.c_str(),svrList[i].c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}
	else if(!strShip.empty())
	{
		sprintf (sql, "insert into blm_emop_etl.t50_emop_registered_user_fleet(USER_ID,SHIPID) values ('%s','%s')",\
			strUId.c_str(),strShip.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	if(iOpt==0)
	{				
		sprintf (sql, "insert into blm_emop_etl.t50_emop_users(USER_ID,PASSWORD,DEPARTMENT_CODE,NAME,TEL,FAX,MOBILE,EMAIL,START_DT,END_DT,COMPANY_KEY,VALID_FLAG) values ('%s','%s','%s','%s','%s','%s','%s','%s',%s,%s,'%s','%d')",\
			strUId.c_str(),strPwd.c_str(), strDpt.c_str(),strRn.c_str(),strTel.c_str(),strFax.c_str(),strMobile.c_str(),strEmail.c_str(),strSt.c_str(),strEt.c_str(),strCorpId.c_str(),iType);
	}
	else
	{
		sprintf (sql, "UPDATE blm_emop_etl.t50_emop_users SET DEPARTMENT_CODE='%s',NAME='%s',TEL='%s',FAX='%s',MOBILE='%s',EMAIL='%s',START_DT='%s',END_DT='%s',COMPANY_KEY='%s' WHERE USER_ID = '%s'",\
			strDpt.c_str(),strRn.c_str(),strTel.c_str(),strFax.c_str(),strMobile.c_str(),strEmail.c_str(), strSt.c_str(),strEt.c_str(),strCorpId.c_str(),strUId.c_str());
	}

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x650e  (mt=0x65,st=0x0e)
//{seq:”s014”,uid:”u000012”}
//{seq:”s014”,eid:0}
int eMOPUserMgrSvc::DelUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::DelUser]bad format:", jsonString, 1);
	//	string strCskey = root.getv("cskey", "");
	string strUid = root.getv("uid", "");//部门key
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_users WHERE USER_ID = '%s'", strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_user_authorities WHERE USER_ID = '%s'", strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_registered_user_fleet WHERE USER_ID = '%s'", strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x650f  (mt=0x65,st=0x0f)
//{seq:”s015”,uid:”u000012”,dkey:”C0904201”}
//{seq:”s015”,eid:0}
int eMOPUserMgrSvc::ModifyUserDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{// 
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::ModifyUserDpt]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strDkey= root.getv("dkey", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE blm_emop_etl.t50_emop_users SET DEPARTMENT_CODE='%s' WHERE USER_ID = '%s'",\
		strDkey.c_str(), strUId.c_str());	
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	sprintf (sql, "delete from blm_emop_etl.t50_emop_registered_user_fleet WHERE USER_ID = '%s'", strUId.c_str());	
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6510  (mt=0x65,st=0x10)
//{seq:”s016”,uid:”u000012”}
//{seq:”s016”,uid:””,rn:””,pwd:””,tel:””,fax:””,mobile:””,email:””,st:””,et:””,dept:””role:”r001,r002”,ships:”s001,s002”,type:0} 
int eMOPUserMgrSvc::GetUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetUserInfo]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"select PASSWORD,DEPARTMENT_CODE,COMPANY_KEY,NAME,TEL,FAX,MOBILE,EMAIL,VALID_FLAG,UNIX_TIMESTAMP(START_DT) as startdt,UNIX_TIMESTAMP(END_DT)as enddt from blm_emop_etl.t50_emop_users where USER_ID='%s'",strUId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 101);
	char na[128]="";
	char pwd[100] = "";
	char tel[100] = "";
	char fax[100] = "";
	char mobile[100] = "";
	char email[100] = "";
	char dept[100] = "";
	int iFlag=-1;
	int iSt=0;
	int iEt=0;
	char corpkey[100]="";

	if(psql->NextRow())
	{		
		READMYSQL_STR(PASSWORD, pwd);
		READMYSQL_STR(DEPARTMENT_CODE, dept);
		READMYSQL_STR(COMPANY_KEY, corpkey);
		READMYSQL_STR(NAME, na);
		READMYSQL_STR(TEL, tel);
		READMYSQL_STR(FAX, fax);
		READMYSQL_STR(MOBILE, mobile);
		READMYSQL_STR(EMAIL, email);
		READMYSQL_INT(VALID_FLAG,iFlag, -1);
		READMYSQL_INT(startdt, iSt,-1);
		READMYSQL_INT(enddt, iEt,-1);
		out << "{seq:\"" << strSeq.c_str() << "\",uid:\"" << strUId.c_str()<<\
			"\",rn:\"" << na<<"\",pwd:\"" << pwd<<"\",tel:\"" << tel<<"\",fax:\"" << fax<<"\",mobile:\"" << mobile<<"\",email:\"" << \
			email<<"\",st:" << iSt<<",et:" << iEt<<",compkey:\"" << corpkey<<"\",dept:\"" << dept<<"\",type:" << iFlag<<",";//compkey:”C001” 
	}
	//	sprintf(sql,\
	"select ROLE_ID from blm_emop_etl.t50_emop_user_authorities where USER_ID='%s'",strUId.c_str());
	sprintf(sql,\
		"select t1.ROLE_ID,t2.Name_CN from blm_emop_etl.T50_EMOP_USER_AUTHORITIES t1 left join blm_emop_etl.T50_EMOP_USER_ROLE t2 on t1.ROLE_ID=t2.Role_ID where t1.USER_ID='%s'",strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int cnt=0;
	string strTmp="";
	out <<"roles:[";
	while(psql->NextRow())
	{		
		READMYSQL_STR(ROLE_ID, dept);
		READMYSQL_STR(Name_CN, email);
		if (cnt++)
			out<<",";
		out<<"{id:\""<<dept<<"\",na:\""<<email<<"\"}";
	}
	out <<"],ships:[";
	sprintf(sql,\
		"select t1.SHIPID,t2.NAME_CN,t2.NAME_EN from blm_emop_etl.t50_emop_registered_user_fleet t1 left join blm_emop_etl.t41_emop_ship t2 on t1.SHIPID=t2.SHIPID where t1.USER_ID='%s'",strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	cnt=0;
	while(psql->NextRow())
	{		
		READMYSQL_STR(SHIPID, dept);
		READMYSQL_STR(NAME_CN, email);
		READMYSQL_STR(NAME_EN, mobile);
		if (cnt++)
			out<<",";
		strTmp=email;
		if(strTmp.empty())
		{
			strTmp=mobile;
		}
		out<<"{sid:\""<<dept<<"\",na:\""<<strTmp.c_str()<<"\"}";
	}
	out <<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6511  (mt=0x65,st=0x11)
//{seq:”s017”,uid:”u000012”,ship:”shipid1,shipid2”}
//{seq:”s017”,eid:0}
int eMOPUserMgrSvc::AddUserShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::AddUserShip]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");
	string strShips=root.getv("ship", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	Tokens svrList;	

	if(strShips.find(",") != string::npos)
	{					 
		svrList = StrSplit(strShips, ",");
		for(int i=0; i<(int)svrList.size(); i++)
		{
			sprintf (sql, "insert into blm_emop_etl.t50_emop_registered_user_fleet(USER_ID,SHIPID) values ('%s','%s')", strUId.c_str(),svrList[i].c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}
	else
	{
		sprintf (sql, "insert into blm_emop_etl.t50_emop_registered_user_fleet(USER_ID,SHIPID) values ('%s','%s')", strUId.c_str(),strShips.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6512  (mt=0x65,st=0x12)
//{seq:”s018”,uid:”u000012”,ship:”shipid”}
//{seq:”s018”,eid:0}0:
int eMOPUserMgrSvc::DelUserShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::DelUserShip]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");
	string strShips=root.getv("ship", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	Tokens svrList;	
	if(strShips.find(",") != string::npos)
	{					 
		svrList = StrSplit(strShips, ",");
		for(int i=0; i<(int)svrList.size(); i++)
		{
			sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_registered_user_fleet WHERE USER_ID='%s' and SHIPID='%s'", strUId.c_str(),svrList[i].c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}
	else
	{
		sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_registered_user_fleet WHERE USER_ID='%s' and SHIPID='%s'", strUId.c_str(),strShips.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6513  (mt=0x65,st=0x13)
//{seq:”s019”,uid:”admin”} 
//{seq:”s019”,ships:[{sid:”s001”,sn:”船舶1”},{sid:”s002”,sn:”船舶2”},{}]}
int eMOPUserMgrSvc::GetAllUserShips(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetAllUserShips]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",ships:[[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";//t41_emop_ship
	sprintf(sql,\
		"select t2.SHIPID,t2.NAME_CN,t2.NAME_EN,t2.MMSI,t3.SISTER_SHIPS_GROUP_ID from blm_emop_etl.t50_emop_registered_user_fleet t1 left join \
		blm_emop_etl.t41_emop_ship t2 on t1.SHIPID=t2.SHIPID left join \
		blm_emop_etl.t50_emop_sister_ships_group t3 ON t1.SHIPID=t3.SHIPID \
		where t1.USER_ID='%s' order by t3.SISTER_SHIPS_GROUP_ID",strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[128]="";
	char nc[100] = "";
	char ne[100] = "";
	int mmsi;
	char builder[100] = "";
	string strSNa;
	string strPreB,strCurB;
	int cnt=0;
	int iBe=0;
	while(psql->NextRow())
	{		
		READMYSQL_STR(SHIPID, id);
		READMYSQL_STR(NAME_CN, nc);
		READMYSQL_STR(NAME_EN, ne);
		READMYSQL_INT(MMSI, mmsi,0);
		strSNa=nc;
		if(strSNa.empty())
			strSNa=ne;
		READMYSQL_STR(SISTER_SHIPS_GROUP_ID, builder);
		strCurB=builder;
		if(strCurB!=strPreB||strCurB.empty())
		{
			if (iBe++)
			{
				out<<"],[";
				cnt=0;
				strPreB=strCurB;
			}
		}

		if (cnt++)
			out<<","; 

		out << "{sid:\"" << id << "\",sn:\"" <<strSNa.c_str()<< "\",mmsi:" <<mmsi<<"}";

	}
	out<<"]]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6516  (mt=0x65,st=0x16)
//{seq:”s020”,uid:”admin”}
//{seq:”s020”,eid:0,code:0}
int eMOPUserMgrSvc::SearchUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::SearchUser]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";//t41_emop_ship
	sprintf(sql,\
		"select USER_ID from blm_emop_etl.t50_emop_users where USER_ID='%s'",strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iCode=0;
	if(psql->GetRowCount()>0)
	{		
		iCode=1;
	}
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:"<<iCode<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6517  (mt=0x65,st=0x17)
//{seq:”s021”,shipId:”S0001”} 
//{seq:”s021”,depts:[{key:”D001”,na:”部门1”},{key:”D002”,na:”部门2”},{..}]}
int eMOPUserMgrSvc::SearchShipDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::SearchShipDpt]bad format:", jsonString, 1);
	string strShipid = root.getv("shipId", "");
	string strSeq= root.getv("seq", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",depts:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";//t41_emop_ship
	sprintf(sql,\
		"select DEPARTMENT_CODE,NAME_CN from blm_emop_etl.t50_emop_department_code where REGISTED_OBJECT_ID='%s'",strShipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[128]="";
	char na[100] = "";
	int cnt=0;
	while(psql->NextRow())
	{		
		READMYSQL_STR(DEPARTMENT_CODE, id);
		READMYSQL_STR(NAME_CN, na);
		if (cnt++)
			out<<","; 
		out << "{key:\"" << id<< "\",na:\"" <<na<<"\"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6514  (mt=0x65,st=0x14)
//{seq:”s023”,uid:”u000012”,type:0,oldpwd:”12345”,newpwd:”453121”}
//{seq:”s023”,eid:0,code:0}
int eMOPUserMgrSvc::ResetPwd(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::ResetPwd]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strOldPwd= root.getv("oldpwd", "");
	string strNewPwd= root.getv("newpwd", "");
	int iType=root.getv("type", -1);
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	if(iType==0)
	{
		sprintf(sql,\
			"select PASSWORD from blm_emop_etl.t50_emop_users where USER_ID='%s'",strUId.c_str());
		char pwd[100] = "";
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->NextRow())
		{		
			READMYSQL_STR(PASSWORD, pwd);			
			string strTmp=pwd;
			if(strTmp==strOldPwd)
			{
				sprintf (sql, "UPDATE blm_emop_etl.t50_emop_users SET PASSWORD='%s' WHERE USER_ID = '%s'",\
					strNewPwd.c_str(), strUId.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
				out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"code:0"<<"}";
			}
			else
				out<< "{eid:0=-1,"<<"seq:\""<<strSeq.c_str()<<"code:-1"<<"}";
		}
		else
		{			
			out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"code:0"<<"}";
		}
	}
	else
	{
		sprintf (sql, "UPDATE blm_emop_etl.t50_emop_users SET PASSWORD='%s' WHERE USER_ID = '%s'",\
			strNewPwd.c_str(), strUId.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:0"<<"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6515  (mt=0x65,st=0x15)
//{seq:”s024”,uid:”u000012”,usertype:”0”}
//{seq:”s024”,eid:0}
int eMOPUserMgrSvc::EnableUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::EnableUser]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	int iUType= root.getv("usertype", -1);
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";	
	sprintf (sql, "UPDATE blm_emop_etl.t50_emop_users SET VALID_FLAG='%d' WHERE USER_ID = '%s'",\
		iUType, strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6516  (mt=0x65,st=0x18)
//{seq:”s022”,uid: "administrator",key:""}
//{seq:”s022”,sn:"新厦门",sid:"s001",depts:[{na:"轮机",dk:"d001",dt:"0",us:[{uid:"用户1",ut:"0"},{uid:"用户2",ut:"1"}]},{na:"电器",dk:"d002",dt:"1",us:[{uid:"用户3",ut:"0"},{uid:"用户4",ut:"0"}]}]}
int eMOPUserMgrSvc::GetAllUserGeneralInfo_S(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetAllUserGeneralInfo_S]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strKey=root.getv("key", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",depts:[";
	// 获取公司所有部门
	GetCorpDpt(out,strKey,true);
	out<<"]}";	
	return 0;
}
// CmdId:0x6519  (mt=0x65,st=0x19)
//{seq:”s021”,uid:”admin”} 
//{seq:”000”,roles:[{id:”r001”,na:”角色1”},{id:”r002”,na:”角色2”}]}
int eMOPUserMgrSvc::GetUserRoles(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GetUserRoles::GetUserRoles]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",roles:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";//t41_emop_ship
	sprintf(sql,\
		"select t1.ROLE_ID,t2.Name_CN from blm_emop_etl.T50_EMOP_USER_AUTHORITIES t1 left join blm_emop_etl.T50_EMOP_USER_ROLE t2 on t1.ROLE_ID=t2.Role_ID where USER_ID='%s'",strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[128]="";
	char na[100] = "";
	int cnt=0;
	while(psql->NextRow())
	{		
		READMYSQL_STR(ROLE_ID, id);
		READMYSQL_STR(Name_CN, na);
		if (cnt++)
			out<<","; 
		out << "{id:\"" << id<< "\",na:\"" <<na<<"\"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6520  (mt=0x65,st=0x20)
//{seq:”s021”,uid:”admin”}
//{seq:”sdf”,id:”d001”,name:”部门1”}
int eMOPUserMgrSvc::GetUserDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GetUserRoles::GetUserDpt]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";//t41_emop_ship
	sprintf(sql,\
		"select t1.DEPARTMENT_CODE,t2.NAME_CN from blm_emop_etl.t50_emop_users t1 left join blm_emop_etl.t50_emop_department_code t2 on t1.DEPARTMENT_CODE=t2.DEPARTMENT_CODE where t1.USER_ID='%s'",strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[128]="";
	char na[100] = "";
	int cnt=0;
	if(psql->NextRow())
	{		
		READMYSQL_STR(DEPARTMENT_CODE, id);
		READMYSQL_STR(NAME_CN, na);
		if (cnt++)
			out<<","; 
		out << "id:\"" << id<< "\",na:\"" <<na<<"\"";
	}
	out<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

// 获取用户所在公司，公司部门，船舶信息
int eMOPUserMgrSvc::GetCompanyDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetCompanyDpt]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";//t41_emop_ship
	//公司信息***********************
	char companyId[100];
	char companyName[200];

	sprintf(sql,"SELECT  t_company.COMPANY_KEY,t_company.NAME \
				FROM blm_emop_etl.t41_company t_company LEFT JOIN blm_emop_etl.t50_emop_users t_user ON t_company.COMPANY_KEY=t_user.COMPANY_KEY \
				WHERE t_user.USER_ID='%s'",strUId.c_str());
	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3); 

	if(psql->NextRow()){
		READMYSQL_STR(COMPANY_KEY, companyId);
		READMYSQL_STR(NAME, companyName); 
	}else{
		out<<"{seq:\""<<strSeq<<"\",cid:\"\",cna:\"\",cdept:[], cfleet:[]}";
		return 0;
	} 
	//公司部门信息********************************
	list<Depart> companyDepartList;
	sprintf(sql,"SELECT DEPARTMENT_CODE,NAME_CN,SHIP_FLAG FROM blm_emop_etl.t50_emop_department_code\
				WHERE REGISTED_OBJECT_ID='%s' AND BOARD_FLAG=0 AND  (PARENT_DEPT_CODE IS NULL OR PARENT_DEPT_CODE='')",companyId);
	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char departid[100]="";
	char departname[200]="";
	int type=0;

	while(psql->NextRow())
	{		
		READMYSQL_STR(DEPARTMENT_CODE, departid);
		READMYSQL_STR(NAME_CN, departname);
		READMYSQL_INT(SHIP_FLAG, type,0); 
		Depart depart;
		depart.departId = departid;
		depart.departName = departname;
		depart.departType = type;

		companyDepartList.push_back(depart);
	}


	//公司二级部门信息********************************
	for(list<Depart>::iterator it=companyDepartList.begin();it!=companyDepartList.end();it++){ 
		DEBUG_LOG(it->departId.c_str());
		sprintf(sql,"SELECT DEPARTMENT_CODE,NAME_CN,SHIP_FLAG FROM blm_emop_etl.t50_emop_department_code \
					WHERE BOARD_FLAG=0 AND  PARENT_DEPT_CODE='%s'",it->departId.c_str());
		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		while(psql->NextRow())
		{		
			READMYSQL_STR(DEPARTMENT_CODE, departid);
			READMYSQL_STR(NAME_CN, departname);
			READMYSQL_INT(SHIP_FLAG, type,0);

			Depart depart;
			depart.departId = departid;
			depart.departName = departname;
			depart.departType = type;

			it->childDepart.push_back(depart);
		}

	}

	//公司船队****************************************** 
	list<ShipDepart> ships;
	char shipid[100];
	char shipName[200];
	sprintf(sql,"SELECT t_ship.SHIPID,t_ship.NAME_CN \
				FROM  blm_emop_etl.t50_emop_registed_fleet t_fleet LEFT JOIN  blm_emop_etl.t41_emop_ship t_ship ON \
				t_fleet.SHIPID=t_ship.SHIPID \
				WHERE t_fleet.REGISTED_COMP_ID='%s'",companyId);
	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{		
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(NAME_CN, shipName);

		ShipDepart ship;
		ship.shipId = shipid;
		ship.shipName = shipName; 

		ships.push_back(ship);
	}
	//公司船队部门****************************************** 
	for(list<ShipDepart>::iterator it=ships.begin();it!=ships.end();it++){ 
		sprintf(sql,"SELECT DEPARTMENT_CODE,name_CN FROM blm_emop_etl.t50_emop_department_code \
					WHERE REGISTED_OBJECT_ID='%s' AND BOARD_FLAG=1",it->shipId.c_str());
		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		while(psql->NextRow())
		{		
			READMYSQL_STR(DEPARTMENT_CODE, departid);
			READMYSQL_STR(NAME_CN, departname);

			Depart depart;
			depart.departId = departid;
			depart.departName = departname;  

			it->departList.push_back(depart);
		}
	}

	out<<"{seq:\""<<strSeq<<"\",cid:\""<<companyId<<"\",cna:\""<<companyName<<"\",cdept:[";
	//公司部门 
	for(list<Depart>::iterator it=companyDepartList.begin();it!=companyDepartList.end();it++){
		if(it!=companyDepartList.begin())
			out<<",";

		out<<"{did:\""<<it->departId<<"\",dna:\""<<it->departName<<"\",dt:"<<it->departType<<",subdept:[";
		list<Depart> &childList = it->childDepart;
		for(list<Depart>::iterator it2=childList.begin();it2!=childList.end();it2++){
			if(it2!=childList.begin())
				out<<","; 
			out<<"{did:\""<<it2->departId<<"\",dna:\""<<it2->departName<<"\",dt:"<<it2->departType<<"}";
		}

		out<<"]}";
	}
	out<<"],cfleet:[";
	//公司船队
	for(list<ShipDepart>::iterator it=ships.begin();it!=ships.end();it++){
		if(it!=ships.begin())
			out<<",";

		out<<"{sid:\""<<it->shipId<<"\",sna:\""<<it->shipName<<"\",sdept:[";
		list<Depart> &shipDepart = it->departList;
		for(list<Depart>::iterator it2=shipDepart.begin();it2!=shipDepart.end();it2++){
			if(it2!=shipDepart.begin())
				out<<","; 
			out<<"{did:\""<<it2->departId<<"\",dna:\""<<it2->departName<<"\"}";
		}

		out<<"]}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}

// 查询某角色下的所有用户
int  eMOPUserMgrSvc::GetRoleUsers(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetRoleUsers]bad format:", jsonString, 1); 
	string strRoleID = root.getv("rkey", ""); 
	string strSeq= root.getv("seq", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,"SELECT USER_ID FROM blm_emop_etl.t50_emop_user_authorities WHERE ROLE_ID='%s'",strRoleID.c_str());

	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3); 

	out<<"{seq:\""<<strSeq<<"\",users:["; 
	int idx=0;
	char uid[100] = "";
	while(psql->NextRow())
	{
		if(idx>0)
			out<<","; 
		READMYSQL_STR(USER_ID, uid);  
		out<<"\""<<uid<<"\"";

		++idx;
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
} 

// 修改用户状态
int  eMOPUserMgrSvc::UpdateUserState(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetRoleUsers]bad format:", jsonString, 1); 
	string strSeq= root.getv("seq", ""); 
	string strUid = root.getv("uid", ""); 
	int status= root.getv("status", 0); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,"UPDATE blm_emop_etl.t50_emop_users SET VALID_FLAG=%d WHERE USER_ID='%s'",status,strUid.c_str());

	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Execute(sql), 3); 

	out<<"{seq:\""<<strSeq<<"\",eid:0}"; 

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
} 
// 将已有角色添加给用户，或删除用户已经有的的角色
int  eMOPUserMgrSvc::AddOrDelUserRole(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetRoleUsers]bad format:", jsonString, 1); 
	string strSeq= root.getv("seq", ""); 
	string strUid = root.getv("uid", ""); 
	string strRolesId= root.getv("rkey", ""); 
	int type= root.getv("op", 0); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	Tokens idsVec=StrSplit(strRolesId,",");
	for(Tokens::iterator it=idsVec.begin();it!=idsVec.end();it++){ 
		if(type==0){ //添加
			sprintf(sql,"INSERT INTO blm_emop_etl.t50_emop_user_authorities(ROLE_ID,USER_ID) VALUE('%s','%s')",it->c_str(),strUid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql), 3); 
		}

		if(type==1){//删除
			sprintf(sql,"DELETE   FROM  blm_emop_etl.t50_emop_user_authorities WHERE ROLE_ID='%s' AND USER_ID='%s';",it->c_str(),strUid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql), 3); 
		} 
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}"; 

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
} 
// 将已有船舶添加给用户，或删除用户已经有的船舶
int  eMOPUserMgrSvc::AddOrDelUserShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetRoleUsers]bad format:", jsonString, 1); 
	string strSeq= root.getv("seq", ""); 
	string strUid = root.getv("uid", ""); 
	string strShipIds= root.getv("sid", ""); 
	int type= root.getv("op", 0); 

	MySql* psql = CREATE_MYSQL;
	out<<"{seq:\""<<strSeq<<"\",eid:0}"; 

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
} 

// 获取公司某部门下所有用户
int eMOPUserMgrSvc::GetDptUsers(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetCompanyDpt]bad format:", jsonString, 1);
	int reqtype = root.getv("type", 0); //0:公司、1:船舶
	string strId = root.getv("csid", "");
	string strDid = root.getv("did", ""); //空位所有部门
	string strSeq= root.getv("seq", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char appSql[1024] = "";//t41_emop_ship
	char uid[100];
	char name[200]="";
	char tel[100]="";
	char fax[100]="";
	char mobile[100]="";
	char email[100]="";
	int stime=0;
	int etime=0;
	char job[100]=""; 
	int state=0; 
	int cstype=0; // 0:岸端、1:船端

	char departId[200]="";
	char departName[200]="";

	char roleId[200]="";
	char roleName[200]=""; 

	char shipId[200]="";
	char shipName[200]=""; 

	list<User> users;
	if(reqtype==0)
	{//公司
		//用户信息
		if(!strDid.empty())
			sprintf(appSql," AND t_user.DEPARTMENT_CODE='%s'",strDid.c_str());

		sprintf(sql,"SELECT\
					t_depart.BOARD_FLAG,t_user.USER_ID,t_user.NAME,t_user.JOB,t_user.TEL,t_user.MOBILE,t_user.EMAIL,t_user.FAX,UNIX_TIMESTAMP(t_user.START_DT) AS U_START_DT,UNIX_TIMESTAMP(t_user.END_DT) AS U_END_DT,t_user.VALID_FLAG,\
					t_depart.DEPARTMENT_CODE,t_depart.NAME_CN AS DEPART_NAME\
					FROM blm_emop_etl.t50_emop_users t_user  \
					JOIN blm_emop_etl.t50_emop_department_code t_depart ON t_user.DEPARTMENT_CODE=t_depart.DEPARTMENT_CODE \
					JOIN blm_emop_etl.t50_emop_user_authorities t_auth ON  t_auth.USER_ID=t_user.USER_ID\
					WHERE t_user.COMPANY_KEY='%s' AND t_depart.BOARD_FLAG=0 %s",strId.c_str(),appSql);

		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		while(psql->NextRow())
		{  
			READMYSQL_STR(USER_ID, uid);
			READMYSQL_STR(NAME, name);
			READMYSQL_STR(JOB, job);
			READMYSQL_STR(TEL, tel);
			READMYSQL_STR(FAX, fax);
			READMYSQL_STR(USER_ID, mobile);
			READMYSQL_STR(NAME, email);
			READMYSQL_INT(U_START_DT, stime,0);
			READMYSQL_INT(U_END_DT, etime,0);
			READMYSQL_INT(VALID_FLAG, state,0); 
			READMYSQL_STR(DEPARTMENT_CODE, departId);
			READMYSQL_STR(DEPART_NAME, departName); 
			cstype=0; // 0:岸端、1:船端

			User user;
			user.uid = uid;
			user.name = name;
			user.job = job;
			user.tel = tel;
			user.fax = fax;
			user.mobile = mobile;
			user.email = email;
			user.stime = stime;
			user.etime = etime;
			user.state = state;
			user.cstype = cstype;
			user.depart.departId = departId;
			user.depart.departName = departName;  

			users.push_back(user);
		} 
	}

	if(reqtype==1)
	{//船舶
		//用户信息
		if(!strDid.empty())
			sprintf(appSql," AND t_user.DEPARTMENT_CODE='%s'",strDid.c_str());

		sprintf(sql,"SELECT \
					t_depart.BOARD_FLAG,t_user.USER_ID,t_user.NAME,t_user.JOB,t_user.TEL,t_user.MOBILE,t_user.EMAIL,t_user.FAX, \
					UNIX_TIMESTAMP(t_user.START_DT) AS U_START_DT,UNIX_TIMESTAMP(t_user.END_DT) AS U_END_DT,t_user.VALID_FLAG, \
					t_depart.DEPARTMENT_CODE,t_depart.NAME_CN AS DEPART_NAME, \
					t_ship.NAME_CN AS SHIP_NAME \
					FROM blm_emop_etl.t50_emop_depart_authorities t_auth  \
					JOIN blm_emop_etl.t50_emop_department_code t_depart ON t_auth.DEPARTMENT_CODE=t_depart.DEPARTMENT_CODE \
					JOIN blm_emop_etl.t50_emop_users t_user  ON  t_auth.DEPARTMENT_CODE=t_user.DEPARTMENT_CODE \
					JOIN blm_emop_etl.t41_emop_ship t_ship ON t_ship.SHIPID=t_auth.SHIPID \
					WHERE t_auth.SHIPID='%s' AND t_depart.BOARD_FLAG=1  %s",strId.c_str(),appSql);

		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		while(psql->NextRow())
		{  
			READMYSQL_STR(USER_ID, uid);
			READMYSQL_STR(NAME, name);
			READMYSQL_STR(JOB, job);
			READMYSQL_STR(TEL, tel);
			READMYSQL_STR(FAX, fax);
			READMYSQL_STR(USER_ID, mobile);
			READMYSQL_STR(NAME, email);
			READMYSQL_INT(U_START_DT, stime,0);
			READMYSQL_INT(U_END_DT, etime,0);
			READMYSQL_INT(VALID_FLAG, state,0); 
			READMYSQL_STR(DEPARTMENT_CODE, departId);
			READMYSQL_STR(DEPART_NAME, departName); 
			cstype=0; // 0:岸端、1:船端

			User user;
			user.uid = uid;
			user.name = name;
			user.name = job;
			user.name = tel;
			user.name = fax;
			user.name = mobile;
			user.name = email;
			user.stime = stime;
			user.etime = etime;
			user.state = state;
			user.cstype = cstype;
			user.depart.departId = departId;
			user.depart.departName = departName;  

			users.push_back(user);
		} 
	}

	if(users.size()<=0){  
		out<<"{seq:\""<<strSeq<<"\", users:[]}";
		return 0;
	}

	out<<"{seq:\""<< strSeq <<"\", users:["; 
	for(list<User>::iterator it=users.begin();it!=users.end();it++){
		//角色
		sprintf(sql,"SELECT t_role.ROLE_ID,t_role.NAME_CN AS ROLE_NAME \
					FROM blm_emop_etl.t50_emop_user_role t_role \
					JOIN blm_emop_etl.t50_emop_user_authorities t_auth ON  t_role.ROLE_ID = t_auth.ROLE_ID\
					WHERE t_auth.USER_ID='%s' AND t_role.BOARD_FLAG=0;",it->uid.c_str());

		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		if(it!=users.begin())
			out<<",";

		out<<"{uid:\""<<it->uid<<"\",una:\""<< it->name <<"\",tel:\""<<it->tel<<"\",fax:\""<<it->fax<<"\",";
		out<<"mobile:\""<<it->mobile<<"\",email:\""<<it->email<<"\",stime:"<<it->stime<<",etime:"<<it->etime<<",duty:\""<<it->job<<"\",";
		out<<"did:\""<<it->depart.departId<<"\",dna:\""<<it->depart.departName<<"\" ,ustate:"<<it->state<<",cstype:"<<it->cstype<<",roles:[";

		int idx = 0;
		while(psql->NextRow())
		{
			if(idx>0)
				out<<",";

			READMYSQL_STR(ROLE_ID, roleId);
			READMYSQL_STR(ROLE_NAME, roleName); 
			out<<"{rid:\""<<roleId<<"\",rna:\""<<roleName<<"\"}";

			++idx;
		} 
		out<<"],ships:[";
		//船舶
		sprintf(sql,"SELECT t_ship.SHIPID,t_ship.NAME_CN FROM  blm_emop_etl.t41_emop_ship t_ship \
					JOIN blm_emop_etl.t50_emop_registered_user_fleet t_fleet ON t_fleet.SHIPID = t_ship.SHIPID \
					WHERE t_fleet.USER_ID='%s'",it->uid.c_str());

		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		idx = 0;
		while(psql->NextRow())
		{
			READMYSQL_STR(SHIPID, shipId);
			READMYSQL_STR(NAME_CN, shipName); 

			if(idx>0)
				out<<","; 

			idx++;
			out<<"{sid:\""<<shipId<<"\",sna:\""<<shipName<<"\"}";
		} 
		out<<"]}";
	} 
	out<<"]}";


	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}

//获取船舶资料详细信息0xf1

int eMOPUserMgrSvc::GetShipDetailInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetShipDetailInfo]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string strShipId = root.getv("shipid", "");
	string strSeq= root.getv("seq", "");

	char sql[1024]="";

	sprintf(sql,"select T1.SHIPID,T1.IMO,T1.MMSI,T1.NAME_CN,T1.NAME_EN,T1.CALLSIGN,T1.OWNER,T1.MANAGER,T1.FLAG,T1.REG_PORT,\
				T1.FLAG_REG_NO,T1.SHIP_REG_NO,T1.CLASS_REG_NO,T1.TURBINE_CLASS,T1.HULL_CLASS,T1.BUILDER,T1.BUILD_DT,\
				T1.KEEL_LAY_DT,T1.RELEASE_DT,T1.CREW_NUMBER,T1.PASSENGER_NUMBER,T1.DESIGN_SPEED,T1.TRIAL_SPEED,\
				T1.OPERATIONAL_SPEED,T1.CURRENT_SPEED,T1.ENDURANCE,T2.GROSS,T2.NET,T2.TROP_DWT,T2.SUMM_DWT,T2.WINT_DWT,\
				T2.STRUC_DWT,T2.LIGHT_DWT,T2.LIGHT_DISP,T2.HEAVY_DISP,T2.SUEZ_GROSS,T2.SUEZ_NET,T2.PANAMA_GROSS,\
				T2.PANAMA_NET,T2.LTPC,T2.HTPC,T3.LOA,T3.LBP,T3.M_BEAM,T3.M_DEPTH,T3.MAX_HIGHT,T3.MAX_BEAM,\
				T3.TROP_DRAFT,T3.SUMM_DRAFT,T3.WINT_DRAFT,T3.HEAD_DRAFT,T3.MIDD_DRAFT,T3.TAIL_DRAFT,T3.TROP_FB,T3.SUMM_FB,T3.WINT_FB,\
				T4.THEORY_CONSUM,T4.FEUL_CONSUMPTION,T4.DIESEL_CONSUMPTION,T4.LUBRICATE_CONSUMPTION,T4.FRESHWATER_CONSUMPTION,\
				T5.FUEL_OIL,T5.DIESEL,T5.OIL_PAN,T5.FRESH_WATER,T5.BALLAST_WATER,T5.SLOP,t5.DUMTANK,\
				T6.HOLD_ID,T6.NAME_CN AS LNAME_CN,T6.NAME_EN AS LNAME_EN,T6.LOCATION,T6.VOLUME,T6.GRAVITY_CENTRE,T6.MAX_INERTIA,\
				T7.SHIP_TYPE,T7.SHIP_BOW,T7.SHIP_TAIL,T7.ENGINE_ROOM,T7.RUDDER,T7.PRIMER_PAINT,T7.ANTICORR_PAINT,T7.ANTIFOUL_PAINT,T7.PAINT_MAKER\
				from blm_emop_etl.T41_EMOP_SHIP T1 \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_TONNAGE T2 ON T1.SHIPID=T2.SHIPID \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_DIMENSION T3 ON T1.SHIPID=T3.SHIPID \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_CONSUMPTION T4 ON T1.SHIPID=T4.SHIPID \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_TANK_CAPACITY T5 ON T1.SHIPID=T5.SHIPID \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_LIUQID_HOLD T6 ON T1.SHIPID=T6.SHIPID \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_HULL T7 ON T1.SHIPID=T7.SHIPID \
				where t1.shipid='%s'",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char shipid[32]="";
	char name_cn[64]="";
	char name_en[64]="";
	char imo[16]="";
	char mmsi[16]="";
	char callsign[16]="";
	char flag[10]="";
	char regport[32]="";
	char shiptp[32]="";//船舶类型
	char bow[64]="";//船艏
	char stern[64]="";//艉型
	char rudder[64]="";//舵型
	char engine[64]="";//机舱
	char loa[16]=""; //总长
	char lbp[16]=""; //型长
	char beam[16]=""; //型宽
	char depth[16]="";//型深
	char maxhigh[16]="";//最大高度
	char maxbreadth[16]="";//最大宽度

	char fb_trop[16]="";//热带干舷
	char fb_summer[16]="";//夏季干舷
	char fb_winter[16]="";//冬季干舷
	char draft_trop[16]=""; //热带吃水
	char draft_summ[16]=""; //夏季吃水
	char draft_winter[16]="";//冬季吃水
	char draft_head[16]="";//船艏吃水
	char draft_midd[16]="";//船舯吃水
	char draft_tail[16]="";//船艉吃水

	char grosston[16]="";//总吨
	char netton[16]="";//净吨
	char dwt_drop[16]="";//热带DWT
	char dwt_summ[16]="";//夏季DWT
	char dwt_winter[16]="";//冬季DWT
	char dwt_struct[16]="";//结构DWT
	char dwt_empty[16]="";//空载DWT

	char dsp_empty[16]="";//空载吃水
	char dsp_full[16]="";//满载吃水

	char sue_gross[16]="";//苏伊士总吨
	char sue_net[16]="";//苏伊士净吨
	char panama_gross[16]="";//巴拿马总吨
	char panama_net[16]="";//巴拿马净吨

	char tpc_empty[16]="";//空载TPC
	char tpc_full[16]=""; //满载TPC

	char speed_design[8]="";//设计航速
	char speed_trail[8]="";//试航航速
	char speed_run[8]="";//营运航速
	char speed_current[8]="";//现状航速
	char speed_durance[16]="";//续航能力


	char fuel_oil_ton[16]="";//燃油容量
	char fuel_oil_consump[16]="";//燃油消耗

	char diesel_ton[16]=""; //柴油容量
	char diesel_consump[16]="";//柴油消耗

	char lub_ton[16]="";//滑油容量
	char lub_consump[16]="";//滑油容量

	char freshwater_ton[16]="";
	char freshwater_consump[16]="";

	char ballastwater[16]=""; //压载水容量
	char slop[16]="";//污油水舱容量
	char dumtank[16]="";//污水存放舱容量

	char liquidh_id[32]="";//油水舱ID
	char liquidh_name[128]="";//油水舱名称
	char liquidh_locate[128]="";//油水舱位置
	char liquidh_volum[128]="";//油水舱体积
	char liquidh_gravity[128]="";//重心
	char liquidh_max_inertia[128]="";//最大惯性


	if (psql->NextRow())
	{

		READMYSQL_STR(SHIPID, shipid)
			READMYSQL_STR(IMO, imo)
			READMYSQL_STR(MMSI, mmsi)
			READMYSQL_STR(NAME_CN, name_cn)
			READMYSQL_STR(NAME_EN, name_en)
			READMYSQL_STR(CALLSIGN, callsign)
			READMYSQL_STR(FLAG, flag)
			READMYSQL_STR(REG_PORT, regport)

			READMYSQL_STR(SHIP_TYPE, shiptp)
			READMYSQL_STR(SHIP_BOW, bow)
			READMYSQL_STR(SHIP_TAIL, stern)
			READMYSQL_STR(RUDDER, rudder)
			READMYSQL_STR(ENGINE_ROOM, engine)

			READMYSQL_STR(LOA, loa)
			READMYSQL_STR(LBP, lbp)
			READMYSQL_STR(M_BEAM, beam)
			READMYSQL_STR(M_DEPTH, depth)
			READMYSQL_STR(MAX_HIGHT, maxhigh)
			READMYSQL_STR(MAX_BEAM, maxbreadth)


			READMYSQL_STR(TROP_FB, fb_trop)
			READMYSQL_STR(SUMM_FB, fb_summer)
			READMYSQL_STR(WINT_FB, fb_winter)
			READMYSQL_STR(TROP_DRAFT, draft_trop)
			READMYSQL_STR(SUMM_DRAFT, draft_summ)
			READMYSQL_STR(WINT_DRAFT, draft_winter)
			READMYSQL_STR(HEAD_DRAFT, draft_head)
			READMYSQL_STR(MIDD_DRAFT, draft_midd)
			READMYSQL_STR(TAIL_DRAFT, draft_tail)



			READMYSQL_STR(GROSS, grosston)
			READMYSQL_STR(NET, netton)
			READMYSQL_STR(TROP_DWT, dwt_drop)
			READMYSQL_STR(SUMM_DWT, dwt_summ)
			READMYSQL_STR(WINT_DWT, dwt_winter)
			READMYSQL_STR(STRUC_DWT, dwt_struct)
			READMYSQL_STR(LIGHT_DWT, dwt_empty) 
			READMYSQL_STR(LIGHT_DISP, dsp_empty)
			READMYSQL_STR(HEAVY_DISP, dsp_full)

			READMYSQL_STR(SUEZ_GROSS, sue_gross)
			READMYSQL_STR(SUEZ_NET, sue_net)
			READMYSQL_STR(PANAMA_GROSS, panama_gross)
			READMYSQL_STR(PANAMA_NET, panama_net)
			READMYSQL_STR(LTPC, tpc_empty)
			READMYSQL_STR(HTPC, tpc_full)


			READMYSQL_STR(DESIGN_SPEED, speed_design)
			READMYSQL_STR(TRIAL_SPEED, speed_trail)
			READMYSQL_STR(OPERATIONAL_SPEED, speed_run)
			READMYSQL_STR(CURRENT_SPEED, speed_current)
			READMYSQL_STR(ENDURANCE, speed_durance)



			READMYSQL_STR(FUEL_OIL, fuel_oil_ton)
			READMYSQL_STR(FEUL_CONSUMPTION, fuel_oil_consump)
			READMYSQL_STR(DIESEL, diesel_ton)
			READMYSQL_STR(DIESEL_CONSUMPTION, diesel_consump)
			READMYSQL_STR(OIL_PAN, lub_ton)
			READMYSQL_STR(LUBRICATE_CONSUMPTION, lub_consump)
			READMYSQL_STR(FRESH_WATER, freshwater_ton)
			READMYSQL_STR(FRESHWATER_CONSUMPTION, freshwater_consump)
			READMYSQL_STR(BALLAST_WATER, ballastwater)
			READMYSQL_STR(SLOP, slop)
			READMYSQL_STR(DUMTANK, dumtank)



			READMYSQL_STR(HOLD_ID, liquidh_id)
			READMYSQL_STR(LNAME_CN, liquidh_name)
			if (strlen(liquidh_name)==0)
			{
				READMYSQL_STR(LNAME_EN, liquidh_name)
			}
			READMYSQL_STR(LOCATION, liquidh_locate)
				READMYSQL_STR(VOLUME, liquidh_volum)
				READMYSQL_STR(GRAVITY_CENTRE, liquidh_gravity)
				READMYSQL_STR(MAX_INERTIA, liquidh_max_inertia)
	}


	out<< "{seq:\"" << strSeq << "\",sid:\"" << strShipId << "\",na:\"" << name_cn <<"\",ena:\"" << name_en << "\",cs:\"";
	out<< callsign <<"\",imo:\"" << imo<< "\",mmsi:\"" << mmsi << "\",flag:\"" << flag <<"\",reg:\"" << regport;
	out<< "\",st:\""<< shiptp<< "\",bow:\"" << bow << "\",stern:\"" << stern <<"\",rudder:\"" << rudder<<"\",engine:\"";
	out<< engine <<"\",loa:\"" << loa<< "\",lbp:\"" << lbp << "\",beam:\"" << beam <<"\",depth:\"" << depth;
	out<< "\",maxh:\""<< maxhigh<< "\",maxb:\"" << maxbreadth << "\",fb:[\"" <<fb_trop<<"\",\""<<fb_summer<<"\",\""<<fb_winter<<"\"],mt:[\"";
	out<<grosston<<"\",\""<<netton<<"\"],suez:[\""<<sue_gross<<"\",\""<<sue_net<<"\"],panama:[\"";
	out<<panama_gross<<"\",\""<<panama_net<<"\"],disp:[\""<<dsp_full<<"\",\""<<dsp_empty<<"\"],dwt:[\"";
	out<<dwt_drop<<"\",\""<<dwt_summ<<"\",\""<<dwt_winter<<"\",\""<<dwt_struct<<"\",\""<<dwt_empty<<"\"],tpc:[\"";
	out<<tpc_empty<<"\",\""<<tpc_full<<"\"],draft:[\""<<draft_trop<<"\",\""<<draft_summ<<"\",\""<<draft_winter<<"\",\"";
	out<<draft_head<<"\",\""<<draft_midd<<"\",\""<<draft_tail<<"\"],speed:[\""<<speed_design<<"\",\""<<speed_trail<<"\",\""<<speed_run<<"\",\"";
	out<<speed_current<<"\",\""<<speed_durance<<"\"],consum:[\""<<fuel_oil_ton<<"\",\""<<fuel_oil_consump<<"\",\""<<diesel_ton<<"\",\"";
	out<<diesel_consump<<"\",\""<<lub_ton<<"\",\""<<lub_consump<<"\",\""<<freshwater_ton<<"\",\""<<freshwater_consump<<"\",\""<<ballastwater<<"\",\"";
	out<<slop<<"\",\""<<dumtank<<"\"],oilwater:[\""<<liquidh_name<<"\",\""<<liquidh_locate<<"\",\""<<liquidh_volum<<"\",\""<<liquidh_gravity<<"\",\""<<liquidh_max_inertia;
	out<<"\"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//获取船舶历史0xf2
int eMOPUserMgrSvc::GetShipHistoryList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetShipHistoryList]bad format:", jsonString, 1);

	string strShipId = root.getv("shipid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf (sql, "SELECT SHIPID,UPDATE_TYPE,UPDATE_DATE,OLD_VALUE,NEW_VALUE FROM blm_emop_etl.T41_EMOP_SHIP_UPDATES \
				  where SHIPID='%s'",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	int cnt = 0;

	char shipid[32] = "";
	char type[10]="";
	char updatedt[20] = "";
	char oldval[256]="";
	char newval[256]="";


	while (psql->NextRow())
	{
		READMYSQL_STR(SHIPID, shipid)
			READMYSQL_STR(UPDATE_TYPE, type)
			READMYSQL_STR(UPDATE_DATE, updatedt)
			READMYSQL_STR(OLD_VALUE, oldval)
			READMYSQL_STR(NEW_VALUE, newval)

			if (cnt++)
				out << ",";

		out << "{year:\"" << updatedt << "\",type:\"" << type << "\",name:\"" << newval <<"\"}";
	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);


}

//获取船舶通讯0xf3
int eMOPUserMgrSvc::GetShipCommunicationList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetShipCommunicationList]bad format:", jsonString, 1);

	string strShipId = root.getv("shipid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf (sql, "select COMM_ID,COM_TYPE,COM_NO from blm_emop_etl.T41_EMOP_SHIP_COMMUNICATION \
				  where SHIPID='%s'",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	int cnt = 0;

	char comm_id[10]="";
	char comm_type[20] = "";
	char comm_no[256]="";
	string detail="";
	string belong="";


	while (psql->NextRow())
	{
		READMYSQL_STR(COMM_ID, comm_id)
			READMYSQL_STR(COM_TYPE, comm_type)
			READMYSQL_STR(COM_NO, comm_no)

			if (cnt++)
				out << ",";

		out << "{type:\"" << comm_type << "\",number:\"" << comm_no << "\",detail:\"" << detail <<"\",belong:\"" << belong <<"\"}";
	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}

//获取船舶设备0xf4
int eMOPUserMgrSvc::GetShipEquipmentList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetShipEquipmentList]bad format:", jsonString, 1);

	string strShipId = root.getv("shipid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf (sql, "select T1.QUANTITY,T2.EQUIP_MODEL_ID,T2.MODE,T2.COMPANY_NAME,T3.NAME \
				  from blm_emop_etl.T41_EMOP_SHIP_EQUIPS T1, blm_emop_etl.T50_EMOP_EQUIP_MODEL T2,blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES T3 \
				  WHERE T1.SHIPID='%s' AND T1.EQUIP_MODEL_ID=T2.EQUIP_MODEL_ID \
				  AND T3.SHIPID=T1.SHIPID AND T3.EQUIP_MODEL_ID=T1.EQUIP_MODEL_ID",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	int cnt = 0;
	char equip_id[10]="";
	char equip_mode[128]="";//型号
	char equip_maker[256]="";//制造商
	char equip_name[128]="";//
	int quantity=0;

	while (psql->NextRow())
	{
		READMYSQL_STR(EQUIP_MODEL_ID, equip_id)
			READMYSQL_STR(MODE, equip_mode)
			READMYSQL_STR(COMPANY_NAME, equip_maker)
			READMYSQL_STR(NAME, equip_name)
			READMYSQL_INT(QUANTITY, quantity,0)

			if (cnt++)
				out << ",";

		out << "{id:\"" << equip_id << "\",name:\"" << equip_name << "\",mo:\"" << equip_mode <<"\",vendor:\"";
		out	<< equip_maker <<"\",number:\"" << quantity <<"\"}";
	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
// 船舶基础数据
//0x65f5
int eMOPUserMgrSvc::GetShipBaseData(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetShipBaseData]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string strShipId = root.getv("sid", "");
	string strSeq= root.getv("seq", "");

	char sql[1024]="";

	sprintf(sql,"select IMO,MMSI,NAME_CN,NAME_EN,CALLSIGN,FLAG,REG_PORT,EMAIL from blm_emop_etl.T41_EMOP_SHIP where shipid='%s'",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char name_cn[64]="";
	char name_en[64]="";
	char imo[16]="";
	char mmsi[16]="";
	char callsign[16]="";
	char flag[10]="";
	char regport[32]="";
	char email[100]="";
	//	char mnc[100]="";//T41_EMOP_SHIP_COMMUNICATION
	//	char abm[100]="";//T41_EMOP_SHIP_COMMUNICATION	   
	if (psql->NextRow())
	{
		READMYSQL_STR(IMO, imo)
			READMYSQL_STR(MMSI, mmsi)
			READMYSQL_STR(NAME_CN, name_cn)
			READMYSQL_STR(NAME_EN, name_en)
			READMYSQL_STR(CALLSIGN, callsign)
			READMYSQL_STR(FLAG, flag)
			READMYSQL_STR(REG_PORT, regport)
			READMYSQL_STR(EMAIL, email)
	}
	string strC,strABM;
	GetShipComminfo(strC,strABM,strShipId);
	out<< "{seq:\"" << strSeq << "\",id:\"" << strShipId << "\",na:\"" << name_cn <<"\",ena:\"" << name_en << "\",call:\"";
	out<< callsign <<"\",imo:\"" << imo<< "\",mmsi:\"" << mmsi << "\",nation:\"" << flag <<"\",home_port:\"" << regport;
	out<< "\",email:\""<< email<< "\",mnc:\"" << strC.c_str() << "\",abm:\"" << strABM.c_str() ;
	out<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
int eMOPUserMgrSvc::GetShipComminfo(string &o_strC,string &o_strABM,string &i_strShipid)
{
	MySql* psql1 = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT COM_TYPE,COM_NO FROM blm_emop_etl.T41_EMOP_SHIP_COMMUNICATION WHERE SHIPID='%s'",i_strShipid.c_str());
	if(!(psql1->Query(sql)))
	{RELEASE_MYSQL_RETURN(psql1, 3);}
	char comno[100] = "";
	int iType=-1;
	o_strC="";o_strABM="";
	string strA="";string strB="";string strM="";
	int cnt1 = 0;int cnt2 = 0;int cnt3 = 0;int cnt0 = 0;
	while (psql1->NextRow())
	{
		READMYSQL_INT1(COM_TYPE, iType,-1,psql1);
		READMYSQL_STR1(COM_NO, comno,psql1);

		if(iType <0 || iType>3)
			continue;
		if(iType==2)// c
		{
			if (cnt2++)
				o_strC += ",";
			o_strC+=comno;
		}
		else if(iType==0)
		{
			if (cnt0++)
				strA += ",";
			strA+=comno;
		}
		else if(iType==1)
		{
			if (cnt1++)
				strB += ",";
			strB+=comno;
		}
		else if(iType==3)
		{
			if (cnt3++)
				strM += ",";
			strM+=comno;
		}
	}
	o_strABM+=strA;o_strABM+="/";o_strABM+=strB;o_strABM+="/";o_strABM+=strM;
	RELEASE_MYSQL_RETURN(psql1, 0);
}
//0x65f6 
int eMOPUserMgrSvc::GetShipDetail1(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetShipDetail1]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string strShipId = root.getv("sid", "");
	string strSeq= root.getv("seq", "");

	char sql[1024]="";

	sprintf(sql,"select T1.OWNER,T1.MANAGER,T1.FLAG_REG_NO,T1.SHIP_REG_NO,T1.CLASS_REG_NO,T1.TURBINE_CLASS,T1.HULL_CLASS,T1.BUILDER,T1.BUILD_DT,\
				T1.KEEL_LAY_DT,T1.RELEASE_DT,T1.CREW_NUMBER,T1.COMP_SERVICE_DT,T1.BOW_NUM, T1.BOW_TYPE,\
				T2.LOA,T2.LBP,T2.M_BEAM,T2.M_DEPTH,T2.MAX_HIGHT,T2.MAX_BEAM,\
				T3.SHIP_TYPE,T3.SHIP_BOW,T3.SHIP_TAIL,T3.ENGINE_ROOM,T3.RUDDER,T3.PRIMER_PAINT,T3.ANTICORR_PAINT,T3.ANTIFOUL_PAINT,T3.PAINT_MAKER\
				from blm_emop_etl.T41_EMOP_SHIP T1 \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_DIMENSION T2 ON T1.SHIPID=T2.SHIPID \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_HULL T3 ON T1.SHIPID=T3.SHIPID \
				where T1.shipid='%s'",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	// T41_EMOP_SHIP
	char owner[256]="";
	char manager[256]="";
	char nation_code[128]="";//	国籍证书初次登记号码
	char ship_mark[128]="";//	船舶登记号
	char class_mark[128]="";//	船级登记号
	char class_ship[128]="";//		船体船级
	char class_spare[128]="";//		轮机船级
	char build_comp[128]="";//			建造厂家
	char build_code[128]="";//			建造编号
	char time_keel[32]="";//			铺龙骨时间
	char time_factory[32]="";//			出厂日期
	int sea_man=0;//	船员定员
	char time_service[32]="";//	公司营运日期
	char head_num[128]="";//	船艏数量
	char head_type[128]="";//	船艏型号
	//T41_EMOP_SHIP_HULL
	char ship_type[32]="";//船型
	char bow[64]="";//船艏
	char stern[64]="";//艉型
	char rudder[64]="";//舵型
	char engine[64]="";//机舱位
	char  paint[64]="";//	底漆
	char paint_rust[64]="";//	防锈漆
	char paint_dirt[64]="";//	防污漆
	char paint_create[64]="";//	制造厂
	// T41_EMOP_SHIP_DIMENSION
	char loa[16]=""; //总长
	char lbp[16]=""; //两柱间长
	char beam[16]=""; //型宽
	char depth[16]="";//型深
	char maxhigh[16]="";//最大高度
	char maxbreadth[16]="";//最大宽度

	if (psql->NextRow())
	{  
		// T41_EMOP_SHIP
		READMYSQL_STR(OWNER, owner)
			READMYSQL_STR(MANAGER, manager)
			READMYSQL_STR(FLAG_REG_NO, nation_code)
			READMYSQL_STR(SHIP_REG_NO, ship_mark)
			READMYSQL_STR(CLASS_REG_NO, class_mark)
			READMYSQL_STR(HULL_CLASS, class_ship)
			READMYSQL_STR(TURBINE_CLASS, class_spare)
			READMYSQL_STR(BUILDER, build_comp)
			READMYSQL_STR(BUILD_DT, build_code)
			READMYSQL_STR(KEEL_LAY_DT, time_keel)
			READMYSQL_STR(RELEASE_DT, time_factory)
			READMYSQL_INT(CREW_NUMBER,sea_man,0)
			READMYSQL_STR(COMP_SERVICE_DT, time_service)
			READMYSQL_STR(BOW_NUM, head_num)
			READMYSQL_STR(BOW_TYPE, head_type)
			//T41_EMOP_SHIP_HULL
			READMYSQL_STR(SHIP_TYPE, ship_type)
			READMYSQL_STR(SHIP_BOW, bow)
			READMYSQL_STR(SHIP_TAIL, stern)
			READMYSQL_STR(RUDDER, rudder)
			READMYSQL_STR(ENGINE_ROOM, engine)
			READMYSQL_STR(PRIMER_PAINT, paint)
			READMYSQL_STR(ANTICORR_PAINT,paint_rust)
			READMYSQL_STR(ANTIFOUL_PAINT,paint_dirt)
			READMYSQL_STR(PAINT_MAKER,paint_create)
			// T41_EMOP_SHIP_DIMENSION
			READMYSQL_STR(LOA, loa)
			READMYSQL_STR(LBP, lbp)
			READMYSQL_STR(M_BEAM, beam)
			READMYSQL_STR(M_DEPTH, depth)
			READMYSQL_STR(MAX_HIGHT, maxhigh)
			READMYSQL_STR(MAX_BEAM, maxbreadth)
	}
	out<<"{seq:\""<< strSeq.c_str()<< "\",ship_own:\"" << owner << "\",ship_comp:\"" << manager <<"\",nation_code:\"" << nation_code << "\",ship_mark:\"";
	out<< ship_mark <<"\",class_mark:\"" << class_mark<< "\",class_ship:\"" << class_ship << "\",class_spare:\"" << class_spare <<"\",ship_type:\"" << ship_type;
	out<< "\",bow_type:\"" << bow << "\",tail:\"" << stern <<"\",cabin:\"" << engine<<"\",helm:\"";
	out<< rudder<< "\",build_comp:\"" << build_comp<< "\",build_code:\"" << build_code<< "\",time_keel:\"" << time_keel<< "\",time_factory:\"" << time_factory<< "\",time_service:\"" << time_service;
	out<<"\",total_long:\"" << loa<< "\",grap_long:\"" << lbp << "\",type_width:\"" << beam <<"\",type_height:\"" << depth;
	out<< "\",max_height:\""<< maxhigh<< "\",max_width:\"" << maxbreadth << "\",sea_man:\"" << sea_man<< "\",head_num:\"" << head_num<< "\",head_type:\"" << head_type;
	out<< "\",paint:\""<< paint<< "\",paint_rust:\"" << paint_rust << "\",paint_dirt:\"" << paint_dirt<< "\",paint_create:\"" << paint_create;
	out<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//0x65f7
int eMOPUserMgrSvc::GetShipDetail2(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetShipDetail2]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string strShipId = root.getv("sid", "");
	string strSeq= root.getv("seq", "");

	char sql[1024]="";

	sprintf(sql,"select  T1.APP_CURRENT,T1.CATHODIC_PROT,T1.AL_ANODE,\
				T2.GROSS,T2.NET,T2.TROP_DWT,T2.SUMM_DWT,T2.WINT_DWT,T2.STRUC_DWT,T2.LIGHT_DWT,T2.LIGHT_DISP,\
				T2.HEAVY_DISP,T2.SUEZ_GROSS,T2.SUEZ_NET,T2.PANAMA_GROSS,T2.PANAMA_NET,T2.LTPC,T2.HTPC,\
				T3.TROP_DRAFT,T3.SUMM_DRAFT,T3.WINT_DRAFT,T3.HEAD_DRAFT,T3.MIDD_DRAFT,T3.TAIL_DRAFT,T3.TROP_FB,T3.SUMM_FB,T3.WINT_FB\
				from blm_emop_etl.T41_EMOP_SHIP_HULL T1 LEFT JOIN \
				blm_emop_etl.T41_EMOP_SHIP_TONNAGE T2 ON T1.SHIPID=T2.SHIPID LEFT JOIN \
				blm_emop_etl.T41_EMOP_SHIP_DIMENSION T3 ON T1.SHIPID=T3.SHIPID \
				where T1.shipid='%s'",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	// T41_EMOP_SHIP_HULL
	char impressed_current[16]="";//	外加电流
	char cathodic_pro[16]="";//	阴极保护
	char aluminum_anode[16]="";//	铝阳极
	//T41_EMOP_SHIP_TONNAGE
	char gross_ton[16]="";//	总吨 国际吨位
	char net_ton[16]="";//	净吨 国际吨位
	char dw_tropic[16]="";//	热带 载重吨
	char dw_summer[16]="";//	夏季 载重吨
	char dw_winter[16]="";//	冬季 载重吨
	char dw_structure[16]="";//	结构 载重吨
	char dw_lightweight[16]="";//	空船重量 载重吨
	char in_ballast[16]="";//	空载 排水量
	char capacity_load[16]="";// 	满载 排水量
	char s_gross_ton[16]="";//	总吨 苏伊士运河
	char s_net_ton[16]="";//	净吨 苏伊士运河
	char p_gross_ton[16]="";//	总吨 巴拿马运河
	char p_net_ton[16]="";//	净吨 巴拿马运河
	char tpc_in_ballast[16]="";//	空载 厘米吃水吨TPC
	char tpc_capacity_load[16]="";// 	满载 厘米吃水吨TPC
	//T41_EMOP_SHIP_DIMENSION
	char draft_tropic[16]="";//	热带 吃水
	char draft_summer[16]="";//	夏季 吃水
	char draft_winter[16]="";//	冬季 吃水
	char head_ship[16]="";//	船艏 空载吃水
	char mid_ship[16]="";//	船舯 空载吃水
	char end_ship[16]="";//	船艉 空载吃水
	char fb_tropic[16]="";//	热带 干舷
	char fb_summer[16]="";//	夏季 干舷
	char fb_winter[16]="";//	冬季 干舷
	if (psql->NextRow())
	{
		//T41_EMOP_SHIP_HULL
		READMYSQL_STR(APP_CURRENT, impressed_current)
			READMYSQL_STR(CATHODIC_PROT, cathodic_pro)
			READMYSQL_STR(AL_ANODE, aluminum_anode)
			//T41_EMOP_SHIP_TONNAGE
			READMYSQL_STR(GROSS, gross_ton)
			READMYSQL_STR(NET, net_ton)
			READMYSQL_STR(TROP_DWT, dw_tropic)
			READMYSQL_STR(SUMM_DWT, dw_summer)
			READMYSQL_STR(WINT_DWT, dw_winter)
			READMYSQL_STR(STRUC_DWT, dw_structure)
			READMYSQL_STR(LIGHT_DWT, dw_lightweight) 
			READMYSQL_STR(LIGHT_DISP, in_ballast)
			READMYSQL_STR(HEAVY_DISP, capacity_load)
			READMYSQL_STR(SUEZ_GROSS, s_gross_ton)
			READMYSQL_STR(SUEZ_NET, s_net_ton)
			READMYSQL_STR(PANAMA_GROSS, p_gross_ton)
			READMYSQL_STR(PANAMA_NET, p_net_ton)
			READMYSQL_STR(LTPC, tpc_in_ballast)
			READMYSQL_STR(HTPC, tpc_capacity_load)
			//T41_EMOP_SHIP_DIMENSION
			READMYSQL_STR(TROP_DRAFT, draft_tropic)
			READMYSQL_STR(SUMM_DRAFT, draft_summer)
			READMYSQL_STR(WINT_DRAFT, draft_winter)
			READMYSQL_STR(HEAD_DRAFT, head_ship)
			READMYSQL_STR(MIDD_DRAFT, mid_ship)
			READMYSQL_STR(TAIL_DRAFT, end_ship)
			READMYSQL_STR(TROP_FB, fb_tropic)
			READMYSQL_STR(SUMM_FB, fb_summer)
			READMYSQL_STR(WINT_FB, fb_winter)
	}
	//{seq:””,:””,:””,:””,:””,:” ,:””,:””,:””,:””,:””,:””}

	out<< "{seq:\"" << strSeq << "\",impressed_current:\"" << impressed_current << "\",cathodic_pro:\"" << cathodic_pro <<"\",aluminum_anode:\"" << aluminum_anode << "\",gross_ton:\"";
	out<< gross_ton <<"\",net_ton:\"" << net_ton<< "\",dw_tropic:\"" << dw_tropic << "\",dw_summer:\"" << dw_summer <<"\",dw_winter:\"" << dw_winter;
	out<< "\",dw_structure:\""<< dw_structure<< "\",dw_lightweight:\"" << dw_lightweight << "\",in_ballast:\"" << in_ballast <<"\",capacity_load:\"" << capacity_load<<"\",s_gross_ton:\"";
	out<< s_gross_ton <<"\",s_net_ton:\"" << s_net_ton<< "\",p_gross_ton:\"" << p_gross_ton << "\",p_net_ton:\"" << p_net_ton <<"\",tpc_in_ballast:\"" << tpc_in_ballast;
	out<< "\",tpc_capacity_load:\""<< tpc_capacity_load<< "\",draft_tropic:\"" << draft_tropic << "\",draft_summer:\"" <<draft_summer<< "\",draft_winter:\"" <<draft_winter;
	out<< "\",head_ship:\""<< head_ship<< "\",mid_ship:\"" << mid_ship << "\",end_ship:\"" <<end_ship<< "\",fb_tropic:\"" <<fb_tropic  ;
	out<< "\",fb_summer:\""<< fb_summer<< "\",fb_winter:\"" << fb_winter;
	out<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//0x65f8
int eMOPUserMgrSvc::GetShipDetail3(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetShipDetail3]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string strShipId = root.getv("sid", "");
	string strSeq= root.getv("seq", "");

	char sql[1024]="";

	sprintf(sql,"select T1.DESIGN_SPEED,T1.ENDURANCE,t2.THEORY_CONSUM,t3.FUEL_OIL,t3.DIESEL,t3.OIL_PAN,t3.FRESH_WATER,t3.BALLAST_WATER,t3.SLOP,t3.DUMTANK, \
				T4.POWER_MODEL,T4.POWER_V,T4.POWER_I,T4.POWER_P from blm_emop_etl.T41_EMOP_SHIP T1 \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_CONSUMPTION T2 ON T1.SHIPID=T2.SHIPID \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_TANK_CAPACITY T3 ON T1.SHIPID=T3.SHIPID \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP_POWER_OUTLET T4 ON T1.SHIPID=T4.SHIPID \
				where t1.shipid='%s'",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	//T41_EMOP_SHIP
	char speed[64]="";//	设计航速 航速
	char endurance[64]="";//	续航能力 航速
	//T41_EMOP_SHIP_CONSUMPTION
	char oid_consume[64]="";//	理论油耗 航速
	//T41_EMOP_SHIP_TANK_CAPACITY	
	char fuel[64]="";//	燃油 油水舱储量
	char diesel[64]="";//	柴油 油水舱储量
	char main_oil[64]="";//	主滑油油底壳 油水舱储量
	char fresh_water[64]="";//	淡水 油水舱储量
	char imp_water[64]="";//	压载水 油水舱储量
	char dirt_oil[64]="";//	污油水舱 油水舱储量
	char dirt_water[64]="";//	污水存放舱 油水舱储量
	//T41_EMOP_SHIP_POWER_OUTLET
	char outlet_type[64]="";//	型号 电源插座
	char outlet_voltage[64]="";//	电压 电源插座
	char outlet_electric[64]="";//	电流 电源插座
	char outlet_power[64]="";//	功率 电源插座

	if (psql->NextRow())
	{
		READMYSQL_STR(DESIGN_SPEED, speed)
			READMYSQL_STR(ENDURANCE, endurance)

			READMYSQL_STR(THEORY_CONSUM, oid_consume)

			READMYSQL_STR(FUEL_OIL, fuel)
			READMYSQL_STR(DIESEL, diesel)
			READMYSQL_STR(OIL_PAN, main_oil)
			READMYSQL_STR(FRESH_WATER, fresh_water)
			READMYSQL_STR(BALLAST_WATER, imp_water)
			READMYSQL_STR(SLOP, dirt_oil)
			READMYSQL_STR(DUMTANK, dirt_water)

			READMYSQL_STR(POWER_MODEL, outlet_type)
			READMYSQL_STR(POWER_V, outlet_voltage)
			READMYSQL_STR(POWER_I, outlet_electric)
			READMYSQL_STR(POWER_P, outlet_power)
	}
	ST_SAVEBOAT stSaveboat;
	GetSaveboatInfo(stSaveboat,strShipId);
	//{ ”,dirt_oil:””,dirt_water:””,left_boat:””,lb_style:””,right_boat:””,rb_style:””,left_raft:””,:””,:””,:””,:””,:””,:””,:””}
	out<< "{seq:\"" << strSeq << "\",speed:\"" << speed << "\",oid_consume:\"" << oid_consume <<"\",endurance:\"" << endurance << "\",fuel:\"";
	out<< fuel <<"\",diesel:\"" << diesel<< "\",main_oil:\"" << main_oil << "\",fresh_water:\"" << fresh_water <<"\",imp_water:\"" << imp_water;
	out<< "\",dirt_oil:\""<< dirt_oil<< "\",dirt_water:\"" << dirt_water << "\",left_boat:\"" << stSaveboat.left_boat<<"\",lb_style:\"" << stSaveboat.lb_style <<"\",right_boat:\"";
	out<<stSaveboat.right_boat<<"\",rb_style:\"" <<stSaveboat.rb_style<< "\",left_raft:\"" << stSaveboat.left_raft<< "\",lr_style:\"" << stSaveboat.lr_style;
	out<< "\",right_raft:\"" << stSaveboat.right_raft<< "\",rr_style:\"" << stSaveboat.rr_style<< "\",outlet_type:\"" <<outlet_type ;
	out<< "\",outlet_voltage:\""<< outlet_voltage<< "\",outlet_electric:\"" << outlet_electric << "\",outlet_power:\"" << outlet_power;
	out<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
int eMOPUserMgrSvc::GetSaveboatInfo(ST_SAVEBOAT &o_stSaveboat,string &i_strShipid)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";
	sprintf(sql,"select * from blm_emop_etl.T41_EMOP_SHIP_SAVE_BOAT  where shipid='%s'",i_strShipid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iType=0;
	char pos[8]="";
	string strPos;
	while (psql->NextRow())
	{
		READMYSQL_STR(SAVE_BOAT_POSITION, pos)
			READMYSQL_INT(SAVE_BOAT_TYPE,iType,-1)		
			strPos=pos;
		if(iType==0)//救生艇 boat
		{
			if(strPos=="L")
			{
				READMYSQL_STR(SAVE_BOAT_NUM, o_stSaveboat.left_boat)
					READMYSQL_STR(SAVE_BOAT_MODEL, o_stSaveboat.lb_style)
			}
			else
			{
				READMYSQL_STR(SAVE_BOAT_NUM, o_stSaveboat.right_boat)
					READMYSQL_STR(SAVE_BOAT_MODEL, o_stSaveboat.rb_style)
			}
		}else if(iType==1)
		{
			if(strPos=="L")
			{
				READMYSQL_STR(SAVE_BOAT_NUM, o_stSaveboat.left_raft)
					READMYSQL_STR(SAVE_BOAT_MODEL, o_stSaveboat.lr_style)
			}
			else
			{
				READMYSQL_STR(SAVE_BOAT_NUM, o_stSaveboat.right_raft)
					READMYSQL_STR(SAVE_BOAT_MODEL, o_stSaveboat.rr_style)
			}
		} 
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}
// 0x65f9
int eMOPUserMgrSvc::GetShipEquipments(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPUserMgrSvc::GetShipEquipments]bad format:", jsonString, 1);

	string strShipId = root.getv("sid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t0.PART_CAT_ID,t0.NAME AS TPNAME_CN,t0.NAME_EN AS TPNAME_EN,T1.EQUIP_MODEL_ID,t1.NAME_CN,t1.NAME_EN,t1.CWBT,t1.QUANTITY,T3.Equip_Type_Code \
				  FROM blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t0 \
				  LEFT JOIN blm_emop_etl.T50_EMOP_SHIP_EQUIP T1 ON t0.EQUIP_MODEL_ID = t1.EQUIP_MODEL_ID \
				  LEFT JOIN blm_emop_etl.T50_EMOP_EQUIP_MODEL t2 ON t1.EQUIP_MODEL_ID=t2.EQUIP_MODEL_ID LEFT JOIN \
				  blm_emop_etl.T50_EMOP_EQUIP_TYPE t3 ON t2.EQUIP_TYPE_CODE=t3.Equip_Type_Code \
				  WHERE T1.SHIPID='%s' ORDER BY t3.Equip_Type_Code ",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	char id[64]="";//	设备ID
	char cn[128]="";//	中文名称
	char en[128]="";//	英文名称
	char cwbt[128]="";//	cwbt
	int num=0;//	数量
	char bid[128]="";//	类别ID
	char bname[128]="";//	类别名称
	char bname1[128]="";//	类别名称
	string strPreB,strCurB;
	int iBe=0;
	int cnt = 0;
	int cnt1 = 0;
	while (psql->NextRow())
	{
		READMYSQL_STR(PART_CAT_ID, id)
			READMYSQL_STR(NAME_CN, cn)
			READMYSQL_STR(NAME_EN, en)
			READMYSQL_STR(CWBT, cwbt)
			READMYSQL_INT(QUANTITY, num,0)
			READMYSQL_STR(Equip_Type_Code, bid)
			READMYSQL_STR(TPNAME_CN, bname)
			READMYSQL_STR(TPNAME_EN, bname1)
			//{:””,:””, equips:[{id:””,cn:””,en:””,cwbt:””,num:””},
			strCurB=bid;
		if(cnt1++==0)
			out << "{bid:\""<< bid<<"\",bname:\""<<bname<<"\",equips:[";
		if(strCurB!=strPreB||strCurB.empty())
		{
			if (iBe++)
			{
				out<<"]},";
				out << "{bid:\""<< bid<<"\",bname:\""<<bname<<"\",equips:[";
				cnt=0;
				strPreB=strCurB;
			}
		}
		if (cnt++)
			out << ",";

		out << "{id:\"" << id << "\",cn:\"" << cn << "\",en:\"" << en << "\",cwbt:\"" << cwbt << "\",num:\"" << num<<"\"}";
	}

	out << "]}]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


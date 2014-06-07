#include "blmcom_head.h"
#include "YTSvc.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"
#include "xbase64.h"
#include "json.h"

IMPLEMENT_SERVICE_MAP(YTSvc)

YTSvc::YTSvc()
{

}

YTSvc::~YTSvc()
{

}

bool YTSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_YANTAI, this))
        return false;

	int seconds = 5;
	g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(seconds), ACE_Time_Value(seconds));

    SERVICE_MAP(MID_YT_LATEST,YTSvc,getLatestEvents);
    SERVICE_MAP(MID_YT_HISTORY,YTSvc,getHistoryEvents);
	SERVICE_MAP(MID_YT_UPDATE,YTSvc,updateEvent);
	SERVICE_MAP(MID_YT_GET_NOTICES,YTSvc,getNotices);
	SERVICE_MAP(MID_YT_ADD_NOTICE,YTSvc,postNotice);
	SERVICE_MAP(MID_YT_EDIT_NOTICE,YTSvc,editNotice);
	SERVICE_MAP(MID_YT_DEL_NOTICE,YTSvc,deleteNotice);
	SERVICE_MAP(MID_YT_GET_SECTIONS,YTSvc,getSections);
	SERVICE_MAP(MID_YT_ADD_SECTION,YTSvc,addSection);
	SERVICE_MAP(MID_YT_DEL_SECTION,YTSvc,deleteSection);

	//SERVICE_MAP(0x11,YTSvc,GetAllUserGeneralInfo_C);
	SERVICE_MAP(0x12,YTSvc,GetGetAllRoles);
	SERVICE_MAP(0x13,YTSvc,GetRoleAuthorities);
	SERVICE_MAP(0x14,YTSvc,SetUserRoleName);
	SERVICE_MAP(0x15,YTSvc,NewUserRole);
	SERVICE_MAP(0x16,YTSvc,DelUserRole);
	SERVICE_MAP(0x17,YTSvc,ModifyRoleAuthorities);
	SERVICE_MAP(0x18,YTSvc,NewDpt);
	SERVICE_MAP(0x19,YTSvc,UpdateDpt);
	SERVICE_MAP(0x1c,YTSvc,DelDpt);
	SERVICE_MAP(0x1d,YTSvc,NewUser);
	SERVICE_MAP(0x1e,YTSvc,DelUser);
	//SERVICE_MAP(0x1f,YTSvc,ModifyUserDpt);
	//SERVICE_MAP(0x20,YTSvc,GetUserInfo);
	SERVICE_MAP(0x24,YTSvc,ResetPwd);
	SERVICE_MAP(0x26,YTSvc,SearchUser);
	SERVICE_MAP(0x29,YTSvc,GetUserRoles);
	SERVICE_MAP(0x30,YTSvc,GetUserDpt);
	SERVICE_MAP(0x31,YTSvc,GetCompanyDpt);
	SERVICE_MAP(0x32,YTSvc,SearchShipDpt);
	SERVICE_MAP(0x33,YTSvc,GetDptUsers);
	SERVICE_MAP(0x34,YTSvc,GetRoleUsers);
	SERVICE_MAP(0x35,YTSvc,AddOrDelUserRole);

    DEBUG_LOG("[YTSvc::Start] OK......................................");
    return true;
}

int YTSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	return 0;
}

int YTSvc::getLatestEvents(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::getLatestEvents]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	int sdt = root.getv("sdt", 0);
	int edt = root.getv("edt", 0);
	if(sdt >= edt) return 1;

	int count = 0;
	out << FormatString("{seq:\"%s\",data:[", seq.c_str());

	char sql[1024];
	sprintf(sql, "SELECT tid,eventid,mmsi,name,type,time,endlon,endlat,STATUS FROM aisdb.t41_aisevt_yt WHERE TIME BETWEEN %d AND %d order by time desc", sdt, edt);
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		if(count != 0) out << ",";
		out << FormatString("{tid:\"%s\",mm:%s,sname:\"%s\",ei:\"%s\",lat:%s,lon:%s,dt:%s,status:%s}",
						psql->GetField("tid"),
						psql->GetField("mmsi"),
						NOTNULL(psql->GetField("name")),
						psql->GetField("eventid"),
						psql->GetField("endlat"),
						psql->GetField("endlon"),
						psql->GetField("time"),
						psql->GetField("status"));
		count++;
	}

	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::getHistoryEvents(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[ YTSvc::getHistoryEvents]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	int sdt = root.getv("sdt", 0);
	int edt = root.getv("edt", 0);
	string ei = root.getv("eid", "");
	string status = root.getv("status", "");
	int start = root.getv("start", 0);
	int end = root.getv("end", 0);
	if(sdt >= edt || start > end) return 1;

	int total = 0;
	int filter = 0;
	int count = 0;
	out << FormatString("{seq:\"%s\",events:[", seq.c_str());

	char totalSql[1024];
	sprintf(totalSql, "SELECT count(1) as total FROM aisdb.t41_aisevt_yt WHERE (TIME BETWEEN %d AND %d)", sdt, edt);

	char sql[1024];
	sprintf(sql, "SELECT tid,eventid,mmsi,name,type,time,endlon,endlat,STATUS FROM aisdb.t41_aisevt_yt WHERE (TIME BETWEEN %d AND %d)", sdt, edt);
	if(!ei.empty()) {
		strcat(sql, FormatString(" AND (eventid='%s')", ei.c_str()).c_str());
	}
	if(!status.empty()) {
		strcat(sql, FormatString(" AND (status='%s')", status.c_str()).c_str());
	}
	strcat(sql, " order by time desc");

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(totalSql)&&psql->NextRow(), 3);
	READMYSQL_INT(total, total, 0);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	filter = psql->GetRowCount();

	int sno = 1;
	while(psql->NextRow()) {
		if(sno < start) {
			sno++;
			continue;
		}

		if(count != 0) out << ",";
		out << FormatString("{tid:\"%s\",mm:%s,sname:\"%s\",stype:%s,ei:\"%s\",lat:%s,lon:%s,dt:%s,status:%s}",
			psql->GetField("tid"),
			psql->GetField("mmsi"),
			NOTNULL(psql->GetField("name")),
			psql->GetField("type"),
			psql->GetField("eventid"),
			psql->GetField("endlat"),
			psql->GetField("endlon"),
			psql->GetField("time"),
			psql->GetField("status"));

		count++;
		if(count >= (end-start+1))
			break;
	}

	out << FormatString("],total:%d,filter:%d}", total, filter);
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::updateEvent(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::updateEvent]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string tid = root.getv("tid", "");
	int status = root.getv("status", 0);
	if(tid.empty()) return 1;

	char sql[1024];
	sprintf(sql, "update aisdb.t41_aisevt_yt set status = '%d' where tid='%s'", status, tid.c_str());

	MySql* psql = CREATE_MYSQL;
	psql->Execute(sql);
	RELEASE_MYSQL_NORETURN(psql);

	out << FormatString("{seq:\"%s\",eid:0,tid:\"%s\"}", seq.c_str(), tid.c_str());
	return 0;
}

int YTSvc::getNotices(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::getNotices]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	int start = root.getv("start", 0);
	int end = root.getv("end", 0);
	string type = root.getv("type", "");
	if(start > end)
		return 1;

	char sql[1024];
	strcpy(sql, "select id,type,title,ori,content,time from t41_yt_notice");
	if(!type.empty())
		strcat(sql, FormatString(" where type='%s'", type.c_str()).c_str());
	strcat(sql, "  order by time desc");
	strcat(sql, FormatString(" limit %d,%d", start-1, (end-start+1)).c_str());

	int total = 0;
	out << FormatString("{seq:\"%s\",data:[", seq.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		if(total != 0) out << ",";
		total++;

		out << FormatString("{id:\"%s\",type:%s,title:\"%s\",ori:\"%s\",time:%s,cont:\"%s\"}",
						psql->GetField("id"),
						psql->GetField("type"),
						psql->GetField("title"),
						psql->GetField("ori"),
						psql->GetField("time"),
						psql->GetField("content"));
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::postNotice(const char* pUid, const char* jsonString, std::stringstream& out)
{
	int now = (int)time(NULL);

	JSON_PARSE_RETURN("[YTSvc::postNotice]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	int type = root.getv("type", 0);
	string title = root.getv("title", "");
	string ori = root.getv("ori", "");
	int time = root.getv("time", now);
	string content = root.getv("cont", "");

	int noticeID = -1;

	char sql[4*1024];
	sprintf(sql, "insert into t41_yt_notice(type,title,ori,time,content) values ('%d','%s','%s','%d','%s')", type, title.c_str(), ori.c_str(), time, content.c_str());
	MySql* psql = CREATE_MYSQL;
	if(psql->Execute(sql) > 0)
		noticeID = psql->GetInsertId();

	out << FormatString("{seq:\"%s\",id:\"%d\"}", seq.c_str(), noticeID);
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::editNotice(const char* pUid, const char* jsonString, std::stringstream& out)
{
	int now = (int)time(NULL);

	JSON_PARSE_RETURN("[YTSvc::editNotice]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string id = root.getv("id", "");
	int type = root.getv("type", 0);
	string title = root.getv("title", "");
	string ori = root.getv("ori", "");
	int time = root.getv("time", now);
	string content = root.getv("cont", "");

	char sql[4*1024];
	sprintf(sql, "update t41_yt_notice set type='%d',title='%s',ori='%s',time='%d',content='%s' where id='%s'", type, title.c_str(), ori.c_str(), time, content.c_str(), id.c_str());
	MySql* psql = CREATE_MYSQL;
	if(psql->Execute(sql) > 0)
		out << FormatString("{seq:\"%s\",eid:0}", seq.c_str());
	else
		out << FormatString("{seq:\"%s\",eid:1}", seq.c_str());
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::deleteNotice(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::deleteNotice]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string id = root.getv("id", "");
	StrReplace(id, "|", ",");

	char sql[1024];
	sprintf(sql, "delete from t41_yt_notice where id in(%s)", id.c_str());
	MySql* psql = CREATE_MYSQL;
	if(psql->Execute(sql) > 0)
		out << FormatString("{seq:\"%s\",eid:0}", seq.c_str());
	else
		out << FormatString("{seq:\"%s\",eid:1}", seq.c_str());
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::getSections(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::getSections]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");

	char sql[1024];
	strcpy(sql, "select id,startx,starty,endx,endy,time from t41_yt_section");

	int total = 0;
	out << FormatString("{seq:\"%s\",data:[", seq.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		if(total != 0) out << ",";
		total++;

		out << FormatString("{id:\"%s\",time:%s,startx:%s,starty:%s,endx:%s,endy:%s}",
			psql->GetField("id"),
			psql->GetField("time"),
			psql->GetField("startx"),
			psql->GetField("starty"),
			psql->GetField("endx"),
			psql->GetField("endy"));
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::addSection(const char* pUid, const char* jsonString, std::stringstream& out)
{
	int now = (int)time(NULL);

	JSON_PARSE_RETURN("[YTSvc::addSection]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	int time = root.getv("time", now);
	double startx = root.getv("startx", 0.0);
	double starty = root.getv("starty", 0.0);
	double endx = root.getv("endx", 0.0);
	double endy = root.getv("endy", 0.0);

	int sectionID = -1;

	char sql[4*1024];
	sprintf(sql, "insert into t41_yt_section(startx,starty,endx,endy,time) values ('%f','%f','%f','%f','%d')", startx, starty, endx, endy, time);
	MySql* psql = CREATE_MYSQL;
	if(psql->Execute(sql) > 0)
		sectionID = psql->GetInsertId();

	out << FormatString("{seq:\"%s\",id:\"%d\"}", seq.c_str(), sectionID);
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::deleteSection(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::deleteSection]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string id = root.getv("id", "");

	char sql[1024];
	sprintf(sql, "delete from t41_yt_section where id in (%s)", id.c_str());
	MySql* psql = CREATE_MYSQL;
	if(psql->Execute(sql) > 0)
		out << FormatString("{seq:\"%s\",eid:0}", seq.c_str());
	else
		out << FormatString("{seq:\"%s\",eid:1}", seq.c_str());
	RELEASE_MYSQL_RETURN(psql, 0);
}

// 根据userid获取公司id
int YTSvc::GetCorpInfo(string &o_strCorpid,string &o_strCorpName,string &i_strUid)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t2.COMPANY_KEY,t2.NAME FROM t41_yt_users t1 LEFT JOIN t41_company t2 ON t1.COMPANY_KEY=t2.COMPANY_KEY WHERE t1.USER_ID='%s'",i_strUid.c_str());
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
// 获取公司所有部门[{na:"",dk:"",dt:"",users:[]},...,]
int YTSvc::GetCorpDpt(std::stringstream& out,string &i_strCorpId)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select DEPARTMENT_CODE,NAME_CN,SHIP_FLAG from t41_yt_department_code where REGISTED_OBJECT_ID='%s'",i_strCorpId.c_str());
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
		GeDptUser(out,strKey);
		out<<"]}";
	}	
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 获取部门所有用户{uid:"",ut:"",ships:[{sn:
int YTSvc::GeDptUser(std::stringstream& out,string &i_strDptId)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select USER_ID,VALID_FLAG from t41_yt_users where DEPARTMENT_CODE='%s'",i_strDptId.c_str());
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
		out << "{uid:\"" << id<<"\",ut:"<<iFlag<<"}";		
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
typedef struct KeyValue
{
	string m_strKey;
	string m_strValue;
}_KEY_VALUE;
int YTSvc::GetSubData(std::stringstream& out,short &o_nSt,string &i_strKey,string &i_strRoleid,MySql* psql )
{
	char sql[1024] = "";	
	sprintf (sql, "select * from t41_yt_sub_mod_code where MAIN_MODULE_ID='%s'",i_strKey.c_str());
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
	}
	o_nSt =1;
	for(int i=0;i<vecT1.size();i++)
	{
		if (cnt++)
			out << ",";

		short o_funSt = 0;
		out << "{na:\"" << vecT1.at(i).m_strValue.c_str()  << "\",key:\"" << vecT1.at(i).m_strKey.c_str() <<"\",rm:[";
		GetFuncData(out,o_funSt,vecT1.at(i).m_strKey,i_strRoleid,psql);
		out<<"]";
		out<<",st:"<<o_funSt<<"}";
		if(o_funSt!=1)
			o_nSt = 0;

	}
	return 0;
}

int YTSvc::GetFuncData(std::stringstream& out,short &o_nSt,string &i_strKey,string &i_strRoleid,MySql* psql)
{
	char sql[1024] = "";	
	sprintf (sql, "select * from t41_yt_mod_func where SUB_MODULE_CODE='%s'",i_strKey.c_str());
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
	}
	o_nSt = 1;
	for(int i=0;i<vecT1.size();i++)
	{
		sprintf (sql, "select * from t41_yt_role_authorities where ROLE_ID='%s' and EMOP_FUNCTION_CODE='%s'",i_strRoleid.c_str(),vecT1.at(i).m_strKey.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->GetRowCount()>0)
		{
			st=1; 
		}
		else{
			o_nSt=0;
			st=0;
		}
		if (cnt++)
			out << ",";
		out << "{na:\"" << vecT1.at(i).m_strValue.c_str() << "\",key:\"" << vecT1.at(i).m_strKey.c_str()<<"\",st:"<<st<<"}";
	}
	return 0;
}

int YTSvc::GetAllUserGeneralInfo_C(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetAllUserGeneralInfo_C]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strUId=root.getv("uid", "");
	string strCorpId,strCorpName;
	// 根据userid获取公司id
	GetCorpInfo(strCorpId,strCorpName,strUId);
	out<<"{seq:\""<<strSeq.c_str()<<"\",com:\""<<strCorpName.c_str()<<"\",ck:\""<<strCorpId.c_str()<<"\",depts:[";
	// 获取公司所有部门
	GetCorpDpt(out,strCorpId);
	out<<"]}";

	return 0;
}

int YTSvc::GetGetAllRoles(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetGetAllRoles]bad format:", jsonString, 1);
	string strCskey = root.getv("cskey", "");	
	string strSeq= root.getv("seq", "");
	out<< "{seq:\""<<strSeq.c_str()<<"\",roles:[";	
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"select * from t41_yt_user_role where REGISTED_COMP_ID='%s'",strCskey.c_str());
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

int YTSvc::GetRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetRoleAuthorities]bad format:", jsonString, 1);
	string strRkey= root.getv("rkey", "");
	string strSeq= root.getv("seq", "");
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";	
	out<< "{seq:\""<<strSeq.c_str()<<"\",auth:\"YanTai\",key:\"001\""<<",pm:[";
	sprintf (sql, "select * from t41_yt_mod_code");
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
	}
	rst=1;
	for(int i=0;i<vecT1.size();i++)
	{
		if (cnt++)
			out << ",";
		out << "{na:\"" << vecT1.at(i).m_strValue.c_str() << "\",key:\"" << vecT1.at(i).m_strKey.c_str()<<"\",cm:[";
		GetSubData(out,st,vecT1.at(i).m_strKey,strRkey,psql);
		if(st==1)
			rst=1;
		out<<"]";
		out <<",st:"<<st<<"}";
		if(st!=1)
			rst = 0;
	}
	out<<"]";
	out <<",st:"<<rst<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::SetUserRoleName(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::SetUserRoleName]bad format:", jsonString, 1);
	string strRk= root.getv("rkey", "");
	string strRn= root.getv("rn", "");
	string strSeq= root.getv("seq", "");
	int roletype= root.getv("rtype", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_yt_user_role SET NAME_CN = '%s',BOARD_FLAG=%d WHERE ROLE_ID = '%s'",\
		strRn.c_str(), roletype,strRk.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::NewUserRole(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::NewUserRole]bad format:", jsonString, 1);
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
	sprintf (sql, "insert into t41_yt_user_role(ROLE_ID,NAME_CN, NAME_EN,BOARD_FLAG,REGISTED_COMP_ID) values ('%s','%s','%s','%d','%s')",\
		strRoleId.c_str(),strRn.c_str(), strRn.c_str(), iType,strCskey.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",rkey:\""<<strRoleId<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::DelUserRole(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::DelUserRole]bad format:", jsonString, 1);
	string strRkey = root.getv("rkey", "");
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select * from t41_yt_user_authorities where ROLE_ID='%s'",strRkey.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->GetRowCount()>0)//如果该角色下有用户不能删除
	{
		out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:1"<<"}";
	}
	else
	{
		sprintf (sql, "DELETE FROM t41_yt_user_role WHERE ROLE_ID = '%s'", strRkey.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		sprintf (sql, "DELETE FROM t41_yt_user_authorities WHERE ROLE_ID = '%s'", strRkey.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:0"<<"}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::ModifyRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::ModifyRoleAuthorities]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strRk= root.getv("rk", "");
	string strSm= root.getv("sm", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "DELETE FROM t41_yt_role_authorities WHERE ROLE_ID = '%s'", strRk.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	Tokens svrList;	
	if(strSm.find(",") != string::npos)
	{					 
		svrList = StrSplit(strSm, ",");
		for(int i=0; i<(int)svrList.size(); i++)
		{
			sprintf (sql, "insert into t41_yt_role_authorities(ROLE_ID,EMOP_FUNCTION_CODE) values ('%s','%s')", strRk.c_str(),svrList[i].c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}
	else
	{
		sprintf (sql, "insert into t41_yt_role_authorities(ROLE_ID,EMOP_FUNCTION_CODE) values ('%s','%s')", strRk.c_str(),strSm.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::NewDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::NewDpt]bad format:", jsonString, 1);
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
		"insert into t41_yt_department_code(DEPARTMENT_CODE,PARENT_DEPT_CODE,REGISTED_OBJECT_ID,NAME_CN,BOARD_FLAG,SHIP_FLAG) values ('%s','%s','%s','%s','%d','%d')",\
		strDptId.c_str(),strParentDptId.c_str(),strCskey.c_str(), strDpt.c_str(), iDType,iType);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",key:\""<<strDptId.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::UpdateDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::UpdateDpt]bad format:", jsonString, 1);
	string strDpt = root.getv("dept", "");
	string strDkey = root.getv("dkey", "");//公司/船舶key
	int iType=root.getv("type", -1);//（一般部门）；1（特殊部门）
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_yt_department_code SET NAME_CN = '%s',SHIP_FLAG='%d' WHERE DEPARTMENT_CODE = '%s'",\
		strDpt.c_str(), iType,strDkey.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::DelDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::DelDptShip]bad format:", jsonString, 1);
	string strDkey = root.getv("dkey", "");//部门key
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select * from t41_yt_users where DEPARTMENT_CODE='%s'",strDkey.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->GetRowCount()>0)//如果该部门下有用户不能删除
	{
		out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:1"<<"}";
	}
	else
	{
		sprintf (sql, "DELETE FROM t41_yt_department_code WHERE DEPARTMENT_CODE = '%s'", strDkey.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:0"<<"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::NewUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::NewUserRole]bad format:", jsonString, 1);
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
	string strDuty= root.getv("duty", "");

	//
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	sprintf (sql, "DELETE FROM t41_yt_user_authorities WHERE USER_ID = '%s'", strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	Tokens svrList;	
	if(strRole.find(",") != string::npos)
	{					 
		svrList = StrSplit(strRole, ",");
		for(int i=0; i<(int)svrList.size(); i++)
		{
			sprintf (sql, "insert into t41_yt_user_authorities(USER_ID,ROLE_ID) values ('%s','%s')",\
				strUId.c_str(),svrList[i].c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}
	else if(!strRole.empty())
	{
		sprintf (sql, "insert into t41_yt_user_authorities(USER_ID,ROLE_ID) values ('%s','%s')",\
			strUId.c_str(),strRole.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	
	if(iOpt==0)
	{	 
		sprintf (sql, "INSERT INTO t00_user(user_id,PASSWORD,USERTYPE,SOURCETYPE,is_flag,mainflag,viewsat) VALUES('%s','%s',3,0,1,1,0)",\
			strUId.c_str(),strPwd.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		sprintf (sql, "insert into t41_yt_users(USER_ID,PASSWORD,DEPARTMENT_CODE,NAME,TEL,FAX,MOBILE,EMAIL,START_DT,END_DT,COMPANY_KEY,VALID_FLAG,JOB) values ('%s','%s','%s','%s','%s','%s','%s','%s',%s,%s,'%s','%d','%s')",\
			strUId.c_str(),strPwd.c_str(), strDpt.c_str(),strRn.c_str(),strTel.c_str(),strFax.c_str(),strMobile.c_str(),strEmail.c_str(), strSt.c_str(),strEt.c_str(),strCorpId.c_str(),iType,strDuty.c_str());
	}
	else
	{
		sprintf (sql, "UPDATE t41_yt_users SET DEPARTMENT_CODE='%s',NAME='%s',TEL='%s',FAX='%s',MOBILE='%s',EMAIL='%s',JOB='%s',START_DT='%s',END_DT='%s',COMPANY_KEY='%s' WHERE USER_ID = '%s'",\
			strDpt.c_str(),strRn.c_str(),strTel.c_str(),strFax.c_str(),strMobile.c_str(),strEmail.c_str(), strDuty.c_str(),strSt.c_str(),strEt.c_str(),strCorpId.c_str(),strUId.c_str());

		sprintf (sql, "UPDATE t00_user SET PASSWORD='%s' where user_id='%s'",strPwd.c_str(),strUId.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::DelUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::DelUser]bad format:", jsonString, 1);
	string strUid = root.getv("uid", "");//部门key
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "DELETE FROM t41_yt_users WHERE USER_ID = '%s'", strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	sprintf (sql, "DELETE FROM t41_yt_user_authorities WHERE USER_ID = '%s'", strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::ModifyUserDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::ModifyUserDpt]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strDkey= root.getv("dkey", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE t41_yt_users SET DEPARTMENT_CODE='%s' WHERE USER_ID = '%s'",\
		strDkey.c_str(), strUId.c_str());	
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::GetUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetUserInfo]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"select PASSWORD,DEPARTMENT_CODE,COMPANY_KEY,NAME,TEL,FAX,MOBILE,EMAIL,VALID_FLAG,UNIX_TIMESTAMP(START_DT) as startdt,UNIX_TIMESTAMP(END_DT)as enddt from t41_yt_users where USER_ID='%s'",strUId.c_str());

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
	sprintf(sql,\
		"select t1.ROLE_ID,t2.Name_CN from t41_yt_USER_AUTHORITIES t1 left join t41_yt_USER_ROLE t2 on t1.ROLE_ID=t2.Role_ID where t1.USER_ID='%s'",strUId.c_str());
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
	out <<"],ships:[]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::SearchUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::SearchUser]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"select USER_ID from t00_user where USER_ID='%s'",strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iCode=0;
	if(psql->GetRowCount()>0)
	{		
		iCode=1;
	}
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:"<<iCode<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::GetUserRoles(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetUserRoles]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",roles:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"select t1.ROLE_ID,t2.Name_CN from t41_yt_USER_AUTHORITIES t1 left join t41_yt_USER_ROLE t2 on t1.ROLE_ID=t2.Role_ID where USER_ID='%s'",strUId.c_str());
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

int YTSvc::GetUserDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetUserDpt]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"select t1.DEPARTMENT_CODE,t2.NAME_CN from t41_yt_users t1 left join t41_yt_department_code t2 on t1.DEPARTMENT_CODE=t2.DEPARTMENT_CODE where t1.USER_ID='%s'",strUId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[128]="";
	char na[100] = ""; 
	if(psql->NextRow())
	{		
		READMYSQL_STR(DEPARTMENT_CODE, id);
		READMYSQL_STR(NAME_CN, na); 
		out << "id:\"" << id<< "\",na:\"" <<na<<"\"";
	}else{
		out << "id:\"\",na:\"\"";
	}
	out<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::ResetPwd(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::ResetPwd]bad format:", jsonString, 1);
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
			"select PASSWORD from t41_yt_users where USER_ID='%s'",strUId.c_str());
		char pwd[100] = "";
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->NextRow())
		{		
			READMYSQL_STR(PASSWORD, pwd);			
			string strTmp=pwd;
			if(strTmp==strOldPwd)
			{
				sprintf (sql, "UPDATE t00_user SET PASSWORD='%s' WHERE USER_ID = '%s'",\
					strNewPwd.c_str(), strUId.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

				sprintf (sql, "UPDATE t41_yt_users SET PASSWORD='%s' WHERE USER_ID = '%s'",\
					strNewPwd.c_str(), strUId.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
				out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"code:0"<<"}";
			}
			else
				out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"code:-1"<<"}";
		}
		else
		{			
			out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"code:0"<<"}";
		}
	}
	else
	{ 
		sprintf (sql, "UPDATE t00_user SET PASSWORD='%s' WHERE USER_ID = '%s'",\
			strNewPwd.c_str(), strUId.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		sprintf (sql, "UPDATE t41_yt_users SET PASSWORD='%s' WHERE USER_ID = '%s'",\
			strNewPwd.c_str(), strUId.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",code:0"<<"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

int YTSvc::AddOrDelUserRole(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetRoleUsers]bad format:", jsonString, 1); 
	string strSeq= root.getv("seq", ""); 
	string strUid = root.getv("uid", ""); 
	string strRolesId= root.getv("rkey", ""); 
	int type= root.getv("op", 0); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	Tokens idsVec=StrSplit(strRolesId,",");
	for(Tokens::iterator it=idsVec.begin();it!=idsVec.end();it++){ 
		if(type==0){ //添加
			sprintf(sql,"INSERT INTO t41_yt_user_authorities(ROLE_ID,USER_ID) VALUE('%s','%s')",it->c_str(),strUid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql), 3); 
		}

		if(type==1){//删除
			sprintf(sql,"DELETE FROM t41_yt_user_authorities WHERE ROLE_ID='%s' AND USER_ID='%s';",it->c_str(),strUid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql), 3); 
		} 
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}"; 

	RELEASE_MYSQL_RETURN(psql, 0);
}
 
// 获取用户所在公司，公司部门
int YTSvc::GetCompanyDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetCompanyDpt]bad format:", jsonString, 1);
	string strUId = root.getv("uid", "");
	string strSeq= root.getv("seq", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";//t41_emop_ship
	//公司信息***********************
	char companyId[100];
	char companyName[200];

	sprintf(sql,"SELECT  t_company.COMPANY_KEY,t_company.NAME \
				FROM t41_company t_company LEFT JOIN t41_yt_users t_user ON t_company.COMPANY_KEY=t_user.COMPANY_KEY \
				WHERE t_user.USER_ID='%s'",strUId.c_str());
	//DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3); 

	if(psql->NextRow()){
		READMYSQL_STR(COMPANY_KEY, companyId);
		READMYSQL_STR(NAME, companyName); 
	}else{
		out<<"{seq:\""<<strSeq<<"\",cid:\"\",cna:\"\",cdept:[]}";
		return 0;
	} 
	//公司部门信息********************************
	list<Depart> companyDepartList;
	sprintf(sql,"SELECT DEPARTMENT_CODE,NAME_CN,SHIP_FLAG FROM t41_yt_department_code \
				WHERE REGISTED_OBJECT_ID='%s' AND BOARD_FLAG=0 AND  (PARENT_DEPT_CODE IS NULL OR PARENT_DEPT_CODE='')",companyId);
	//DEBUG_LOG(sql);
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
		//DEBUG_LOG(it->departId.c_str());
		sprintf(sql,"SELECT DEPARTMENT_CODE,NAME_CN,SHIP_FLAG FROM t41_yt_department_code \
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
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}
int YTSvc::SearchShipDpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::SearchShipDpt]bad format:", jsonString, 1);
	string strShipid = root.getv("shipId", "");
	string strSeq= root.getv("seq", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",depts:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";//t41_emop_ship
	sprintf(sql,\
		"select DEPARTMENT_CODE,NAME_CN from t41_yt_department_code where REGISTED_OBJECT_ID='%s'",strShipid.c_str());
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
// 获取公司某部门下所有用户
int YTSvc::GetDptUsers(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetCompanyDpt]bad format:", jsonString, 1); 
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
  

	list<User> users;
 
		//用户信息
		if(!strDid.empty())
			sprintf(appSql," AND t_user.DEPARTMENT_CODE='%s'",strDid.c_str());

		sprintf(sql,"SELECT\
					t_depart.BOARD_FLAG,t_user.USER_ID,t_user.NAME,t_user.JOB,t_user.TEL,t_user.MOBILE,t_user.EMAIL,t_user.FAX,UNIX_TIMESTAMP(t_user.START_DT) AS U_START_DT,UNIX_TIMESTAMP(t_user.END_DT) AS U_END_DT,t_user.VALID_FLAG,\
					t_depart.DEPARTMENT_CODE,t_depart.NAME_CN AS DEPART_NAME\
					FROM t41_yt_users t_user  \
					JOIN t41_yt_department_code t_depart ON t_user.DEPARTMENT_CODE=t_depart.DEPARTMENT_CODE \
					JOIN t41_yt_user_authorities t_auth ON  t_auth.USER_ID=t_user.USER_ID\
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
	 
	if(users.size()<=0){  
		out<<"{seq:\""<<strSeq<<"\", users:[]}";
		return 0;
	}

	out<<"{seq:\""<< strSeq <<"\", users:["; 
	for(list<User>::iterator it=users.begin();it!=users.end();it++){
		//角色
		sprintf(sql,"SELECT t_role.ROLE_ID,t_role.NAME_CN AS ROLE_NAME \
					FROM t41_yt_user_role t_role \
					JOIN t41_yt_user_authorities t_auth ON  t_role.ROLE_ID = t_auth.ROLE_ID\
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
		out<<"]}" ;
	} 
	out<<"]}";


	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}


// 查询某角色下的所有用户
int  YTSvc::GetRoleUsers(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[YTSvc::GetRoleUsers]bad format:", jsonString, 1); 
	string strRoleID = root.getv("rkey", ""); 
	string strSeq= root.getv("seq", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,"SELECT USER_ID FROM t41_yt_user_authorities WHERE ROLE_ID='%s'",strRoleID.c_str());

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
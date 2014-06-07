#include "AreaSvc.h"
#include "blmcom_head.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"

IMPLEMENT_SERVICE_MAP(AreaSvc)

AreaSvc::AreaSvc()
{

}

AreaSvc::~AreaSvc()
{

}

bool AreaSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_MYAREA, this))
        return false;

    SERVICE_MAP(SID_MYLOCATEMODIFY,AreaSvc,ModifyOneLocate);
    SERVICE_MAP(SID_MYLOCATEDEL,AreaSvc,DeleteOneLocate);
    SERVICE_MAP(SID_MYLOCATEADD,AreaSvc,AddOneLocate);
    SERVICE_MAP(SID_MYLOCATELIST,AreaSvc,GetLocateList);
    SERVICE_MAP(SID_MYAREADEL,AreaSvc,DeleteOneArea);
    SERVICE_MAP(SID_MYAREAMODIFY,AreaSvc,ModifyOneArea);
    SERVICE_MAP(SID_MYAREAADD,AreaSvc,AddOneArea);
    SERVICE_MAP(SID_MYAREALIST,AreaSvc,GetAreaList);

    DEBUG_LOG("[AreaSvc::Start] OK......................................");

    return true;
}

int AreaSvc::GetAreaList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AreaSvc::GetAreaList]bad format:", jsonString, 1);
    string areauserid = root.getv("userid", "");
	int st = root.getv("st",0);
    if(areauserid.empty())
        return 1;
	char validStr[50]="";
	if(st==0)
		sprintf(validStr, "AND isvalid='1' ");

	if(st==1)
		sprintf(validStr, "AND isvalid='0' ");

	if(st==100)
		sprintf(validStr, "");

    MySql *psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "SELECT myarea_id,myarea_name,xys,AREA,TYPE,isvalid,UNIX_TIMESTAMP(last_up_dt) AS last_up_dt FROM t12_user_myarea WHERE user_id='%s' %s order by myarea_id desc", areauserid.c_str(),validStr);
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    int total = 0;
	char oneRecord[10240];
    out << '[';
    while(psql->NextRow())
    {
		if(total != 0)
			out << ",";
		total++;

		char id[64];
		char name[128];
		char xy[10240];
		char area[1024];
		int type=0;
		int isvalid=0;
		int lastUpDate=0;
		READMYSQL_STR(myarea_id,id);
		READMYSQL_STR(myarea_name,name);
		READMYSQL_STR(xys,xy);
		READMYSQL_STR(area,area);
		READMYSQL_INT(TYPE,type,0);
		READMYSQL_INT(last_up_dt,lastUpDate,0);
		READMYSQL_INT(isvalid,isvalid,0);

		isvalid=(isvalid==1?0:1);//接口0有效，1无效

		sprintf(oneRecord, "{id:\"%s\",name:\"%s\",update:%d,xy:\"%s\",area:\"%s\",type:%d,st:%d}", id, name,lastUpDate, xy, area,type,isvalid);
		out << oneRecord;
    }
    out << ']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int AreaSvc::AddOneArea(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AreaSvc::AddOneArea]bad format:", jsonString, 1);
	string szUid = root.getv("uid", "");
	if(!szUid.empty())
		pUid = szUid.c_str();
	string name		= root.getv("name", "");
    string xy		= root.getv("xy", "");
    string area		= root.getv("area", "");
	int type = root.getv("type", 0);
    if(xy.empty())
        return 1;
   
    //获取当前时间
    char gmt0now[20];
    GmtNow(gmt0now);

    MySql *psql = CREATE_MYSQL;
    char maxid[64];
    char* tsql_maxid = "SELECT MAX(myarea_id+1) maxid FROM t12_user_myarea ";
	CHECK_MYSQL_STATUS(psql->Query(tsql_maxid)&&psql->NextRow(), 3);
    READMYSQL_STR(maxid,maxid);
    
    char sql[10240];
    sprintf(sql, "INSERT INTO t12_user_myarea(myarea_id,myarea_name,xys,user_id,area,isvalid,last_up_dt,type) values('%s','%s','%s','%s','%s','1','%s',%d)",
		maxid, name.c_str(), xy.c_str(), pUid, area.c_str(), gmt0now,type);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out << "{nsucc:0,id:\"" << maxid <<"\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int AreaSvc::ModifyOneArea(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AreaSvc::ModifyOneArea]bad format:", jsonString, 1);
    string id		= root.getv("id", "");
    string name		= root.getv("name", "");
    string xy		= root.getv("xy", "");
    string area		= root.getv("area", "");
    if(id.empty() || xy.empty())
        return 1;

    char gmt0now[20];
    GmtNow(gmt0now);

    MySql *psql = CREATE_MYSQL;
    char sql[10240];
    sprintf(sql, "UPDATE t12_user_myarea SET myarea_name='%s',xys='%s',area='%s',last_up_dt='%s' WHERE myarea_id='%s'",
            name.c_str(), xy.c_str(), area.c_str(), gmt0now, id.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out<<"{nsucc:0}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int AreaSvc::DeleteOneArea(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AreaSvc::DeleteOneArea]bad format:", jsonString, 1);
    string id = root.getv("id", "");
    if(id.empty())
        return 1;

    char gmt0now[20];
    GmtNow(gmt0now);

    MySql *psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "UPDATE t12_user_myarea SET isvalid='0',last_up_dt='%s' WHERE myarea_id='%s'", gmt0now, id.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out<<"{nsucc:0}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int AreaSvc::GetLocateList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	Json root; 
	if(root.parse(jsonString))
	{
		string szUid = root.getv("uid", "");
		if(!szUid.empty())
			pUid = szUid.c_str();
	}

    char sql[1024];
    sprintf(sql, "SELECT mylocate_id,mylocate_name,x,y,DESCRIPTION,SHOW_TYPE,type FROM t12_user_mylocate WHERE user_id='%s' AND isvalid='1' order by mylocate_id desc", pUid);
	
    MySql *psql = CREATE_MYSQL;
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    int total = 0;
	char oneRecord[1024];
    out << '[';
    while(psql->NextRow())
    {
		if(total != 0)
			out << ",";
		total++;

		char id[64];
		char name[128];
		char x[64];
		char y[64];
		char des[1024];
		int type=0;
		int mini;
		READMYSQL_STR(mylocate_id,id);
		READMYSQL_STR(mylocate_name,name);
		READMYSQL_STR(x,x);
		READMYSQL_STR(y,y);
		// zhuxj add
		READMYSQL_STR(DESCRIPTION, des);
		READMYSQL_INT(SHOW_TYPE, mini, -1);
		READMYSQL_INT(type,type,0);
        
		sprintf(oneRecord, "{id:\"%s\",name:\"%s\",x:\"%s\",y:\"%s\",des:\"%s\",mini:\"%d\",type:\"%d\"}", id, name, x, y, des, mini,type);
		out<<oneRecord;
    }
    out << ']';
    RELEASE_MYSQL_RETURN(psql, 0);
}

int AreaSvc::AddOneLocate(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AreaSvc::AddOneLocate]bad format:", jsonString, 1);
	string szUid = root.getv("uid", "");
	if(!szUid.empty())
		pUid = szUid.c_str();

    string name		= root.getv("name", "");
    string x		= root.getv("x", "");
    string y		= root.getv("y", "");
	string des		= root.getv("des", "");
	string mini		= root.getv("mini", "");
	string type		= root.getv("type", "0");
    if(x.empty() || y.empty())
        return 1;

    MySql *psql = CREATE_MYSQL;

    char maxid[64];
    char* tsql_maxid ="SELECT MAX(mylocate_id+1) maxid FROM t12_user_mylocate ";
	CHECK_MYSQL_STATUS(psql->Query(tsql_maxid)&&psql->NextRow(), 3);
    READMYSQL_STR(maxid,maxid);
   
    char sql[1024];
    sprintf(sql, "INSERT INTO t12_user_mylocate(mylocate_id,mylocate_name,x,y,user_id,isvalid,DESCRIPTION,SHOW_TYPE,type)values('%s','%s','%s','%s','%s','1','%s','%s','%s')",
            maxid, name.c_str(), x.c_str(), y.c_str(), pUid, des.c_str(), mini.c_str(), type.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out << FormatString("{nsucc:0,id:\"%s\"}", maxid);
    RELEASE_MYSQL_RETURN(psql, 0);
}

int AreaSvc::DeleteOneLocate(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AreaSvc::DeleteOneLocate]bad format:", jsonString, 1);
    string id = root.getv("id", "");
    if(id.empty())
        return 1;

    MySql *psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "UPDATE t12_user_mylocate SET isvalid='0' WHERE mylocate_id='%s'", id.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out<<"{nsucc:0}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int AreaSvc::ModifyOneLocate(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AreaSvc::ModifyOneLocate]bad format:", jsonString, 1);
	string id	= root.getv("id", "");
    string name = root.getv("name", "");
    string x	= root.getv("x", "");
    string y	= root.getv("y", "");
	string des	= root.getv("des", "");
	string mini = root.getv("mini", "");
	string type = root.getv("type", "0");
    if(x.empty() || y.empty())
        return 1;

    MySql *psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "UPDATE t12_user_mylocate SET mylocate_name='%s',x='%s',y='%s',DESCRIPTION='%s',SHOW_TYPE='%s',type='%s' WHERE mylocate_id='%s'", name.c_str(), x.c_str(), y.c_str(), des.c_str(), mini.c_str(), type.c_str(), id.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out<<"{nsucc:0}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

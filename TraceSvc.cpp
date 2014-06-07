#include "TraceSvc.h"
#include "blmcom_head.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"
#include "json.h"

using namespace std;

IMPLEMENT_SERVICE_MAP(TraceSvc)

TraceSvc::TraceSvc()
{
}

TraceSvc::~TraceSvc()
{
}

bool TraceSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_MYTRACE, this))
        return false;

    SERVICE_MAP(SID_GETalltrace,TraceSvc,alltrace_req);
    SERVICE_MAP(SID_tkindopt,TraceSvc,tkindopt_req);
    SERVICE_MAP(SID_traceopt,TraceSvc,traceopt_req);
    SERVICE_MAP(SID_GETcj_event,TraceSvc,cj_event_req);
    SERVICE_MAP(SID_cj_event_confirm,TraceSvc,cj_event_confirm);

    DEBUG_LOG("[TraceSvc::Start] OK......................................");

    return true;
}

int TraceSvc::cj_event_confirm(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[TraceSvc::cj_event_confirm]bad format:", jsonString, 1);
	int eid		= root.getv("eid", 0);
    int time	= root.getv("time", 0);
    int mmsi	= root.getv("mmsi", 0);
    int ttype	= root.getv("type", 0);
	if(ttype == 0)
		ttype = 2;

	char sql[1024];
    MySql *psql = CREATE_MYSQL;
    
    sprintf(sql, "update t45_pilot_cj_illegal_event set status = %d where mmsi = %d and starttime = %d and eventid = %d and status = 0",
					ttype,mmsi,time,eid);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out << "{nsucc:0}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int TraceSvc::cj_event_req(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[TraceSvc::cj_event_req]bad format:", jsonString, 1);
	long startT = root.getv("startdt", 0);
    long endT	= root.getv("enddt", 0);
    
	char sql[1024];
	MySql *psql = CREATE_MYSQL;

	if(startT<0&&endT<0)
    {
        strcpy(sql, "select eventid, mmsi, shipname, shiptype, cargotype, starttime,endtime  from t45_pilot_cj_illegal_event where status in (0,1)");
    }
    else
    {
        sprintf(sql, "select eventid, mmsi, shipname, shiptype, cargotype, starttime,endtime  from t45_pilot_cj_illegal_event where status in (0,1) ", startT, endT);
    }
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	vector<_CJEVENT> resultList;
    std::map<int, vector<_CJEVENT> > mapMmsiEvents;
    typedef std::map<int, vector<_CJEVENT> >::iterator IterMmsiEvent;
    IterMmsiEvent iterMmsiEvent;

	while(psql->NextRow())
	{
		_CJEVENT cjEvent;
		READMYSQL_INT(eventid, cjEvent.m_nEventID, 0);
		READMYSQL_INT(mmsi, cjEvent.m_nMmsi, 0);
		char buff[1024] = {'\0'};
		READMYSQL_STR(shipname, buff);
		cjEvent.m_szName = buff;
		READMYSQL_INT(shiptype, cjEvent.m_nShipType, 0);
		READMYSQL_INT(cargotype, cjEvent.m_nCargoType, 0);
		READMYSQL_INT(starttime, cjEvent.m_lStart, 0);
		READMYSQL_INT(endtime, cjEvent.m_lEnd, 0);
		resultList.push_back(cjEvent);
	}

    //封装返回串
    out<<"[";
    vector<_CJEVENT>::iterator it=resultList.begin();
    for (; it!=resultList.end(); it++)
    {
        if (it!=resultList.begin())
        {
            out<<',';
        }
        StrReplace(it->m_szName, "[", "");
        StrReplace(it->m_szName, "]", "");
        StrReplace(it->m_szName, "\"", "");
        if(it->m_lEnd>0)  //如果没有结束时间，则返回的endtime为空
        {
            out<<FormatString("{eid:\"%d\",mmsi:\"%d\",sname:\"%s\",sdt:\"%ld\",edt:\"%ld\",shtp:%d,ctp:%d}",
                              it->m_nEventID,it->m_nMmsi,it->m_szName.c_str(),it->m_lStart,it->m_lEnd,it->m_nShipType,it->m_nCargoType);
        }
        else
        {
            out<<FormatString("{eid:\"%d\",mmsi:\"%d\",sname:\"%s\",sdt:\"%ld\",edt:\"\",shtp:%d,ctp:%d}",
                              it->m_nEventID,it->m_nMmsi,it->m_szName.c_str(),it->m_lStart,it->m_nShipType,it->m_nCargoType);
        }
    }
    out<<"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int TraceSvc::traceopt_req(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[TraceSvc::traceopt_req]bad format:", jsonString, 1);

    _MYTRACE mytrace;
    mytrace.m_strSI = root.getv("si", "");
    mytrace.m_strCI = root.getv("ci", "");
    mytrace.m_strMM = root.getv("mm", "");
    mytrace.m_strTN = root.getv("tn", "");
    mytrace.m_strSN = root.getv("sn", "");
    mytrace.m_iST	= root.getv("st", 0);
    mytrace.m_iET	= root.getv("et", 0);
    mytrace.m_iType	= root.getv("type", 0);

    MySql *psql = CREATE_MYSQL;
    switch(mytrace.m_iType)
    {
    case 0://添加轨迹
        if(!trace_add(pUid,mytrace,psql))  RELEASE_MYSQL_RETURN(psql, 3);
        break;
    case 1://删除轨迹
        if(!trace_del(pUid,mytrace,psql)) RELEASE_MYSQL_RETURN(psql, 3);
        break;
    case 2://彻底清除轨迹
        if(!trace_clear(pUid,mytrace,psql)) RELEASE_MYSQL_RETURN(psql, 3);
        break;
    case 3://恢复已删除轨迹
        if(!trace_recover(pUid,mytrace,psql)) RELEASE_MYSQL_RETURN(psql, 3);
        break;
    case 4://修改轨迹
        if(!trace_edit(pUid,mytrace,psql)) RELEASE_MYSQL_RETURN(psql, 3);
        break;
    default:
        RELEASE_MYSQL_RETURN(psql, 2);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

int TraceSvc::tkindopt_req(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[TraceSvc::tkindopt_req]bad format:", jsonString, 1);
    
    string ci = root.getv("ci", "");
    string cn = root.getv("cn", "");
    int itype = root.getv("type", 0);

	MySql *psql = CREATE_MYSQL;
    switch(itype)
    {
    case 0://添加分类
        if(!tracekind_add(pUid,ci.c_str(),cn.c_str(),psql)) RELEASE_MYSQL_RETURN(psql, 3);
        break;
    case 1://删除分类
        if(!tracekind_del(pUid,ci.c_str(),psql)) RELEASE_MYSQL_RETURN(psql, 3);
        break;
    case 2://彻底清除分类
        if(!tracekind_clear(pUid,ci.c_str(),psql)) RELEASE_MYSQL_RETURN(psql, 3);
        break;
    case 3://恢复已删除分类
        if(!tracekind_recover(pUid,ci.c_str(),psql)) RELEASE_MYSQL_RETURN(psql, 3);
        break;
    case 4://修改分类名称
        if(!tracekind_edit(pUid,ci.c_str(),cn.c_str(),psql)) RELEASE_MYSQL_RETURN(psql, 3);
        break;
    default:
        RELEASE_MYSQL_RETURN(psql, 2);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

int TraceSvc::alltrace_req(const char* pUid, const char* jsonString, std::stringstream& out)
{
	char sql[1024];
   
    map<string,_TKIND> m_CatMap;
    sprintf(sql, "select t1.userid,t1.categoryid,t1.categoryname,t1.create_dt,t1.trash_flag,v1.num from T41_myTrace_Category t1 left join t41_remark_count v1 on t1.userid=v1.user_id and t1.categoryid=v1.object_id and v1.type=5 where t1.userid ='%s'", pUid);
    
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->GetRowCount() == 0)
	{
		//添加默认分类
		sprintf(sql, "insert into T41_myTrace_Category(userid,categoryid,categoryname,update_dt,create_dt,trash_flag) values('%s','%s','%s','%s','%s',0)",
					pUid,"00","My Trace","","");
		CHECK_MYSQL_STATUS(psql->Execute(sql)>0, 3);

		_TKIND m_CatInfo;
		m_CatInfo.m_btrash_flag=0;
		strcpy(m_CatInfo.m_btcategoryid,"00");
		strcpy(m_CatInfo.m_szCategoryname,"My Trace");
		strcpy(m_CatInfo.updatet_dt,"");
		strcpy(m_CatInfo.create_dt,"");
		m_CatMap["00"]=m_CatInfo;
	}
	else
	{
		while(psql->NextRow())
		{
			_TKIND m_CatInfo;
			READMYSQL_STR(categoryid, m_CatInfo.m_btcategoryid);
			READMYSQL_STR(categoryname, m_CatInfo.m_szCategoryname);
			READMYSQL_STR(create_dt, m_CatInfo.create_dt);
			READMYSQL_INT(num, m_CatInfo.m_RemarkNum,0);
			READMYSQL_INT(trash_flag, m_CatInfo.m_btrash_flag,0);
			m_CatMap[m_CatInfo.m_btcategoryid]=m_CatInfo;
		}
	}

	//2.获取分类下轨迹
    sprintf(sql, "select t1.categoryid,t1.shipid,t1.mmsi,t1.shipname,t1.tracename,t1.create_dt,t1.start_dt,t1.end_dt,t1.trash_flag,v1.num num from T41_myTrace_Ship t1 left join t41_remark_count v1 on t1.userid=v1.user_id and concat(t1.shipid,t1.start_dt,t1.end_dt)=v1.object_id and v1.type=6 where t1.userid ='%s'", pUid);
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	while(psql->NextRow())
	{
		_MYTRACE mTrace;
		mTrace.m_strSI=NOTNULL(psql->GetField("shipid"));
		mTrace.m_strCI=NOTNULL(psql->GetField("categoryid"));
		mTrace.m_strMM=NOTNULL(psql->GetField("mmsi"));
		mTrace.m_strSN=NOTNULL(psql->GetField("shipname"));
		mTrace.m_strTN=NOTNULL(psql->GetField("tracename"));
		mTrace.m_crDt=NOTNULL(psql->GetField("create_dt"));
		const char *startDt=NOTNULL(psql->GetField("start_dt"));
		const char *endDt=NOTNULL(psql->GetField("end_dt"));
		const char *RmkNum=NOTNULL(psql->GetField("num"));
		const char *TrashFlag=NOTNULL(psql->GetField("trash_flag"));
		mTrace.m_iST=atol(startDt);
		mTrace.m_iET=atol(endDt);
		mTrace.m_RemarkNum=atoi(RmkNum);
		mTrace.m_TrashFlag=atoi(TrashFlag);
		if (m_CatMap.find(mTrace.m_strCI)!=m_CatMap.end())
		{
			m_CatMap[mTrace.m_strCI].m_TraceList.push_back(mTrace);
		}
	}

    //封装返回串
    out<<"[";
    map<string,_TKIND>::iterator it=m_CatMap.begin();
    for (; it!=m_CatMap.end(); it++)
    {
        _TKIND _Category=it->second;
        vector<_MYTRACE> _VslList=_Category.m_TraceList;
        if (it!=m_CatMap.begin())
        {
            out<<',';
        }
        out<<"{\"cn\":\""<<_Category.m_szCategoryname<<"\""
           <<",\"cr\":\""<<_Category.create_dt<<"\","
           <<FormatString("\"ci\":\"%s\",\"ts\":%d,\"rn\":%d",_Category.m_btcategoryid,_Category.m_btrash_flag,_Category.m_RemarkNum)
           <<",\"trs\":[";
        for (int j=0; j<(int)_VslList.size(); j++)
        {
            _MYTRACE _VslInfo=_VslList[j];
            if (j>0)
            {
                out<<',';
            }
            out<<"{\"si\":\""<<_VslInfo.m_strSI<<"\""
               <<",\"sn\":\""<<_VslInfo.m_strSN<<"\""
               <<",\"mm\":\""<<_VslInfo.m_strMM<<"\""
               <<",\"tn\":\""<<_VslInfo.m_strTN<<"\""
               <<",\"cr\":\""<<_VslInfo.m_crDt<<"\","
               <<FormatString("\"st\":%d,\"et\":%d,\"rn\":%d,\"ts\":%d",_VslInfo.m_iST,_VslInfo.m_iET,_VslInfo.m_RemarkNum,_VslInfo.m_TrashFlag)
               <<'}';
        }
        out<<"]}";
    }
    out<<"]";
    RELEASE_MYSQL_RETURN(psql, 0);
}

bool TraceSvc::tracekind_add(const char* i_szUid,const char* i_szCI,const char*i_szCN,MySql *psql)
{
	//获取GMT0的当前时间
    char gmt0now[20];
    GmtNow(gmt0now);

	char sql[1024];
    sprintf(sql, "insert into T41_myTrace_Category(Userid,Categoryid,Categoryname,create_dt,update_dt,trash_flag) values('%s','%s','%s','%s','%s',%d)",
		i_szUid,i_szCI,i_szCN,gmt0now,gmt0now,0);

	if(psql->Execute(sql)<0)
        return false;
    return true;
}

bool TraceSvc::tracekind_del(const char* i_szUid,const char* i_szCI,MySql *psql)
{
    char gmt0now[20];
    GmtNow(gmt0now);

	char sql[1024];
    // 所有轨迹置trash_flag1
    sprintf(sql, "update T41_myTrace_Ship set trash_flag=1,update_dt='%s' where userId='%s' and Categoryid='%s'", gmt0now,i_szUid,i_szCI);
    if(psql->Execute(sql)<0)
        return false;

    // 轨迹类别置trash_flag1
    sprintf(sql, "update T41_myTrace_Category set trash_flag=1,update_dt='%s' where userId='%s' and Categoryid='%s'", gmt0now,i_szUid,i_szCI);
    if(psql->Execute(sql)<0)
        return false;

    return true;
}

bool TraceSvc::tracekind_recover(const char* i_szUid,const char* i_szCI,MySql *psql)
{
    char gmt0now[20];
    GmtNow(gmt0now);

    // 所有轨迹置trash_flag0
    char *sql="update T41_myTrace_Ship set trash_flag=0,update_dt='%s' where userId='%s' and Categoryid='%s'";
    if(psql->Execute(FormatString(sql,gmt0now,i_szUid,i_szCI).c_str())<0)
        return false;

    // 轨迹类别置trash_flag1
    char *sql1="update T41_myTrace_Category set trash_flag=0,update_dt='%s' where userId='%s' and Categoryid='%s'";
    if(psql->Execute(FormatString(sql1,gmt0now,i_szUid,i_szCI).c_str())<0)
        return false;

    return true;
}

bool TraceSvc::tracekind_edit(const char* i_szUid,const char* i_szCI,const char*i_szCN,MySql *psql)
{
    char gmt0now[20];
    GmtNow(gmt0now);

    char *sql="update T41_myTrace_Category set Categoryname='%s',update_dt='%s' where  userId='%s' and Categoryid='%s'";
    if(psql->Execute(FormatString(sql,i_szCN,gmt0now,i_szUid,i_szCI).c_str())<0)
        return false;

    return true;
}

bool TraceSvc::tracekind_clear(const char* i_szUid,const char* i_szCI,MySql *psql)
{
    //删除分类中的轨迹
    char *sql="delete from T41_myTrace_Ship where userId='%s' and Categoryid='%s'";
    if(psql->Execute(FormatString(sql,i_szUid,i_szCI).c_str())<0)
        return false;

    //删除轨迹分类
    char *sql1="delete from T41_myTrace_Category where userId='%s' and Categoryid='%s'";
    if(psql->Execute(FormatString(sql1,i_szUid,i_szCI).c_str())<0)
        return false;

    return true;
}

bool TraceSvc::trace_add(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql)
{
	//获取GMT0的当前时间
	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
    sprintf(sql, "insert into T41_myTrace_Ship(Userid,shipid,categoryid,mmsi,tracename,shipname,start_dt,end_dt,create_dt,update_dt,trash_flag) values('%s','%s','%s','%s','%s','%s',%d,%d,'%s','%s',%d)",
				i_szUid,i_mytrace.m_strSI.c_str(),i_mytrace.m_strCI.c_str(),i_mytrace.m_strMM.c_str(),i_mytrace.m_strTN.c_str(),i_mytrace.m_strSN.c_str(),i_mytrace.m_iST,i_mytrace.m_iET,gmt0now,gmt0now,0);
    if(psql->Execute(sql)<0)
        return false;

    return true;
}

bool TraceSvc::trace_del(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql)
{
    // 轨迹类别置trash_flag1
	char sql[1024];
    sprintf(sql, "update T41_myTrace_Ship set trash_flag=1 where Userid='%s' and shipid='%s'and start_dt=%d and end_dt=%d",
			i_szUid,i_mytrace.m_strSI.c_str(),i_mytrace.m_iST,i_mytrace.m_iET);
    if(psql->Execute(sql)<0)
        return false;

    return true;
}

bool TraceSvc::trace_edit(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql)
{
	char gmt0now[20];
	GmtNow(gmt0now);
    
	char sql[1024];
    sprintf(sql, "update T41_myTrace_Ship set tracename='%s',update_dt='%s' where Userid='%s' and shipid='%s' and start_dt=%d and end_dt=%d",
				i_mytrace.m_strTN.c_str(),gmt0now,i_szUid,i_mytrace.m_strSI.c_str(),i_mytrace.m_iST,i_mytrace.m_iET);
    if(psql->Execute(sql)<0)
        return false;

    return true;
}

bool TraceSvc::trace_recover(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql)
{
    // 轨迹类别置trash_flag1
	char sql[1024];
    sprintf(sql, "update T41_myTrace_Ship set trash_flag=0 where Userid='%s'and shipid='%s'and start_dt=%d and end_dt=%d",
				i_szUid,i_mytrace.m_strSI.c_str(),i_mytrace.m_iST,i_mytrace.m_iET);
    if(psql->Execute(sql)<0)
        return false;

    return true;
}

bool TraceSvc::trace_clear(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql)
{
    //删除分类中的轨迹
	char sql[1024];
    sprintf(sql, "delete from T41_myTrace_Ship where Userid='%s' and shipid='%s' and start_dt=%d and end_dt=%d",
				i_szUid,i_mytrace.m_strSI.c_str(),i_mytrace.m_iST,i_mytrace.m_iET);
    if(psql->Execute(sql)<0)
        return false;

    return true;
}

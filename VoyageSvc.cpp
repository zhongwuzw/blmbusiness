#include "blmcom_head.h"
#include "VoyageSvc.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "IBusinessService.h"
#include "kSQL.h"
#include "Util.h"
#include "ObjectPool.h"

IMPLEMENT_SERVICE_MAP(VoyageSvc)

VoyageSvc::VoyageSvc()
{
}

VoyageSvc::~VoyageSvc()
{
}

bool VoyageSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_VOYAGE, this))
        return false;

    SERVICE_MAP(SID_VOY_POSTBOOKMODIFY,VoyageSvc,MoiPostBookEta);
    SERVICE_MAP(SID_VOY_ADDAISTIME,VoyageSvc,AddAisModiTime);
    SERVICE_MAP(SID_VOY_GETAISTIME,VoyageSvc,GetAisModiTime);
    SERVICE_MAP(SID_VOY_PORTMANAGER,VoyageSvc,PortManage);
    SERVICE_MAP(SID_VOY_GETPORTCALL,VoyageSvc,GetPortcall);
    SERVICE_MAP(SID_VOY_POSTBOOKMANAGER,VoyageSvc,PositionbookManage);
    SERVICE_MAP(SID_VOY_POSTBOOKADD,VoyageSvc,AddPositionbook);
    SERVICE_MAP(SID_VOY_GETPOSTIONBOOK,VoyageSvc,GetPositionbook);
    SERVICE_MAP(SID_VOY_SETCONSUMPTION,VoyageSvc,SetConsumption);
    SERVICE_MAP(SID_VOY_INFMANAGER,VoyageSvc,VoyageinfoManage);
    SERVICE_MAP(SID_VOY_CLSMANAGER,VoyageSvc,CategoryManage);
    SERVICE_MAP(SID_VOY_GETVOYINFO,VoyageSvc,GetVoyageInfo);

    DEBUG_LOG("[VoyageSvc::Start] OK......................................");
    return true;
}

int VoyageSvc::GetVoyageInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    char sql[2048];
    MySql *psql = CREATE_MYSQL;
    
    sprintf(sql, "select type,object_id,num from t41_remark_count where user_id = '%s' and ( type = 2 or type = 3 )", pUid);
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	std::vector<REMARK_INFO>  remarkVec;
	while(psql->NextRow())
	{
		REMARK_INFO oneremark;
		oneremark.num = atoi(NOTNULL(psql->GetField("num")));
		oneremark.object_id = NOTNULL(psql->GetField("object_id"));
		oneremark.type = atoi(NOTNULL(psql->GetField("type")));
		remarkVec.push_back(oneremark);
	}

    //取得航次下做过ais轨迹修正的
    Tokens  aismodiVec;
    sprintf(sql, " select distinct b.voyage_id as voyage_id from t41_voyage_category a,t41_voyage_aisinf b,t41_voyage_info c " \
                 " where a.user_id = '%s' and b.user_id = '%s' and c.user_id = '%s' and a.voyagecategory_id = c.voyagecategory_id and b.voyage_id = c.voyage_id ", pUid,pUid,pUid);
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{
		string onevoyageid = NOTNULL(psql->GetField("voyage_id"));
		aismodiVec.push_back(onevoyageid);
	}

    sprintf(sql, "select d.category_upd_dt,d.voyage_upd_dt,d.voyagecategory_id,d.categoryname,d.category_delstatus, " \
                   " d.VOYAGE_ID, d.NAME, d.SHIP_ID,d.SHIP_NAME, d.SHOW_ROUTE, d.AIS_TRACK,d.DEL_STATUS,c.FO_NO_LOAD, c.FO_FULL_LAOD, c.FO_AT_PORT, c.DO_NAVIGATION, " \
                   " c.DO_AT_PORT, c.FW_NAVIGATION, c.FW_AT_PORT,d.MMSI  from (" \
				   " SELECT UNIX_TIMESTAMP(b.upd_dt) AS category_upd_dt,UNIX_TIMESTAMP(a.upd_dt) AS voyage_upd_dt,b.voyagecategory_id, " \
				   " b.name AS categoryname,b.del_status AS category_delstatus," \
				   " IFNULL(a.VOYAGE_ID,-1) VOYAGE_ID, a.NAME, a.SHIP_ID,a.SHIP_NAME, a.SHOW_ROUTE, a.AIS_TRACK,a.DEL_STATUS,IFNULL(a.MMSI,'-1') AS MMSI " \
				   " FROM   t41_voyage_category  b left join t41_voyage_info a " \
				   " on a.user_id = b.user_id and a.VOYAGECATEGORY_ID =  b.VOYAGECATEGORY_ID  and a.DEL_STATUS!=2 " \
				   " WHERE b.USER_ID = '%s'" \
				   " AND b.del_status != 2  " \
				   " )d left join t41_voyage_consume c " \
                   " on d.VOYAGE_ID = c.VOYAGE_ID and c.User_id = '%s' ", pUid,pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    out<<'[';
    const char* lastcatID = NULL;
    bool isfirstvoyage = true;
    int remarknum = 0;
    while(psql->NextRow())
    {
		const char *thiscatid = NOTNULL(psql->GetField("voyagecategory_id"));
		if(lastcatID==NULL || strcasecmp(lastcatID,thiscatid)!=0)  //是一个新的category
		{
			if(lastcatID!=NULL)out<<"]},";							//不是第一个category，前面补逗号

			//找到其备注数目
			remarknum = 0;
			if(!remarkVec.empty())
			{
				for(int k=0; k<(int)remarkVec.size(); k++)
				{
					if(remarkVec.at(k).type==2&& strcmp(remarkVec.at(k).object_id.c_str(),thiscatid)==0)
					{
						remarknum = remarkVec.at(k).num;
						break;
					}
				}
			}
			out<<FormatString("{cdt:\"%s\",ci:\"%s\",cn:\"%s\",cd:\"%s\",crk:\"%d\",voys:[",NOTNULL(psql->GetField("category_upd_dt")),thiscatid,NOTNULL(psql->GetField("categoryname")),NOTNULL(psql->GetField("category_delstatus")),remarknum);
			isfirstvoyage = true;
		}
		lastcatID = thiscatid;
		if(!isfirstvoyage)out<<',';
		string tmpstr = NOTNULL(psql->GetField("SHIP_NAME"));
		tmpstr = StrReplace(tmpstr,"\"","\\\"");
		//找到其备注数目
		remarknum = 0;
		if(!remarkVec.empty()) 
		{
			for(int k=0; k<(int)remarkVec.size(); k++)
			{
				if(remarkVec.at(k).type==3&& strcmp(remarkVec.at(k).object_id.c_str(),NOTNULL(psql->GetField("VOYAGE_ID")))==0)
				{
					remarknum = remarkVec.at(k).num;
					break;
				}
			}
		}
		
		if(atoi(psql->GetField("VOYAGE_ID")) !=-1 )
		{
			//判断其是否做过ais轨迹修正
			int ismodiais = 0;
			for(int k=0; k<(int)aismodiVec.size(); k++)
			{
				if( strcmp(aismodiVec.at(k).c_str(),NOTNULL(psql->GetField("VOYAGE_ID")))==0)
				{
					ismodiais = 1;
					break;
				}
			}
			string mmsistr = psql->GetField("MMSI");
			if(mmsistr=="-1")mmsistr = "";
			out<<FormatString("{vdt:\"%s\",vi:\"%s\",vn:\"%s\",vsi:\"%s\",vsn:\"%s\",vsmsi:\"%s\","\
				"vr:\"%s\",va:\"%s\",vfn:\"%s\",vff:\"%s\",vfa:\"%s\",vdn:\"%s\","\
				"vda:\"%s\",vwn:\"%s\",vwa:\"%s\",vd:\"%s\",vrk:\"%d\",modais:\"%d\"}"
				,NOTNULL(psql->GetField("voyage_upd_dt"))
				,NOTNULL(psql->GetField("VOYAGE_ID"))
				,NOTNULL(psql->GetField("NAME"))
				,NOTNULL(psql->GetField("SHIP_ID"))
				,tmpstr.c_str()
				,mmsistr.c_str()
				,NOTNULL(psql->GetField("SHOW_ROUTE"))
				,NOTNULL(psql->GetField("AIS_TRACK"))
				,NOTNULL(psql->GetField("FO_NO_LOAD"))
				,NOTNULL(psql->GetField("FO_FULL_LAOD"))
				,NOTNULL(psql->GetField("FO_AT_PORT"))
				,NOTNULL(psql->GetField("DO_NAVIGATION"))
				,NOTNULL(psql->GetField("DO_AT_PORT"))
				,NOTNULL(psql->GetField("FW_NAVIGATION"))
				,NOTNULL(psql->GetField("FW_AT_PORT"))
				,NOTNULL(psql->GetField("DEL_STATUS"))
				,remarknum
				,ismodiais
				);
			isfirstvoyage = false;
		}
	}
	if(psql->GetRowCount()>0)out<<"]}"; 
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::CategoryManage(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::CategoryManage]bad format:", jsonString, 1);
	string categoryid = root.getv("ci", "");
	string categoryname = root.getv("cn", "");
	int optype = atoi(root.getv("type", ""));

	char gmt0now[20];
	GmtNow(gmt0now);

	string categorynamestr = StrReplace(categoryname,"'","\\'");

	char sql[1024];
    MySql *psql = CREATE_MYSQL;
    
    if(optype==0)   //添加分类
    {
        strcpy(sql, "select ifnull(max(voyagecategory_id),999999) as lastid from t41_voyage_category ");
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

        long lastid = 1+atol(psql->GetField("lastid"));
        sprintf(sql, "insert into t41_voyage_category(VOYAGECATEGORY_ID, USER_ID, NAME, DEL_STATUS,upd_dt)values(%ld,'%s','%s',0,'%s')",lastid,pUid,categorynamestr.c_str(),gmt0now);
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		out<<FormatString("{eid:0,ci:\"%ld\"}",lastid);
    }
    else if(optype==1)    //删除分类
    {
        sprintf(sql, "update t41_voyage_category set DEL_STATUS = 1  where VOYAGECATEGORY_ID = %s ", categoryid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==2)    //恢复已删除分类
    {
        sprintf(sql, "update t41_voyage_category set DEL_STATUS = 0  where VOYAGECATEGORY_ID = %s ", categoryid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==3)    //清空分类
    {
        sprintf(sql, "update t41_voyage_category set DEL_STATUS = 2  where VOYAGECATEGORY_ID = %s ", categoryid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==4)    //修改分类名称
    {
        sprintf(sql, "update t41_voyage_category set NAME = '%s'  where VOYAGECATEGORY_ID = %s ", categoryname.c_str(),categoryid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
	else
		RELEASE_MYSQL_RETURN(psql, 1);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::VoyageinfoManage(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::GetContactStat]bad format:", jsonString, 1);
   
	char sql[1024];
    MySql *psql = CREATE_MYSQL;
    
	char gmt0now[20];
	GmtNow(gmt0now);

    string ci = root.getv("ci", "");
    string vi = root.getv("vi", "");
    int optype = atoi(root.getv("type", ""));
    string vn = root.getv("vn", "");
    string vsi = root.getv("vsi", "");
    string vsn = root.getv("vsn", "");
    string vr = root.getv("vr", "");
    string va = root.getv("va", "");
    string vsmsi = root.getv("vsmsi", "");
    string vnstr = StrReplace(vn,"'","\\'");

    if(optype==0)   //新建航次，航次ID自动加1
    {
        strcpy(sql, "select ifnull(max(voyage_id),100000000000000000)+1 as lastid from t41_voyage_info where voyage_id < 101000000000000000 and voyage_id > 100000000000000000 ");
        CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

		string lastid = psql->GetField("lastid");
        sprintf(sql, "insert into t41_voyage_info(VOYAGE_ID, VOYAGECATEGORY_ID, NAME, SHIP_ID, SHOW_ROUTE, AIS_TRACK, DEL_STATUS,upd_dt,user_id)"\
                      "values(%s, %s, '%s', '%s', %s, %s, 0,'%s','%s')",
                       lastid.c_str(),ci.c_str(),vnstr.c_str(),vsi.c_str(),vr.c_str(),va.c_str(),gmt0now,pUid);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out << "{eid:0,vi:\"" << lastid << "\"}";
    }
    else if(optype==1)    //修改航次名称
    {
        sprintf(sql, "update t41_voyage_info set NAME = '%s'  where VOYAGE_ID = %s ", vnstr.c_str(),vi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==2)    //删除航次
    {
        sprintf(sql, "update  t41_voyage_info set DEL_STATUS = 1  where VOYAGE_ID = %s ", vi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==3)    //恢复已删除航次
    {
        sprintf(sql, "update t41_voyage_info set DEL_STATUS = 0  where VOYAGE_ID = %s ", vi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==4)    //清空航次
    {
        sprintf(sql, "update t41_voyage_info set DEL_STATUS = 2  where VOYAGE_ID = %s ", vi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==5)    //修改所属船ID、名称和MSI
    {
        string tmpstr1 = StrReplace(vsn,"'","\\'");
        sprintf(sql, "update t41_voyage_info set SHIP_ID = '%s',ship_name = '%s',mmsi = %s  where VOYAGE_ID = %s ", vsi.c_str(),tmpstr1.c_str(),vsmsi.c_str(),vi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==6)    //修改航次显示信息
    {
        sprintf(sql, "update t41_voyage_info set SHOW_ROUTE = %s  where VOYAGE_ID = %s ", vr.c_str(),vi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==7)    //修改AIS轨迹显示信息
    {
        sprintf(sql, "update t41_voyage_info set AIS_TRACK = %s  where VOYAGE_ID = %s ", va.c_str(),vi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==8)	  //修改航次所属类别信息
    {
        sprintf(sql, "update t41_voyage_info set voyagecategory_id = %s  where VOYAGE_ID = %s ", ci.c_str(),vi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::SetConsumption(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::SetConsumption]bad format:", jsonString, 1);
	string vi = root.getv("vi", "");
	int vfn =atoi(root.getv("vfn", ""));
	int vff =atoi(root.getv("vff", ""));
	int vfa =atoi(root.getv("vfa", ""));
	int vdn =atoi(root.getv("vdn", ""));
	int vda =atoi(root.getv("vda", ""));
	int vwn =atoi(root.getv("vwn", ""));
	int vwa =atoi(root.getv("vwa", ""));

	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
    MySql *psql = CREATE_MYSQL;
    
    sprintf(sql, "replace into t41_voyage_consume(VOYAGE_ID,FO_NO_LOAD,FO_FULL_LAOD,FO_AT_PORT,DO_NAVIGATION,DO_AT_PORT,FW_NAVIGATION,FW_AT_PORT,upd_dt,user_id) " \
                  " values('%s', %d ,%d,%d,%d,%d,%d,%d,'%s','%s' ) ",
                  vi.c_str(),vfn,vff,vfa,vdn,vda,vwn,vwa,gmt0now,pUid);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::GetPositionbook(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::GetPositionbook]bad format:", jsonString, 1);
    string vi = root.getv("vi", "");

	char sql[1024];
    sprintf(sql, "select a.POSTION_BOOK_ID,a.VOYAGE_ID,a.ETA,b.LIST_ORDER_NUM,b.POSTION_NAME,b.X_COORD,"\
                  " b.Y_COORD, b.POSTION_ROUTE, b.LONGTI_INTERVAL, b.SPEED, "\
                  " b.STATUS from t41_voyage_postionbook a,t41_voyage_postion b "\
                  " where a.VOYAGE_ID = '%s' "\
                  " and a.user_id = b.user_id "
                  " and a.user_id = '%s' "
                  " and a.POSTION_BOOK_ID = b.POSTION_BOOK_ID " \
                  " order by POSTION_BOOK_ID", vi.c_str(),pUid);
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    out<<'[';
    const char* lastcatID = NULL;
    bool isfirstvoyage = true;
	while(psql->NextRow())
	{
		const char *pbi = NOTNULL(psql->GetField("POSTION_BOOK_ID"));
		if(lastcatID==NULL || strcasecmp(lastcatID,pbi)!=0)		//是一个新的category
		{
			if(lastcatID != NULL)out<<"]},";					//不是第一个category，前面补逗号
			out<<FormatString("{vid:\"%s\",pbi:\"%s\",eta:\"%s\",pb:[",vi.c_str(),pbi,NOTNULL(psql->GetField("ETA")));
			isfirstvoyage = true;
		}
		lastcatID = pbi;
		if(!isfirstvoyage)out<<',';
		string onevoyage=FormatString("{lon:%s,psn:\"%s\",xc:%s,yc:%s,psr:\"%s\",pli:\"%s\",psp:\"%s\", pst:\"%s\"}"
			,NOTNULL(psql->GetField("LIST_ORDER_NUM"))
			,NOTNULL(psql->GetField("POSTION_NAME"))
			,NOTNULL(psql->GetField("X_COORD"))
			,NOTNULL(psql->GetField("Y_COORD"))
			,NOTNULL(psql->GetField("POSTION_ROUTE"))
			,NOTNULL(psql->GetField("LONGTI_INTERVAL"))
			,NOTNULL(psql->GetField("SPEED"))
			,NOTNULL(psql->GetField("STATUS"))
		);
		out<<onevoyage;
		isfirstvoyage = false;
	}
    if(!isfirstvoyage)out<<"]}";
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::AddPositionbook(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::AddPositionbook]bad format:", jsonString, 1);
	string ci = root.getv("vi", "");
	string dt = root.getv("dt", "");

	char sql[1024];
    MySql *psql = CREATE_MYSQL;

	int num = 0;
	sprintf (sql, "SELECT COUNT(1) AS num FROM t41_voyage_postionbook WHERE VOYAGE_ID = '%s'", ci.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	num = atoi(psql->GetField("num"));

    strcpy(sql, "select ifnull(max(POSTION_BOOK_ID),999999) as lastid from t41_voyage_postionbook ");
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
    
	long lastid = 1+atol(psql->GetField("lastid"));
	if (num)
	{
		lastid--;
		sprintf (sql, "UPDATE t41_voyage_postionbook SET ETA = '%s'  WHERE VOYAGE_ID = '%s'", dt.c_str(), ci.c_str());
	}
	else
		sprintf(sql, "insert into t41_voyage_postionbook(POSTION_BOOK_ID, VOYAGE_ID, ETA,user_id)values(%ld, %s, '%s','%s')",lastid,ci.c_str(),dt.c_str(),pUid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{eid:0,psb:\"" << lastid << "\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::PositionbookManage(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::PositionbookManage]bad format:", jsonString, 1);
	string positiobbookid = root.getv(0, "");
	int needdelete = atoi(root.getv(1, ""));

	char sql[1024];
    MySql* psql = CREATE_MYSQL;
    
    if(needdelete==0)
    {
		sprintf(sql, "delete from t41_voyage_postion where POSTION_BOOK_ID = %s ",positiobbookid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }

    for(int i=2; i<(int)root.size(); i++)
    {
        string oneportdata = root.getv(i, "");
        Tokens dts = StrSplit(oneportdata, "|");
        if(dts.size() < 8)
			RELEASE_MYSQL_RETURN(psql, 1);

        string tmpstr = dts[1];
        tmpstr = StrReplace(tmpstr,"'","\\'");

        sprintf(sql, "insert into t41_voyage_postion (POSTION_BOOK_ID, LIST_ORDER_NUM, POSTION_NAME, X_COORD, Y_COORD, POSTION_ROUTE, LONGTI_INTERVAL, SPEED, STATUS,user_id)" \
                     "  values(%s, %s, '%s', %s, %s, '%s', %s, %s, %s,'%s') ",
					 positiobbookid.c_str(),dts[0].c_str(),tmpstr.c_str(),dts[2].c_str(),dts[3].c_str(),dts[4].c_str(),dts[5].c_str(),dts[6].c_str(),dts[7].c_str(),pUid);
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{vi:"",web:"1"}
int VoyageSvc::GetPortcall(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::GetPortcall]bad format:", jsonString, 1);
	string vi = root.getv("vi", "");
	string web = root.getv("web", "");//区分web版本

	char sql[1024];
    sprintf(sql, "SELECT a.LIST_ORDER_NUM, a.PORT_ID, a.PORT_NAME,\
					a.PORT_PURPOSE, a.PORT_SPEED, a.PORT_WEATHER, a.AT_PORT_TIME, b.eta\
					FROM t41_voyage_port a LEFT JOIN t41_voyage_postionbook b ON a.VOYAGE_ID = b.VOYAGE_ID\
					WHERE a.VOYAGE_ID = %s AND a.user_id = '%s' ", vi.c_str(),pUid);
	 MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
    out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		if (web.empty())
			out<<FormatString("{lon:\"%s\",ppi:\"%s\",ppn:\"%s\",pp:\"%s\",psp:\"%s\",pw:\"%s\",apt:\"%s\"}"
				,NOTNULL(psql->GetField("LIST_ORDER_NUM"))//,vid:\"%s\"
				,NOTNULL(psql->GetField("PORT_ID"))
				,NOTNULL(psql->GetField("PORT_NAME"))
				,NOTNULL(psql->GetField("PORT_PURPOSE"))
				,NOTNULL(psql->GetField("PORT_SPEED"))
				,NOTNULL(psql->GetField("PORT_WEATHER"))
				,NOTNULL(psql->GetField("AT_PORT_TIME"))
				//,vi.c_str()
			);
		else
			out<<FormatString("{lon:\"%s\",ppi:\"%s\",ppn:\"%s\",pp:\"%s\",psp:\"%s\",pw:\"%s\",apt:\"%s\",vid:\"%s\",eta:\"%s\"}"
				,NOTNULL(psql->GetField("LIST_ORDER_NUM"))
				,NOTNULL(psql->GetField("PORT_ID"))
				,NOTNULL(psql->GetField("PORT_NAME"))
				,NOTNULL(psql->GetField("PORT_PURPOSE"))
				,NOTNULL(psql->GetField("PORT_SPEED"))
				,NOTNULL(psql->GetField("PORT_WEATHER"))
				,NOTNULL(psql->GetField("AT_PORT_TIME"))
				,vi.c_str()
				,NOTNULL(psql->GetField("eta"))
			);
	}
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::PortManage(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::PortManage]bad format:", jsonString, 1);
	string voyageid = root.getv(0, "");

	char sql[1024];
    MySql *psql = CREATE_MYSQL;
	
	sprintf(sql, "delete from t41_voyage_port where voyage_id = %s and user_id='%s' ",voyageid.c_str(),pUid);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    for(int i=1; i<(int)root.size(); i++)
    {
		string oneportdata = root.getv(i, "");
        Tokens dts = StrSplit(oneportdata, "|");
        if(dts.size() < 7) 
			RELEASE_MYSQL_RETURN(psql, 1);

        string portnamestr = dts[2];
        portnamestr = StrReplace(portnamestr,"'","\\'");
        sprintf(sql, "insert into t41_voyage_port(VOYAGE_ID, LIST_ORDER_NUM, PORT_ID, PORT_NAME, PORT_PURPOSE, PORT_SPEED, PORT_WEATHER, AT_PORT_TIME,user_id)" \
                      " values(%s, %s,'%s', '%s', '%s', '%s', '%s', '%s','%s') ",voyageid.c_str(),dts[0].c_str(),dts[1].c_str(),portnamestr.c_str(),dts[3].c_str(),dts[4].c_str(),dts[5].c_str(),dts[6].c_str(),pUid);
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    
    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::GetAisModiTime(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::GetAisModiTime]bad format:", jsonString, 1);
    string vi = root.getv("vi", "");

    MySql *psql = CREATE_MYSQL;
    char sql[1024];
	sprintf(sql,"select  MOD_TIME from t41_voyage_aisinf where VOYAGE_ID = %s and user_id = '%s'", vi.c_str(),pUid);
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out<<FormatString("{aisnum:\"%d\",times:[",psql->GetRowCount());
	while(psql->NextRow())
	{
		if(total > 0)
			out << ",";
		total++;

		out<<'\"'<<NOTNULL(psql->GetField("MOD_TIME"))<<'\"';
	}
	out<<"]}";
    
    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::AddAisModiTime(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::AddAisModiTime]bad format:", jsonString, 1);
	string vi1 = root.getv("vi", "");
	string time1 = root.getv("time", "");

	char sql[1024];
    MySql *psql = CREATE_MYSQL;
    
    sprintf(sql, "insert into t41_voyage_aisinf(VOYAGE_ID, MOD_TIME,user_id) values(%s, '%s','%s') ",vi1.c_str(),time1.c_str(),pUid);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int VoyageSvc::MoiPostBookEta(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[VoyageSvc::MoiPostBookEta]bad format:", jsonString, 1);
	string psb = root.getv("psb", "");
	string dt = root.getv("dt", "");

	char sql[1024];
    sprintf(sql, "update t41_voyage_postionbook set eta = '%s' where postion_book_id = %s and user_id = '%s' ",dt.c_str(),psb.c_str(),pUid);
    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

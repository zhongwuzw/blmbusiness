#include "blmcom_head.h"
#include "AdSvc.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"
#include "xbase64.h"
#include "json.h"

IMPLEMENT_SERVICE_MAP(AdSvc)

AdSvc::AdSvc()
{

}

AdSvc::~AdSvc()
{

}

bool AdSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_PANELAD, this))
        return false;

    SERVICE_MAP(SID_REQZONEAD,AdSvc,getZoneItems);
    SERVICE_MAP(SID_REQTYPEAD,AdSvc,getAdByType);
	SERVICE_MAP(SID_PHONE_AD,AdSvc,getPhoneAD);

	//std::stringstream out;
	//getPhoneAD("zhangmin2", "{uid:\"zhangmin2\"}", out);
    DEBUG_LOG("[AdSvc::Start] OK......................................");
    return true;
}

int AdSvc::getZoneItems(const char* pUid, const char* jsonString, std::stringstream& out)
{

    char sql[2048];
    sprintf(sql, "select t1.id, t1.height, t1.width, t1.iframecode, t1.period, t1.times, t1.shutdowntime, \
		t1.positiontype, t2.type, t2.pixelx, t2.pixely, t1.limitx, t1.limity, t1.conflict,t3.usertype,t2.pixelx,t2.pixely \
		from boloomodb_s.t00_advertise_zone t1, t00_advertise_positioncode t2, t00_user t3 \
		where t1.positiontype = t2.id and t1.service_id is null and t3.user_id = '%s'and t3.usertype < 50", pUid);
    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    
	bool bFirst = true;
	out << "[";
	while(psql->NextRow())
	{
		char id[64];
		char height[64];
		char width[64];
		char iframecode[1024];
		char period[64];
		char times[64];
		char shutdowntime[64];
		char positiontype[64];
		char type[64];
		char pixelx[64];
		char pixely[64];
		char limitx[64];
		char limity[64];
		char conflict[1024];
		int usertype;
		int inttype;
		int intpixelx;
		int intpixely;
		int intheight,intwidth;
		READMYSQL_STR(id, id);
		READMYSQL_STR(height, height);
		READMYSQL_STR(width, width);
		READMYSQL_STR(iframecode, iframecode);
		READMYSQL_STR(period, period);
		READMYSQL_STR(times, times);
		READMYSQL_STR(shutdowntime, shutdowntime);
		READMYSQL_STR(positiontype, positiontype);
		READMYSQL_STR(type, type);
		READMYSQL_STR(pixelx, pixelx);
		READMYSQL_STR(pixely, pixely);
		READMYSQL_STR(limitx, limitx);
		READMYSQL_STR(limity, limity);
		READMYSQL_STR(conflict, conflict);

		READMYSQL_INT(usertype, usertype,0);
		inttype = atoi(type);
		intpixelx = atoi(pixelx);
		intpixely = atoi(pixely);
		intheight = atoi(height);
		intwidth = atoi(width);

		if(bFirst)
			bFirst = false;
		else
			out << ",";
		out << "\"" << id << "|" << height << "|" << width << "|" << iframecode << "|" << period << "|" << times << "|" << shutdowntime << "|" << positiontype << "|" << type << "|" << pixelx << "|" << pixely << "|" << limitx << "|" << limity << "|" << conflict << "\"";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int AdSvc::getAdByType(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AdSvc::getAdByType]bad format:", jsonString, 1);

    std::string szType = root.getv("type", "");
    int nType = atoi(szType.c_str());

    char sql[1024];
    std::string adUrl;
    int showTime;
    int height;
    int width;

    MySql* psql = CREATE_MYSQL;
    sprintf(sql, "select iframecode,shutdowntime,height,width from boloomodb_s.t00_advertise_zone where service_id = '%d'", nType);
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
    {
        char url[2048];
        READMYSQL_STR(iframecode, url);
        adUrl = url;

        READMYSQL_INT(shutdowntime, showTime, 0);
        READMYSQL_INT(height, height, 0);
        READMYSQL_INT(width, width, 0);
    }

	int nRet = 0;
    switch(nType)
    {
    case 0: //地图
    {
        nRet = getZoneItems(pUid, jsonString, out);
    }
    break;

    case 1: //船期
    {
        out << "{url:\"" << adUrl << "\"}";
    }
    break;

    case 2:	//运价
    {
        out << "{url:\"" << adUrl << "\"}";
    }
    break;

    case 3: //船舶名录
    {
        double rate = calAdRate(pUid, nType);
        out << "{url:\"" << adUrl << "\",showtime:" << showTime << ",height:" << height << ",width:" << width << ",gl:" << rate << "}";
    }
    break;

    case 4: //港口名录
    {
        double rate = calAdRate(pUid, nType);
        out << "{iframecode:\"" << adUrl <<  "\",showtime:" << showTime << ",height:" << height << ",width:" << width << ",gl:" << rate << "}";
    }
    break;

	case 5: //卫星船舶
	{
		out << "{iframecode:\"" << adUrl <<  "\",showtime:" << showTime << ",height:" << height << ",width:" << width << ",gl:1}";
	}
	break;

    default:
        nRet = 1;
        break;
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//1.注册天数<=5的不弹出广告
//2.广告弹出概率（max(0,min(0.5, 登陆天数/注册天数*日平均在线时长/1800秒*50%))）
double AdSvc::calAdRate(const char* pUid, int nType)
{
    double rate = 1.0;
    double nRegisterDay, nLoginDay, nAvgOnlineTime;

	MySql* psql = CREATE_MYSQL;

	char sql[2048];
	sprintf(sql, "SELECT register_times, login_days, avg_online_times FROM t03_user_accessinfo WHERE user_id = '%d'", pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 0);
	if(psql->NextRow())
	{
		READMYSQL_DOUBLE(register_times, nRegisterDay, 0.0);
		READMYSQL_DOUBLE(login_days, nLoginDay, 0.0);
		READMYSQL_DOUBLE(avg_online_times, nAvgOnlineTime, 0.0);
	}

	if(nRegisterDay <= 5.0)
		RELEASE_MYSQL_RETURN(psql, 0.0);

	rate = nLoginDay / nRegisterDay * nAvgOnlineTime / 1800 * 0.5;
	rate = MAX(0, MIN(0.5 , rate));

	RELEASE_MYSQL_RETURN(psql, rate);
}

/*
{istip:1,url:"http://www.boloomo.com",width:200,height:100} 
其中 istip 为 1 时代表需要弹出广告栏 
为 0，时不需要，后面的几个值可以不填 
url: 广告地址 
width:广告宽度 
height:广告高度 
*/
int AdSvc::getPhoneAD(const char* pUid, const char* jsonString, std::stringstream& out)
{
	out << "{istip:0}";
	return 0;

	JSON_PARSE_RETURN("[AdSvc::getPhoneAD]bad format:", jsonString, 1);

	/*
	1.	时间在活动开始时间和活动结束时间内
	2.	用户ID对应的电话余额小于等于0.15欧
	3.	用户未领取过话费
	4.	用户当前查看过的页面数小于7
	5.  如果用户之前已成功充值过话费，则不能再领取  add nicholas
	6.	用户当前已看到第N（N<7）步，最后一次看的时间为T，若T等于当前日期，则返回第N步对应的网页URL；若T小于当前日期，则返回第N+1步对应的网页URL， 同时将用户看到第几个页面及看的日期更新一下
	*/
	char sql[1024] = {0};
	MySql* psql = CREATE_MYSQL;
	sprintf (sql, "SELECT t2.product FROM t81_family_charge_user t1, t00_user_ip t2 WHERE t1.user_id = t2.user_id AND t1.user_id = '%s'", pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow() && strcasecmp(NOTNULL(psql->GetField("product")), "blm_family") == 0)
	{
		sprintf (sql, "SELECT step_num, page_size, step1_page, step1_display, step2_page, step2_display, pay_status, step3_page, step3_display FROM t81_family_charge_user WHERE user_id = '%s'", pUid);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(!psql->NextRow())
		{
			out << "{istip:0}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}
		string payStatus = NOTNULL(psql->GetField("pay_status"));
		string pSize = NOTNULL(psql->GetField("page_size"));
		int width = 0, height = 0;
		int stepNum = -1;
		int nDisplay = -1;
		sscanf(pSize.c_str(), "%dX%d", &width, &height);
		string url = "";

		if (payStatus[0] == '2')
		{
			out << "{istip:0}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}
		else
		{
			stepNum = atoi(NOTNULL(psql->GetField("step_num")));
			char tmp_display[32] = "";
			char tmp_page[32] = "";
			for (int i = 1; i <= stepNum; i++)
			{
				sprintf (tmp_display, "step%d_display", i);
				nDisplay = atoi(NOTNULL(psql->GetField(tmp_display)));
				if (nDisplay < 10)
				{
					sprintf (tmp_page, "step%d_page", i);
					url = NOTNULL(psql->GetField(tmp_page));
					if (i == 2)
					{
						string::size_type pos = url.find("XXX");
						string oldString = url.substr(pos, 5);
						if (pos == -1)
						{
							out << "{istip:0}";
							RELEASE_MYSQL_RETURN(psql, 0);
						}
						string newString = oldString;
						StrReplace(newString, "XXX", pUid);
						StrReplace(url, oldString, base64_encode((const unsigned char*)newString.c_str(), newString.length()));
					}
					nDisplay++;
					sprintf (sql, "UPDATE  t81_family_charge_user SET %s = '%d' WHERE user_id = '%s'", tmp_display, nDisplay, pUid);
					break;
				}
			}
		}

		if (url.length() > 0)
		{
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
			out << "{istip:1,url:\"" << base64_encode((const unsigned char*)url.c_str(), url.length()) << "\",width:" << width << ",height:" << height << "}";
		}
		else
			out << "{istip:0}";
	}
	else
	{
		// 中国用户
		sprintf(sql, "select country from t00_user where user_id = '%s'", pUid);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(!psql->NextRow() || strcasecmp(NOTNULL(psql->GetField("country")), "chn"))
		{
			out << "{istip:0}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}

		sprintf(sql, "select unix_timestamp(start_dt) as st, unix_timestamp(end_dt) as et from t81_special_act where act_id = '1'");
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(!psql->NextRow())
		{
			out << "{istip:0}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}

		ACE_Date_Time dateStart(ACE_Time_Value(atoi(psql->GetField("st"))));
		ACE_Date_Time dateEnd(ACE_Time_Value(atoi(psql->GetField("et"))));
		ACE_Date_Time today, lastDate;
		
		if(today.day() < dateStart.day() || today.day() > dateEnd.day())
		{
			out << "{istip:0}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}

		//如果用户之前已成功充值过话费，则不能再领取
		sprintf(sql, "select * from boloomodb.t02_order where product_id  = 'BLMPIPPHONE' and status_cd = 3 and user_id = '%s'", pUid);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->NextRow())
		{
			out << "{istip:0}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}

		int nLastPage = 1;
		char extraSql[1024] = {0};
		sprintf(sql, "SELECT last_event, unix_timestamp(last_event_dt) as lt, STATUS FROM t81_special_act_user WHERE user_id = '%s' AND act_id = '1'", pUid);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->NextRow())
		{
			nLastPage = atoi(psql->GetField("last_event"));
			lastDate = ACE_Date_Time(ACE_Time_Value(atoi(psql->GetField("lt"))));
			if(nLastPage>=6 || atoi(psql->GetField("status"))==0) //用户看完全部的页数或者已经领取过话费
			{
				out << "{istip:0}";
				RELEASE_MYSQL_RETURN(psql, 0);
			}
		}

		char gmt0now[20];
		GmtNow(gmt0now);

		if(lastDate.day() == today.day())
		{
			sprintf(sql, "SELECT act_key, act_key_val FROM t81_special_act_meta WHERE act_id = '1' AND act_key in( 'url%d', 'urlw', 'urlh')", nLastPage);
			sprintf(extraSql, "REPLACE INTO t81_special_act_user(act_id, user_id, last_event, last_event_dt, STATUS) VALUES ('1', '%s', '%d', '%s', 1)", pUid, nLastPage, gmt0now);
		}
		else
		{
			sprintf(sql, "SELECT act_key, act_key_val FROM t81_special_act_meta WHERE act_id = '1' AND act_key in( 'url%d', 'urlw', 'urlh')", nLastPage+1);
			sprintf(extraSql, "REPLACE INTO t81_special_act_user(act_id, user_id, last_event, last_event_dt, STATUS) VALUES ('1', '%s', '%d', '%s', 1)", pUid, nLastPage+1, gmt0now);
		}
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->GetRowCount() != 3)
		{
			out << "{istip:0}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}

		int nWidth = 0, nHeight = 0;
		char content[2*1024] = {0};
		while(psql->NextRow())
		{
			const char* pField = psql->GetField("act_key");
			const char* pValue = psql->GetField("act_key_val");
			if(!strcasecmp(pField, "urlw"))
				nWidth = atoi(pValue);
			else if(!strcasecmp(pField, "urlh"))
				nHeight = atoi(pValue);
			else
				strcat(content, pValue);
		}
		if(nWidth == 0 || nHeight == 0 || content[0] == '\0')
		{
			out << "{istip:0}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}
		string szContent(content);
		StrReplace(szContent, "XXXX", pUid);
		out << "{istip:1,url:\"" << base64_encode((const unsigned char*)szContent.c_str(),szContent.length()) << "\",width:" << nWidth << ",height:" << nHeight << "}";

		CHECK_MYSQL_STATUS(psql->Execute(extraSql)>=0, 3);
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}

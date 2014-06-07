#include "blmcom_head.h"
#include "WeatherSvc.h"
#include "UserRegSvc.h"
#include "MessageService.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include "NotifyService.h"
#include "GeoipHandler.h"

using namespace std;

WeatherSvc::WeatherSvc()
{
	m_count = 0;

	struct tm *ptr;
	time_t lt;
	lt = time(NULL);
	ptr = gmtime(&lt);
	m_maxyear = ptr->tm_year +1900;
	m_curryear = ptr->tm_year +1900;
}

WeatherSvc::~WeatherSvc()
{

}

IMPLEMENT_SERVICE_MAP(WeatherSvc)

bool WeatherSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_WEATHER, this))
        return false;

    SERVICE_MAP(SID_GETUSERREGION,WeatherSvc,getUserRegion);
    SERVICE_MAP(SID_GETYEARTYPHOONBASEINFO,WeatherSvc,getTyphoonBaseList);
    SERVICE_MAP(SID_GETONETYPHOONDETAIL,WeatherSvc,getOneTyphoonDetail);
    SERVICE_MAP(SID_GETFORECASTPOINTS,WeatherSvc,getOneTyphoonForecast);
    SERVICE_MAP(SID_GETCJWEATHERINFO,WeatherSvc,getCJWeather);

	DEBUG_LOG("[WeatherSvc::Start] OK......................................");
    return true;
}

int WeatherSvc::getUserRegion(const char* pUid, const char* jsonString, std::stringstream& out)
{
    int nregion = -1;
    string szcode ="";
   
    char sql[1024];
	sprintf(sql, "select ip from t00_user_ip where user_id ='%s'", pUid);

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
	{
		char *oneipstr = (char *)psql->GetField("ip");
		GeoIPRecord* pip = g_GeoipHandler::instance()->GetRecordByName(oneipstr);
		if(pip)
		{
			szcode = pip->country_code;
			GeoIPRecord_delete(pip);
		}
	}
	
    if (!szcode.empty())
    {
        sprintf(sql, "select location from t42_tc_country_area where iso2 ='%s'", szcode.c_str());
        CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->NextRow())
		{
			char *strregion = (char *)psql->GetField("location");
			nregion = atoi(strregion);
		}
    }

    if (nregion == -1)
    {
        nregion = 3;
    }
    
	out << "{region:" << nregion << "}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int WeatherSvc::getCJWeather(const char* pUid, const char* jsonString, std::stringstream& out)
{
    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    strcpy(sql, "select portnamecn,weatherinfo from t41_port_cj_weather");
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		if(total > 0)
			out << ",";
		total++;

		out<<"{id:\""<<NOTNULL(psql->GetField("portnamecn"))
			<<"\",des:\""<<NOTNULL(psql->GetField("weatherinfo"))<<"\"}";
	}
	out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int WeatherSvc::getOneTyphoonForecast(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[WeatherSvc::getOneTyphoonForecast]bad format:", jsonString, 1);
    string szID = root.getv("id", "");
    string szTM = root.getv("tm", "");
    string szDS = root.getv("ds", "");
	
    int narea = -1;
    if (szID.length() == 7)
    {
        narea = szID[4];
    }
    else
        return  2;


    int nsql = 0;
	char sql[1024];
    MySql* psql = CREATE_MYSQL;
    
    if (narea =='3')
    {
        int nds = atoi(szDS.c_str());
        if (nds == 0) //中国预测
        {
            sprintf(sql, "select UNIX_TIMESTAMP(analyses_tm) utc,ct_class,"\
                     "LATITUDE_24HR,LONGITUDE_24HR,PRESSURE_24HR,WIND_SPEED_24HR,"\
                     "LATITUDE_48HR,LONGITUDE_48HR,PRESSURE_48HR,WIND_SPEED_48HR,"\
                     "LATITUDE_72HR,LONGITUDE_72HR,PRESSURE_72HR,WIND_SPEED_72HR,"\
                     "LATITUDE_96HR,LONGITUDE_96HR,PRESSURE_96HR,WIND_SPEED_96HR,"\
                     "LATITUDE_120HR,LONGITUDE_120HR,PRESSURE_120HR,WIND_SPEED_120HR"\
                     " from t42_tc_3_china  where ct_id ='%s' and analyses_tm='%s'", szID.c_str(), szTM.c_str());
        }
        else if (nds ==1) //日本预测
        {
            nsql = 1;
            sprintf(sql, "SELECT * FROM t42_tc_3_japan t where ct_id ='%s' and analyses_tm='%s'", szID.c_str(), szTM.c_str());
        }
    }
    else
    {
        nsql = 2;
        sprintf(sql, "SELECT * FROM t42_tc_12_usa t where ct_id ='%s' and analyses_tm='%s'", szID.c_str(), szTM.c_str());
	}
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    out<<'[';
    while(psql->NextRow())
    {
        if (nsql == 0) //中国预测
        {
            const char* tc_class =  NOTNULL(psql->GetField("CT_CLASS"));
            const char* utc = NOTNULL(psql->GetField("utc"));
            time_t _tm = atoi(utc);
            for (int i=24; i<=120; i=i+24)
            {
                _tm+=86400;
                struct tm *forecasttime = localtime(&_tm);
                char tm_row[50];
                snprintf(tm_row,50,"%d-%d-%d %d:%d:%02d",forecasttime->tm_year+1900,forecasttime->tm_mon+1,forecasttime->tm_mday,forecasttime->tm_hour,forecasttime->tm_min,forecasttime->tm_sec);
                char lat_row[50];
                snprintf(lat_row,50,"LATITUDE_%dHR",i);
                char lon_row[50];
                snprintf(lon_row,50,"LONGITUDE_%dHR",i);
                char wspd_row[50];
                snprintf(wspd_row,50,"WIND_SPEED_%dHR",i);
                char pres_row[50];
                snprintf(pres_row,50,"PRESSURE_%dHR",i);

                const char * wspd	=	NOTNULL(psql->GetField(wspd_row));
                const char * pres	=	NOTNULL(psql->GetField(pres_row));
                const char * lat	=	NOTNULL(psql->GetField(lat_row));
                const char * lon	=	NOTNULL(psql->GetField(lon_row));
                out<<'\"'<<tm_row<<'|'<<tc_class<<'|'<<lat<<'|'
                    <<lon<<'|'<<pres<<'|'<<wspd<<"|N/A|N/A|N/A|N/A\"";
                if(i < 120) out<<',';
            }
        }
        else if (nsql == 1) //日本预测
        {
            const char*tc_class = NOTNULL(psql->GetField("CT_CLASS"));
            for (int i=1; i<=6; i++)
            {
                char tm_row[50];
                snprintf(tm_row,50,"FORECAST_TM_%d",i);
                char lat_row[50];
                snprintf(lat_row,50,"LATITUDE_%d",i);
                char lon_row[50];
                snprintf(lon_row,50,"LONGITUDE_%d",i);
                char spd_row[50];
                snprintf(spd_row,50,"MOVING_SPEED_%d",i);
                char wspd_row[50];
                snprintf(wspd_row,50,"WIND_SPEED_%d",i);
                char pres_row[50];
                snprintf(pres_row,50,"PRESSURE_%d",i);
                char dir_row[50];
                snprintf(dir_row,50,"MOVING_DIRECTION_%d",i);
                char rad7_row[50];
                snprintf(rad7_row,50,"RADIUS_30KT_%d",i);
                char rad10_row[50];
                snprintf(rad10_row,50,"RADIUS_50KT_%d",i);
                const char * tm		=	NOTNULL(psql->GetField(tm_row));
                const char * wspd	=	NOTNULL(psql->GetField(wspd_row));
                const char * pres	=	NOTNULL(psql->GetField(pres_row));
                const char * mspd	=	NOTNULL(psql->GetField(spd_row));
                const char * mdire	=	NOTNULL(psql->GetField(dir_row));
                const char * lat	=	NOTNULL(psql->GetField(lat_row));
                const char * lon	=	NOTNULL(psql->GetField(lon_row));
                const char * rad7	=	NOTNULL(psql->GetField(rad7_row));
                const char * rad10	=	NOTNULL(psql->GetField(rad10_row));
                out<<'\"'<<tm<<'|'<<tc_class<<'|'<<lat<<'|'
                    <<lon<<'|'<<pres<<'|'<<wspd<<'|'<<mspd<<'|'<<mdire<<'|'
                    <<rad7<<'|'<<rad10<<'\"';
                if(i < 6) out<<',';
            }
        }
        else if (nsql == 2) //美国预测
        {
            const char*tc_class = NOTNULL(psql->GetField("CT_CLASS"));
            for (int i=1; i<=5; i++)
            {
                char tm_row[50];
                snprintf(tm_row,50,"FORECAST_TM_%d",i);
                char lat_row[50];
                snprintf(lat_row,50,"LATITUDE_%d",i);
                char lon_row[50];
                snprintf(lon_row,50,"LONGITUDE_%d",i);
                char spd_row[50];
                snprintf(spd_row,50,"MOVING_SPEED_%d",i);
                char wspd_row[50];
                snprintf(wspd_row,50,"WIND_SPEED_%d",i);
                char pres_row[50];
                snprintf(pres_row,50,"PRESSURE_%d",i);
                char dir_row[50];
                snprintf(dir_row,50,"MOVING_DIRECTION_%d",i);
                char rad7_row[50];
                snprintf(rad7_row,50,"RANGE_34_KT_%d",i);
                char rad10_row[50];
                snprintf(rad10_row,50,"RANGE_64_KT_%d",i);
                const char * tm		=	NOTNULL(psql->GetField(tm_row));
                const char * wspd	=	NOTNULL(psql->GetField(wspd_row));
                const char * pres	=	NOTNULL(psql->GetField(pres_row));
                const char * mspd	=	NOTNULL(psql->GetField(spd_row));
                const char * mdire	=	NOTNULL(psql->GetField(dir_row));
                const char * lat	=	NOTNULL(psql->GetField(lat_row));
                const char * lon	=	NOTNULL(psql->GetField(lon_row));
                const char * rad7	=	NOTNULL(psql->GetField(rad7_row));
                const char * rad10	=	NOTNULL(psql->GetField(rad10_row));
                out<<'\"'<<tm<<'|'<<tc_class<<'|'<<lat<<'|'
                    <<lon<<'|'<<pres<<'|'<<wspd<<'|'<<mspd<<'|'<<mdire<<'|'
                    <<rad7<<'|'<<rad10<<'\"';
                if(i < 5)out<<',';
            }
        }
    }
    out<<']';
    RELEASE_MYSQL_RETURN(psql, 0);
}

int WeatherSvc::getOneTyphoonDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[WeatherSvc::getOneTyphoonDetail]bad format:", jsonString, 1);
    string szID = root.getv("id", "");

    int narea = -1;
    if (szID.size() == 7)
    {
        narea = szID[4];
    }
    else
        return 2;

	char sql[1024];
    MySql* psql = CREATE_MYSQL;
   
    //如果年小于当前年值，则证明是历史数据，需要从历史表里读取（to do）
    string id = szID;
    string sql_base;
    int queryyear = atoi(id.substr(0,4).c_str());

    {
        ACE_Guard<ACE_Thread_Mutex> guard(year_cs);
        if(queryyear<m_curryear)
        {
            sprintf(sql, "SELECT t.CT_ID,t.ANALYSES_TM,t.CT_CLASS,t.CENTRAL_PRESSURE,t.CENTRAL_WIND_SPEED,t.LATITUDE,t.LONGITUDE,"\
                       "MAX_RADIUS_30KT_DIR AS MOVING_DIRECTION,NULL AS MOVING_SPEED,MAX_RADIUS_30KT AS 7RADIUS,MAX_RADIUS_50KT AS  10RADIUS "\
                       "FROM t42_tc_his t where ct_id = '%s' order by analyses_tm", szID.c_str());
        }
        else
        {
            if (narea == '1' || narea == '2')
            {
                sprintf(sql, "SELECT t.CT_ID,t.ANALYSES_TM,t.CT_CLASS,t.CENTRAL_PRESSURE,t.CENTRAL_WIND_SPEED,t.LATITUDE,t.LONGITUDE,"\
                         "t.MOVING_DIRECTION,t.MOVING_SPEED,t.RANGE_34_KT 7RADIUS,t.RANGE_64_KT 10RADIUS FROM t42_tc_12_usa t where ct_id ='%s' order by t.analyses_tm", szID.c_str());
            }
            else
            {
                sprintf(sql, "SELECT t.CT_ID,t.ANALYSES_TM,t.CT_CLASS,t.CENTRAL_PRESSURE,t.CENTRAL_WIND_SPEED,t.LATITUDE,t.LONGITUDE,"\
                         "t.MOVING_DIRECTION,t.MOVING_SPEED,t.RADIUS_30KT 7RADIUS,t.RADIUS_50KT 10RADIUS FROM t42_tc_3_japan t where ct_id ='%s' order by t.analyses_tm", szID.c_str());
            }
        }
    }
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
    out<<'[';
    while(psql->NextRow())
    {
		if(total != 0)
			out << ",";
		total++;

        const char* tm = NOTNULL(psql->GetField("ANALYSES_TM"));
        const char* ct = NOTNULL(psql->GetField("CT_CLASS"));
        const char* pres = NOTNULL(psql->GetField("CENTRAL_PRESSURE"));
        const char* spd = NOTNULL(psql->GetField("CENTRAL_WIND_SPEED"));
        const char* mspd = NOTNULL(psql->GetField("MOVING_SPEED"));
        const char* mdire = NOTNULL(psql->GetField("MOVING_DIRECTION"));
        const char* lat = NOTNULL(psql->GetField("LATITUDE"));
        const char* lon = NOTNULL(psql->GetField("LONGITUDE"));
        const char* rad7 = NOTNULL(psql->GetField("7RADIUS"));
        const char* rad10 = NOTNULL(psql->GetField("10RADIUS"));
        out<<"\""<<tm<<"|"<<ct<<"|"<<lat<<"|"<<lon<<"|"<<pres<<"|"<<spd<<"|"<<mspd<<"|"<<mdire<<"|"<<rad7<<"|"<<rad10<<"|"<<szID<<"\"";
    }
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int WeatherSvc::getTyphoonBaseList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[WeatherSvc::getTyphoonBaseList]bad format:", jsonString, 1);
	string year = root.getv("year", "");

	int queryyear;

	char sql[1024];
    MySql* psql = CREATE_MYSQL;

    if (m_count%100 == 0)
    {
        strcpy(sql, "SELECT MAX(SUBSTR(CT_ID,1,4)) maxyear FROM T42_TC_INFO ");
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

		const char* maxyear = psql->GetField("maxyear");

		ACE_Guard<ACE_Thread_Mutex> guard(year_cs);
		m_maxyear = atoi(maxyear);

        struct tm *ptr;
        time_t lt;
        lt = time(NULL);
        ptr = gmtime(&lt);
        m_curryear = ptr->tm_year +1900;
    }
    if(m_maxyear == 0)
    {
        struct tm *ptr;
        time_t lt;
        lt = time(NULL);
        ptr = gmtime(&lt);

        ACE_Guard<ACE_Thread_Mutex> guard(year_cs);
        m_maxyear = ptr->tm_year + 1900;
    }
    if (year.empty())
    {
        queryyear = m_maxyear;
    }
    else
    {
        queryyear = atoi(year.c_str());
    }

    if(queryyear == m_curryear)
    {
        sprintf(sql, " (SELECT t1.ct_id,t1.name_en,t1.name_cn,t1.ct_status,t2.maxtm FROM T42_TC_INFO t1 "\
                 " INNER JOIN (SELECT CT_ID,MAX(ANALYSES_TM) maxtm FROM T42_TC_12_usa GROUP BY CT_ID) AS t2 ON t2.CT_ID = t1.CT_ID  where t1.ct_id like '%d%%' ) "\
                 " union "\
                 " (SELECT t3.ct_id,t3.name_en,t3.name_cn,t3.ct_status,t4.maxtm FROM T42_TC_INFO t3 "\
                 " INNER JOIN (SELECT CT_ID,MAX(ANALYSES_TM) maxtm FROM T42_TC_3_japan GROUP BY CT_ID) AS t4 ON t4.CT_ID = t3.CT_ID  where t3.ct_id like '%d%%' ) order by maxtm",queryyear,queryyear);
    }
    else
    {
        sprintf(sql, "SELECT t1.ct_id,t1.name_en,t1.name_cn,t1.ct_status,t2.maxtm FROM T42_TC_INFO t1 "\
                 " INNER JOIN "\
                 " (SELECT CT_ID,max(analyses_tm) maxtm FROM T42_TC_HIS WHERE CT_ID LIKE '%d%%' GROUP BY CT_ID order by maxtm) AS t2 "\
                 " ON t1.ct_id=t2.ct_id",queryyear);
    }
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    
	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* id		= psql->GetField("ct_id");
		const char* name_en = psql->GetField("name_en");
		const char* name_cn = psql->GetField("name_cn");
		const char* st		= psql->GetField("ct_status");
		const char* maxtm	= psql->GetField("maxtm");

		out<<"{id:\""<< NOTNULL(id)<<"\","
			<<"chn:\""<< NOTNULL(name_cn)<<"\","
			<<"eng:\""<< NOTNULL(name_en)<<"\","
			<<"st:\""<< NOTNULL(st)<<"\","
			<<"lasttime:\""<< NOTNULL(maxtm)<<"\"}";
	}
	out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

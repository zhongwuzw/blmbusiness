#include "blmcom_head.h"
#include "WebSvc.h"
#include "Pinyin4j.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"
#include "xbase64.h"
#include "json.h"

IMPLEMENT_SERVICE_MAP(WebSvc)

WebSvc::WebSvc()
{

}

WebSvc::~WebSvc()
{

}

bool WebSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_WEB, this))
        return false;

	loadCnEn();

    SERVICE_MAP(SID_WEB_ALL_DANGER,WebSvc,getAllDangers);
    SERVICE_MAP(SID_WEB_ALL_PORT,WebSvc,getAllPorts);
	SERVICE_MAP(SID_WEB_ALL_TIDE,WebSvc,getAllTides);
	SERVICE_MAP(SID_WEB_SHIP_REMARK,WebSvc,getShipRemark);
	SERVICE_MAP(SID_WEB_ADD_SHIP_REMARK,WebSvc,addShipRemark);
	SERVICE_MAP(SID_WEB_DEL_SHIP_REMARK,WebSvc,delShipRemark);
	SERVICE_MAP(SID_WEB_ALL_WEATHER,WebSvc,getAllWeathers);
	SERVICE_MAP(SID_WEB_SHANGHAI_FLEET,WebSvc,getShanghaiFleet);
	SERVICE_MAP(SID_WEB_HWD_ALL_CLOCK,WebSvc,getAllClockEvent);
	SERVICE_MAP(SID_WEB_HWD_UPD_CLOCK,WebSvc,updateClockEvent);
	SERVICE_MAP(SID_WEB_TIANTUO,WebSvc,getTiantuo);
	SERVICE_MAP(SID_WEB_SEARCH,WebSvc,search);
	SERVICE_MAP(SID_WEB_BUAA_SEARCH,WebSvc,buaaSearch);

	/*
	const char* jsonString = "{pagecur:1,pagemax:10,name:\"\",flags:\"CHN,JPN\",types:\"\",min_length:0,max_length:0,min_width:0,max_width:0,min_dwt:0,max_dwt:0,min_draft:0,max_draft:0}";
	std::stringstream out;
	buaaSearch("caiwj", jsonString, out);
	*/

    DEBUG_LOG("[WebSvc::Start] OK......................................");
    return true;
}

bool WebSvc::loadCnEn()
{
	MySql* psql = CREATE_MYSQL;

	const char* sql = "SELECT namecn, nameen FROM t16_ocean_weather_direction";
	CHECK_MYSQL_STATUS(psql->Query(sql), false);
	while(psql->NextRow())
	{
		m_mapCnEnDirection[psql->GetField("namecn")] = NOTNULL(psql->GetField("nameen"));
	}

	sql = "SELECT namecn, nameen FROM t16_ocean_weather_point";
	CHECK_MYSQL_STATUS(psql->Query(sql), false);
	while(psql->NextRow())
	{
		m_mapCnEnName[psql->GetField("namecn")] = NOTNULL(psql->GetField("nameen"));
	}

	sql = "SELECT namecn, nameen, picurl FROM t16_ocean_weather_desc";
	CHECK_MYSQL_STATUS(psql->Query(sql), false);
	while(psql->NextRow())
	{
		_WeatherDesc desc;
		READMYSQL_STR(namecn, desc.nameCN);
		READMYSQL_STR(nameen, desc.nameEN);
		READMYSQL_STR(picurl, desc.picUrl);

		m_mapCnEnDesc[desc.nameCN] = desc;
	}

	RELEASE_MYSQL_RETURN(psql, true);
}

int WebSvc::getAllDangers(const char* pUid, const char* jsonString, std::stringstream& out)
{
	char sql[1024];
	sprintf(sql, "select areaid, name, pointset, point, descript, type from t41_dangerous_area");

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";
	int total = 0;
	while(psql->NextRow())
	{
		out << (total==0?"":",") <<
			"[\"" << psql->GetField("areaid") << 
			"\",\"" << psql->GetField("name") << 
			"\",\"" << psql->GetField("pointset") << 
			"\",\"" << psql->GetField("point") << 
			"\",\"" << psql->GetField("descript") << 
			"\",\"" << psql->GetField("type") << "\"]";
		total++;
	}
	out << "]";
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

int WebSvc::getAllPorts(const char* pUid, const char* jsonString, std::stringstream& out)
{
    char sql[1024];
	sprintf(sql, "SELECT t1.portid, t1.name, t1.longitude_dd, t1.latitude_dd, t1.namecn, t1.iso3, t1.level, t2.range FROM t41_port t1 left join t41_port_area t2 on t1.portid = t2.port_id where t1.level is not null");

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";
	int total = 0;
	while(psql->NextRow())
	{
		out << (total==0?"":",") <<
			"[\"" << psql->GetField("portid") << 
			"\",\"" << psql->GetField("name") << 
			"\",\"" << psql->GetField("longitude_dd") << 
			"\",\"" << psql->GetField("latitude_dd") << 
			"\",\"" << psql->GetField("level") << 
			"\",\"" << psql->GetField("namecn") << 
			"\",\"" << psql->GetField("iso3") << 
			"\",\"" << NOTNULL(psql->GetField("range")) << "\"]";
		total++;
	}
	out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int WebSvc::getAllTides(const char* pUid, const char* jsonString, std::stringstream& out)
{
	MySql* psql = CREATE_MYSQL;
	const char* sql = "select port_id, station_id, station_name, longitude, latitude, timezone, level from t41_tide_index WHERE DATA_STAT!=0";
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";
	int total = 0;

	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		_TideInfo tide;
		READMYSQL_STR(port_id, tide.portid);
		READMYSQL_STR(station_id, tide.stationId);
		READMYSQL_STR(station_name, tide.stationName);
		READMYSQL_DOUBLE(longitude, tide.longitude, 0.0);
		READMYSQL_DOUBLE(latitude, tide.latitude, 0.0);
		READMYSQL_INT(timezone, tide.timezone, 0);
		READMYSQL_INT(level, tide.level, 0);

		out << "[\"" 
			<< tide.portid << "\",\"" 
			<< tide.stationId << "\",\"" 
			<< tide.stationName << "\",\"" 
			<< tide.longitude << "\",\"" 
			<< tide.latitude  << "\",\""
			<< tide.timezone << "\",\"" 
			<< tide.level << "\"]";
	}
	out << "]";

	return 0;
}

int WebSvc::getAllWeathers(const char* pUid, const char* jsonString, std::stringstream& out)
{
	MySql* psql = CREATE_MYSQL;
	const char* sql = "SELECT t1.namecn, t1.type, t1.longitude, t1.latitude, t1.forecastdayhour, t2.ForecastHour, t2.Weather, t2.WindDirection, t2.WindLevel, t2.WaveHeight, t2.Visibility\
					  FROM t16_ocean_weather_point t1 LEFT JOIN t16_ocean_weather t2 ON t1.NameCn = t2.NameCn\
					  ORDER BY t1.NameCn, t1.Type, t2.ForecastHour";
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	MapWeather mapWeather;
	while(psql->NextRow())
	{
		_WeatherInfo weather;
		READMYSQL_STR(namecn, weather.name);
		READMYSQL_INT(type, weather.type, 0);
		READMYSQL_FLOAT(longitude, weather.longitude, 0.0);
		READMYSQL_FLOAT(latitude, weather.latitude, 0.0);
		READMYSQL_STR(forecastdayhour, weather.forecastDayHour);
		READMYSQL_STR(ForecastHour, weather.forecastHour);
		READMYSQL_STR(Weather, weather.weather);
		READMYSQL_STR(WindDirection, weather.windDirection);
		READMYSQL_STR(WindLevel, weather.windLevel);
		READMYSQL_STR(WaveHeight, weather.waveHeight);
		READMYSQL_STR(Visibility, weather.visible);

		// 1.目前数据里有个重复的名字（琼州海峡,这里只取一个）
		// 2.对一个地点只返回两个预测的数据（24小时）
		MapWeather::iterator iter = mapWeather.find(weather.name);
		if(iter != mapWeather.end())
		{
			if(iter->second.size() >= 2)
				continue;
			iter->second.push_back(weather);
		}
		else
		{
			VecWeather weathers;
			weathers.push_back(weather);
			mapWeather.insert(MapWeather::value_type(weather.name, weathers));
		}
	}

	out << "[";
	for(MapWeather::iterator iter = mapWeather.begin(); iter != mapWeather.end(); iter++)
	{
		if(iter != mapWeather.begin())
			out << ",";

		VecWeather& weathers = iter->second;
		double longitude	= weathers[0].longitude;
		double latitude		= weathers[0].latitude;
		int type			= weathers[0].type;
		std::string dayhour = weathers[0].forecastDayHour;
		std::string nameEN	= m_mapCnEnName[iter->first];
		std::string picUrl	= "";
		std::stringstream buff;

		for(int i=0; i<(int)weathers.size(); i++)
		{
			if(i!=0)
				buff << "|";

			std::string weatherDescEN = weathers[i].weather;
			MapWeatherDesc::iterator iterDesc = m_mapCnEnDesc.find(weatherDescEN);
			if(iterDesc == m_mapCnEnDesc.end())
			{
				// 数据有问题(该天气描述不在数据库中)
				printf("not found weather:%s\n", CodeConverter::Utf8ToGb2312(weatherDescEN.c_str()).c_str());
			}
			else
			{
				weatherDescEN = iterDesc->second.nameEN;
				if(i==0)
				{
					picUrl = iterDesc->second.picUrl;
				}
			}

			std::string windDirectionEN = weathers[i].windDirection;
			StrStrMapIter iterDirection = m_mapCnEnDirection.find(windDirectionEN);
			if(iterDirection == m_mapCnEnDirection.end())
			{
				// 数据有问题（该方向描述不在数据库中）
				printf("not found wind direction:%s\n", CodeConverter::Utf8ToGb2312(windDirectionEN.c_str()).c_str());
			}
			else
			{
				windDirectionEN = iterDirection->second;
			}

			buff << FormatString("%s,%s,%s,%s,%s,%s,%s,%s", 
				weathers[i].forecastHour, weathers[i].weather, weatherDescEN.c_str(), weathers[i].windDirection, windDirectionEN.c_str(), weathers[i].windLevel, weathers[i].waveHeight, weathers[i].visible);
		}

		out << FormatString("[\"%s\",\"%s\",\"%d\",\"%.4f\",\"%.4f\",\"%s\",\"%s\",", 
			iter->first.c_str(), nameEN.c_str(), type, longitude, latitude, picUrl.c_str(), dayhour.c_str());
		out << "\"" << buff.str() << "\"]";
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int WebSvc::getShipRemark(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WebSvc::getShipRemark]bad format:", jsonString, 1);
	int mmsi = root.getv("mmsi", 0);
	int time = root.getv("time", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[128];
	sprintf(sql, "select time,title,remark,author from t41_ship_remark where mmsi = '%d' and time > '%d'", mmsi, time);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";
	int total = 0;
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		_ShipRemarkInfo remark;
		remark.mmsi = mmsi;
		READMYSQL_INT(time, remark.time, 0);
		READMYSQL_STR(title, remark.title);
		READMYSQL_STR(remark, remark.remark);
		READMYSQL_STR(author, remark.author);
		
		out << FormatString("[\"%d\",\"%d\",\"%s\",\"%s\",\"%s\"]", remark.mmsi, remark.time, remark.title, remark.remark, remark.author);
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int WebSvc::addShipRemark(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WebSvc::addShipRemark]bad format:", jsonString, 1);
	int mmsi = root.getv("mmsi", 0);
	int time = root.getv("time", 0);
	string title = root.getv("title", "");
	string remark = root.getv("remark", "");
	string author = root.getv("author", "");

	MySql* psql = CREATE_MYSQL;
	char sql[10*1024];
	sprintf(sql, "insert ignore into t41_ship_remark(mmsi,time,title,remark,author) values ('%d', '%d', '%s', '%s', '%s')",
					mmsi, time, title.c_str(), remark.c_str(), author.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

int WebSvc::delShipRemark(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WebSvc::delShipRemark]bad format:", jsonString, 1);
	int mmsi = root.getv("mmsi", 0);
	int time = root.getv("time", 0);
	string author = root.getv("author", "");

	MySql* psql = CREATE_MYSQL;
	char sql[10*1024];
	sprintf(sql, "delete from t41_ship_remark where mmsi = %d and time = %d and author = '%s'",
		mmsi, time, author.c_str());
	psql->Execute(sql);

	RELEASE_MYSQL_RETURN(psql, 0);
}

int WebSvc::getShanghaiFleet(const char* pUid, const char* jsonString, std::stringstream& out)
{
	MySql* psql = CREATE_MYSQL;

	//泊位数据
	const char* sql = "SELECT berth_id,portid,NAME,coordinate FROM t41_port_berth WHERE portid = '40440' AND coordinate IS NOT NULL";
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out << "{berth:[";
	
	while(psql->NextRow())
	{
		if(total++ != 0)
			out << ",";

		_BerthInfo berth;
		READMYSQL_STR(berth_id, berth.id);
		READMYSQL_STR(portid, berth.portid);
		READMYSQL_STR(name, berth.name);
		READMYSQL_STR(coordinate, berth.lonlat);

		out << FormatString("{id:\"%s\",portid:\"%s\",name:\"%s\",lonlat:\"%s\"}", berth.id, berth.portid, berth.name, berth.lonlat);
	}
	
	//锚地数据
	sql = "SELECT anchorageid, portid, NAME, locpointset FROM t41_port_anchorage WHERE portid = 40440";
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	total = 0;
	out << "],anchorage:[";

	while(psql->NextRow())
	{
		if(total++ != 0)
			out << ",";

		_AnchorageInfo anchorage;
		READMYSQL_STR(anchorageid, anchorage.id);
		READMYSQL_STR(portid, anchorage.portid);
		READMYSQL_STR(name, anchorage.name);
		READMYSQL_STR(locpointset, anchorage.lonlats);

		out << FormatString("{id:\"%s\",portid:\"%s\",name:\"%s\",lonlats:\"%s\"}", anchorage.id, anchorage.portid, anchorage.name, anchorage.lonlats);
	}

	//船队数据
	sql = "SELECT id, NAME, lonlat FROM t41_fleet_shanghai";
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	total = 0;
	out << "],fleet:[";

	while(psql->NextRow())
	{
		if(total++ != 0)
			out << ",";

		_ShanghaiFleetInfo fleet;
		READMYSQL_STR(id, fleet.id);
		READMYSQL_STR(name, fleet.name);
		READMYSQL_STR(lonlat, fleet.lonlat);

		out << FormatString("{id:\"%s\",name:\"%s\",lonlat:\"%s\"}", fleet.id, fleet.name, fleet.lonlat);
	}

	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int WebSvc::getAllClockEvent(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WebSvc::getAllClockEvent]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	if(uid.empty())
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[10*1024];
	sprintf(sql, "select mmsi, timeflag, clocks from aisdb.t41_aisevt_ship_subscribe_hwd where userid = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out << "[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		out << FormatString("{mmsi:%s,timeflag:%s,clocks:\"%s\"}", psql->GetField("mmsi"), psql->GetField("timeflag"), psql->GetField("clocks"));
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int WebSvc::updateClockEvent(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WebSvc::updateClockEvent]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string mmsi = root.getv("mmsi", "");
	string timeflag = root.getv("timeflag", "");
	string clocks = root.getv("clocks", "");
	if(uid.empty() || mmsi.empty() || timeflag.empty() || clocks.empty())
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[10*1024];
	sprintf(sql, "replace into aisdb.t41_aisevt_ship_subscribe_hwd(userid, mmsi, timeflag, clocks) values ('%s', '%s', '%s', '%s')", uid.c_str(), mmsi.c_str(), timeflag.c_str(), clocks.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

int WebSvc::getTiantuo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	char sql[1024];
	sprintf(sql, "SELECT mmsi, TIME, nav_status, sog, longitude, latitude, cog, true_head, imo, callsign, NAME, ship_type, LENGTH, width, draught, eta, dest FROM aisdb.t41_ais_ship_realtime_tiantuo");

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";
	int total = 0;
	while(psql->NextRow())
	{
		out << (total==0?"":",") <<
			"[\"" << NOTNULL(psql->GetField("mmsi")) << 
			"\",\"" << NOTNULL(psql->GetField("time")) << 
			"\",\"" << NOTNULL(psql->GetField("nav_status")) << 
			"\",\"" << NOTNULL(psql->GetField("sog")) << 
			"\",\"" << NOTNULL(psql->GetField("longitude")) << 
			"\",\"" << NOTNULL(psql->GetField("latitude")) << 
			"\",\"" << NOTNULL(psql->GetField("cog")) << 
			"\",\"" << NOTNULL(psql->GetField("true_head")) << 
			"\",\"" << NOTNULL(psql->GetField("imo")) << 
			"\",\"" << NOTNULL(psql->GetField("callsign")) << 
			"\",\"" << NOTNULL(psql->GetField("NAME")) << 
			"\",\"" << NOTNULL(psql->GetField("ship_type")) << 
			"\",\"" << NOTNULL(psql->GetField("LENGTH")) << 
			"\",\"" << NOTNULL(psql->GetField("width")) << 
			"\",\"" << NOTNULL(psql->GetField("draught")) << 
			"\",\"" << NOTNULL(psql->GetField("eta")) << 
			"\",\"" << NOTNULL(psql->GetField("dest")) << "\"]";
		total++;
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

struct _ShipBase
{
	char shipid[64];
	char mmsi[10];
	char name[64];
	char flag[10];
	char type[3];
	char length[10];
	char width[10];
	char dwt[10];
	char draft[10];
	int lasttm;
};
typedef std::vector<_ShipBase> ShipBaseList;

bool refreshTimeOfShips(ShipBaseList& ships)
{
	if(ships.empty())
		return false;

	stringstream ss;
	ss << "select mmsi, time from aisdb.t41_ais_ship_realtime where mmsi in (";
	for(int i=0; i<(int)ships.size(); i++)
	{
		ss << (i==0?"":",");
		ss << ships[i].mmsi;
	}
	ss << ")";

	map<int, int> mmsiTimeMap;
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(ss.str().c_str()), false);

	while(psql->NextRow())
	{
		int mmsi, time;
		READMYSQL_INT(mmsi, mmsi, 0);
		READMYSQL_INT(time, time, 0);
		mmsiTimeMap[mmsi] = time;
	}

	map<int, int>::iterator iter;
	for(int i=0; i<(int)ships.size(); i++)
	{
		if((iter=mmsiTimeMap.find(atoi(ships[i].mmsi))) != mmsiTimeMap.end())
			ships[i].lasttm = iter->second;
	}

	RELEASE_MYSQL_RETURN(psql, true);
}

//{text:"",pagecur:1,pagemax:10}
int WebSvc::search(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WebSvc::search]bad format:", jsonString, 1);
	const char* text = root.getv("text", "");
	int pagecur = root.getv("pagecur", 0);
	int pagesize = root.getv("pagemax", 0);

	if(pagecur <= 0 || pagesize <= 0 || strlen(text) <= 3)
		return 1;

	char countSql[2048];
	char sql[2048];
	bool isdigit = IsNumeric(text);
	if(isdigit)
	{
		sprintf(countSql, "select count(1) as num from boloomodb.t41_ship where mmsi is not null and (mmsi like '%s%%' or imo like '%s%%')", text, text);
		sprintf(sql, "select shipid, left(shiptype_key,2) as type, name, mmsi, country_code from boloomodb.t41_ship where mmsi is not null and (mmsi like '%s%%' or imo like '%s%%') order by name limit %d,%d", text, text, (pagecur-1)*pagesize, pagesize);
	}
	else
	{
		//将中文转成拼音
		int n = 0;
		char **pytxt = g_Pinyin4j::instance()->FindPinYin(CodeConverter::Utf8ToUnicode(text).c_str(), n);
		if(n == 0)
			return 2;
		std::string _text(pytxt[0]);
		FreeArray(pytxt, n);

		sprintf(countSql, "select count(1) as num from boloomodb.t41_ship where mmsi is not null and (name like '%s%%' or callsign like '%s%%')", _text.c_str(), _text.c_str());
		sprintf(sql, "select shipid, left(shiptype_key,2) as type, name, mmsi, country_code from boloomodb.t41_ship where mmsi is not null and (name like '%s%%' or callsign like '%s%%') order by name  limit %d,%d", _text.c_str(), _text.c_str(), (pagecur-1)*pagesize, pagesize);
	}

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(countSql)&&psql->NextRow(), 3);
	int total = atoi(psql->GetField("num"));
	if(total == 0)
	{
		out<<FormatString("{pagecur:%d,pagesize:%d,amount:0,details:[]}", pagecur, pagesize);
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	ShipBaseList result;
	while(psql->NextRow())
	{
		_ShipBase shipBase;
		shipBase.lasttm = 0;
		READMYSQL_STR(shipid, shipBase.shipid);
		READMYSQL_STR(type, shipBase.type);
		READMYSQL_STR(name, shipBase.name);
		READMYSQL_STR(mmsi, shipBase.mmsi);
		READMYSQL_STR(country_code, shipBase.flag);
		if(strlen(shipBase.mmsi) == 0)
			continue;
		if(shipBase.type[0] == '\0')
			strcpy(shipBase.type, "0");

		result.push_back(shipBase);
	}
	refreshTimeOfShips(result);

	out<<FormatString("{pagecur:%d,pagesize:%d,amount:%d,details:[", pagecur, pagesize, total);
	for(int i=0; i<(int)result.size(); i++)
	{
		if(i != 0)
			out << ",";
		out << FormatString("{mmsi:\"%s\",id:\"%s\",name:\"%s\",country:\"%s\",type:%s,lasttm:%d}", result[i].mmsi, result[i].shipid, result[i].name, result[i].flag, result[i].type, result[i].lasttm);
	}
	out << "]}";

	return 0;
}

//{pagecur:1,pagemax:10,name:"",flags:"CHN,JPN",types:"7,8",min_length:0,max_length:0,min_width:0,max_width:0,min_dwt:0,max_dwt:0,min_draft:0,max_draft:0}
int WebSvc::buaaSearch(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[WebSvc::search]bad format:", jsonString, 1);
	string name = root.getv("name", "");
	string flags = root.getv("flags", "");
	string types = root.getv("types", "");
	int pagecur = root.getv("pagecur", 0);
	int pagesize = root.getv("pagemax", 0);
	int minLength = root.getv("min_length", 0);
	int maxLength = root.getv("max_length", 0);
	int minWidth = root.getv("min_width", 0);
	int maxWidth = root.getv("max_width", 0);
	int minDwt = root.getv("min_dwt", 0);
	int maxDwt = root.getv("max_dwt", 0);
	int minDraft = root.getv("min_draft", 0);
	int maxDraft = root.getv("max_draft", 0);

	// 参数判断
	if(name.empty() && flags.empty() && types.empty() && 
		minLength==0 && maxLength==0 &&
		minWidth==0 && maxWidth==0 &&
		minDwt==0 && maxDwt==0 &&
		minDraft==0 && maxDraft==0)
		return 1;

	if(pagecur <= 0 || pagesize <= 0)
		return 1;

	// 拼SQL
	if(!flags.empty()) {
		StrReplace(flags, ",", "','");
		flags = "'" + flags + "'";
	}

	bool hasAnd = false;
	string buaaCountSQL = "select count(1) as num from t41_ship_buaa where ";
	string buaaSQL = "select shipid,mmsi,name,country,type,length,width,dwt,draft from t41_ship_buaa where ";
	string whereSQL;
	if(!name.empty()) { whereSQL += FormatString("trimname like '%s%%'", name.c_str()); hasAnd = true;}
	if(!flags.empty()) { whereSQL += FormatString("%s country in (%s)", (hasAnd?" and":""), flags.c_str()); hasAnd = true;}
	if(!types.empty()) { whereSQL += FormatString("%s type in (%s)", (hasAnd?" and":""), types.c_str()); hasAnd = true;}
	if(minLength != 0) { whereSQL += FormatString("%s length>=%d", (hasAnd?" and":""), minLength); hasAnd = true; }
	if(maxLength != 0) { whereSQL += FormatString("%s length<=%d", (hasAnd?" and":""), maxLength); hasAnd = true; }
	if(minWidth != 0) { whereSQL += FormatString("%s width>=%d", (hasAnd?" and":""), minWidth); hasAnd = true; }
	if(maxWidth != 0) { whereSQL += FormatString("%s width<=%d", (hasAnd?" and":""), maxWidth); hasAnd = true; }
	if(minDwt != 0) { whereSQL += FormatString("%s dwt>=%d", (hasAnd?" and":""), minDwt); hasAnd = true; }
	if(maxDwt != 0) { whereSQL += FormatString("%s dwt<=%d", (hasAnd?" and":""), maxDwt); hasAnd = true; }
	if(minDraft != 0) { whereSQL += FormatString("%s draft>=%d", (hasAnd?" and":""), minDraft); hasAnd = true; }
	if(maxDraft != 0) { whereSQL += FormatString("%s draft<=%d", (hasAnd?" and":""), maxDraft); hasAnd = true; }
	buaaCountSQL += whereSQL;
	buaaSQL += whereSQL + FormatString(" order by name limit %d,%d", (pagecur-1)*pagesize, pagesize);

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(buaaCountSQL.c_str())&&psql->NextRow(), 3);
	int total = atoi(psql->GetField("num"));
	if(total == 0)
	{
		out<<FormatString("{pagecur:%d,pagesize:%d,amount:0,details:[]}", pagecur, pagesize);
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	CHECK_MYSQL_STATUS(psql->Query(buaaSQL.c_str()), 3);
	ShipBaseList result;
	while(psql->NextRow())
	{
		_ShipBase shipBase;
		shipBase.lasttm = 0;
		READMYSQL_STR(shipid, shipBase.shipid);
		READMYSQL_STR(mmsi, shipBase.mmsi);
		READMYSQL_STR(name, shipBase.name);
		READMYSQL_STR(country, shipBase.flag);
		READMYSQL_STR(type, shipBase.type);
		READMYSQL_STR(length, shipBase.length);
		READMYSQL_STR(width, shipBase.width);
		READMYSQL_STR(dwt, shipBase.dwt);
		READMYSQL_STR(draft, shipBase.draft);
		
		if(strlen(shipBase.mmsi) == 0)
			continue;
		if(shipBase.type[0] == '\0')
			strcpy(shipBase.type, "0");

		result.push_back(shipBase);
	}
	//refreshTimeOfShips(result);

	out<<FormatString("{pagecur:%d,pagesize:%d,amount:%d,details:[", pagecur, pagesize, total);
	for(int i=0; i<(int)result.size(); i++)
	{
		if(i != 0)
			out << ",";
		out << FormatString("{mmsi:\"%s\",id:\"%s\",name:\"%s\",country:\"%s\",type:%d,length:%d,width:%d,dwt:%d,draft:%d}", result[i].mmsi, result[i].shipid, result[i].name, result[i].flag, atoi(result[i].type), atoi(result[i].length), atoi(result[i].width), atoi(result[i].dwt), atoi(result[i].draft));
	}
	out << "]}";

	return 0;
}

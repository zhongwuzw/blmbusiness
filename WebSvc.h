#ifndef __WEB_SVC_H__
#define __WEB_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

struct _WeatherInfo
{
	char name[64];
	char forecastDayHour[20];
	char forecastHour[20];
	char weather[20];
	char windDirection[20];
	char windLevel[20];
	char waveHeight[20];
	char visible[20];

	double	longitude;
	double	latitude;
	int		type; // 1/2/3

	_WeatherInfo() {
		memset(this, 0, sizeof(_WeatherInfo));
	}
};
typedef std::vector<_WeatherInfo> VecWeather;
typedef std::map<std::string, VecWeather> MapWeather;

struct _TideInfo
{
	char portid[64];
	char stationId[64];
	char stationName[64];

	double	longitude;
	double	latitude;
	int		timezone;
	int		level;

	_TideInfo() {
		memset(this, 0, sizeof(_TideInfo));
	}
};

struct _ShipRemarkInfo
{
	int		mmsi;
	int		time;
	char	title[64];
	char	remark[2048];
	char	author[64];

	_ShipRemarkInfo() {
		memset(this, 0, sizeof(_ShipRemarkInfo));
	}
};

struct _BerthInfo
{
	char id[64];
	char portid[64];
	char name[64];
	char lonlat[64];

	_BerthInfo() {
		memset(this, 0, sizeof(_BerthInfo));
	}
};

struct _AnchorageInfo
{
	char id[64];
	char portid[64];
	char name[64];
	char lonlats[2048];

	_AnchorageInfo() {
		memset(this, 0, sizeof(_AnchorageInfo));
	}
};

struct _ShanghaiFleetInfo
{
	char id[64];
	char name[64];
	char lonlat[64];

	_ShanghaiFleetInfo() {
		memset(this, 0, sizeof(_ShanghaiFleetInfo));
	}
};

struct _WeatherDesc
{
	char nameCN[64];
	char nameEN[64];
	char picUrl[64];

	_WeatherDesc() {
		memset(this, 0, sizeof(_WeatherDesc));
	}
};
typedef std::map<std::string, _WeatherDesc> MapWeatherDesc;

class WebSvc : IBusinessService
{
public:
    WebSvc();
    ~WebSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(WebSvc)

private:
    int getAllDangers(const char* pUid, const char* jsonString, std::stringstream& out);
    int getAllPorts(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAllTides(const char* pUid, const char* jsonString, std::stringstream& out);
	int getShipRemark(const char* pUid, const char* jsonString, std::stringstream& out);
	int addShipRemark(const char* pUid, const char* jsonString, std::stringstream& out);
	int delShipRemark(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAllWeathers(const char* pUid, const char* jsonString, std::stringstream& out);
	int getShanghaiFleet(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAllClockEvent(const char* pUid, const char* jsonString, std::stringstream& out);
	int updateClockEvent(const char* pUid, const char* jsonString, std::stringstream& out);
	int getTiantuo(const char* pUid, const char* jsonString, std::stringstream& out);
	int search(const char* pUid, const char* jsonString, std::stringstream& out);
	int buaaSearch(const char* pUid, const char* jsonString, std::stringstream& out);

private:
	bool			loadCnEn();
	StrStrMap		m_mapCnEnName;
	StrStrMap		m_mapCnEnDirection;
	MapWeatherDesc	m_mapCnEnDesc;
};

typedef ACE_Singleton<WebSvc, ACE_Null_Mutex> g_WebSvc;

#endif

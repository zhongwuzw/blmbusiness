#ifndef __WEATHER_SVC_H__
#define __WEATHER_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include <vector>

using namespace std;

class WeatherSvc : IBusinessService
{
public:
    WeatherSvc();
    ~WeatherSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(WeatherSvc)

private:
    int getUserRegion(const char* pUid, const char* jsonString, std::stringstream& out);
    int getCJWeather(const char* pUid, const char* jsonString, std::stringstream& out);
    int getOneTyphoonForecast(const char* pUid, const char* jsonString, std::stringstream& out);
    int getOneTyphoonDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int getTyphoonBaseList(const char* pUid, const char* jsonString, std::stringstream& out);

    ACE_Thread_Mutex year_cs;

    int m_maxyear;	//用来记录当前台风记录里的最大年份和最小年份
    int m_count;	//用来计数，每20次请求需要重新读取一次最大年份和最小年份
    int m_curryear; //当前年

};

typedef ACE_Singleton<WeatherSvc, ACE_Null_Mutex> g_WeatherSvc;




#endif

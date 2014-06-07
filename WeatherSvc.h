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

    int m_maxyear;	//������¼��ǰ̨���¼��������ݺ���С���
    int m_count;	//����������ÿ20��������Ҫ���¶�ȡһ�������ݺ���С���
    int m_curryear; //��ǰ��

};

typedef ACE_Singleton<WeatherSvc, ACE_Null_Mutex> g_WeatherSvc;




#endif

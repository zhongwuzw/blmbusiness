#ifndef __MONITOR_SVC_H__
#define __MONITOR_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"
#include <vector>
#include <map>

class MonitorSvc : IBusinessService
{
public:
    MonitorSvc();
    ~MonitorSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(MonitorSvc)

private:
    int getMonitorsOfEntify(const char* pUid, const char* jsonString, std::stringstream& out);
    int addMonitor(const char* pUid, const char* jsonString, std::stringstream& out);
    int updateAuthority(const char* pUid, const char* jsonString, std::stringstream& out);
    int dealApply(const char* pUid, const char* jsonString, std::stringstream& out);
    int apply(const char* pUid, const char* jsonString, std::stringstream& out);
    int getBerthsOfPort(const char* pUid, const char* jsonString, std::stringstream& out);
    int getMonitorsOfRegion(const char* pUid, const char* jsonString, std::stringstream& out);
	int applyEntityMonitors(const char* pUid, const char* jsonString, std::stringstream& out);
	int updEntityMonitorsIP(const char* pUid, const char* jsonString, std::stringstream& out);
};

typedef std::map<std::string, int> vpMAP;

typedef struct _CameraInfo
{
    int id;
	int owntm;
	int x;
    int y;
	int ntp;
	int nconnect;
	int port;
	int bseipc;
    char name[64];
    int is_public;
    char owner[32];
    char addr[64];
	char path[256];
    vpMAP viewers;
    vpMAP applyers;
    vpMAP refusers;

	std::string toJsonViewers()
    {
        char buff[1024] = "";
        int len = sprintf (buff, "{na:\"%s\",tm:%d}", owner, owntm);
        for (vpMAP::iterator iTer = viewers.begin(); iTer != viewers.end(); iTer++)
        {
			if (!strcmp((*iTer).first.c_str(), owner))
				continue;
            len += sprintf (buff + len, ",{na:\"%s\",tm:%d}", (*iTer).first.c_str(), (*iTer).second);
        }
        return std::string(buff);
    }

    std::string toJsonApplyers()
    {
        char buff[1024] = "";
        int len = 0;
        for (vpMAP::iterator iTer = applyers.begin(); iTer != applyers.end(); iTer++)
        {
            if (iTer == applyers.begin())
                len += sprintf (buff + len, "{na:\"%s\",tm:%d}", (*iTer).first.c_str(), (*iTer).second);
            else
                len += sprintf (buff + len, ",{na:\"%s\",tm:%d}", (*iTer).first.c_str(), (*iTer).second);
        }
        return std::string(buff);
    }

    std::string toJsonRefusers()
    {
        char buff[1024] = "";
        int len = 0;
        for (vpMAP::iterator iTer = refusers.begin(); iTer != refusers.end(); iTer++)
        {
            if (iTer == refusers.begin())
                len += sprintf (buff + len, "{na:\"%s\",tm:%d}", (*iTer).first.c_str(), (*iTer).second);
            else
                len += sprintf (buff + len, ",{na:\"%s\",tm:%d}", (*iTer).first.c_str(), (*iTer).second);
        }
        return std::string(buff);
    }
} CameraInfo;

typedef std::map<int,CameraInfo> CameraInfoMap;

typedef struct _BerthOfPort
{
    char id[20];
    char name[100];
} BerthOfPort;

typedef std::vector<BerthOfPort> berthVec;

typedef struct _PositionOfCamera
{
    char id[64];
	char name[32];
    int type;
    double x;
    double y;
} PositionOfCamera;

typedef std::map<std::string,PositionOfCamera> posCMAP;


typedef ACE_Singleton<MonitorSvc, ACE_Null_Mutex> g_MonitorSvc;

#endif

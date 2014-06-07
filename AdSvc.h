#ifndef __AD_SVC_H__
#define __AD_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

class AdSvc : IBusinessService
{
public:
    AdSvc();
    ~AdSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(AdSvc)

private:
    int getZoneItems(const char* pUid, const char* jsonString, std::stringstream& out);
    int getAdByType(const char* pUid, const char* jsonString, std::stringstream& out);
	int getPhoneAD(const char* pUid, const char* jsonString, std::stringstream& out);

private:
    double calAdRate(const char* pUid, int nType);
};

typedef ACE_Singleton<AdSvc, ACE_Null_Mutex> g_AdSvc;

#endif

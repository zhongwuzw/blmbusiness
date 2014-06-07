#ifndef __MOBILE_SVC_H__
#define __MOBILE_SVC_H__

#include "IBusinessService.h"

class MobileSvc : IBusinessService
{
public:
     MobileSvc(void);
    ~MobileSvc(void);

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(MobileSvc)

private:
	int GetUserMobileBindInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetMsgUnit(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetExechange(const char* pUid, const char* jsonString, std::stringstream& out);
};
typedef ACE_Singleton<MobileSvc, ACE_Null_Mutex> g_MobileSvc;

#endif

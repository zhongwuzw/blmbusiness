#ifndef _MESSAGESERVICE_H
#define _MESSAGESERVICE_H

#include "define.h"
#include "BaseService.h"

class IBusinessService;

class CMessageService : public CBaseService
{
public:
    CMessageService();
    bool   RegisterCmd(uint16 u2Mid, IBusinessService* svc);
	virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);

protected:
    bool   DoMessage(void* pMessage);
    uint16 GetMessageCmd(void* pMessage);
    void   CheckMessagePool();

private:
    typedef std::map<uint16, IBusinessService*> MapBusinessSvc;
    typedef MapBusinessSvc::iterator IterMapBusinessSvc;
    MapBusinessSvc m_mapSvc;
};

typedef ACE_Singleton<CMessageService, ACE_Null_Mutex> g_MessageService;
#endif

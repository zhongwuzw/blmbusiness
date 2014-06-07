#ifndef __SMS_MAIL_SVC_H__
#define __SMS_MAIL_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

using namespace std;

class SmsMailSvc : IBusinessService, ACE_Event_Handler
{
public:
    SmsMailSvc();
    ~SmsMailSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(SmsMailSvc)
	static int CheckAndBilling(const string& userid, const string& mobile, int nCount=1);

public:
    int sendSms(const char* pUid, const char* jsonString, std::stringstream& out);
    int sendMail(const char* pUid, const char* jsonString, std::stringstream& out);
	int sendFreeSms(const char* pUid, const char* jsonString, std::stringstream& out);

private:
    int getSmsBal(const char* pUid, const char* jsonString, std::stringstream& out);
	int useSmsBal(const char* pUid, const char* jsonString, std::stringstream& out);
	int sendSmsChat(const char* pUid, const char* jsonString, std::stringstream& out);		
};

typedef ACE_Singleton<SmsMailSvc, ACE_Null_Mutex> g_SmsMailSvc;

#endif

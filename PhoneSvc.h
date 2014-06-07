#ifndef __PHONE_SVC_H__
#define __PHONE_SVC_H__

#include "IBusinessService.h"

class PhoneSvc : IBusinessService
{
public:
    PhoneSvc();
    ~PhoneSvc();
    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(PhoneSvc)

public:
	int GetSipNo(const char* pUid, const char* jsonString, std::stringstream& out);

private:
    int GetPhoneServer(const char* pUid, const char* jsonString, std::stringstream& out);    
    int GetAllContact(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetOneContactDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int UpdateContact(const char* pUid, const char* jsonString, std::stringstream& out);
    int DeleteContact(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetAllGroup(const char* pUid, const char* jsonString, std::stringstream& out);
    int UpdateGroup(const char* pUid, const char* jsonString, std::stringstream& out);
    int DeleteGroup(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetBalance(const char* pUid, const char* jsonString, std::stringstream& out);
    int UpdateContactTel(const char* pUid, const char* jsonString, std::stringstream& out);
    int IsBalEnough(const char* pUid, const char* jsonString, std::stringstream& out);
	int SetSipNo(const char* pUid, const char* jsonString, std::stringstream& out);
	int VerifySipNo(const char* pUid, const char* jsonString, std::stringstream& out);
	int blmPhoneRecharge(const char* pUid, const char* jsonString, std::stringstream& out);
	int updAppUpdInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAppUpdInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int iPhoneBuyService(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAppPrice(const char* pUid, const char* jsonString, std::stringstream& out);

    std::map<std::string,std::map<std::string,double> > m_mapRates;
    ACE_RW_Thread_Mutex g_cs;

};

typedef ACE_Singleton<PhoneSvc, ACE_Null_Mutex> g_PhoneSvc ;

#endif

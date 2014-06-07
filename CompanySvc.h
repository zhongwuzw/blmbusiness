#ifndef COMPANYSVC_H
#define COMPANYSVC_H

#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"
#include "CompanySearch.h"
using namespace std;

class CompanySvc : IBusinessService, ACE_Event_Handler
{
public:
    CompanySvc();
    virtual ~CompanySvc();
    virtual bool Start();
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(CompanySvc)

private:
    int OnCompanySearchMsg(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetCompanyInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetServerPort(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetContactStat(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetShipCategory(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetShipList(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetShipPosition(const char* pUid, const char* jsonString, std::stringstream& out);

private:
    CompanySearch   m_companydatas;
};

typedef ACE_Singleton<CompanySvc, ACE_Null_Mutex> g_CompanySvc;

#endif // COMPANYSVC_H

#ifndef AREASVC_H
#define AREASVC_H
#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"

class AreaSvc : IBusinessService
{
public:
    AreaSvc();
    virtual ~AreaSvc();
    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(AreaSvc)

private:
    int ModifyOneLocate(const char* pUid, const char* jsonString, std::stringstream& out);
    int DeleteOneLocate(const char* pUid, const char* jsonString, std::stringstream& out);
    int AddOneLocate(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetLocateList(const char* pUid, const char* jsonString, std::stringstream& out);
    int DeleteOneArea(const char* pUid, const char* jsonString, std::stringstream& out);
    int ModifyOneArea(const char* pUid, const char* jsonString, std::stringstream& out);
    int AddOneArea(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetAreaList(const char* pUid, const char* jsonString, std::stringstream& out);
};

typedef ACE_Singleton<AreaSvc, ACE_Null_Mutex> g_AreaSvc;

#endif // AREASVC_H

#ifndef __VOYAGE_SVC_H__
#define __VOYAGE_SVC_H__

#include "IBusinessService.h"

struct  REMARK_INFO
{
    int type;
    std::string object_id;
    int num;
};

class VoyageSvc : IBusinessService
{
public:
    VoyageSvc();
    ~VoyageSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(VoyageSvc)

private:
    int MoiPostBookEta(const char* pUid, const char* jsonString, std::stringstream& out);
    int AddAisModiTime(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetAisModiTime(const char* pUid, const char* jsonString, std::stringstream& out);
    int PortManage(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPortcall(const char* pUid, const char* jsonString, std::stringstream& out);
    int PositionbookManage(const char* pUid, const char* jsonString, std::stringstream& out);
    int AddPositionbook(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPositionbook(const char* pUid, const char* jsonString, std::stringstream& out);
    int SetConsumption(const char* pUid, const char* jsonString, std::stringstream& out);
    int VoyageinfoManage(const char* pUid, const char* jsonString, std::stringstream& out);
    int CategoryManage(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetVoyageInfo(const char* pUid, const char* jsonString, std::stringstream& out);
};

typedef ACE_Singleton<VoyageSvc, ACE_Null_Mutex> g_VoyageSvc;

#endif

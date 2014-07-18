#ifndef __VESSEL_SVC_H__
#define __VESSEL_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include "ShipAis.h"
#include "CompanySvc.h"

struct CommuInfo
{
    char number[32];
    char sysType[64];
    char useType[64];
    char sysTypecd[8];
};

struct CompanyInfo
{
    char cmpyid[18];
    char cmpyname[256];
	char cmpyaddr[256];
	char cmpycry[32];
};

struct CompanyHis
{
    char cmpyid[18];
    char cmpyname[256];
    char code[12];
    char year[8];
};

struct VslUserInfo
{
    char usrid[64];
    char usrname[256];
	int  type;
    char updt[32];
	char country[10];
	char mobile[32];
};
struct CmpyData
{
    string cmypid;
    string name;
    string tel;
    string fax;
    string email;
    string address;
    string country;
    string homepage;
};

class VesselSvc : IBusinessService, ACE_Event_Handler
{
public:
    VesselSvc();
    ~VesselSvc();

	CAisShip aisShip;

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
    virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(VesselSvc)

private:
    int OnUserEmailExists(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnRegisterUser(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnActivateUser(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnSendActivation(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnRegisterInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int OnMobileBound(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnUserEmailPwd(const char* pUid, const char* jsonString, std::stringstream& out);
	int OnUserResetPwd(const char* pUid, const char* jsonString, std::stringstream& out);

    int OnVesselDirMsg(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslBaseInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslHisDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslLogos(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslCommunication(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslCmpyContact(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslSpecDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslMachineryDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslCapacityDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslCertDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslCrewDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVslEventList(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqAttenVslUserCount(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqAttenVslUserList(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnVesselDirMsg2(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVesselContact(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnReqVesselIdByMMSI(const char* pUid, const char* jsonString, std::stringstream& out);
	int getCCompanyInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getSameCompShip(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBaseShipInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getMmsi(const char* pUid, const char* jsonString, std::stringstream& out);
	int OnVesselDirMsg3(const char* pUid, const char* jsonString, std::stringstream& out);
	int OnVesselDirMsg4(const char* pUid, const char* jsonString, std::stringstream& out);
	int judgeShipid(const char* pUid, const char* jsonString, std::stringstream& out);
	int imoGetMmsi(const char* pUid, const char* jsonString, std::stringstream& out);
	int getVesselVeryDetails(const char* pUid, const char* jsonString, std::stringstream& out);
	int getVesselPSC(const char* pUid, const char* jsonString, std::stringstream& out);
	
	CAisShip &GetAisShip(){return aisShip;}


private:
    int OnVesselDirMsg3_4(const char* pUid, const char* jsonString, std::stringstream& out,bool bonlycnss);

private:
	long m_lastTelCodeTime;


};

//条件参量

typedef ACE_Singleton<VesselSvc, ACE_Null_Mutex> g_VesselSvc;

#endif

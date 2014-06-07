#ifndef __AIS_SVC_H__
#define __AIS_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include <vector>

using namespace std;

struct _FamilyUserInfo
{
	char userid[64];
	Tokens telnos;
	char email[64];

	char shipid[64];
	int mmsi;
	int last_evt;
	int usertype;

	_FamilyUserInfo() {
		userid[0] = 0;
		email[0] = 0;
		shipid[0] = 0;
		mmsi = last_evt = 0;
	}
};
typedef std::multimap<int, _FamilyUserInfo> MapFamilyUser;
typedef MapFamilyUser::iterator MapFamilyUserIter;
typedef std::pair<MapFamilyUserIter, MapFamilyUserIter> MapFamilyUserRange;

typedef std::map<std::string, _FamilyUserInfo> MapUserFamily;
typedef MapUserFamily::iterator MapUserFamilyIter;

class AisSvc : IBusinessService, ACE_Event_Handler
{
public:
    AisSvc();
    ~AisSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

	void OnAisEvent(const char* eventString);

    DECLARE_SERVICE_MAP(AisSvc)
private:
	int checkExpired();
	bool checkConnect();
	bool checkConnect_udp();

private:
    int GetDetailOfMmsList(const char* pUid, const char* jsonString, std::stringstream& out);
	int updConShip(const char* pUid, const char* jsonString, std::stringstream& out);
	int updUserTelno(const char* pUid, const char* jsonString, std::stringstream& out);
	int getTelnoandBal(const char* pUid, const char* jsonString, std::stringstream& out);
	int getShipInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int modifyDest(const char* pUid, const char* jsonString, std::stringstream& out);

	int sendShipsMsg();
	int dieBoy(_FamilyUserInfo userInfo, MapUserFamily& noMoneyMap, int type=-1);
	int notifyNoMoney(MapUserFamily& noMoneyMap);

private:
	int m_LastUpdate;
	bool m_bTrackConnected;
	MapFamilyUser m_FamilyUsers;
	ACE_SOCK_Connector m_Connector;
	ACE_Thread_Mutex m_Lock;
	ACE_Thread_Mutex m_ConnectLock;
	ACE_SOCK_Stream	m_TrackPeer;

	bool loadFamilyUser();
	bool loadFamilyUser(MySql* psql, const char* sql, MapFamilyUser& familyUserMap);
};

typedef ACE_Singleton<AisSvc, ACE_Null_Mutex> g_AisSvc;

#endif

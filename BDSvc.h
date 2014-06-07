#ifndef __BD_SVC_H__
#define __BD_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

class BDSvc : IBusinessService, ACE_Event_Handler
{
public:
    BDSvc();
    ~BDSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(BDSvc)

private:
    int zhjGroup(const char* pUid, const char* jsonString, std::stringstream& out);
	int cardDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int allYhj(const char* pUid, const char* jsonString, std::stringstream& out);
	int allPos(const char* pUid, const char* jsonString, std::stringstream& out);
	int position(const char* pUid, const char* jsonString, std::stringstream& out);
	int track(const char* pUid, const char* jsonString, std::stringstream& out);
	int tx(const char* pUid, const char* jsonString, std::stringstream& out);
	int txhz(const char* pUid, const char* jsonString, std::stringstream& out);
	int search(const char* pUid, const char* jsonString, std::stringstream& out);
	int remark(const char* pUid, const char* jsonString, std::stringstream& out);
	int p2p(const char* pUid, const char* jsonString, std::stringstream& out);
	int broadcast(const char* pUid, const char* jsonString, std::stringstream& out);
	int allYjbj(const char* pUid, const char* jsonString, std::stringstream& out);
	int recvSms(const char* pUid, const char* jsonString, std::stringstream& out);

private:
	bool checkConnect(ACE_SOCK_Connector& m_Connector, ACE_SOCK_Stream& m_Peer);
};

typedef ACE_Singleton<BDSvc, ACE_Null_Mutex> g_BDSvc;

#endif

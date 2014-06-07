#ifndef __MESSENGER_SVC_H__
#define __MESSENGER_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

class MessageSvc : IBusinessService
{
public:
    MessageSvc(void);
    ~MessageSvc(void);
    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(MessageSvc)

private:
    //“µŒÒ«Î«Û
    int GetUserGroupList(const char* pUid, const char* jsonString, std::stringstream& out);
    int ContactGroupManager(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetContactInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetBlmServiceUserList(const char* pUid, const char* jsonString, std::stringstream& out);
    int GeUsersNickList(const char* pUid, const char* jsonString, std::stringstream& out);
    int ReqMucRoomList(const char* pUid, const char* jsonString, std::stringstream& out);
    int ReqMucRoomMemberList(const char* pUid, const char* jsonString, std::stringstream& out);
    int QueryMucRoomCount(const char* pUid, const char* jsonString, std::stringstream& out);
    int QueryMucRoomList(const char* pUid, const char* jsonString, std::stringstream& out);
    int ReqMucRoomDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int ReqUserAuth(const char* pUid, const char* jsonString, std::stringstream& out);
    int ReqUserTelsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int ReqSmsCount(const char* pUid, const char* jsonString, std::stringstream& out);
    int ReqUserTelEmailInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int ReqChangjiangUserLevel(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnUserRegistCmpy(const char* pUid, const char* jsonString, std::stringstream& out);
    int OnUserUnRegistCmpy(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetFamilyDaShiList(const char* pUid, const char* jsonString, std::stringstream& out);

    bool _AddContactGroup(const char *usrid,const char *grpId,const char *grpname,MySql *psql);
    bool _DelContactGroup(const char *usrid,const char *grpId,MySql *psql);
    bool _ModifyContactGroup(const char *usrid,const char *grpId,const char *grpname,MySql *psql);


};

typedef ACE_Singleton<MessageSvc, ACE_Null_Mutex> g_MessageSvc;

#endif

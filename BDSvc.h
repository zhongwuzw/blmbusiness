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

//请求组列表
	int getGroupListInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//创建修改组
	int AddOrEditGroupPeople(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除组
	int DeleteGroupPeople(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//返回组下的用户
	int getGroupPeople(const char* pUid, const char* jsonString, std::stringstream& out);

	//编辑用户
	int EditPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//添加或修改联系组
	int AddOrEditGroupContacts(const char* pUid, const char* jsonString, std::stringstream& out);

	//获取联系人组
	int getContactsPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//添加或修改联系人
	int AddOrEditContactsInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除联系人组
	int DeleteGroupContacts(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除联系人
	int deleteContactInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//修改用户密码
	int editUserPassword(const char* pUid, const char* jsonString, std::stringstream& out);

	//获取管理卡列表
	int getManageCardList(const char* pUid, const char* jsonString, std::stringstream& out);


	//分配卡
	int allocCards(const char* pUid, const char* jsonString, std::stringstream& out);

	//添加用户
	int AddPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取卡详细信息
	int getCardInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//回收卡
	int recycleCards(const char* pUid, const char* jsonString, std::stringstream& out);

	//解绑卡
	int releaseCardBind(const char* pUid, const char* jsonString, std::stringstream& out);

	//获得联系人组信息
	int getContactsGroupListInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	//获取绑定用户的卡列表
	int getBindCardList(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取某绑定卡的用户
	int getBindCardUsers(const char* pUid, const char* jsonString, std::stringstream& out);

	//修改卡信息
	int EditCardInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//获取用户信息
	int getPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	//检测用户名是否存在
	int checkExistUser(const char* pUid, const char* jsonString, std::stringstream& out);

	// 删除白名单
	int deleteWhiteUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	// 同意或拒绝白名单的申请
	int agreeOrDisagreeWhiteUser(const char* pUid, const char* jsonString, std::stringstream& out);


	// 添加计费信息
	int addChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	// 修改计费信息
	int editChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	// 删除计费信息
	int deleteChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	// 绑定北斗卡号
	int bindBeiDouCard(const char* pUid, const char* jsonString, std::stringstream& out);


	// 发送信息
	int sendMsgInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	// 获取计费信息
	int getChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	// 获取申请的白名单
	int getApplyWhiteUser(const char* pUid, const char* jsonString, std::stringstream& out);

	// 获取管理的卡列表
	int getManageCardList1(const char* pUid, const char* jsonString, std::stringstream& out);

	// 获取电话前缀
	int getPhonePrefix(const char* pUid, const char* jsonString, std::stringstream& out);


	// 编辑白名单
	int editWhiteUser(const char* pUid, const char* jsonString, std::stringstream& out);

	// 短信白名单管理
	int allWhitelistApply(const char* pUid, const char* jsonString, std::stringstream& out);
	int dealWhitelistApply(const char* pUid, const char* jsonString, std::stringstream& out);

	int getWhiteUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	int addWhiteUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);

private:
	bool checkConnect(ACE_SOCK_Connector& m_Connector, ACE_SOCK_Stream& m_Peer);

	// 短信白名单
	bool checkExistsCard(int addr);
	int  checkWhiteList(std::string& mobile, int addr);
	bool dealWhitelistApply(std::string& mobile, int addr, int status, bool isApply=false);
};

typedef ACE_Singleton<BDSvc, ACE_Null_Mutex> g_BDSvc;

#endif

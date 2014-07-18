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

//�������б�
	int getGroupListInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//�����޸���
	int AddOrEditGroupPeople(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ����
	int DeleteGroupPeople(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//�������µ��û�
	int getGroupPeople(const char* pUid, const char* jsonString, std::stringstream& out);

	//�༭�û�
	int EditPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ӻ��޸���ϵ��
	int AddOrEditGroupContacts(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ȡ��ϵ����
	int getContactsPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ӻ��޸���ϵ��
	int AddOrEditContactsInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ����ϵ����
	int DeleteGroupContacts(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ����ϵ��
	int deleteContactInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//�޸��û�����
	int editUserPassword(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ȡ�����б�
	int getManageCardList(const char* pUid, const char* jsonString, std::stringstream& out);


	//���俨
	int allocCards(const char* pUid, const char* jsonString, std::stringstream& out);

	//����û�
	int AddPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ����ϸ��Ϣ
	int getCardInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//���տ�
	int recycleCards(const char* pUid, const char* jsonString, std::stringstream& out);

	//���
	int releaseCardBind(const char* pUid, const char* jsonString, std::stringstream& out);

	//�����ϵ������Ϣ
	int getContactsGroupListInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	//��ȡ���û��Ŀ��б�
	int getBindCardList(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡĳ�󶨿����û�
	int getBindCardUsers(const char* pUid, const char* jsonString, std::stringstream& out);

	//�޸Ŀ���Ϣ
	int EditCardInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ȡ�û���Ϣ
	int getPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	//����û����Ƿ����
	int checkExistUser(const char* pUid, const char* jsonString, std::stringstream& out);

	// ɾ��������
	int deleteWhiteUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	// ͬ���ܾ�������������
	int agreeOrDisagreeWhiteUser(const char* pUid, const char* jsonString, std::stringstream& out);


	// ��ӼƷ���Ϣ
	int addChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	// �޸ļƷ���Ϣ
	int editChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	// ɾ���Ʒ���Ϣ
	int deleteChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	// �󶨱�������
	int bindBeiDouCard(const char* pUid, const char* jsonString, std::stringstream& out);


	// ������Ϣ
	int sendMsgInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	// ��ȡ�Ʒ���Ϣ
	int getChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	// ��ȡ����İ�����
	int getApplyWhiteUser(const char* pUid, const char* jsonString, std::stringstream& out);

	// ��ȡ����Ŀ��б�
	int getManageCardList1(const char* pUid, const char* jsonString, std::stringstream& out);

	// ��ȡ�绰ǰ׺
	int getPhonePrefix(const char* pUid, const char* jsonString, std::stringstream& out);


	// �༭������
	int editWhiteUser(const char* pUid, const char* jsonString, std::stringstream& out);

	// ���Ű���������
	int allWhitelistApply(const char* pUid, const char* jsonString, std::stringstream& out);
	int dealWhitelistApply(const char* pUid, const char* jsonString, std::stringstream& out);

	int getWhiteUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	int addWhiteUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);

private:
	bool checkConnect(ACE_SOCK_Connector& m_Connector, ACE_SOCK_Stream& m_Peer);

	// ���Ű�����
	bool checkExistsCard(int addr);
	int  checkWhiteList(std::string& mobile, int addr);
	bool dealWhitelistApply(std::string& mobile, int addr, int status, bool isApply=false);
};

typedef ACE_Singleton<BDSvc, ACE_Null_Mutex> g_BDSvc;

#endif

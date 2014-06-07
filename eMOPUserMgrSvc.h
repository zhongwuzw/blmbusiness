#ifndef __EMOP_USERMGR_SVC_H__
#define __EMOP_USERMGR_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

using namespace std;

//������Ϣ

struct Depart
{
	string departId;
	string departName;
	int departType;
	list<Depart> childDepart;
	Depart():departId(""),departName(""),departType(0){}
};


//������Ϣ
struct ShipDepart
{
	string shipId;
	string shipName; 
	list<Depart> departList;
	ShipDepart():shipId(""),shipName(""){}
};
 
 
//�û���Ϣ
struct User 
{
	string uid;
	string name;
	string tel;
	string fax;
	string mobile;
	string email;
	int stime;
	int etime;
	string job;  
	int state; 
	int cstype; // 0:���ˡ�1:����

	Depart depart;//���� 


	User():uid(""),name(""),tel(""),fax(""),mobile(""),email(""),stime(0),etime(0),job(""),state(0),cstype(0){}
};



typedef struct KeyValue
{
	string m_strKey;
	string m_strValue;
}_KEY_VALUE;
//T41_EMOP_SHIP_SAVE_BOAT
typedef struct SHIP_SAVEBOAT
{
	char left_boat[64];//	��ͧ ����ͧ
	char lb_style[64];//	��ͧ��ʽ ����ͧ
	char right_boat[64];//	��ͧ ����ͧ
	char rb_style[64];//	��ͧ��ʽ ����ͧ
	char left_raft[64];//	���� ������
	char lr_style[64];//	������ʽ ������
	char right_raft[64];//	���� ������
	char rr_style[64];//	������ʽ ������
}ST_SAVEBOAT;
class eMOPUserMgrSvc : IBusinessService, ACE_Event_Handler
{
public:
    eMOPUserMgrSvc();
    ~eMOPUserMgrSvc();

    virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(eMOPUserMgrSvc)
// data
private:
// method
private:
	//��ȡ��˾�û������Ҫ��Ϣ
    int GetAllUserGeneralInfo_C(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ɫ����
	int GetGetAllRoles(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ɫ��Ӧ��Ȩ��
	int GetRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out);
	// �޸Ľ�ɫ����
	int SetUserRoleName(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ӽ�ɫ
	int NewUserRole(const char* pUid, const char* jsonString, std::stringstream& out);
	// ɾ����ɫ
	int DelUserRole(const char* pUid, const char* jsonString, std::stringstream& out);
	// �޸Ľ�ɫȨ��
	int ModifyRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��Ӳ���
	int NewDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	// �༭����
	int UpdateDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	//��Ӳ��Ŵ��Ӵ���
	int NewDptShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ�����Ŵ��Ӵ���
	int DelDptShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ������
	int DelDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	// ����û�
	int NewUser(const char* pUid, const char* jsonString, std::stringstream& out);
	// ɾ���û�
	int DelUser(const char* pUid, const char* jsonString, std::stringstream& out);
	//�����û�����
	int ModifyUserDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	//�鿴�û�����
	int GetUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//�û���Ӵ���
	int AddUserShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//�û�ɾ������
	int DelUserShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//�û�ӵ�еĴ���
	int GetAllUserShips(const char* pUid, const char* jsonString, std::stringstream& out);
	//�����û�
	int SearchUser(const char* pUid, const char* jsonString, std::stringstream& out);
	//���ݴ���ID���Ҵ�����Ӧ�Ĳ���
	int SearchShipDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��������
	int ResetPwd(const char* pUid, const char* jsonString, std::stringstream& out);
	//����/�ָ��û�
	int EnableUser(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ�����û������Ҫ��Ϣ
	int GetAllUserGeneralInfo_S(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��ȡ�û���ɫ
	int GetUserRoles(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��ȡ�û����ڲ���
	int GetUserDpt(const char* pUid, const char* jsonString, std::stringstream& out);
    // ��ȡ�û����ڹ�˾����˾���ţ�������Ϣ
	int GetCompanyDpt(const char* pUid, const char* jsonString, std::stringstream& out);
    // ��ȡ��˾ĳ�����������û�
	int GetDptUsers(const char* pUid, const char* jsonString, std::stringstream& out); 
	// ��ѯĳ��ɫ�µ������û�
	int GetRoleUsers(const char* pUid, const char* jsonString, std::stringstream& out); 
	// �޸��û�״̬
	int UpdateUserState(const char* pUid, const char* jsonString, std::stringstream& out); 
	// �����н�ɫ��Ӹ��û�����ɾ���û��Ѿ��еĵĽ�ɫ
	int AddOrDelUserRole(const char* pUid, const char* jsonString, std::stringstream& out); 
	// �����д�����Ӹ��û�����ɾ���û��Ѿ��еĴ���
	int AddOrDelUserShip(const char* pUid, const char* jsonString, std::stringstream& out); 

	//
	int GetSubData(std::stringstream& out,short &o_nSt,string &i_strKey,string &i_strRoleid ,MySql* psql);
	int GetFuncData(std::stringstream& out,short &o_nSt,string &i_strKey,string &i_strRoleid,MySql* psql);
	//
	int GetCorpInfo(string &o_strCorpid,string &o_strCorpName,string &i_strUid);
	int GetCorpDpt(std::stringstream& out,string &i_strCorpId,bool i_bShip=false); 
	int GeDptUser(std::stringstream& out,string &i_strDptId,bool i_bShip=false);
	int GetUserShips(std::stringstream& out,string &i_strUId);
	int GetDptShips(std::stringstream& out,string &i_strDptId);
	int GetCorpShips(std::stringstream& out,string &i_strCorpId);



	//��ȡ����������ϸ��Ϣ0xf1

	int GetShipDetailInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ȡ������ʷ0xf2
    int GetShipHistoryList(const char* pUid, const char* jsonString, std::stringstream& out);
 
	//��ȡ����ͨѶ0xf3
    int GetShipCommunicationList(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ȡ�����豸0xf4
	int GetShipEquipmentList(const char* pUid, const char* jsonString, std::stringstream& out);
	///
	int GetShipBaseData(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetShipDetail1(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetShipDetail2(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetShipDetail3(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetShipEquipments(const char* pUid, const char* jsonString, std::stringstream& out);
// other method
	int GetShipComminfo(string &o_strC,string &o_strABM,string &i_strShipid);
	int GetSaveboatInfo(ST_SAVEBOAT &o_stSaveboat,string &i_strShipid);
};

typedef ACE_Singleton<eMOPUserMgrSvc, ACE_Null_Mutex> g_eMOPUserMgrSvc;
 


#endif

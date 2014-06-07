#ifndef __BERTH_SVC_H__
#define __BERTH_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

using namespace std;

class BerthSvc : IBusinessService, ACE_Event_Handler
{
public:
    BerthSvc();
    ~BerthSvc();

    virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(BerthSvc)

private:
    int getPortBerthList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthCompInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthShipInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthMYWInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getEquipmentList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getStorageList(const char* pUid, const char* jsonString, std::stringstream& out);
    
	//int getBerthCameraInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthCompManagerInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getTerminalInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getTerminalList(const char* pUid, const char* jsonString, std::stringstream& out);
	int newTerminalItem(const char* pUid, const char* jsonString, std::stringstream& out);
    int delTerminalItem(const char* pUid, const char* jsonString, std::stringstream& out);
	int udpOperatorsDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int newBerthItem(const char* pUid, const char* jsonString, std::stringstream& out);
	int delBerthItem(const char* pUid, const char* jsonString, std::stringstream& out);
	int newBerthEquipment(const char* pUid, const char* jsonString, std::stringstream& out);
	int delBerthEquipment(const char* pUid, const char* jsonString, std::stringstream& out);
	int newTerminalStorage(const char* pUid, const char* jsonString, std::stringstream& out);
	int delTerminalStorage(const char* pUid, const char* jsonString, std::stringstream& out);
	int getStatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getMoorShipInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getCargoInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updShipBaseInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getInOutList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getUndealWarn(const char* pUid, const char* jsonString, std::stringstream& out);
	int exclWarning(const char* pUid, const char* jsonString, std::stringstream& out);
	int getStatIndex(const char* pUid, const char* jsonString, std::stringstream& out);
	int getInOutDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int getContactInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int addShipContactPerson(const char* pUid, const char* jsonString, std::stringstream& out);
	int delShipContactPerson(const char* pUid, const char* jsonString, std::stringstream& out);
	int getUserAuth(const char* pUid, const char* jsonString, std::stringstream& out);

	int getShipDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int udpShipDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int getSystemUser(const char* pUid, const char* jsonString, std::stringstream& out);
	int updUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int delUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updUserPassword(const char* pUid, const char* jsonString, std::stringstream& out);
	int checkUser(const char* pUid, const char* jsonString, std::stringstream& out);
	int getSystemAuthrity(const char* pUid, const char* jsonString, std::stringstream& out);
	int setUserAuth(const char* pUid, const char* jsonString, std::stringstream& out);
	int getPortOrgan(const char* pUid, const char* jsonString, std::stringstream& out);

	int getConcernShips(const char* pUid, const char* jsonString, std::stringstream& out);
	int addConcernShips(const char* pUid, const char* jsonString, std::stringstream& out);
	int delConcernShips(const char* pUid, const char* jsonString, std::stringstream& out);

	int getCompanyList(const char* pUid, const char* jsonString, std::stringstream& out);
	int delCompanyItem(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAlertList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAlertUsers(const char* pUid, const char* jsonString, std::stringstream& out);
	int setAlertUsers(const char* pUid, const char* jsonString, std::stringstream& out);

	int getRemoveShip(const char* pUid, const char* jsonString, std::stringstream& out);
	int delRemoveShip(const char* pUid, const char* jsonString, std::stringstream& out);
	int getLocalShip(const char* pUid, const char* jsonString, std::stringstream& out);
	int searchShipByName(const char* pUid, const char* jsonString, std::stringstream& out);
	int setAlertLevel(const char* pUid, const char* jsonString, std::stringstream& out);

	int delThruptInfo(const char* pUid, const char* jsonString, std::stringstream& out);
};

typedef ACE_Singleton<BerthSvc, ACE_Null_Mutex> g_BerthSvc;

typedef struct _EquipmentInfo_B
{
	char id[64];     
	char seq[64];			//���
    char name[32];			//����
    char type[32];			//��е����
    char spec[32];			//���
	char capacity[32];		//
    char validdt[32];		//��Ч��
    char producer[64];		//������
	char berthid[32];
	char berthname[128];
}EquipmentInfo_B;

typedef std::map<std::string, EquipmentInfo_B> mEquipmentInfo_B;

typedef struct _WarehouseInfo_B
{
	char id[32];
	char type[32];			//����
 	char area[32];			//���
	char capacity[32];		//����
 	char businesscard[64];	//��Ӫ֤��
}WarehouseInfo_B;

typedef std::map<std::string, WarehouseInfo_B> mWarehouseInfo_B;

typedef struct _StorageInfo_B
{
	char id[32];
	char type[64];			//����
 	char area[32];			//���
	char capacity[32];		//����
 	char businesscard[64];	//��Ӫ֤��
	char terminalid[32];
	char terminalname[64];
}StorageInfo_B;

typedef std::map<std::string, StorageInfo_B> mStorageInfo_B;


struct _CargoInfo
{
	double tw;		//������totalweight
	double cw;		//��ת�� ������*����
	char cid[32];			//����ID
	char billway[64];			//�˵���
	int tp1;				//��������1      0��ͨ����1 ��װ��2 ��(��)��
	char tp2[32];			//��������2
	int load;				//װж���� 0װ�� 1ж��
	char own[128];			//����
	double fr;				//�˷�
	int amount;             //����
	_CargoInfo():tw(0.0),cw(0.0),tp1(0),fr(0.0),load(0)
	{

	}
};

struct _UsrTelInfo
{
      char mobile[32];
	  char usrname[64];
};
struct _AlertUserInfo
{
	char objid[32];
	char objname[64];
	int objtype;
	vector<_UsrTelInfo> vecTelUsrs;
};


struct _ThruptFlowInfo
{
	char flowid[32];		//������ˮID
	char shipid[32];		//����id
	char shipnm[64];		//��������,
	char shipchnm[64];		//��������
	int mmsi;
	char regport[64];		//������
	double dwt;				//���ض�λ
	char op[32];			//������Ӫ��
	char opcd[64];			//��Ӫ֤��
	char bid[20];			//ͣ����λID
	char bnm[100];			//��λ����
	char hid[20];           //����ID           
	char tid[20];           //��ͷID
	int arrivetm;			//����ʱ��
	int leavetm;			//���ʱ��
	float pspan;				//�ڸ�ʱ��
	double lpan;			//װжʱ��
	int trade;				//����ó 0��ó 1��ó
	int loadflag;           //װж�� 0װ�� 1ж��
	char sport[64];			//ʼ����
	char dport[64];			//Ŀ�ĸ�
	char opid[64];			//������ID
	char opnm[128];			//������
	vector<_CargoInfo> cargoList; //�����б�

	_ThruptFlowInfo():mmsi(0),dwt(0),arrivetm(0),pspan(0),lpan(0.0),trade(0)
	{
	}
};


#endif

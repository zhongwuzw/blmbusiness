#ifndef __EMOPREPAIR_SVC_H__
#define __EMOPREPAIR_SVC_H__
#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"
#include <string>

using namespace std;

struct RepairCategory
{ 
	std::string m_code;
	std::string m_name; 
};

 
struct RepairShip
{ 
	std::string m_id;
	std::string m_name_cn;  
};



class eMOPRepairSvc : IBusinessService
{
public:
    eMOPRepairSvc();
    virtual ~eMOPRepairSvc();
    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(eMOPRepairSvc)

private: 
	//���������***********************************************
	//����ȫ���������
	int GetAllRepairCat(const char* pUid, const char* jsonString, std::stringstream& out);
	//��������������������������
	int GetCatItems(const char* pUid, const char* jsonString, std::stringstream& out);
	//�����������µĴ���,�豸����������
	int GetItemDetail(const char* pUid, const char* jsonString, std::stringstream& out); 
	//�������޸�������
	int NewOrUpdateRepairItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ��������
	int DelRepairItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//���󴬲��豸�����й�����
	int GetAllShipWorkCard(const char* pUid, const char* jsonString, std::stringstream& out); 
	//���ӻ���ɾ������
	int NewOrDelItemShips(const char* pUid, const char* jsonString, std::stringstream& out);
	//���ӻ���ɾ���豸
	int NewOrDelItemEquips(const char* pUid, const char* jsonString, std::stringstream& out);
	//���ӻ���ɾ��������
	int NewOrDelWorkCard(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ������ѡ��
	int DeleteRepairItem(const char* pUid, const char* jsonString, std::stringstream& out);

	//��������***********************************************
	//���������б�
	int GetRepairApplyList(const char* pUid, const char* jsonString, std::stringstream& out);
    //����������ϸ����
	int GetRepairApplyDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//���ӻ����޸�����
	int AddOrUpdateRepairApply(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ�����������б�
	int GetShipRepairItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ������
	int DeleteRepairApply(const char* pUid, const char* jsonString, std::stringstream& out);
	//��������״̬
	int UpdateRepairApplyStatus(const char* pUid, const char* jsonString, std::stringstream& out);
    
	//���޹���***********************************************
    //�����޹���
	int GetFactoryRepair(const char* pUid, const char* jsonString, std::stringstream& out);
	//����ĳ����������δ��ʼ��δ������ĵĹ��̵�
	int GetShipEngineeringNotBegin(const char* pUid, const char* jsonString, std::stringstream& out);
	//����ĳ�����¼����б�
	int GetShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//�޸ĳ���
	int AddOrUpdateFactoryRepair(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ������
	int DeleteFactoryRepair(const char* pUid, const char* jsonString, std::stringstream& out);
    //������ӹ��̵�
	int AddOrDelEnginerringToFactory(const char* pUid, const char* jsonString, std::stringstream& out);

	//���̵�����***********************************************
	//���󹤳̵�
	int GetEngineering(const char* pUid, const char* jsonString, std::stringstream& out);
	//���̵���ϸ����
	int GetEngineeringDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//���ӻ����޸Ĺ��̵�
	int NewOrUpdateEngineering(const char* pUid, const char* jsonString, std::stringstream& out);
	//����ĳ������δ�ɹ��̵�����״̬Ϊδ��ʼ�������б�
	int GetNotStartEngineering(const char* pUid, const char* jsonString, std::stringstream& out);
	//�������
	int FinishRepairApply(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ�����̵�
	int DeleteEngineering(const char* pUid, const char* jsonString, std::stringstream& out);


	//�豸���µĹ����������У�
	int getEquipWorkCard(std::string &equipCardId,std::string &cardArrStr);
	//�����豸���µĹ�������������
	int getRepairEquipWorkCard(std::string &shipId,std::string &itemCode,std::string &cwbt,std::string &cardArrStr);
	//�������� Name
	int getFactoryRepairContent(std::string &id,std::string &outStr);
	//�������� id Name
	int getFactoryRepairContent2(std::string &id,std::string &outStr);
	//���̵�����˵����
	int getOtherDrections(std::string &proid,std::string &outStr);
	//���̵�����������
	int getRepairApplies(std::string &proid,std::string &outStr); 
	//���޹���Ĺ��̵�����
	int getFactoryEngineer(std::string &id,std::string &outStr); 
	//���鹫˾�Ƿ���ڣ���������Ҫ�����µĹ�˾
	bool checkNewCompany(std::string &cid,std::string &cname);


	
};

typedef ACE_Singleton<eMOPRepairSvc, ACE_Null_Mutex> g_eMOPRepairSvc;

#endif // AREASVC_H

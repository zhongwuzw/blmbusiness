#ifndef __EMOPEQUIP_SVC_H__
#define __EMOPEQUIP_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include "eMOPUserMgrSvc.h"

using namespace std;

class eMOPEquipSvc : IBusinessService, ACE_Event_Handler
{
public:
    eMOPEquipSvc();
    ~eMOPEquipSvc();

    virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(eMOPEquipSvc)
// data
//private:
//	int m_iFNumApply;// ���뵥��ˮ��
private:
	// ---------------------------------------------------------------------------------------
	// �����ֲ�
	// ---------------------------------------------------------------------------------------
	//��ȡĳһ���豸�ӿ�
    int getAllEquipment(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ�豸���Խӿ�
	int GetEquipPara(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ӻ��߱༭�豸�ӿ�
	int NewEquipment(const char* pUid, const char* jsonString, std::stringstream& out);
	// ɾ���豸�ӿ�
	int DelEquip(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ�豸�����б�
    int GetEquipParts(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ�������Խӿ�
	int GetPartsPara(const char* pUid, const char* jsonString, std::stringstream& out);
	//����±����ͱ༭�����ӿ�
	int NewParts(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetCheckInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// ɾ�������ӿ�
	int DelParts(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��ȡ����·���ӿ�
	int GetPaths(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��ȡ�����ɹ���ʷ����ӿ�
	int GetPartsOrderHis(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��ȡ�������������ʷ
	int GetPartsStoreHis(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��ȡ���������ϸ
	int GetPartsInstock(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��λ���ýӿ�
	int SetStorePos(const char* pUid, const char* jsonString, std::stringstream& out);
	//���������ӿ�
	int SearchPartsOnShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//�ߴ�Ԥ���ʹ�Ԥ��
	int GetWarndata(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ����������
	// ---------------------------------------------------------------------------------------
	// ���뵥
	// ---------------------------------------------------------------------------------------
	//���뵥����
	int ApplySearch(const char* pUid, const char* jsonString, std::stringstream& out);
	// �������뵥
	int NewApplyOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��������
	int SearchAppOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	//������������
	int GetAppOrderDetails(const char* pUid, const char* jsonString, std::stringstream& out);
	//�����޸Ľӿ�
	int ModifyAppOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	//����������
	int AppOrderOpt(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ȡ���յ��б�
	int GetCheckList(const char* pUid, const char* jsonString, std::stringstream& out);
	int CreateCheckList(const char* pUid, const char* jsonString, std::stringstream& out);
	//����״̬����
	int GetApporderNum(const char* pUid, const char* jsonString, std::stringstream& out);
	// ��ȡ������Ϣ
	int GetWorkfllow(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ��Ӧ���б�
	int GetAllPartsCorps(const char* pUid, const char* jsonString, std::stringstream& out);
	//����ѯ�۵�
	int MkInquirySheete(const char* pUid, const char* jsonString, std::stringstream& out);
	// ---------------------------------------------------------------------------------------
	// ѯ�۵�
	// ---------------------------------------------------------------------------------------
	//��ȡѯ�۵��б�
	int GetInquirySheeteList(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ѯѯ�۵���ϸ��Ϣ
	int GetInquirySheetDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//����ѯ�۵���ѯ��ѯ�۵��Ĳɹ���
	int GetAppOrderDetail_Iqsid(const char* pUid, const char* jsonString, std::stringstream& out);
	//����������ʷ�б�
	int GetIqsHis_Partsid(const char* pUid, const char* jsonString, std::stringstream& out);
	//�ɹ�����Ӧ��ѯ�۵��б�
	int GetIqsList_Appo(const char* pUid, const char* jsonString, std::stringstream& out);
	//ѯ�۵���Ӧ�ı����б�
	int GetPartsList_Iqs(const char* pUid, const char* jsonString, std::stringstream& out);
	//�ɹ�����Ӧ�ı����б�
	int GetPartsList_Appo(const char* pUid, const char* jsonString, std::stringstream& out);
	//������Ӧ��ѯ�۵��б�
	int GetIqsList_Partsid(const char* pUid, const char* jsonString, std::stringstream& out);
	//������Ӧ�Ķ�����
	int GetAppoList_Partsid(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ��ѯ�۵�
	int DeleteIqs(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ȡ����������Ϣ
	int GetWorkFlowInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	
	int EquipPurchaseCheck(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ������
	int DelEquipFile(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ�����뵥
	int DelApplyInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ɱ���
	int CompleteIqs(const char* pUid, const char* jsonString, std::stringstream& out);
	//���ɶ�����---ֱ�Ӷ���/�ȼ۶���
	int MkPurchaseSheete(const char* pUid, const char* jsonString, std::stringstream& out);
	// ---------------------------------------------------------------------------------------
	// ������
	// ---------------------------------------------------------------------------------------
	//����������
	int SearchPurchase(const char* pUid, const char* jsonString, std::stringstream& out);
	//����������
	int GetPurchaseDetail(const char* pUid, const char* jsonString, std::stringstream& out);	
	//�������޸�
	int ModifyPurchase(const char* pUid, const char* jsonString, std::stringstream& out);
	//������״̬����
	int GetPchStatusCount(const char* pUid, const char* jsonString, std::stringstream& out);
	//������������
	int PurchaseOpt(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ����������ʷ
	int GetPartsHis(const char* pUid, const char* jsonString, std::stringstream& out);
	// other func
	int newAppOrderNum(string &o_strAppNum,string &i_strShipId);
	int GetDptFromUserid(string &o_strDptid,const string &i_strUid);
	int GetSubData(std::stringstream& out,string &i_strKey,MySql* psql );	
	int GetAllSteps(vector <string>&o_vecSteps,MySql* psql,string &i_strTaskid);


	// ---------------------------------------------------------------------------------------
	// �̵㡢�����¶�����ͳ�� add by jt 
	// ---------------------------------------------------------------------------------------

    //��ȡ������ⵥ�б� 0x50
    int GetInstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡĳ��ⵥ�����еı����б� 0x51
	int GetInstockOrderPartList(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ��������ⵥ��ĳ���� 0x52
	int DeleteInstockOrderPartItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//������ⵥ���ų�ȷ�� 0x53
	/*int ConformPartInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);*/
	//��ȡ�����������б���������⣩ 0x54
	int GetPurchaseOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//�������뵥��ȡѯ�۵� 0x55
	int GetApplyInqueryInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    //�������뵥��ȡ������0x56
    int GetApplyOrdersInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//�޸ı�������
	int EditCheckNum(const char* pUid, const char* jsonString, std::stringstream& out);


	//��ȡ���뵥����0x57
	 int GetDetailApplyInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	 //�������뵥id��ȡ�����б���Ϣ
	 int GetEquipListInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ȡѯ�۵����� 0x58
     int GetIqsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	 //�ύ�������ⵥ 0x59
	 /*int SetPartOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);*/


	 int GetIqsEquipList(const char* pUid, const char* jsonString, std::stringstream& out);

	 //ɾ���������ⵥ��ĳ����0x5a
	 //int DeleteOutstockOrderPartItem(const char* pUid, const char* jsonString, std::stringstream& out);

	int GetPurchaseEquipList(const char* pUid, const char* jsonString, std::stringstream& out);


	 //�������ⵥ���ų�ȷ��0x5b
	 int ConformPartOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//�������뵥�޸�
	int EditApllyInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	//���󱸼��̵���Ϣ 0x60
	int GetPartsInventoryCounting(const char* pUid, const char* jsonString, std::stringstream& out);
    //���ñ����̵� 0x61
	int SetPartsInventoryCount(const char* pUid, const char* jsonString, std::stringstream& out);
	//�����¶�����ͳ�� 0x62
    int GetPartsMonthInOutStatistic(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetPics(string &o_strPics,string &i_strShipid,string &i_strCatid);

public:
	int GetEquipModelIdByCode(string &name,string &outModelId);
	int AddEquipModel(string &name,string &newId);

};

typedef ACE_Singleton<eMOPEquipSvc, ACE_Null_Mutex> g_eMOPEquipSvc;
 

struct PartMonthStatistic 
{
	string part_id;
	string part_name;
	string equip_id;
	string equip_model;//�豸�ͺ�
	string equip_name;
	int stocknum;   //��ǰ���
	string locateid;
	int appnum;    //��������
	int instonum;//�������
	int outstonum;//��������


	PartMonthStatistic():part_id(""),stocknum(0),appnum(0),instonum(0),outstonum(0)
	{

	}

};

struct PuchaseOrderItem
{

	string orderid;
	string orderdt;
	string departid;
	string departnm;
	int status;//-1 �Ѷ���/0�������/1ȫ�����

	PuchaseOrderItem():orderid(""),orderdt(""),departid(""),departnm(""),status(-1)
	{

	}
};

#endif

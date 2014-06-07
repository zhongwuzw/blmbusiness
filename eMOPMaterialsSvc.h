#ifndef __EMOPMATERIALS_SVC_H__
#define __EMOPMATERIALS_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

using namespace std;

typedef struct MaterialsIOInfo
{
	char m_szID[64];// ����id
	int m_iIOCount;//���� 
	int m_iStock;//������������ 
	int m_iType;//��������
	long m_lOpData;//����ʱ��
}ST_MLIOINFO;
typedef map<string,ST_MLIOINFO*> MLIOMAP;
class eMOPMaterialsSvc : IBusinessService, ACE_Event_Handler
{
public:
    eMOPMaterialsSvc();
    ~eMOPMaterialsSvc();

    virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(eMOPMaterialsSvc)

private:
	//������keyֵtype 0 1 2 ��Ӧ1�����2�����3�����
	int GetNewCagegoryKey(int type,string &newKey);
	// 
	int GetPics(string &o_strPics,string &i_strMid);
	// 6.1���������ֲ����Ϸ�����Ϣ
    int getMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.2���������ֲ����Ϸ�����Ϣ
	int NewMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.3�޸������ֲ����Ϸ�����Ϣ
	int UpdateMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.4ɾ�������ֲ����Ϸ�����Ϣ
	int DelMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.5���������ֲ�����µ�������Ϣ
    int getCatMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.6���������ֲ�������Ϣ
	int NewMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.7�޸������ֲ�������Ϣ
	int UpdateMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.8ɾ�������ֲ�������Ϣ
	int DelMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.9���������ֲ�������Ϣ
	int SearchMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.10���󴬴����Ϸ�����Ϣ
	int getShipMCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.11���󴬴�������Ϣ
	int getShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.12��������������Ϣ
	int NewShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.13	�޸Ĵ���������Ϣ
	int UpdateShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.14	ɾ������������Ϣ
	int DelShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.15	��������������Ϣ
	int SearchShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// �����������
	// 2.1	��ȡ����������
	int GetBaseData_l(const char* pUid, const char* jsonString, std::stringstream& out);
	// 2.2	��ȡ�Ҳ��������
	int GetBaseData_r(const char* pUid, const char* jsonString, std::stringstream& out);
	// 2.3	�������ݽӿ�
	int NewBaseData(const char* pUid, const char* jsonString, std::stringstream& out);
	// 2.5	ɾ�����ݽӿ�
	int DelBaseData(const char* pUid, const char* jsonString, std::stringstream& out);

	//��λ���� �������ڵ���ӽڵ����� 0xf5
    int GetStorageSubNodeList(const char* pUid, const char* jsonString, std::stringstream& out);
	//��λ���� ����ĳ����Žڵ��ȡ�������ṹ 0xf6
	int GetStorageTreeList(const char* pUid, const char* jsonString, std::stringstream& out);
	//��λ���� ��ӡ��޸Ŀ�λ�ڵ� 0xf7
	int NewModfiyStorageNode(const char* pUid, const char* jsonString, std::stringstream& out);
	//��λ���� ɾ����λ�ڵ� 0xf8
	int DeleteStorageNode(const char* pUid, const char* jsonString, std::stringstream& out);
    //��ע���� 0xf9
	int RemarkManager(const char* pUid, const char* jsonString, std::stringstream& out);
    //��������******* 
	int AddAttach(const char* pUid, const char* jsonString, std::stringstream& out);
	int DeleteAttach(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetAttach(const char* pUid, const char* jsonString, std::stringstream& out);

	// 6.19	��������ģ���б�
	int GetAllMterialsTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.20	��������ģ����Ϣ
	int GetMterialsTmpInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.21	������������ģ������
	int NewMterialsTmpName(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.22	�����޸�����ģ������
	int ModifyMterialsTmpName(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.23	����ɾ������ģ������
	int DelMterialsTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.24	�����������ϵ�ģ��
	int AddMterialsToTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.25	�����޸�����ģ��
	int ModifyMterialsOnTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.26	����ɾ������ģ�����������Ϣ
	int DelMterialsOnTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.28	����������Ϣ
	int getMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// ���ϲɹ�

//2.1	
	int getAppList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAppMainInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAppSubInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAPPStatus(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertAppMainIfon(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertAppMaterialFromStore(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAppMaterialDetial(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertAppMaterialDetial(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertAPPStatus(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertAPPStatusForComment(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertAppMaterialNum(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertInquiryStatus(const char* pUid, const char* jsonString, std::stringstream& out);
	int getProviderList(const char* pUid, const char* jsonString, std::stringstream& out);
	int applyConfirm(const char* pUid, const char* jsonString, std::stringstream& out);
	int generatePriceInquiry(const char* pUid, const char* jsonString, std::stringstream& out);
	int PriceInquiryList(const char* pUid, const char* jsonString, std::stringstream& out);
	int PriceInquiryMain(const char* pUid, const char* jsonString, std::stringstream& out);
	int PriceInquirySub(const char* pUid, const char* jsonString, std::stringstream& out);
	int generatePriceInquiryQuoted(const char* pUid, const char* jsonString, std::stringstream& out);
	int sendPriceInquiryQuotedMaterial(const char* pUid, const char* jsonString, std::stringstream& out);
	int getPriceInquiryProcess(const char* pUid, const char* jsonString, std::stringstream& out);
	int getPriceInquiryQuotedHistory(const char* pUid, const char* jsonString, std::stringstream& out);

	int reqInquiryByPurchase(const char* pUid, const char* jsonString, std::stringstream& out);
	int reqMaterialByInquiry(const char* pUid, const char* jsonString, std::stringstream& out);
	int reqMaterialByPurchase(const char* pUid, const char* jsonString, std::stringstream& out);
	int reqInquiryByMaterial(const char* pUid, const char* jsonString, std::stringstream& out);
	int reqOrderData(const char* pUid, const char* jsonString, std::stringstream& out);
	int generatePurchaseOrders(const char* pUid, const char* jsonString, std::stringstream& out);
	int generatePurchaseOrderDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAppListAfterShipSign(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAppMainInfoAfterShipSign(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAppSubInfoAfterShipSign(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAppStatusAfterShipSign(const char* pUid, const char* jsonString, std::stringstream& out);

	int getOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getOrderMain(const char* pUid, const char* jsonString, std::stringstream& out);
	int getOrderSub(const char* pUid, const char* jsonString, std::stringstream& out);
	int getOrderPro(const char* pUid, const char* jsonString, std::stringstream& out);
	int sendOrderConfirm(const char* pUid, const char* jsonString, std::stringstream& out);
	int sendOrderMaterialNum(const char* pUid, const char* jsonString, std::stringstream& out);
	int changeOrderStatus(const char* pUid, const char* jsonString, std::stringstream& out);
	int changeAppStatus(const char* pUid, const char* jsonString, std::stringstream& out);
	//int insertInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//int getEquimentMemory(const char* pUid, const char* jsonString, std::stringstream& out);
	//int insertEquimentMemory(const char* pUid, const char* jsonString, std::stringstream& out);
	//int updEquimentMemory(const char* pUid, const char* jsonString, std::stringstream& out);
	//int getSameCompShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//int getEquimentStandard(const char* pUid, const char* jsonString, std::stringstream& out);
	//int insertEquimentStandard(const char* pUid, const char* jsonString, std::stringstream& out);
	//int ApplyeMOPMaterialsSvcAuthority(const char* pUid, const char* jsonString, std::stringstream& out);
	//int DealWithUserApplication(const char* pUid, const char* jsonString, std::stringstream& out);
	//int getEquipmentAuth(const char* pUid, const char* jsonString, std::stringstream& out);
	//int delEquipmentAuth(const char* pUid, const char* jsonString, std::stringstream& out);	
	//int getHistorySp(const char* pUid, const char* jsonString, std::stringstream& out);	
	//int updEquipmentWh(const char* pUid, const char* jsonString, std::stringstream& out);
	//int getEqPict(const char* pUid, const char* jsonString, std::stringstream& out);	
	//int updEqPict(const char* pUid, const char* jsonString, std::stringstream& out);
	//int delEqPict(const char* pUid, const char* jsonString, std::stringstream& out);


	//��ȡ������ⵥ�б� 0x50
	int GetInstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡĳ��ⵥ�����е������б� 0x51
	int GetInstockOrderMaterialList(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ��������ⵥ��ĳ���� 0x52
	int DeleteInstockOrderMaterialItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//������ⵥ���ų�ȷ�� 0x53
	int ConformMaterialInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ���϶������б����ϴ���⣩ 0x54
	int GetPurchaseOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡĳ�������µ�������Ϣ�б� 0x55
	int GetPurchaseOrderMaterialList(const char* pUid, const char* jsonString, std::stringstream& out);
	//�ύ������ⵥ0x56
	int SetMaterialInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ȡ���ϳ��ⵥ�б� 0x57
	int GetOutstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡĳ���ⵥ�����е������б� 0x58
	int GetOutstockOrderMaterialList(const char* pUid, const char* jsonString, std::stringstream& out);
	//�ύ���ϳ��ⵥ 0x59
	int SetMaterialOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ�����ϳ��ⵥ��ĳ����0x5a
	int DeleteOutstockOrderMaterialItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//���ϳ��ⵥ���ų�ȷ��0x5b
	int ConformMaterialOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);
    //ĳ����ĳ�����������е���ⵥ�б�0x5c
	int GetMaterialItemInStockList(const char* pUid, const char* jsonString, std::stringstream& out);

	//���������̵���Ϣ 0x60
	int GetMaterialInventoryCounting(const char* pUid, const char* jsonString, std::stringstream& out);
	//���������̵� 0x61
	int SetMaterialInventoryCount(const char* pUid, const char* jsonString, std::stringstream& out);
	// ���ϳ����ͳ��
	// ���󴬴����ϲɹ���ʷ
	int GetMlOrderHis_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// ���󴬴����ϳ������ʷ
	int GetMlStoreHis_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// ���󴬴����Ͽ����ϸ
	int GetMlStockDetail_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// ���󴬴����ϳ���ͳ��
	int GetMlOutstockStat_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// ���󴬴��������Ľ���ͳ��
	int GetMlIOstockStat_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// ���󴬴��������ͳ��
	int GetMlInstockStat_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
// 
private:
	MLIOMAP m_mapMlio;
};

typedef ACE_Singleton<eMOPMaterialsSvc, ACE_Null_Mutex> g_eMOPMaterialsSvc;

typedef struct MaterialsInfo
{
	char m_szID[64];
	char m_szCatID[64];
	char m_szCode[32];
	char m_szSupplier_Code[32];
	char m_szName_cn[128];
	char m_szName_en[128];
	char m_szUnit[10];//
	double m_ffApplyPeriod;//��������
	double m_ffLifeTime;//ʹ������
	char m_szRemark[1024];//��ע
	long m_lOpData;//����޸�����
}_MaterialsInfo;

typedef std::map<std::string, _MaterialsInfo> MaterialsInfoMap; //key:id

typedef struct Materials_Cats
{
	char m_szID[64];
	char m_szName_cn[128];
	char m_szName_en[128];
	char m_szRemark[1024];//��ע
	long m_lOpData;//����޸�����
}_Materials_Cats;

typedef std::map<std::string, _Materials_Cats> MaterialsCatsMap; //key:id

struct StorageItem
{
	string id;
	string name;
	int level;

	StorageItem():id(""),name(""),level(-1)
	{

	}
};


struct AppInfo
{
	string Appid;
	string AppDate;
	string ship;
	string depart;

	AppInfo():Appid(""),AppDate(""),ship(""),depart("")
	{

	}

};



#endif

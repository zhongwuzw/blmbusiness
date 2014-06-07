#ifndef __EMOPMATERIALS_SVC_H__
#define __EMOPMATERIALS_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

using namespace std;

typedef struct MaterialsIOInfo
{
	char m_szID[64];// 物料id
	int m_iIOCount;//数量 
	int m_iStock;//操作后库存数量 
	int m_iType;//操作类型
	long m_lOpData;//操作时间
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
	//请求新key值type 0 1 2 对应1级类别，2级类别，3级类别
	int GetNewCagegoryKey(int type,string &newKey);
	// 
	int GetPics(string &o_strPics,string &i_strMid);
	// 6.1请求物料手册物料分类信息
    int getMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.2新增物料手册物料分类信息
	int NewMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.3修改物料手册物料分类信息
	int UpdateMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.4删除物料手册物料分类信息
	int DelMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.5请求物料手册类别下的物料信息
    int getCatMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.6新增物料手册物料信息
	int NewMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.7修改物料手册物料信息
	int UpdateMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.8删除物料手册物料信息
	int DelMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.9搜索物料手册物料信息
	int SearchMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.10请求船存物料分类信息
	int getShipMCatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.11请求船存物料信息
	int getShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.12新增船存物料信息
	int NewShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.13	修改船存物料信息
	int UpdateShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.14	删除船存物料信息
	int DelShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.15	搜索船存物料信息
	int SearchShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 基础数据设计
	// 2.1	获取左侧基础数据
	int GetBaseData_l(const char* pUid, const char* jsonString, std::stringstream& out);
	// 2.2	获取右侧具体数据
	int GetBaseData_r(const char* pUid, const char* jsonString, std::stringstream& out);
	// 2.3	新增数据接口
	int NewBaseData(const char* pUid, const char* jsonString, std::stringstream& out);
	// 2.5	删除数据接口
	int DelBaseData(const char* pUid, const char* jsonString, std::stringstream& out);

	//库位管理 请求树节点的子节点数据 0xf5
    int GetStorageSubNodeList(const char* pUid, const char* jsonString, std::stringstream& out);
	//库位管理 根据某抽屉号节点获取整个树结构 0xf6
	int GetStorageTreeList(const char* pUid, const char* jsonString, std::stringstream& out);
	//库位管理 添加、修改库位节点 0xf7
	int NewModfiyStorageNode(const char* pUid, const char* jsonString, std::stringstream& out);
	//库位管理 删除库位节点 0xf8
	int DeleteStorageNode(const char* pUid, const char* jsonString, std::stringstream& out);
    //备注管理 0xf9
	int RemarkManager(const char* pUid, const char* jsonString, std::stringstream& out);
    //附件管理******* 
	int AddAttach(const char* pUid, const char* jsonString, std::stringstream& out);
	int DeleteAttach(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetAttach(const char* pUid, const char* jsonString, std::stringstream& out);

	// 6.19	请求物料模板列表
	int GetAllMterialsTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.20	请求物料模板信息
	int GetMterialsTmpInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.21	请求增加物料模板名称
	int NewMterialsTmpName(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.22	请求修改物料模板名称
	int ModifyMterialsTmpName(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.23	请求删除物料模板名称
	int DelMterialsTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.24	请求增加物料到模板
	int AddMterialsToTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.25	请求修改物料模板
	int ModifyMterialsOnTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.26	请求删除物料模板里的物料信息
	int DelMterialsOnTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	// 6.28	请求物料信息
	int getMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 物料采购

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


	//获取物料入库单列表 0x50
	int GetInstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取某入库单下所有的物料列表 0x51
	int GetInstockOrderMaterialList(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除备件入库单中某物料 0x52
	int DeleteInstockOrderMaterialItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//物料入库单部门长确认 0x53
	int ConformMaterialInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取物料订购单列表（物料待入库） 0x54
	int GetPurchaseOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取某订购单下的物料信息列表 0x55
	int GetPurchaseOrderMaterialList(const char* pUid, const char* jsonString, std::stringstream& out);
	//提交物料入库单0x56
	int SetMaterialInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);

	//获取物料出库单列表 0x57
	int GetOutstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取某出库单下所有的物料列表 0x58
	int GetOutstockOrderMaterialList(const char* pUid, const char* jsonString, std::stringstream& out);
	//提交物料出库单 0x59
	int SetMaterialOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除物料出库单中某备件0x5a
	int DeleteOutstockOrderMaterialItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//物料出库单部门长确认0x5b
	int ConformMaterialOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);
    //某船舶某部门物料所有的入库单列表0x5c
	int GetMaterialItemInStockList(const char* pUid, const char* jsonString, std::stringstream& out);

	//请求物料盘点信息 0x60
	int GetMaterialInventoryCounting(const char* pUid, const char* jsonString, std::stringstream& out);
	//设置物料盘点 0x61
	int SetMaterialInventoryCount(const char* pUid, const char* jsonString, std::stringstream& out);
	// 物料出入库统计
	// 请求船存物料采购历史
	int GetMlOrderHis_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// 请求船存物料出入库历史
	int GetMlStoreHis_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// 请求船存物料库存详细
	int GetMlStockDetail_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// 请求船存物料出库统计
	int GetMlOutstockStat_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// 请求船存物料消耗接收统计
	int GetMlIOstockStat_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	// 请求船存物料入库统计
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
	double m_ffApplyPeriod;//申领周期
	double m_ffLifeTime;//使用寿命
	char m_szRemark[1024];//备注
	long m_lOpData;//最后修改日期
}_MaterialsInfo;

typedef std::map<std::string, _MaterialsInfo> MaterialsInfoMap; //key:id

typedef struct Materials_Cats
{
	char m_szID[64];
	char m_szName_cn[128];
	char m_szName_en[128];
	char m_szRemark[1024];//备注
	long m_lOpData;//最后修改日期
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

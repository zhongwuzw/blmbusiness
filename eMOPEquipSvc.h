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
//	int m_iFNumApply;// 申请单流水号
private:
	// ---------------------------------------------------------------------------------------
	// 备件手册
	// ---------------------------------------------------------------------------------------
	//获取某一级设备接口
    int getAllEquipment(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取设备属性接口
	int GetEquipPara(const char* pUid, const char* jsonString, std::stringstream& out);
	//添加或者编辑设备接口
	int NewEquipment(const char* pUid, const char* jsonString, std::stringstream& out);
	// 删除设备接口
	int DelEquip(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取设备备件列表
    int GetEquipParts(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取备件属性接口
	int GetPartsPara(const char* pUid, const char* jsonString, std::stringstream& out);
	//添加新备件和编辑备件接口
	int NewParts(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetCheckInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	// 删除备件接口
	int DelParts(const char* pUid, const char* jsonString, std::stringstream& out);
	// 获取备件路径接口
	int GetPaths(const char* pUid, const char* jsonString, std::stringstream& out);
	// 获取备件采购历史请求接口
	int GetPartsOrderHis(const char* pUid, const char* jsonString, std::stringstream& out);
	// 获取备件属出入库历史
	int GetPartsStoreHis(const char* pUid, const char* jsonString, std::stringstream& out);
	// 获取备件库存详细
	int GetPartsInstock(const char* pUid, const char* jsonString, std::stringstream& out);
	// 库位设置接口
	int SetStorePos(const char* pUid, const char* jsonString, std::stringstream& out);
	//搜索备件接口
	int SearchPartsOnShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//高储预警低储预警
	int GetWarndata(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取备件入库情况
	// ---------------------------------------------------------------------------------------
	// 申请单
	// ---------------------------------------------------------------------------------------
	//申请单搜索
	int ApplySearch(const char* pUid, const char* jsonString, std::stringstream& out);
	// 生成申请单
	int NewApplyOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	// 搜索订单
	int SearchAppOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	//订单详情请求
	int GetAppOrderDetails(const char* pUid, const char* jsonString, std::stringstream& out);
	//订单修改接口
	int ModifyAppOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	//订单审核意见
	int AppOrderOpt(const char* pUid, const char* jsonString, std::stringstream& out);

	//获取验收单列表
	int GetCheckList(const char* pUid, const char* jsonString, std::stringstream& out);
	int CreateCheckList(const char* pUid, const char* jsonString, std::stringstream& out);
	//订单状态数量
	int GetApporderNum(const char* pUid, const char* jsonString, std::stringstream& out);
	// 获取流程信息
	int GetWorkfllow(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取供应商列表
	int GetAllPartsCorps(const char* pUid, const char* jsonString, std::stringstream& out);
	//生成询价单
	int MkInquirySheete(const char* pUid, const char* jsonString, std::stringstream& out);
	// ---------------------------------------------------------------------------------------
	// 询价单
	// ---------------------------------------------------------------------------------------
	//获取询价单列表
	int GetInquirySheeteList(const char* pUid, const char* jsonString, std::stringstream& out);
	//查询询价单详细信息
	int GetInquirySheetDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//根据询价单查询该询价单的采购单
	int GetAppOrderDetail_Iqsid(const char* pUid, const char* jsonString, std::stringstream& out);
	//备件报价历史列表
	int GetIqsHis_Partsid(const char* pUid, const char* jsonString, std::stringstream& out);
	//采购单对应的询价单列表
	int GetIqsList_Appo(const char* pUid, const char* jsonString, std::stringstream& out);
	//询价单对应的备件列表
	int GetPartsList_Iqs(const char* pUid, const char* jsonString, std::stringstream& out);
	//采购单对应的备件列表
	int GetPartsList_Appo(const char* pUid, const char* jsonString, std::stringstream& out);
	//备件对应的询价单列表
	int GetIqsList_Partsid(const char* pUid, const char* jsonString, std::stringstream& out);
	//备件对应的订购单
	int GetAppoList_Partsid(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除询价单
	int DeleteIqs(const char* pUid, const char* jsonString, std::stringstream& out);

	//获取订单流程信息
	int GetWorkFlowInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	
	int EquipPurchaseCheck(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除附件
	int DelEquipFile(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除申请单
	int DelApplyInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//完成报价
	int CompleteIqs(const char* pUid, const char* jsonString, std::stringstream& out);
	//生成订购单---直接订购/比价订购
	int MkPurchaseSheete(const char* pUid, const char* jsonString, std::stringstream& out);
	// ---------------------------------------------------------------------------------------
	// 订购单
	// ---------------------------------------------------------------------------------------
	//订购单搜索
	int SearchPurchase(const char* pUid, const char* jsonString, std::stringstream& out);
	//订购单详情
	int GetPurchaseDetail(const char* pUid, const char* jsonString, std::stringstream& out);	
	//订购单修改
	int ModifyPurchase(const char* pUid, const char* jsonString, std::stringstream& out);
	//订购单状态数量
	int GetPchStatusCount(const char* pUid, const char* jsonString, std::stringstream& out);
	//订购单意见审核
	int PurchaseOpt(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取备件申请历史
	int GetPartsHis(const char* pUid, const char* jsonString, std::stringstream& out);
	// other func
	int newAppOrderNum(string &o_strAppNum,string &i_strShipId);
	int GetDptFromUserid(string &o_strDptid,const string &i_strUid);
	int GetSubData(std::stringstream& out,string &i_strKey,MySql* psql );	
	int GetAllSteps(vector <string>&o_vecSteps,MySql* psql,string &i_strTaskid);


	// ---------------------------------------------------------------------------------------
	// 盘点、备件月度增减统计 add by jt 
	// ---------------------------------------------------------------------------------------

    //获取备件入库单列表 0x50
    int GetInstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取某入库单下所有的备件列表 0x51
	int GetInstockOrderPartList(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除备件入库单中某备件 0x52
	int DeleteInstockOrderPartItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//备件入库单部门长确认 0x53
	/*int ConformPartInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);*/
	//获取备件订购单列表（备件待入库） 0x54
	int GetPurchaseOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	//根据申请单获取询价单 0x55
	int GetApplyInqueryInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    //根据申请单获取订购单0x56
    int GetApplyOrdersInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//修改备件数量
	int EditCheckNum(const char* pUid, const char* jsonString, std::stringstream& out);


	//获取申请单详情0x57
	 int GetDetailApplyInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	 //根据申请单id获取备件列表信息
	 int GetEquipListInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//获取询价单详情 0x58
     int GetIqsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	 //提交备件出库单 0x59
	 /*int SetPartOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);*/


	 int GetIqsEquipList(const char* pUid, const char* jsonString, std::stringstream& out);

	 //删除备件出库单中某备件0x5a
	 //int DeleteOutstockOrderPartItem(const char* pUid, const char* jsonString, std::stringstream& out);

	int GetPurchaseEquipList(const char* pUid, const char* jsonString, std::stringstream& out);


	 //备件出库单部门长确认0x5b
	 int ConformPartOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//备件申请单修改
	int EditApllyInfo(const char* pUid, const char* jsonString, std::stringstream& out);


	//请求备件盘点信息 0x60
	int GetPartsInventoryCounting(const char* pUid, const char* jsonString, std::stringstream& out);
    //设置备件盘点 0x61
	int SetPartsInventoryCount(const char* pUid, const char* jsonString, std::stringstream& out);
	//备件月度增减统计 0x62
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
	string equip_model;//设备型号
	string equip_name;
	int stocknum;   //当前库存
	string locateid;
	int appnum;    //申请数量
	int instonum;//入库数量
	int outstonum;//出库数量


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
	int status;//-1 已订购/0部分入库/1全部入库

	PuchaseOrderItem():orderid(""),orderdt(""),departid(""),departnm(""),status(-1)
	{

	}
};

#endif

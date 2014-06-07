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
	//修理项管理***********************************************
	//请求全部修理类别
	int GetAllRepairCat(const char* pUid, const char* jsonString, std::stringstream& out);
	//根据类别请求修理项的所有数据
	int GetCatItems(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求修理项下的船舶,设备卡，工作卡
	int GetItemDetail(const char* pUid, const char* jsonString, std::stringstream& out); 
	//建或者修改修理项
	int NewOrUpdateRepairItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除修理项
	int DelRepairItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求船舶设备下所有工作卡
	int GetAllShipWorkCard(const char* pUid, const char* jsonString, std::stringstream& out); 
	//增加或者删除船舶
	int NewOrDelItemShips(const char* pUid, const char* jsonString, std::stringstream& out);
	//增加或者删除设备
	int NewOrDelItemEquips(const char* pUid, const char* jsonString, std::stringstream& out);
	//增加或者删除工作卡
	int NewOrDelWorkCard(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除工作选项
	int DeleteRepairItem(const char* pUid, const char* jsonString, std::stringstream& out);

	//修理单管理***********************************************
	//请求修理单列表
	int GetRepairApplyList(const char* pUid, const char* jsonString, std::stringstream& out);
    //请求修理单详细数据
	int GetRepairApplyDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//增加或者修改修理单
	int AddOrUpdateRepairApply(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取船舶修理项列表
	int GetShipRepairItem(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除修理单
	int DeleteRepairApply(const char* pUid, const char* jsonString, std::stringstream& out);
	//更改修理单状态
	int UpdateRepairApplyStatus(const char* pUid, const char* jsonString, std::stringstream& out);
    
	//厂修管理***********************************************
    //请求厂修管理单
	int GetFactoryRepair(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求某船舶下所有未开始的未被分配的的工程单
	int GetShipEngineeringNotBegin(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求某船舶下检验列表
	int GetShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//修改厂修
	int AddOrUpdateFactoryRepair(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除厂修
	int DeleteFactoryRepair(const char* pUid, const char* jsonString, std::stringstream& out);
    //厂修添加工程单
	int AddOrDelEnginerringToFactory(const char* pUid, const char* jsonString, std::stringstream& out);

	//工程单管理***********************************************
	//请求工程单
	int GetEngineering(const char* pUid, const char* jsonString, std::stringstream& out);
	//工程单详细数据
	int GetEngineeringDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//增加或者修改工程单
	int NewOrUpdateEngineering(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求某船舶的未派工程单的且状态为未开始的修理单列表
	int GetNotStartEngineering(const char* pUid, const char* jsonString, std::stringstream& out);
	//完成修理单
	int FinishRepairApply(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除工程单
	int DeleteEngineering(const char* pUid, const char* jsonString, std::stringstream& out);


	//设备卡下的工作卡（所有）
	int getEquipWorkCard(std::string &equipCardId,std::string &cardArrStr);
	//修理单设备卡下的工作卡（修理单）
	int getRepairEquipWorkCard(std::string &shipId,std::string &itemCode,std::string &cwbt,std::string &cardArrStr);
	//厂修内容 Name
	int getFactoryRepairContent(std::string &id,std::string &outStr);
	//厂修内容 id Name
	int getFactoryRepairContent2(std::string &id,std::string &outStr);
	//工程单其他说明项
	int getOtherDrections(std::string &proid,std::string &outStr);
	//工程单的修理单集合
	int getRepairApplies(std::string &proid,std::string &outStr); 
	//厂修管理的工程单集合
	int getFactoryEngineer(std::string &id,std::string &outStr); 
	//检验公司是否存在，不存在需要插入新的公司
	bool checkNewCompany(std::string &cid,std::string &cname);


	
};

typedef ACE_Singleton<eMOPRepairSvc, ACE_Null_Mutex> g_eMOPRepairSvc;

#endif // AREASVC_H

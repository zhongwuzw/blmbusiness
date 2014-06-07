#ifndef __EMOPMAINTENANCES_SVC_H__
#define __EMOPMAINTENANCES_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include <vector>
#include <string> 
															
using namespace std;
class CMaintainPlan
{
public:
	CMaintainPlan()
	{}
	~CMaintainPlan()
	{}
	char m_szId[30];
	int m_iScYear;
	int m_iScMon;
	int m_iStatus;
	long m_iFinishTime;
	int m_iFinishYear;
	int m_iFinishMon;
	int m_iTiming;
	int m_iTotalTiming;
};
class WorkCard
{
public:
	WorkCard()
	{}
	~WorkCard()
	{}
	char m_szId[30];
	char m_szLlevel[2];
	char m_szOwner[20];
	char m_szExcuteway[20];
	int m_iStatus;
	int m_iTFlag;
	int m_iPeriod;
	int m_iTolerence;
	int m_iAddFlag;
	int m_iExpiredt;
	map<string,CMaintainPlan*> m_mapMTPlan;
};
class CDeviceCard
{
public:
	CDeviceCard()
	{
	}
	~CDeviceCard()
	{
	}
	char m_szShipid[30];
	char m_szDptid[30];
	char m_szDptname[30];
	char m_szId[30];
	char m_szCwbt[30];
	char m_szEqname_cn[30];
	char m_szEqname_en[30];
	map<string,WorkCard*> m_mapWorkcard;
};
class CMTSteps
{
public:
	CMTSteps()
	{}
	~CMTSteps()
	{}
	char m_szMtid[32];
	char m_szOperator[32];
	char m_szWktaskid[32];
	char m_szWkstepid[32];
	long m_lOpdate;
	int m_iStatus;
	
};
class eMOPMaintenanceSvc : IBusinessService, ACE_Event_Handler
{
public:
	eMOPMaintenanceSvc();
	~eMOPMaintenanceSvc();

	virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

	DECLARE_SERVICE_MAP(eMOPMaintenanceSvc)

private:
	// 根据船舶和部门请求设备卡列表信息 0x01
	int GetEquipCardList(const char* pUid, const char* jsonString, std::stringstream& out);
	// 请求设备卡详细信息 0x02
	int GetEquipCardDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	// 新增设备卡 0x03
	int NewEquipCardItem(const char* pUid, const char* jsonString, std::stringstream& out);
	// 修改设备卡 0x04
	int ModifyEquipCardItem(const char* pUid, const char* jsonString, std::stringstream& out);
	// 删除设备卡 0x05
	int DeleteEquipCardItem(const char* pUid, const char* jsonString, std::stringstream& out);
	
	// 查看设备更换历史 0x33
	int GetEquipChangeHis(const char* pUid, const char* jsonString, std::stringstream& out);


    //根据设备卡ID 获取工作卡列表信息 0x06
	int GetWorkCardList(const char* pUid, const char* jsonString, std::stringstream& out);
    // 请求工作卡详细信息 0x07
	int GetWorkCardDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	// 新增工作卡 0x08
	int NewWorkCardItem(const char* pUid, const char* jsonString, std::stringstream& out);
	// 修改工作卡 0x09
	int ModifyWorkCardItem(const char* pUid, const char* jsonString, std::stringstream& out);
	// 删除工作卡 0x0a
	int DeleteWorkCardItem(const char* pUid, const char* jsonString, std::stringstream& out);

	//根据船舶和部门请求工作卡计划列表 0x10
	int GetWorkPlanList(const char* pUid, const char* jsonString, std::stringstream& out);
	//首排计划 0x11
	int FirstTimeAssignPlan(const char* pUid, const char* jsonString, std::stringstream& out);
    //变更计划 0x12
	int ChangePlanTime(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求某计划报告基本信息0x13
	int GetWorkPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求计划保养总工作流 0x14
    int GetMaintenanceWorkFlow(const char* pUid, const char* jsonString, std::stringstream& out);
	//船端填写实绩报告 0x15
	int ShipFillPlanReport(const char* pUid, const char* jsonString, std::stringstream& out);
	//实绩报告确认、反馈、批复 0x16
	int ConformPlanReport(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求某计划已执行工作流0x17
	int GetPlanWorkFlow(const char* pUid, const char* jsonString, std::stringstream& out);

	//请求常规保养计划实时运行信息 0x20
	int RoutineMaintenanceList(const char* pUid, const char* jsonString, std::stringstream& out);
	//设置常规保养值状态 0x21
	int SetRoutineMaintenanceVal(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求常规保养计划维护模板信息 0x22
	int GetRoutineMaintenanceTemplate(const char* pUid, const char* jsonString, std::stringstream& out);
    //新增常规保养项 0x23
	int NewRoutineMaintenanceItem(const char* pUid, const char* jsonString, std::stringstream& out);
    //删除常规保养项 0x24
	int DelRoutineMaintenanceItem(const char* pUid, const char* jsonString, std::stringstream& out);
    //常规保养签字 0x25
	int SignRoutineMaintenanceItem(const char* pUid, const char* jsonString, std::stringstream& out);
    //修改常规保养工作卡
	int UpdateRoutineMaintenanceWorkCard(const char* pUid, const char* jsonString, std::stringstream& out);

	//请求定时设备(子设备)运行记录(岸端) 0x26
	int GetEquipRunRecord_Shore(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求主设备运行时间记录 0x27
	int GetMainEquipTimeRecord(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求子设备各级保养计划 0x28
	int GetSubEquipMaintenancePlan(const char* pUid, const char* jsonString, std::stringstream& out);
	//子设备累计运行时间 0x29
    int GetSubEquipRunTime(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求定时设备(子设备)当前记录(船端) 0x2a
	int GetEquipRunRecord_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	//设置定时设备(子设备)运行时间(船端) 0x2b
    int SetEquipRunRecord_Ship(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//计划调整历史 0x2a
	int GetWorkPlanChangeHistory(const char* pUid, const char* jsonString, std::stringstream& out);
	//
	//请求主系统代码
	int GetMainSysCode(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetCWBT(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetWCardplan(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetUnfinishedTasks(const char* pUid, const char* jsonString, std::stringstream& out);
	// data
	map<string,CDeviceCard*> m_mapDevicecard;
	int LoadDevicecard();
	int LoadWorkcard(map<string,WorkCard*> &o_mapWorkcard,string &i_strDeviceid);
	int LoadPlan(map<string,CMaintainPlan*> &o_mapMPlan,string &i_strWorkcardid);
	vector<CMTSteps*> m_vecSteps;
	int LoadMTSteps();
	void CleanMTSteps();
	bool GetMTSteps(CMTSteps &o_stData,string &i_strIndex);
};

//设备运转信息
struct EquipRunInfo
{
	string equipId;
	string cwbt;
	string equipName;
	long runtm_curmonth;
	long runtm_total;
	long runtm_before;
	long runtm_after;
	string startdt;
	string recorddt;

	EquipRunInfo():equipId(""),runtm_curmonth(0),runtm_total(0),runtm_before(0),runtm_after(0)
	{

	}
};

struct RoutineMaintenanceItem
{
	string id;
	string name;
	string cwbt;
	vector<int> vecStatus;
};

typedef ACE_Singleton<eMOPMaintenanceSvc, ACE_Null_Mutex> g_eMOPMaintenanceSvc;


struct WorkCardPlanInfo
{
	string planid;
	int status;
	int year;
	int month;
	string plandt;
	string finishdt;
	long runtime;
	string cwbt;
	string equipname;
	string grade;
	string pduty;
	int period;
	int battach;
	string m_strDptname;
	long timing;
	string m_strDptid; 

	WorkCardPlanInfo():planid(""),status(0),period(0),battach(0),runtime(0),year(0),month(0)
	{

	}
};



#endif

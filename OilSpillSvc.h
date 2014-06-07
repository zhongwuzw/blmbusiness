#ifndef __OILSPILL_SVC_H__
#define __OILSPILL_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include <string>
using namespace std;

struct TaskAreaItem  
{
	string areaid;
	string areaname;
	string areatype;
	string points;
	string zarea;//区域面积
	string surplus;
};

struct TaskPointThick
{
	float xpos;
	float ypos;
	float thickval;

	TaskPointThick():xpos(361),ypos(361),thickval(0)
	{

	}
};

struct EqualLevelItem 
{
	char id[32];
	string location;
	int level;
};

class OilSpillSvc : IBusinessService
{
public:
	OilSpillSvc(void);
	~OilSpillSvc(void);
	virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

	DECLARE_SERVICE_MAP(OilSpillSvc)

	 int GetOilSpillTaskList(const char* pUid, const char* jsonString, std::stringstream& out);
     int GetOilSpillSubTaskList(const char* pUid, const char* jsonString, std::stringstream& out);
     int AddUpdOilSpillTask(const char* pUid, const char* jsonString, std::stringstream& out);
	 int GetOilSpillTaskDetail(const char* pUid, const char* jsonString, std::stringstream& out);
     int DelOilSpillTask(const char* pUid, const char* jsonString, std::stringstream& out);

     
     int GetOilDetail(const char* pUid, const char* jsonString, std::stringstream& out);
     int GetOilSpillForcast(const char* pUid, const char* jsonString, std::stringstream& out);
	 int GetOilSpillAnalysis(const char* pUid, const char* jsonString, std::stringstream& out);


	  int AddUpdSubTaskEmergency(const char* pUid, const char* jsonString, std::stringstream& out);
      int GetSubTaskEmergencyDetail(const char* pUid, const char* jsonString, std::stringstream& out);

	  int AddUpdSubTaskObervation(const char* pUid, const char* jsonString, std::stringstream& out);
	  int GetSubTaskObervationDetail(const char* pUid, const char* jsonString, std::stringstream& out);


	  int AddUpdSubTaskVideo(const char* pUid, const char* jsonString, std::stringstream& out);
	  int GetSubTaskVideoDetail(const char* pUid, const char* jsonString, std::stringstream& out);

	  int DelOilSpillSubTask(const char* pUid, const char* jsonString, std::stringstream& out);
	  int GetOilSpillTaskStatus(const char* pUid, const char* jsonString, std::stringstream& out);
	  int GetOilSpillTaskCalStatus(const char* pUid, const char* jsonString, std::stringstream& out);


	  int GetOilSpillTaskTrace(const char* pUid, const char* jsonString, std::stringstream& out);

	  int WtiGetOilTrace(const char* pUid, const char* jsonString, std::stringstream& out);
      int WtiGetDispPoints(const char* pUid, const char* jsonString, std::stringstream& out);
	  int WtiGetPointDetail(const char* pUid, const char* jsonString, std::stringstream& out);

	  //------------------------------溢油处置计划相关接口----------------------------------------

	  //请求所有溢油作业船舶列表0x41
	  int GetOilShiplist(const char* pUid, const char* jsonString, std::stringstream& out);
	  //请求溢油作业船舶详细信息0x22
	  int GetOilShipRelations(const char* pUid, const char* jsonString, std::stringstream& out);
      //请求物资码头列表0x42
	  int GetOilGoodsDocklist(const char* pUid, const char* jsonString, std::stringstream& out);
	  //修改船舶信息0x43
	  int ModifyShipInfo(const char* pUid, const char* jsonString, std::stringstream& out);

      //请求溢油处置计划列表 0x30
	  int GetOilPlanlist(const char* pUid, const char* jsonString, std::stringstream& out);
	  //溢油处置计划详细信息 0x31
      int GetOilPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out);
      //新建(修改)溢油处置计划 0x32
	  int NewModifyOilPlan(const char* pUid, const char* jsonString, std::stringstream& out);
	  //溢油处置计划操作（出发、开始作业、结束作业、删除计划） 0x33
	  int OperatOilPlan(const char* pUid, const char* jsonString, std::stringstream& out);
	  //新建（修改）保障计划申请 0x34
	  int NewModifySuppPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //删除保障计划申请 0x35
	  int DeleteSuppPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);

	  //新建（修改）转驳计划申请 0x36
	  int NewModifyTransferPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //删除转驳计划申请 0x37
	  int DeleteTransferPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //请求所有待处理保障计划 0x38
	  int GetAllSuppPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //新建（修改）溢油保障计划 0x39
	  int NewModifySuppPlanItem(const char* pUid, const char* jsonString, std::stringstream& out);
      //溢油保障计划详细信息0x3A
	  int GetOilSuppPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	  //删除保障计划0x3B
	  int DeleteSuppPlanItem(const char* pUid, const char* jsonString, std::stringstream& out);

	  //请求所有待处理转驳计划 0x3C
	  int GetAllTransferPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //新建（修改）溢油转驳计划 0x3D
	  int NewModifyTransferPlanItem(const char* pUid, const char* jsonString, std::stringstream& out);
	  //溢油转驳计划详细信息3E
       int GetOilTransferPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	   //删除保障计划0x3F
	   int DeleteTransferPlanItem(const char* pUid, const char* jsonString, std::stringstream& out);
	   //设置溢油境界区域0x40
	   int SetOilSpillAlertArea(const char* pUid, const char* jsonString, std::stringstream& out);



private:
	//业务请求


};

typedef ACE_Singleton<OilSpillSvc, ACE_Null_Mutex> g_OilSpillSvc;

inline string OilSpillGetCurrentTmStr()
{
	char timestr[20]="";
	time_t lt= time(NULL);
	struct tm *ptr = localtime(&lt);
	strftime(timestr,20,"%Y-%m-%d %H:%M:%S",ptr);

	return string(timestr);
}

#endif

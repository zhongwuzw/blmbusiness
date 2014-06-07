#ifndef __BLM_SAR_SVR_H__
#define __BLM_SAR_SVR_H__

#include <string>
#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"

using namespace std;

class SARSvc: IBusinessService
{
public:
	SARSvc();
	~SARSvc();

	virtual bool Start(); 
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	DECLARE_SERVICE_MAP(SARSvc)

private:
	int getAccidents(const char* pUid, const char* jsonString, std::stringstream& out);
	int modifyAccident(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAccident(const char* pUid, const char* jsonString, std::stringstream& out);
	int deleteAccident(const char* pUid, const char* jsonString, std::stringstream& out);
	int getLKP(const char* pUid, const char* jsonString, std::stringstream& out);
	int modifyLKP(const char* pUid, const char* jsonString, std::stringstream& out);
	int deleteLKP(const char* pUid, const char* jsonString, std::stringstream& out);
	int modifyForecast(const char* pUid, const char* jsonString, std::stringstream& out);
	int deleteForecast(const char* pUid, const char* jsonString, std::stringstream& out);
	int getForecast(const char* pUid, const char* jsonString, std::stringstream& out);
	int getForecastTrack(const char* pUid, const char* jsonString, std::stringstream& out);
	int modifyForecastTrack(const char* pUid, const char* jsonString, std::stringstream& out);
	int customWind(const char* pUid, const char* jsonString, std::stringstream& out);

	//新建(修改)事故 0x01
	int NewModifyAccident(const char* pUid, const char* jsonString, std::stringstream& out);
    //删除事故 0x02
	int DeleteAccident(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取事故列表 0x03
	int GetAccidentList(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取某事故下所有的漂移预测列表 0x04
	int GetAccForcastList(const char* pUid, const char* jsonString, std::stringstream& out);
    //新建(修改)漂移预测 0x05
    int NewModifyAccForcast(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除漂移预测 0x06
	int DeleteAccForcast(const char* pUid, const char* jsonString, std::stringstream& out);
    //请求漂移预测详细信息 0x07
	int GetForcastDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求漂移预测轨迹 0x08
	int GetForcastTraceInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求搜救计划列表 0x09
	int GetPlanList(const char* pUid, const char* jsonString, std::stringstream& out);
	//新建(修改)搜救计划 0x10
	int NewModifyPlan(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求搜救计划报告 0x11
	int GetPlanReport(const char* pUid, const char* jsonString, std::stringstream& out);	
	//新建(修改)搜救计划报告 0x12
	int NewModifyPlanReport(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求SRU LIST 0x13
	int GetSRUList(const char* pUid, const char* jsonString, std::stringstream& out);
	//新建(修改)SRU 0x14
	int NewModifySRUList(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求属性详细信息 0x15
	int GetPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//更新搜救计划状态 0x16
	int UpdatePlanStatus(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除搜救计划 0x17
	int DeletePlan(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除计划SRU 0x18
	int DeletePlanSRU(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求最新的一个事故 0x20
	int GetNewestAccident(const char* pUid, const char* jsonString, std::stringstream& out); 
	//请求基地列表 0x32
	int GetBaseList(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求加油地列表 0x33
	int GetStationList(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求基地详细信息 0x34
	int GetBaseInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求加油地详细信息 0x35
	int GetStationInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//添加或修改一条基地详细信息 0x36
	int AddOrModBase(const char* pUid, const char* jsonString, std::stringstream& out);
	//添加或修改一条加油地详细信息 0x37
	int AddOrModStation(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除一条基地信息  0x38
	int DeleteBase(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除一条加油地信息 0x39
	int DeleteStation(const char* pUid, const char* jsonString, std::stringstream& out);

	//请求SRU列表0x61
	int GetSRUAndTypeList(const char* pUid, const char* jsonString, std::stringstream& out);

	//请求SRU详细信息0x64
	int GetSRUInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//新建（修改）SRU类型0x62
	int NewModifySRUType(const char* pUid, const char* jsonString, std::stringstream& out);

	//新建（修改）SRU0x65
	int NewModifySRU(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除SRU类型0x63
	int DeleteSRUType(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除SRU0x66
	int DeleteSRU(const char* pUid, const char* jsonString, std::stringstream& out);

	//搜救船舶列表0x90
	int GetRescueShipStatistic(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//搜救船舶列表0x91
	int GetRescuePlaneStatistic(const char* pUid, const char* jsonString, std::stringstream& out);

private:
	std::string getAccidentID();
	std::string getPersionID(const std::string& accid);
	std::string getLkpID();
	std::string getForecastID();
};

typedef ACE_Singleton<SARSvc, ACE_Null_Mutex> g_SARSvc;
#endif

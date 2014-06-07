#ifndef __EQUIPMENT_SVC_H__
#define __EQUIPMENT_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

using namespace std;

class EquipmentSvc : IBusinessService, ACE_Event_Handler
{
public:
    EquipmentSvc();
    ~EquipmentSvc();

    virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(EquipmentSvc)

private:
    int getCategoryInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updCategoryInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int addCategoryInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int delCategoryInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getEquimentList(const char* pUid, const char* jsonString, std::stringstream& out);
	int updEquimentInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int addEquimentInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int delEquimentInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getDataList(const char* pUid, const char* jsonString, std::stringstream& out);
	int updDataInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int addDataInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int delDataInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getEquimentMemory(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertEquimentMemory(const char* pUid, const char* jsonString, std::stringstream& out);
	int updEquimentMemory(const char* pUid, const char* jsonString, std::stringstream& out);
	int getSameCompShip(const char* pUid, const char* jsonString, std::stringstream& out);
	int getEquimentStandard(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertEquimentStandard(const char* pUid, const char* jsonString, std::stringstream& out);
	int ApplyEquipmentSvcAuthority(const char* pUid, const char* jsonString, std::stringstream& out);
	int DealWithUserApplication(const char* pUid, const char* jsonString, std::stringstream& out);
	int getEquipmentAuth(const char* pUid, const char* jsonString, std::stringstream& out);
	int delEquipmentAuth(const char* pUid, const char* jsonString, std::stringstream& out);	
	int getHistorySp(const char* pUid, const char* jsonString, std::stringstream& out);	
	int updEquipmentWh(const char* pUid, const char* jsonString, std::stringstream& out);
	int getEqPict(const char* pUid, const char* jsonString, std::stringstream& out);	
	int updEqPict(const char* pUid, const char* jsonString, std::stringstream& out);
	int delEqPict(const char* pUid, const char* jsonString, std::stringstream& out);
};

typedef ACE_Singleton<EquipmentSvc, ACE_Null_Mutex> g_EquipmentSvc;

typedef struct _EquipmentInfo{
	char cid[64];
	char eqid[64];
	char eqnm[32];
	char mtp[64];
	char spe[64];
	char unit[10];
	double sp;
	char unm[64];
	double inv;
	char desc[1024 * 10];
	set<string> cks;		//surplus表的仓库号
	set<string> cki;		//inv表的仓库号
	set<string> c_time;
	int qnum;				//本季度流水量
}EquipmentInfo;

typedef std::map<std::string, EquipmentInfo> EquipmentInfoMap; //key:eqid@compid

typedef struct _EquipmentHistoryInfo{
	char eqid[64];
	char eqnm[32];
	char mtp[64];
	char dhh[64];
	char spe[64];
	char mafr[64];
	double sp0;			//期初余量
	double sp1;			//期末余量
	double incr;		//本期增长
	double decr;		//本期减少
	char remark[1024 * 10];
}EquipmentHisInfo;

typedef std::map<std::string, EquipmentHisInfo> EquipmentHisInfoMap; //key:eqid

typedef struct _SurplusHistory
{
	char eqid[64];
	char shipid[32];
	char cid[64];
	double surplus;
	double incr;
	double decr;
}SurplusHistory;

typedef std::map<std::string, SurplusHistory> SurplusHisInfoMap; //key:eqid

#endif

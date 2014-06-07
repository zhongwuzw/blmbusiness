#ifndef __BERTH_SVC_H__
#define __BERTH_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

using namespace std;

class BerthSvc : IBusinessService, ACE_Event_Handler
{
public:
    BerthSvc();
    ~BerthSvc();

    virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(BerthSvc)

private:
    int getPortBerthList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthCompInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthShipInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthMYWInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getEquipmentList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getStorageList(const char* pUid, const char* jsonString, std::stringstream& out);
    
	//int getBerthCameraInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getBerthCompManagerInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getTerminalInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getTerminalList(const char* pUid, const char* jsonString, std::stringstream& out);
	int newTerminalItem(const char* pUid, const char* jsonString, std::stringstream& out);
    int delTerminalItem(const char* pUid, const char* jsonString, std::stringstream& out);
	int udpOperatorsDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int newBerthItem(const char* pUid, const char* jsonString, std::stringstream& out);
	int delBerthItem(const char* pUid, const char* jsonString, std::stringstream& out);
	int newBerthEquipment(const char* pUid, const char* jsonString, std::stringstream& out);
	int delBerthEquipment(const char* pUid, const char* jsonString, std::stringstream& out);
	int newTerminalStorage(const char* pUid, const char* jsonString, std::stringstream& out);
	int delTerminalStorage(const char* pUid, const char* jsonString, std::stringstream& out);
	int getStatInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getMoorShipInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getCargoInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updShipBaseInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getInOutInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getInOutList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getUndealWarn(const char* pUid, const char* jsonString, std::stringstream& out);
	int exclWarning(const char* pUid, const char* jsonString, std::stringstream& out);
	int getStatIndex(const char* pUid, const char* jsonString, std::stringstream& out);
	int getInOutDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int getContactInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int addShipContactPerson(const char* pUid, const char* jsonString, std::stringstream& out);
	int delShipContactPerson(const char* pUid, const char* jsonString, std::stringstream& out);
	int getUserAuth(const char* pUid, const char* jsonString, std::stringstream& out);

	int getShipDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int udpShipDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int getSystemUser(const char* pUid, const char* jsonString, std::stringstream& out);
	int updUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int delUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updUserPassword(const char* pUid, const char* jsonString, std::stringstream& out);
	int checkUser(const char* pUid, const char* jsonString, std::stringstream& out);
	int getSystemAuthrity(const char* pUid, const char* jsonString, std::stringstream& out);
	int setUserAuth(const char* pUid, const char* jsonString, std::stringstream& out);
	int getPortOrgan(const char* pUid, const char* jsonString, std::stringstream& out);

	int getConcernShips(const char* pUid, const char* jsonString, std::stringstream& out);
	int addConcernShips(const char* pUid, const char* jsonString, std::stringstream& out);
	int delConcernShips(const char* pUid, const char* jsonString, std::stringstream& out);

	int getCompanyList(const char* pUid, const char* jsonString, std::stringstream& out);
	int delCompanyItem(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAlertList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAlertUsers(const char* pUid, const char* jsonString, std::stringstream& out);
	int setAlertUsers(const char* pUid, const char* jsonString, std::stringstream& out);

	int getRemoveShip(const char* pUid, const char* jsonString, std::stringstream& out);
	int delRemoveShip(const char* pUid, const char* jsonString, std::stringstream& out);
	int getLocalShip(const char* pUid, const char* jsonString, std::stringstream& out);
	int searchShipByName(const char* pUid, const char* jsonString, std::stringstream& out);
	int setAlertLevel(const char* pUid, const char* jsonString, std::stringstream& out);

	int delThruptInfo(const char* pUid, const char* jsonString, std::stringstream& out);
};

typedef ACE_Singleton<BerthSvc, ACE_Null_Mutex> g_BerthSvc;

typedef struct _EquipmentInfo_B
{
	char id[64];     
	char seq[64];			//编号
    char name[32];			//名称
    char type[32];			//机械类型
    char spec[32];			//规格
	char capacity[32];		//
    char validdt[32];		//有效期
    char producer[64];		//生产商
	char berthid[32];
	char berthname[128];
}EquipmentInfo_B;

typedef std::map<std::string, EquipmentInfo_B> mEquipmentInfo_B;

typedef struct _WarehouseInfo_B
{
	char id[32];
	char type[32];			//类型
 	char area[32];			//面积
	char capacity[32];		//能力
 	char businesscard[64];	//运营证号
}WarehouseInfo_B;

typedef std::map<std::string, WarehouseInfo_B> mWarehouseInfo_B;

typedef struct _StorageInfo_B
{
	char id[32];
	char type[64];			//类型
 	char area[32];			//面积
	char capacity[32];		//能力
 	char businesscard[64];	//运营证号
	char terminalid[32];
	char terminalname[64];
}StorageInfo_B;

typedef std::map<std::string, StorageInfo_B> mStorageInfo_B;


struct _CargoInfo
{
	double tw;		//吞吐量totalweight
	double cw;		//周转量 总质量*距离
	char cid[32];			//货物ID
	char billway[64];			//运单号
	int tp1;				//货物类型1      0普通货物1 集装箱2 客(滚)船
	char tp2[32];			//货物类型2
	int load;				//装卸类型 0装货 1卸货
	char own[128];			//货主
	double fr;				//运费
	int amount;             //数量
	_CargoInfo():tw(0.0),cw(0.0),tp1(0),fr(0.0),load(0)
	{

	}
};

struct _UsrTelInfo
{
      char mobile[32];
	  char usrname[64];
};
struct _AlertUserInfo
{
	char objid[32];
	char objname[64];
	int objtype;
	vector<_UsrTelInfo> vecTelUsrs;
};


struct _ThruptFlowInfo
{
	char flowid[32];		//吞吐流水ID
	char shipid[32];		//船舶id
	char shipnm[64];		//船舶名称,
	char shipchnm[64];		//船舶名称
	int mmsi;
	char regport[64];		//船籍港
	double dwt;				//载重吨位
	char op[32];			//船舶经营人
	char opcd[64];			//运营证号
	char bid[20];			//停靠泊位ID
	char bnm[100];			//泊位名称
	char hid[20];           //港区ID           
	char tid[20];           //码头ID
	int arrivetm;			//到港时间
	int leavetm;			//离港时间
	float pspan;				//在港时长
	double lpan;			//装卸时长
	int trade;				//内外贸 0内贸 1外贸
	int loadflag;           //装卸货 0装货 1卸货
	char sport[64];			//始发港
	char dport[64];			//目的港
	char opid[64];			//经办人ID
	char opnm[128];			//经办人
	vector<_CargoInfo> cargoList; //货物列表

	_ThruptFlowInfo():mmsi(0),dwt(0),arrivetm(0),pspan(0),lpan(0.0),trade(0)
	{
	}
};


#endif

#ifndef __FLEET_SVC_H__
#define __FLEET_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"
#include "ShipAis.h"

using namespace std;

typedef map<string,Tokens > TokenMap;
typedef TokenMap::iterator TokenMapIter;

struct FltVessel
{
    char vid[20];
    int mmsi;
    char name[128];
    char alias[128];
    char crdt[20];
    int status;
    int trash_flag;
    int remark_num;
    int auto_flag;
    int valid_flag;
    FltVessel():mmsi(0),trash_flag(0),remark_num(0),auto_flag(0),valid_flag(1)
    {
        vid[0]='\0';
        name[0]='\0';
        alias[0]='\0';
    }
};
struct FltCategory
{
    char catId[5];
	char parentCatId[5]; //父级目录
    char cname[128];
    char crdt[20];
    int trash_flag;
    int remark_num;
    int auto_flag;
    int email;
    int sms;
    std::string emailStr;
    std::string telnoStr;
    std::vector<FltVessel> vslList;

    FltCategory():trash_flag(0),remark_num(0),auto_flag(0),email(0),sms(0)
    {
        catId[0]='\0';
		parentCatId[0]='\0';
        cname[0]='\0';
        crdt[0]='\0';
        emailStr="";
        telnoStr="";
    }
};

struct VslSubscribe
{
    int mmsi;
    int emai;
    int sms;

    VslSubscribe():mmsi(0),emai(0),sms(0)
    {

    }
};

struct VslSubscribeNew
{
    int mmsi;
    int emai;
    int sms;
    std::string EmalStr;
    std::string TelnoStr;

    VslSubscribeNew():mmsi(0),emai(0),sms(0)
    {
        EmalStr="";
        TelnoStr="";
    }
};

struct WillArriveShip
{
    int mmsi;
    int expatedate;
    int portid;
    char shipname[128];
    char imo[20];
    int shiptype;
    WillArriveShip()
    {
        memset(this,0,sizeof(WillArriveShip));
    }
};
typedef map<int,vector<WillArriveShip> > ArriveMap;
typedef ArriveMap::iterator ArriveIter;

struct DFArea
{
    char areaid[20];
    char pointset[2048];
    char name[256];
    char userid[32];
    int areatype;
    DFArea()
    {
        memset(this,0,sizeof(*this));
    }
};
typedef map<int,vector<DFArea> > AreaMap;
typedef AreaMap::iterator AreaMapIter;

//  0-myareaid | 1-systemareaid…..
//&    portid-3-5 |portid-3-5……
//&    0-cargo,huo | 1-CHN,ENG,DGF |2-DGA,DGB
//&    0,999@100,999 @ 0,999 @ 0,999 @ 0,999 @ 0,999-00011  0表示或 1表示与

struct DynamicFleet
{
    std::string userid;
    std::string categoryid;
    bool isAreaConEmpty;
    bool isDestPortConEmpty;
	std::vector<int> vecMmsi;			// 筛选后的船舶
    Tokens WillArrive;					// 目的港筛选条件
    Tokens SecondCon;					// 第二大类筛选条件
    Tokens ThirdCon;					// 第三大类筛选条件
};
typedef map<string, DynamicFleet> MapDF;
typedef MapDF::iterator MapDFIter;

/////////////////////////// 长江动态船队计算相关 ////////////////////////////
struct TPilotShip
{
    int		mmsi;
    char	shipid[128];
    char	nameCn[128];
    int		dangerLevel;
    int		isSpec;
    int		isYH;

    TPilotShip() : dangerLevel(0), isSpec(0), isYH(0) {}

    std::string toSqlValue(const std::string& catName)
    {
        char buff[512];
        sprintf(buff, "('','%s','%s','%d','%s', CURRENT_TIMESTAMP())",shipid,catName.c_str(),mmsi,nameCn);
        return buff;
    }
};

typedef std::vector<TPilotShip>  VecPilotShip;
typedef std::map<std::string, VecPilotShip> MapPilotDynamicFleet;
// event template
typedef struct EventTmp
{
	char m_szTId[128];
	char m_szTName[128];
//	char m_nType; // 1邮件 2短信
	string m_strEmailEvent;// 
	string m_strSmsEvent;
	string m_strEmailTime;
	string m_strSmsTime;
	string m_strEmail;//
	string m_strSms;
	string m_strShips;//
	long m_lStartTime;
	long m_lEndTime;
}
ST_EVENTTMP,*P_EVENTTMP;
/////////////////////////////////////////////////////////////////////////////

class FleetSvc : IBusinessService, ACE_Event_Handler
{
public:
    FleetSvc();
    ~FleetSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
    virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(FleetSvc)

	//长江动态船队计算
	bool CalculateCjDynamicFleet();
	//动态船队：计算动态船队
	bool CalDynamicFleet();
	//动态船队：加载港区信息
	bool LoadPortArea();
	//动态船队：加载用户自定义区域信息
	bool LoadUserArea();
	//动态船队：加载动态船队
	bool LoadDynamicFleet();
	//动态船队：加载预计到港信息
	bool LoadWillArriveShip();
	//动态船队：是否为空
	bool IsDFMapEmpty() { return m_mapDF.empty(); }

private:
    int	GetDynamicFleet4CJ(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetDynamicFleetShip(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetFleetCondition(const char* pUid, const char* jsonString, std::stringstream& out);
    int SetFleetCondition(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetDynamicFleetSub(const char* pUid, const char* jsonString, std::stringstream& out);
    int DelFleetEvent(const char* pUid, const char* jsonString, std::stringstream& out);
    int MatchManualAddVsl(const char* pUid, const char* jsonString, std::stringstream& out);
    int SetSubscribeBinding(const char* pUid, const char* jsonString, std::stringstream& out);
    int SetSubscribe(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetSubscribeList(const char* pUid, const char* jsonString, std::stringstream& out);
    int FleetRemarkManager(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetRemarkList(const char* pUid, const char* jsonString, std::stringstream& out);
    int FleetVslManager(const char* pUid, const char* jsonString, std::stringstream& out);
    int FleetCategoryManager(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetFleetVslList(const char* pUid, const char* jsonString, std::stringstream& out);
	int FleetManagesWeb(const char* pUid, const char* jsonString, std::stringstream& out);
	int FleetShipMove(const char* pUid, const char* jsonString, std::stringstream& out);
	// 事件模板
	int GetAllEventTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	int AddEventTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	int DeleteEventTmp(const char* pUid, const char* jsonString, std::stringstream& out);
	int UpdateEventTmp(const char* pUid, const char* jsonString, std::stringstream& out);

private:
	int		_AddCatRemark(const char *usrid,const char *catId,const char *subject,const char *body,MySql *psql,std::stringstream& out);
    int		_AddVslRemark(const char *usrid,const char *shipid,const char *subject,const char *body,MySql *psql,std::stringstream& out);
    int		_DelCatRemark(const char *usrid,const char *catId,const char *remarkid,MySql *psql,std::stringstream& out);
    int		_DelVslRemark(const char *usrid,const char *shipid,const char *remarkid,MySql *psql,std::stringstream& out);
    int		_ModCatRemark(const char *usrid,const char *remarkid,const char *subject,const char *body,MySql *psql,std::stringstream& out);
    int		_ModVslRemark(const char *usrid,const char *remarkid,const char *subject,const char *body,MySql *psql,std::stringstream& out);
    
	int		_AddFleetVsl(const char *usrid,const char *catId,int mmsi,const char *shipid,const char *shipname,const char *alias,bool manual,MySql *psql,std::stringstream& out);
    int		_DelFleetVsl(const char *usrid,int mmsi,const char *shipid,MySql *psql,std::stringstream& out);
    int		_PurgeFleetVsl(const char *usrid,int mmsi,const char *shipid,MySql *psql,std::stringstream& out);
    int		_RecoverFleetVsl(const char *usrid,int mmsi,const char *shipid,MySql *psql,std::stringstream& out);
    int		_ModifyFleetAlias(const char *usrid,const char *shipid,const char *alias,MySql *psql,std::stringstream& out);
    int		_AutoPosFleetVsl(const char *usrid,const char *shipid,bool autoPos,MySql *psql,std::stringstream& out);

    int		_AddFleetCategory(const char *usrid,const char *catId,const char *catname,MySql *psql,std::stringstream& out);
    int		_DelFleetCategory(const char *usrid,const char *catId,MySql *psql,std::stringstream& out);
    int		_PurgeFleetCategory(const char *usrid,const char *catId,MySql *psql,std::stringstream& out);
    int		_RecoverFleetCategory(const char *usrid,const char *catId,MySql *psql,std::stringstream& out);
    int		_ModifyFleetCategory(const char *usrid,const char *catId,const char *catname,MySql *psql,std::stringstream& out);
    int		_AutoPosFleetCategory(const char *usrid,const char *catId,bool autoPos,MySql *psql,std::stringstream& out);

	int		_AddFleetWeb(const char *usrid, int mmsi, const char *shipid, const char *shipname, const char *alias, MySql *psql, std::stringstream& out);
	int		_GetFleetStatisticWeb(const char *shipid, MySql *psql, std::stringstream& out);
	int		_ConfirmFleet(const char *uid, const char *shipid, MySql *psql, std::stringstream& out);
	
	void	clearCjDynamicFleet();
	Tokens	getPilotCategory(TPilotShip& pilot);
	void	fillregion(IntSet& regionset );
	void	parseAreaCondition(string & condition,DynamicFleet& df);
	int		getEventBinaryFlag(string eventid);
	void	saveToMysql();

	void	calDynamicFleet2();
	void	calDynamicFleet3();

private:
    bool subscribeEvent(MySql* psql,const char* channel_type,const char* objectID,const Tokens& mmsiList,const char* lang);
    bool updateEvent(MySql* psql,const char* channel_type,const char* objectID,const vector<int>& eventList,const char* lang);
    bool unsubscribeEvent(MySql* psql,const char* channel_type,const char* objectId);
    bool unsubscribeEvent(MySql* psql,const char* channel_type,const char* objectId,const char* mmsi);

private:
    StrStrMap							m_mapPilotCategory;
    MapPilotDynamicFleet				m_mapPilotDynamicFleet;

    AreaMap								m_mapPortArea;			//regionid to area
    AreaMap								m_mapUserArea;			//regionid to area
    MapDF								m_mapDF;				//key是userid+空格+categoryid
    TokenMap							m_mapPortid2DF;			//key是portid  value是userid+空格+categoryid
    TokenMap							m_mapUserid2DF;			//key是userid +空格+自定义区域id  value是userid+空格+categoryid
	ArriveMap							m_mapWillArrive;

    ACE_Thread_Mutex					willarrive_cs;
	//add  Mutex  test
	ACE_Thread_Mutex					willarrive_cs_tmp;

	long	m_lastCjDynamic;
	long	m_lastWillArrive;
};

typedef ACE_Singleton<FleetSvc, ACE_Null_Mutex> g_FleetSvc;

#endif

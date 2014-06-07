#ifndef __DISPATCH_SVC_H__
#define __DISPATCH_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include "Util.h"
#include "Geom.h"

const double WUHAN_LON = 114.3011;
const double NANJING_LON = 118.8579;
const double JIANGYIN_LON = 120.0511;
const double JIANGYIN_LAT = 31.9531;
const double BAOSHAN_LON = 121.4864;
const double BAOSHAN_LAT = 31.4386;
const double FLOAT1_LON = 121.3342;
const double FLOAT1_LAT = 31.55;
const double FLOAT1_CJK_DIST = 60; //nm
extern POLYGON g_CJPolygon;

struct TDispLog
{
    char	m_strOperator[128];					//操作用户ID
    char	m_strOperateDt[128];				//操作时间
    char	m_strOperateContent[2048];			//内容

    TDispLog()
    {
        memset(this, 0, sizeof(TDispLog));
    }

    std::string toJson();
};

struct TDispRmk
{
    char	m_strRmkID[128];					//备注ID
    char	m_strVoyID[128];					//引航申请ID
    char	m_strUserID[128];					//用户ID
    char	m_strTitle[128];					//标题
    char	m_strContent[2048];					//内容
    long	m_lUpdTime;							//更新时间

    TDispRmk()
    {
        memset(this, 0, sizeof(TDispRmk));
    }

    std::string toJson();
};

struct TDetailCatContact
{
    int     m_nContactCatID;		//联系人分类ID
    char	m_strContactCatID[64];	//联系人分类
    char	m_strContactUserID[64];	//联系人ID
    char    m_strContactName[64];	//联系人名
    char	m_strContactTel[64];	//联系人电话
    char	m_strContactEmail[64];	//联系人email

    TDetailCatContact()
    {
        memset(this, 0, sizeof(TDetailCatContact));
    }

    std::string toJson()
    {
        char buff[1024];
        sprintf(buff, "[\"%d\",\"%s\",\"%s\",\"%s\",\"%s\"]", m_nContactCatID, m_strContactUserID, m_strContactName, m_strContactTel, m_strContactEmail);
        return std::string(buff);
    }
};

struct CjPilotInfo
{
    char m_szVoyageID[64];				//引航申请ID

    char m_szApplyCmpID[64];			//代理公司ID
    char m_szApplyCmpName[64];			//代理公司名
    char m_szApplyUser[64];				//申请人
    char m_szApplyName[64];				//申请人名字

    int	 m_nDeptPortID;					//起始港口ID
    double m_dbDeptLon;					//起始港口经度
    double m_dbDeptLat;					//起始港口纬度
    char m_szDeptStationID[64];			//起始港口所属引航站ID
    char m_szDeptPilotID[64];			//起始引航点ID
    char m_szDeptPilotName[64];			//起始引航点名称
    int  m_nDestPortID;					//目的港口ID
    double m_dbDestLon;					//目的港口经度
    double m_dbDestLat;					//目的港口纬度
    char m_szDestStationID[64];			//目的港口所属引航站ID
    char m_szDestPilotID[64];			//目的引航点ID
    char m_szDestPilotName[64];			//目的引航点名称
    char m_szStationID[64];				//处理该引航申请的引航站ID
    char m_szVoyageData[5*1024];		//航线数据

    int  m_nMiles;						//总里程
    int  m_nPhase;						//分段
    int	 m_nPilotType;					//进江出江移泊
    int  m_nVoyType;					//外贸内贸

    char m_szCargoName[64];				//货物名称
    int  m_nCargoQuantity;				//货物重量
    int	 m_nDangerLevel;				//危险品级别
    int  m_nIsSpec;						//是否特种船

    char m_szShipID[64];				//船舶ID
    char m_szMmsi[64];					//船舶mmsi
    char m_szNameEn[64];				//船舶英文名
    char m_szNameCn[64];				//船舶中文名
    char m_szImo[64];					//船舶IMO
    char m_szCallsign[64];				//船舶呼号
    char m_szFlag[64];					//船旗
    double m_dbSpd;						//船速
    double  m_dbLength;					//长
    double  m_dbWidth;					//宽
    double  m_dbDepth;					//深
    double	m_dbHeight;					//水面高度
    double  m_dbDraughtFor;				//前吃水
    double  m_dbDraughAfter;			//后吃水
    int  m_nAge;						//船龄
    int  m_nGrossTon;					//总吨
    int  m_nNetTon;						//净吨
    int  m_nDWT;						//载重吨
    int  m_nLoadedDisplament;			//排水吨
    char m_szOwner[64];					//船东
    char m_szManager[64];				//运营商
    char m_szBuilder[64];				//建造商
    TDetailCatContact m_VslContacts[4]; //联系人

    int		m_nAisStatus;				//当前船舶状态
    double	m_dbAisSpd;					//当前船速
    double  m_dbAisLon;					//当前经度
    double	m_dbAisLat;					//当前纬度
    long	m_lAisTime;					//当前船舶信号时间

    bool	m_bAlert;					//是否预警过

    long m_lApply;						//申请时间
    long m_lAPDT;						//申请开航时间
    long m_lETD;						//计划开航时间
    long m_lATD;						//实际开航时间
    long m_lETA;						//计划到达时间
    long m_lATA;						//实际到达时间
    long m_lEDC;						//预计抵澄时间
    long m_lADC;						//实际抵澄时间
    int  m_nJT;							//交通船时间

    int  m_nStatus;						//当前状态

    //分段引航相关(m_nPhase!=1)
    StrSet pilotorList1;					//第一段派遣的引航员ID
    StrSet pilotorList2;					//第一段派遣的引航员ID
    int  m_nMiles1;							//第一段里程
    int  m_nMiles2;							//第二段里程

    char	m_strAccepterID[64];			//受理人ID
    long    m_lAcceptTime;					//受理时间
    int     m_nAgree;						//0代表未受理 1代表同意 2代表不同意
    char    m_strAgreement[2048];			//受理意见
    char    m_strSummary[2048];				//总结
    int     m_nAudit;						//当为三超为夜航船舶时才以下面的审批
    char    m_strID_zhan[128];				//站领导ID
    long    m_lTime_zhan;					//站领导审批时间
    int		m_nAudit_zhan;					//站领导是否同意
    char    m_strAudit_zhan[2048];			//审批意见
    char    m_strID_anjibu[128];			//安技部ID
    long    m_lTime_anjibu;					//安技部审批时间
    int		m_nAudit_anjibu;				//安技部是否同意
    char    m_strAudit_anjibu[2048];		//审批意见
    char    m_strID_zongdiaoshi[128];		//总调室ID
    long    m_lTime_zongdiaoshi;			//总调室审批时间
    int		m_nAudit_zongdiaoshi;			//总调室是否同意
    char    m_strAudit_zongdiaoshi[2048];	//审批意见

    CjPilotInfo()
    {
        m_szApplyCmpName[0] = '\0';
        m_szApplyName[0]	= '\0';
        m_szVoyageData[0]	= '\0';

        m_szDeptPilotName[0] = '\0';
        m_szDestPilotName[0] = '\0';

        m_nMiles1 = m_nMiles2 = 0;

        m_nAisStatus = -1;
        m_bAlert = false;
    }

    std::string toPilotApplyJson();
    std::string toDetailPilotApplyJson();
    std::string toPilotPlanJson();
    std::string toPilotManVslJson();
};

typedef std::vector<CjPilotInfo*> VecPilot;
typedef std::vector<CjPilotInfo> VecPilot2;
typedef std::map<std::string, CjPilotInfo> MapPilot;
typedef std::map<int, CjPilotInfo*> MapMmsiPilot;

void MapPilotToMmsi(MapPilot& mapPilots, MapMmsiPilot& mapMmsis);

struct CjUserPilot
{
    char	m_szUserID[64];				//用户ID
    char	m_szName[64];				//用户名
    char	m_szPilotLevel[2];			//引航员级别
    int		m_nPilotLevel;				//引航员级别(整数)

    char	m_szVoyageLine[20];			//证书航线
    char	m_szTel[64];				//电话
    char	m_szEmail[64];				//email

    int		m_nPosition;				//职位
    char	m_szGrade[3];				//档次(引航员级别进一步细化)

    int		m_nAuth;					//是否有审批权限
    char	m_szPilotID[64];			//所属引航站ID
    char	m_szOrgID[64];				//所属机构ID
    int		m_nOrgType;					//所属机构类型

    int		m_nStatus;					//当前状态
    int     m_nShiRen;					//被调派的适任状态 1:适任 2:配班 3:实习
    char	m_szVoyageID[64];			//被调派的引航ID
    long	m_lAvaibleTime;				//可用时间

    CjUserPilot()
    {
        memset(this, 0, sizeof(CjUserPilot));
        m_nShiRen = 1;
    }

    std::string toJson(int pilotLevel=-1);
};
typedef std::vector<CjUserPilot*> VecUserPilot;
typedef std::map<std::string, CjUserPilot*> MapUserPilot;

struct CjPortPilot
{
    int		m_nPortID;					//港口ID
    char    m_szPilotID[64];			//港口所属引航站ID

    CjPortPilot()
    {
        memset(this, 0, sizeof(CjPortPilot));
    }
};
typedef std::map<int, CjPortPilot*> MapPortPilot;

struct UserStatus
{
    std::string userid;
    int			status;
    long	    avaible;

    UserStatus(CjUserPilot* pUser) : userid(pUser->m_szUserID), status(pUser->m_nStatus), avaible(pUser->m_lAvaibleTime) {}
};
typedef std::vector<UserStatus> VecUserStatus;

class DispatchSvc : IBusinessService, ACE_Event_Handler
{
public:
    DispatchSvc();
    ~DispatchSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
    virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    int			 GetVslDistETA(CjPilotInfo* pInfo, long& t1, long& t2, long& t3);
    void		 GetVslETA1(CjPilotInfo* pInfo, long& t1, long tm, double lon, double lat, double spd);
    CjPilotInfo* GetPilotInfo(const std::string& vid);
    CjUserPilot* GetUserInfo(const std::string& uid);

    ACE_Recursive_Thread_Mutex& GetThreadLock()
    {
        return m_ThreadLock;
    }
    MapPilot& GetPilots()
    {
        return m_mapPilot;
    }
    MapUserPilot& GetUsers()
    {
        return m_mapUserPilot;
    }

    bool makePilotStart(CjPilotInfo* pVoy, MySql* psql);
    bool makePilotEnd(CjPilotInfo* pVoy, MySql* psql);
    bool makePilotExpired(CjPilotInfo* pVoy, MySql* psql);

    DECLARE_SERVICE_MAP(DispatchSvc)

private:
    int getPilotApply(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotApplyDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotMan(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotVoy(const char* pUid, const char* jsonString, std::stringstream& out);
    int chgPilotStatus(const char* pUid, const char* jsonString, std::stringstream& out);
    int chgPilotSL(const char* pUid, const char* jsonString, std::stringstream& out);
    int chgPilotZJ(const char* pUid, const char* jsonString, std::stringstream& out);
    int chgPilotSP(const char* pUid, const char* jsonString, std::stringstream& out);
    int chgPilotStartTime(const char* pUid, const char* jsonString, std::stringstream& out);
    int dispatchPilotMan(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotManVsl(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotVslDist(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotManStation(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotPlan(const char* pUid, const char* jsonString, std::stringstream& out);
    int delPilotMan(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotLog(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotRmk(const char* pUid, const char* jsonString, std::stringstream& out);
    int updPilotRmk(const char* pUid, const char* jsonString, std::stringstream& out);
    int delPilotRmk(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotedMan(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotManContact(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPilotStatus(const char* pUid, const char* jsonString, std::stringstream& out);

private:
    bool loadUserData(MySql* psql);
    bool loadPilotApplyData(MySql* psql);

    bool makePilotCancel(CjPilotInfo* pVoy, MySql* psql, bool bReject=false, const std::string& rmk="", CjUserPilot* pUser=NULL);
    void updatePilotorStatus(VecUserStatus& statusList, MySql* psql);

    void parsePilots(const char* pilots, CjPilotInfo* pVoy, int nSeq, int nType);
    void parsePilots(const char* pilots, VecUserPilot& pilotList, std::vector<int>& levelList, int nShiRen);
    static int getShiRen(CjUserPilot* pUser, CjPilotInfo* pVoy);
    static int getLonPosition(double lon);

private:
    ACE_Recursive_Thread_Mutex   m_ThreadLock;      //线程锁

    char			m_szMaxVoyageID[64];
    long			m_lLastUpdTime;
    MapPilot		m_mapPilot;
    MapUserPilot	m_mapUserPilot;
    MapPortPilot	m_mapPortPilot;
};

typedef ACE_Singleton<DispatchSvc, ACE_Null_Mutex> g_DispatchSvc;

#endif

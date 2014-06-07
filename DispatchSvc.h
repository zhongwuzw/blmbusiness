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
    char	m_strOperator[128];					//�����û�ID
    char	m_strOperateDt[128];				//����ʱ��
    char	m_strOperateContent[2048];			//����

    TDispLog()
    {
        memset(this, 0, sizeof(TDispLog));
    }

    std::string toJson();
};

struct TDispRmk
{
    char	m_strRmkID[128];					//��עID
    char	m_strVoyID[128];					//��������ID
    char	m_strUserID[128];					//�û�ID
    char	m_strTitle[128];					//����
    char	m_strContent[2048];					//����
    long	m_lUpdTime;							//����ʱ��

    TDispRmk()
    {
        memset(this, 0, sizeof(TDispRmk));
    }

    std::string toJson();
};

struct TDetailCatContact
{
    int     m_nContactCatID;		//��ϵ�˷���ID
    char	m_strContactCatID[64];	//��ϵ�˷���
    char	m_strContactUserID[64];	//��ϵ��ID
    char    m_strContactName[64];	//��ϵ����
    char	m_strContactTel[64];	//��ϵ�˵绰
    char	m_strContactEmail[64];	//��ϵ��email

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
    char m_szVoyageID[64];				//��������ID

    char m_szApplyCmpID[64];			//����˾ID
    char m_szApplyCmpName[64];			//����˾��
    char m_szApplyUser[64];				//������
    char m_szApplyName[64];				//����������

    int	 m_nDeptPortID;					//��ʼ�ۿ�ID
    double m_dbDeptLon;					//��ʼ�ۿھ���
    double m_dbDeptLat;					//��ʼ�ۿ�γ��
    char m_szDeptStationID[64];			//��ʼ�ۿ���������վID
    char m_szDeptPilotID[64];			//��ʼ������ID
    char m_szDeptPilotName[64];			//��ʼ����������
    int  m_nDestPortID;					//Ŀ�ĸۿ�ID
    double m_dbDestLon;					//Ŀ�ĸۿھ���
    double m_dbDestLat;					//Ŀ�ĸۿ�γ��
    char m_szDestStationID[64];			//Ŀ�ĸۿ���������վID
    char m_szDestPilotID[64];			//Ŀ��������ID
    char m_szDestPilotName[64];			//Ŀ������������
    char m_szStationID[64];				//������������������վID
    char m_szVoyageData[5*1024];		//��������

    int  m_nMiles;						//�����
    int  m_nPhase;						//�ֶ�
    int	 m_nPilotType;					//���������Ʋ�
    int  m_nVoyType;					//��ó��ó

    char m_szCargoName[64];				//��������
    int  m_nCargoQuantity;				//��������
    int	 m_nDangerLevel;				//Σ��Ʒ����
    int  m_nIsSpec;						//�Ƿ����ִ�

    char m_szShipID[64];				//����ID
    char m_szMmsi[64];					//����mmsi
    char m_szNameEn[64];				//����Ӣ����
    char m_szNameCn[64];				//����������
    char m_szImo[64];					//����IMO
    char m_szCallsign[64];				//��������
    char m_szFlag[64];					//����
    double m_dbSpd;						//����
    double  m_dbLength;					//��
    double  m_dbWidth;					//��
    double  m_dbDepth;					//��
    double	m_dbHeight;					//ˮ��߶�
    double  m_dbDraughtFor;				//ǰ��ˮ
    double  m_dbDraughAfter;			//���ˮ
    int  m_nAge;						//����
    int  m_nGrossTon;					//�ܶ�
    int  m_nNetTon;						//����
    int  m_nDWT;						//���ض�
    int  m_nLoadedDisplament;			//��ˮ��
    char m_szOwner[64];					//����
    char m_szManager[64];				//��Ӫ��
    char m_szBuilder[64];				//������
    TDetailCatContact m_VslContacts[4]; //��ϵ��

    int		m_nAisStatus;				//��ǰ����״̬
    double	m_dbAisSpd;					//��ǰ����
    double  m_dbAisLon;					//��ǰ����
    double	m_dbAisLat;					//��ǰγ��
    long	m_lAisTime;					//��ǰ�����ź�ʱ��

    bool	m_bAlert;					//�Ƿ�Ԥ����

    long m_lApply;						//����ʱ��
    long m_lAPDT;						//���뿪��ʱ��
    long m_lETD;						//�ƻ�����ʱ��
    long m_lATD;						//ʵ�ʿ���ʱ��
    long m_lETA;						//�ƻ�����ʱ��
    long m_lATA;						//ʵ�ʵ���ʱ��
    long m_lEDC;						//Ԥ�Ƶֳ�ʱ��
    long m_lADC;						//ʵ�ʵֳ�ʱ��
    int  m_nJT;							//��ͨ��ʱ��

    int  m_nStatus;						//��ǰ״̬

    //�ֶ��������(m_nPhase!=1)
    StrSet pilotorList1;					//��һ����ǲ������ԱID
    StrSet pilotorList2;					//��һ����ǲ������ԱID
    int  m_nMiles1;							//��һ�����
    int  m_nMiles2;							//�ڶ������

    char	m_strAccepterID[64];			//������ID
    long    m_lAcceptTime;					//����ʱ��
    int     m_nAgree;						//0����δ���� 1����ͬ�� 2����ͬ��
    char    m_strAgreement[2048];			//�������
    char    m_strSummary[2048];				//�ܽ�
    int     m_nAudit;						//��Ϊ����Ϊҹ������ʱ�������������
    char    m_strID_zhan[128];				//վ�쵼ID
    long    m_lTime_zhan;					//վ�쵼����ʱ��
    int		m_nAudit_zhan;					//վ�쵼�Ƿ�ͬ��
    char    m_strAudit_zhan[2048];			//�������
    char    m_strID_anjibu[128];			//������ID
    long    m_lTime_anjibu;					//����������ʱ��
    int		m_nAudit_anjibu;				//�������Ƿ�ͬ��
    char    m_strAudit_anjibu[2048];		//�������
    char    m_strID_zongdiaoshi[128];		//�ܵ���ID
    long    m_lTime_zongdiaoshi;			//�ܵ�������ʱ��
    int		m_nAudit_zongdiaoshi;			//�ܵ����Ƿ�ͬ��
    char    m_strAudit_zongdiaoshi[2048];	//�������

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
    char	m_szUserID[64];				//�û�ID
    char	m_szName[64];				//�û���
    char	m_szPilotLevel[2];			//����Ա����
    int		m_nPilotLevel;				//����Ա����(����)

    char	m_szVoyageLine[20];			//֤�麽��
    char	m_szTel[64];				//�绰
    char	m_szEmail[64];				//email

    int		m_nPosition;				//ְλ
    char	m_szGrade[3];				//����(����Ա�����һ��ϸ��)

    int		m_nAuth;					//�Ƿ�������Ȩ��
    char	m_szPilotID[64];			//��������վID
    char	m_szOrgID[64];				//��������ID
    int		m_nOrgType;					//������������

    int		m_nStatus;					//��ǰ״̬
    int     m_nShiRen;					//�����ɵ�����״̬ 1:���� 2:��� 3:ʵϰ
    char	m_szVoyageID[64];			//�����ɵ�����ID
    long	m_lAvaibleTime;				//����ʱ��

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
    int		m_nPortID;					//�ۿ�ID
    char    m_szPilotID[64];			//�ۿ���������վID

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
    ACE_Recursive_Thread_Mutex   m_ThreadLock;      //�߳���

    char			m_szMaxVoyageID[64];
    long			m_lLastUpdTime;
    MapPilot		m_mapPilot;
    MapUserPilot	m_mapUserPilot;
    MapPortPilot	m_mapPortPilot;
};

typedef ACE_Singleton<DispatchSvc, ACE_Null_Mutex> g_DispatchSvc;

#endif

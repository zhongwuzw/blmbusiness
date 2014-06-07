#ifndef __ACCOUNT_SVC_H__
#define __ACCOUNT_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include <vector>

using namespace std;

class AccountSvc : IBusinessService, ACE_Event_Handler
{
public:
    AccountSvc();
    ~AccountSvc();

    virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(AccountSvc)

private:
    int getSummary(const char* pUid, const char* jsonString, std::stringstream& out);
    int getResume(const char* pUid, const char* jsonString, std::stringstream& out);
    int updResume(const char* pUid, const char* jsonString, std::stringstream& out);
    int getVerifyCode(const char* pUid, const char* jsonString, std::stringstream& out);
    int verifyCode(const char* pUid, const char* jsonString, std::stringstream& out);
    int chgPassword(const char* pUid, const char* jsonString, std::stringstream& out);
    int getCompany(const char* pUid, const char* jsonString, std::stringstream& out);
    int updCompany(const char* pUid, const char* jsonString, std::stringstream& out);
    int updCompanyDept(const char* pUid, const char* jsonString, std::stringstream& out);
    int delCompanyDept(const char* pUid, const char* jsonString, std::stringstream& out);
    int chgCompanyDept(const char* pUid, const char* jsonString, std::stringstream& out);
    int uplAuth(const char* pUid, const char* jsonString, std::stringstream& out);
    int getAuth(const char* pUid, const char* jsonString, std::stringstream& out);
    int uplCompanyAuth(const char* pUid, const char* jsonString, std::stringstream& out);
    int uplCompanyLogo(const char* pUid, const char* jsonString, std::stringstream& out);
    int getCompanyAuth(const char* pUid, const char* jsonString, std::stringstream& out);
    int getCompanyLogo(const char* pUid, const char* jsonString, std::stringstream& out);
    int getCompanySummary(const char* pUid, const char* jsonString, std::stringstream& out);
    int updCompanySummary(const char* pUid, const char* jsonString, std::stringstream& out);
    int getCompanyPersonal(const char* pUid, const char* jsonString, std::stringstream& out);
    int getCompanyCowoker(const char* pUid, const char* jsonString, std::stringstream& out);
    int inviteCoworker(const char* pUid, const char* jsonString, std::stringstream& out);
    int apply(const char* pUid, const char* jsonString, std::stringstream& out);
    int getAllSendInvite(const char* pUid, const char* jsonString, std::stringstream& out);
    int getAllUnDeal(const char* pUid, const char* jsonString, std::stringstream& out);
    int getAllRecvInvite(const char* pUid, const char* jsonString, std::stringstream& out);
    int dealInvite(const char* pUid, const char* jsonString, std::stringstream& out);
    int acceptApply(const char* pUid, const char* jsonString, std::stringstream& out);
    int quitCompany(const char* pUid, const char* jsonString, std::stringstream& out);
    int setAdmin(const char* pUid, const char* jsonString, std::stringstream& out);
    int kickAss(const char* pUid, const char* jsonString, std::stringstream& out);
    int getVessel(const char* pUid, const char* jsonString, std::stringstream& out);
    int updVessel(const char* pUid, const char* jsonString, std::stringstream& out);
    int delVessel(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPort(const char* pUid, const char* jsonString, std::stringstream& out);
    int updPort(const char* pUid, const char* jsonString, std::stringstream& out);
    int delPort(const char* pUid, const char* jsonString, std::stringstream& out);
    int getProduct(const char* pUid, const char* jsonString, std::stringstream& out);
    int getJifen(const char* pUid, const char* jsonString, std::stringstream& out);
    int updOrder(const char* pUid, const char* jsonString, std::stringstream& out);
    int allProduct(const char* pUid, const char* jsonString, std::stringstream& out);
    int productDesc(const char* pUid, const char* jsonString, std::stringstream& out);
    int getUserOrders(const char* pUid, const char* jsonString, std::stringstream& out);
    int updateProductParam(const char* pUid, const char* jsonString, std::stringstream& out);
    int getAllUserAuthType(const char* pUid, const char* jsonString, std::stringstream& out);
    int applyVerifyPersonal(const char* pUid, const char* jsonString, std::stringstream& out);
    int dealPersonalApply(const char* pUid, const char* jsonString, std::stringstream& out);
    int getPersonalDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int applyVerifyCompany(const char* pUid, const char* jsonString, std::stringstream& out);
    int dealCompanyApply(const char* pUid, const char* jsonString, std::stringstream& out);
    int getCompanyDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int interMember(const char* pUid, const char* jsonString, std::stringstream& out);
	int sendInvoice(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetOrderList(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetDetailOfOrder(const char* pUid, const char* jsonString, std::stringstream& out);
	int DealAuthority(const char* pUid, const char* jsonString, std::stringstream& out);
	int getUserId(const char* pUid, const char* jsonString, std::stringstream& out);
	int getTelBal(const char* pUid, const char* jsonString, std::stringstream& out);
	int getUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getUserIdFromThirdId(const char* pUid, const char* jsonString, std::stringstream& out);
	int updThirdId(const char* pUid, const char* jsonString, std::stringstream& out);
	int getExchange(const char* pUid, const char* jsonString, std::stringstream& out);
	int getConnectCharge(const char* pUid, const char* jsonString, std::stringstream& out);
	int confirmUserPasswd(const char* pUid, const char* jsonString, std::stringstream& out);
	int logout(const char* pUid, const char* jsonString, std::stringstream& out);
	int autoRegister(const char* pUid, const char* jsonString, std::stringstream& out);	
	int getDepNum(const char* pUid, const char* jsonString, std::stringstream& out);
	int getDepDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	int queUserService(const char* pUid, const char* jsonString, std::stringstream& out);
	int getPortList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getShipList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getPortBrief(const char* pUid, const char* jsonString, std::stringstream& out);
	int getServiceName(const char* pUid, const char* jsonString, std::stringstream& out);
	int updPhoneUserPasswd(const char* pUid, const char* jsonString, std::stringstream& out);
	int iPhoneRegister(const char* pUid, const char* jsonString, std::stringstream& out);
	int insertIOSDevice(const char* pUid, const char* jsonString, std::stringstream& out);
	int updIOSUser(const char* pUid, const char* jsonString, std::stringstream& out);
	int registerStraight(const char* pUid, const char* jsonString, std::stringstream& out);
	int relateIphonePc(const char* pUid, const char* jsonString, std::stringstream& out);
	int getThirdPassword(const char* pUid, const char* jsonString, std::stringstream& out);
};

typedef ACE_Singleton<AccountSvc, ACE_Null_Mutex> g_AccountSvc;


//�û�����(����)���빫˾��״̬
enum USER_REQCMP_STATUS
{
    REQUEST_WAIT,REQUEST_REJECT,REQUEST_ACCEPT,REQUEST_TIMEOUT
};
//�������룩�ȴ���,���뱻�ܾ�,�����ѽ���

typedef struct _t_account_base
{
    int m_nPoints;						//����
    
    int m_bPersonAuth;					//�����Ƿ񾭹���֤
    int m_bCmpAuth;						//��˾�Ƿ񾭹���֤
    int m_bAdmin;						//�Ƿ�Ϊ��˾����Ա
    int m_nAdminNum;					//��˾����Ա����    
    int m_nBeiYaoqing;					//������ĸ���
    int m_nYaoqing;						//����ĸ���
    int m_nQingqiu;						//�ж��ٸ������ǹ���Ա����Ч��    
    int m_lResumeDt;					//��������޸�����    
    int m_nRequest;						//�û�������빫˾��״̬
    int m_nRemindProdunit;
    int email_vd;
    int mobile_vd;
	double m_nPercent;					//������Ϣ������
	char m_strCompanyID[64];			//������˾ID����û��������˾����Ϊ�մ�
    char m_strCmpName[100];				//������˾������ʾ�������ƣ���Ϊ�գ�����ʾӢ������
    char m_strCmpEngName[100];			//��˾Ӣ����

	//�ⲿ�������ݶ�Ϊ0�Ϳմ��������֮ǰ��ʼ��
	
	int m_nBuyProductType;				//�ѹ����Ʒ����
	int m_nRemindProductNum;			//���Ѳ�Ʒ����
	char m_strResumeID[64];				//����ID	
    char m_strRemindProductID[64];		//���Ѳ�ƷID���������ѣ���Ϊ�մ�
    char m_strRemindProductName[256];	//���Ѳ�ƷName
} TAccountBaseInfo;

//��˾��֤��Ϣ
enum CMP_AUTHTYPE {UNSUBMITED,UNAUDITED/*����֤*/,AUTHED/*ͨ����֤*/,NOTAUTHED/*δͨ��*/};
//��˾������Ϣ
typedef struct _t_account_detail_departinfo
{

    int m_strDepartID;
    int m_nSeq; //ȥ��������seq��
    string m_strName;
    string m_strNameLocation;
    string m_strRemark;
    string m_strRemarkLocation;
} TAccountDetailDepartInfo;
//��˾�����Ϣ
typedef struct _t_cmp_introductioninfo
{
    Tokens m_vecServerType; //��������ID
    Tokens m_vecServerPort; //����ĸۿ�
    string m_strKeywords;			//�ؼ���
    string m_strAdvantageInfo;		//���Ƽ��
    string m_strBirefInfo;			//��˾���
} TCmpIntroduction;

//��˾��Ϣ
typedef struct _t_account_detail_cmpinfo
{
    string m_strCmpID;
    string m_strLogoID;
    string m_strName;
    string m_strLocalName;
    string m_strCountry;
    string m_strCity;
    string m_strTels;
    string m_strFaxs;
    string m_strEmail;
    string m_strWebsite;
    string m_strAddress;
    string m_strCountryLocation;
    string m_strCityLocation;
    string m_strAddressLocation;
    CMP_AUTHTYPE m_nCmpAuthType;

    long m_strAuthTime;//�����ֶ�

    vector< TAccountDetailDepartInfo > m_vecDepart;
    int m_nIntroPercent; //��˾���������
    TCmpIntroduction m_cmpIntroduction;//��˾���
} TAccountDetailCmpInfo;

typedef struct _t_account_detail_cmp_dep_user//��˾�����û���Ϣ
{
    string m_strUserID;
    string m_strCmpID;
    string m_strDepID;
    string m_strPos;
    string m_strTelFax;
    string m_strHeadOffice;
    string m_strTollFree;
    string m_strEmCall;
    string m_strFlag;
    string m_strIsAdmin;
    string m_strUserType;
    string m_strApplyType;
    string m_strInviter;
    string m_strVerifier;
    string m_strUpd_Dt;
} TAccountDetailCmpDepUser;

//��˾ͬ����Ϣ
typedef struct _CompanyCoWorker
{
    char firstna[20];
    char lastna[20];
    char nickna[20];
    char uid[20];
    int badmin;
    char departid[20];
    char departna[20];
    char tel[20];
    char email[64];

    std::string toJson1()
    {
        char buff[1024];
        sprintf(buff, "{firstna:\"%s\",lastna:\"%s\",nick:\"%s\",uid:\"%s\",badmin:%d,departid:\"%s\",departna:\"%s\",tel:\"%s\",email:\"%s\"}",
                firstna, lastna, nickna, uid, badmin, departid, departna, tel, email);
        return std::string(buff);
    }

    std::string toJson2()
    {
        char buff[1024];
        sprintf(buff, ",{firstna:\"%s\",lastna:\"%s\",nick:\"%s\",uid:\"%s\",badmin:%d,departid:\"%s\",departna:\"%s\",tel:\"%s\",email:\"%s\"}",
                firstna, lastna, nickna, uid, badmin, departid, departna, tel, email);
        return std::string(buff);
    }
} CompanyCoWorker;

//������Ϣ
typedef struct _SendInvite
{
    char uid[20];
    char nst[10];
} SendInvite;

typedef struct _RecvInvite
{
    char inviter[20];
    char companyID[64];
    char companyName[64];
} RecvInvite;

typedef struct _COMPINFOSIMPLE_
{
    string id;
    string name;
    string engname;
    bool   CmpAuth;
    bool   bAdmin;
    int    reqStatus;
    bool   Apply;
    _COMPINFOSIMPLE_()
    {
        id = "";
        name = "";
        engname = "";
        CmpAuth = false;
        bAdmin = false;
        reqStatus = 0;
        Apply  = false;
    }
} COMPINFOSIMPLE;

typedef vector<COMPINFOSIMPLE> COMP_FLAG;

typedef struct _PRODUCTINFO
{
    int status; // buy status
    int num;
    int buydt;
    int validdt;
    int invaliddt;
    double total_price;
    double price;
    char name[256];
    Tokens para;
} PRODUCTINFO;

typedef vector<PRODUCTINFO> ProductInfoVEC;

typedef struct _JIFEN
{
    int num;
    int change_dt;
    int expire_dt;
    char src[256];
} JIFEN;

typedef vector<JIFEN> JiFenVEC;

typedef struct _PRODUCTNAME
{
    char id[32];
    char name_en[256];
    char name_cn[256];
} PRODUCTNAME;

typedef vector<PRODUCTNAME> ProductNameVEC;

typedef struct _MY_ORDER_INFO
{

    int status;
    int order_time;
    int pay_time;
    int expire_time;
    double price;
    double amount;
    char currency[5];
    char order_id[64];
    char pro_name[256];
} MYORDERINFO;

typedef vector<MYORDERINFO> orderInfoVEC;

typedef struct _VERIFYPERSONALINFO
{
    int seq;
    int status;
    int update_dt;
    int review_dt;
    char uid[32];
    char nickname[255];
    char reviewer[32];
    char cause[128];
} VERIFYPERSONALINFO;

typedef vector<VERIFYPERSONALINFO> VerifyPInfoVEC;

typedef struct _COMPANYSEARCHINFO
{
    int seq;
    int appdt;
    int rdt;
    int ntp;
    char id[32];
    char name[100];
    char appid[32];
    char cause[128];
    char reviewer[32];
    char addr[256];
    char web[256];
} COMPANYSEARCHINFO;

typedef vector<COMPANYSEARCHINFO> CSInfoVEC;


typedef struct _ORDERINFO
{
	char ordid[64];
	int paydt;			//
	char payid[64];
	int status;
	char uids[1024 * 3];
	int ktsts;			//
	double amout;
	int expiredt;		//����
	int days;			//����
	int invsts;
	char operid[64];	//����
}OrderInfo;

typedef vector<OrderInfo> OrderInfos;

typedef struct _ConnectCharge
{
	double connectcharge;
	char dialpre[20]; 
}ConnectCharge;
typedef vector<ConnectCharge> ConnectCharges;

typedef struct _UserPortInfo
{
	int portid;
}UserPortInfo;
typedef vector<UserPortInfo> UserPortInfoList;

typedef struct _UserShipInfo
{
	char shipid[20];
}UserShipInfo;
typedef vector<UserShipInfo> UserShipInfoList;

#define USER_VERIFY_DATA_TYPE			0
#define USER_VERIFY_STATUS				0
#define COMP_VERIFY_STATUS				0
#define COMP_DEFINE_DATATYPE			-1
#define USER_DEFINE_DATATYPE			-1
#define COMPANY_REMARK_SIZE 6*1024
#define CMPLEN							32
#define APPLY_TITLE_CN					"������빫˾"
#define APPLY_TITLE_EN					"Apply to join company"
#define EMAIL_CONTAINT_CN				"%s ������빫˾ %s"
#define EMAIL_CONTAINT_EN				"%s apply to join into company %s"
#define SERVNUMBER                      4   //�������ĸ���

typedef vector<_CompanyCoWorker> CWVec;
typedef vector<_SendInvite> SIVec;
typedef vector<_RecvInvite> RIVec;
#endif

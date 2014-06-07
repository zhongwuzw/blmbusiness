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


//用户请求(邀请)加入公司的状态
enum USER_REQCMP_STATUS
{
    REQUEST_WAIT,REQUEST_REJECT,REQUEST_ACCEPT,REQUEST_TIMEOUT
};
//请求（邀请）等待中,加入被拒绝,加入已接受

typedef struct _t_account_base
{
    int m_nPoints;						//积分
    
    int m_bPersonAuth;					//个人是否经过认证
    int m_bCmpAuth;						//公司是否经过认证
    int m_bAdmin;						//是否为公司管理员
    int m_nAdminNum;					//公司管理员个数    
    int m_nBeiYaoqing;					//被邀请的个数
    int m_nYaoqing;						//邀请的个数
    int m_nQingqiu;						//有多少个请求（是管理员才有效）    
    int m_lResumeDt;					//简历最后修改日期    
    int m_nRequest;						//用户请求加入公司的状态
    int m_nRemindProdunit;
    int email_vd;
    int mobile_vd;
	double m_nPercent;					//个人信息完整度
	char m_strCompanyID[64];			//所属公司ID，若没有所属公司，则为空串
    char m_strCmpName[100];				//所属公司优先显示本地名称，若为空，则显示英文名称
    char m_strCmpEngName[100];			//公司英文名

	//这部分数据暂定为0和空串，在输出之前初始化
	
	int m_nBuyProductType;				//已购买产品种类
	int m_nRemindProductNum;			//提醒产品数量
	char m_strResumeID[64];				//简历ID	
    char m_strRemindProductID[64];		//提醒产品ID，若无提醒，则为空串
    char m_strRemindProductName[256];	//提醒产品Name
} TAccountBaseInfo;

//公司认证信息
enum CMP_AUTHTYPE {UNSUBMITED,UNAUDITED/*待验证*/,AUTHED/*通过认证*/,NOTAUTHED/*未通过*/};
//公司部门信息
typedef struct _t_account_detail_departinfo
{

    int m_strDepartID;
    int m_nSeq; //去掉，不用seq了
    string m_strName;
    string m_strNameLocation;
    string m_strRemark;
    string m_strRemarkLocation;
} TAccountDetailDepartInfo;
//公司简介信息
typedef struct _t_cmp_introductioninfo
{
    Tokens m_vecServerType; //服务类型ID
    Tokens m_vecServerPort; //服务的港口
    string m_strKeywords;			//关键字
    string m_strAdvantageInfo;		//优势简介
    string m_strBirefInfo;			//公司简介
} TCmpIntroduction;

//公司信息
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

    long m_strAuthTime;//新增字段

    vector< TAccountDetailDepartInfo > m_vecDepart;
    int m_nIntroPercent; //公司简介完整度
    TCmpIntroduction m_cmpIntroduction;//公司简介
} TAccountDetailCmpInfo;

typedef struct _t_account_detail_cmp_dep_user//公司部门用户信息
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

//公司同事信息
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

//邀请信息
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
	int expiredt;		//保留
	int days;			//保留
	int invsts;
	char operid[64];	//保留
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
#define APPLY_TITLE_CN					"申请加入公司"
#define APPLY_TITLE_EN					"Apply to join company"
#define EMAIL_CONTAINT_CN				"%s 申请加入公司 %s"
#define EMAIL_CONTAINT_EN				"%s apply to join into company %s"
#define SERVNUMBER                      4   //服务器的个数

typedef vector<_CompanyCoWorker> CWVec;
typedef vector<_SendInvite> SIVec;
typedef vector<_RecvInvite> RIVec;
#endif

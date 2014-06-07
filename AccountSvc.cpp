#include "blmcom_head.h"
#include "AccountSvc.h"
#include "UserRegSvc.h"
#include "MessageService.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "SmsMail.h"
#include "Util.h"
#include "NotifyService.h"
#include "json.h"
#include <iostream>
#include <fstream>
#include "md5.h"
#include "PhoneSvc.h"
#include "SmsMailSvc.h"

using namespace std;

#define INVOICE_TITLE "Invoice from BoLooMo"
#define INVOICE_CONTENT "Dear Sir/Madam,\n \
Please find an invoice attached for recent work. Don't hesitate to reply with any questions.\n \
Thank you for your business. We look forward to serving you again.\n\n \
*******************************************************************************\n \
Sincerely,\n \
The BLM-Shipping Sales Team\n \
ADD: Room 1502-1503, ZhongGuanCun e-Plaza fortune center , ZhongGuanCun Street No.11, Haidian District , Beijing , China\n \
Service Hotline: 8610-62682266\n \
*******************************************************************************\n"

IMPLEMENT_SERVICE_MAP(AccountSvc)

string service[] = 
{
	"= 'BLMPSATELLITEAIS'",
	"LIKE 'BLMFAMILY_%'"
};

AccountSvc::AccountSvc()
{

}

AccountSvc::~AccountSvc()
{

}

bool AccountSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_ACCOUNT, this))
        return false;

	//int interval = 86400;
	int interval = 60;
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
	if(timerId <= 0)
		return false;

    SERVICE_MAP(SID_ACCOUNT_SUMMARY,AccountSvc,getSummary);
    SERVICE_MAP(SID_ACCOUNT_RESUME,AccountSvc,getResume);
    SERVICE_MAP(SID_ACCOUNT_UPD_RESUME,AccountSvc,updResume);
    SERVICE_MAP(SID_ACCOUNT_REQ_VERIFY,AccountSvc,getVerifyCode);
    SERVICE_MAP(SID_ACCOUNT_VERIFY,AccountSvc,verifyCode);
    SERVICE_MAP(SID_ACCOUNT_CHG_PWD,AccountSvc,chgPassword);
    SERVICE_MAP(SID_ACCOUNT_GET_CMPY,AccountSvc,getCompany);
    SERVICE_MAP(SID_ACCOUNT_UPD_CMPY,AccountSvc,updCompany);
    SERVICE_MAP(SID_ACCOUNT_UPD_CMPY_DEPT,AccountSvc,updCompanyDept);
    SERVICE_MAP(SID_ACCOUNT_DEL_CMPY_DEPT,AccountSvc,delCompanyDept);
    SERVICE_MAP(SID_ACCOUNT_CHG_CMPY_DEPT,AccountSvc,chgCompanyDept);
    SERVICE_MAP(SID_ACCOUNT_UPL_AUTH,AccountSvc,uplAuth);
    SERVICE_MAP(SID_ACCOUNT_GET_AUTH,AccountSvc,getAuth);
    SERVICE_MAP(SID_ACCOUNT_UPL_CMPY_AUTH,AccountSvc,uplCompanyAuth);
    SERVICE_MAP(SID_ACCOUNT_UPL_CMPY_LOGO,AccountSvc,uplCompanyLogo);
    SERVICE_MAP(SID_ACCOUNT_GET_CMPY_AUTH,AccountSvc,getCompanyAuth);
    SERVICE_MAP(SID_ACCOUNT_GET_CMPY_LOGO,AccountSvc,getCompanyLogo);
    SERVICE_MAP(SID_ACCOUNT_GET_CMPY_RESUME,AccountSvc,getCompanySummary);
    SERVICE_MAP(SID_ACCOUNT_UPD_CMPY_RESUME,AccountSvc,updCompanySummary);
    SERVICE_MAP(SID_ACCOUNT_GET_CMPY_PERSONAL,AccountSvc,getCompanyPersonal);
    SERVICE_MAP(SID_ACCOUNT_GET_CMPY_COWORKER,AccountSvc,getCompanyCowoker);
    SERVICE_MAP(SID_ACCOUNT_INVITE,AccountSvc,inviteCoworker);
    SERVICE_MAP(SID_ACCOUNT_APPLY,AccountSvc,apply);
    SERVICE_MAP(SID_ACCOUNT_GET_ALL_SEND_INVITE,AccountSvc,getAllSendInvite);
    SERVICE_MAP(SID_ACCOUNT_GET_ALL_UNDEAL,AccountSvc,getAllUnDeal);
    SERVICE_MAP(SID_ACCOUNT_GET_ALL_RECV_INVITE,AccountSvc,getAllRecvInvite);
    SERVICE_MAP(SID_ACCOUNT_DEAL_INVITE,AccountSvc,dealInvite);
    SERVICE_MAP(SID_ACCOUNT_ACCEPT,AccountSvc,acceptApply);
    SERVICE_MAP(SID_ACCOUNT_QUIT,AccountSvc,quitCompany);
    SERVICE_MAP(SID_ACCOUNT_SET_ADMIN,AccountSvc,setAdmin);
    SERVICE_MAP(SID_ACCOUNT_KICK_ASS,AccountSvc,kickAss);
    SERVICE_MAP(SID_ACCOUNT_GET_VSL,AccountSvc,getVessel);
    SERVICE_MAP(SID_ACCOUNT_UPD_VSL,AccountSvc,updVessel);
    SERVICE_MAP(SID_ACCOUNT_DEL_VSL,AccountSvc,delVessel);
    SERVICE_MAP(SID_ACCOUNT_GET_PORT,AccountSvc,getPort);
    SERVICE_MAP(SID_ACCOUNT_UPD_PORT,AccountSvc,updPort);
    SERVICE_MAP(SID_ACCOUNT_DEL_PORT,AccountSvc,delPort);
    SERVICE_MAP(SID_ACCOUNT_GET_PRODUCT,AccountSvc,getProduct);
    SERVICE_MAP(SID_ACCOUNT_GET_JIFEN,AccountSvc,getJifen);
    SERVICE_MAP(SID_ACCOUNT_UPD_ORDER,AccountSvc,updOrder);
    SERVICE_MAP(SID_ACCOUNT_ALL_PRODUCT,AccountSvc,allProduct);
    SERVICE_MAP(SID_ACCOUNT_PRODUCT_DESC,AccountSvc,productDesc);
    SERVICE_MAP(SID_ACCOUNT_USER_ORDER,AccountSvc,getUserOrders);
    SERVICE_MAP(SID_ACCOUNT_UPD_PRODUCT_PARAM,AccountSvc,updateProductParam);
    SERVICE_MAP(SID_ACCOUNT_ALL_AUTH_TYPE,AccountSvc,getAllUserAuthType);
    SERVICE_MAP(SID_ACCOUNT_APPLY_VERIFY_PERSONAL,AccountSvc,applyVerifyPersonal);
    SERVICE_MAP(SID_ACCOUNT_DEAL_PERSONAL_APPLY,AccountSvc,dealPersonalApply);
    SERVICE_MAP(SID_ACCOUNT_GET_PERSONAL_DETAIL,AccountSvc,getPersonalDetail);
    SERVICE_MAP(SID_ACCOUNT_APPLY_VERIFY_COMPANY,AccountSvc,applyVerifyCompany);
    SERVICE_MAP(SID_ACCOUNT_DEAL_COMPANY_APPLY,AccountSvc,dealCompanyApply);
    SERVICE_MAP(SID_ACCOUNT_GET_COMPANY_DETAIL,AccountSvc,getCompanyDetail);
    SERVICE_MAP(SID_ACCOUNT_INTER_MEMBER,AccountSvc,interMember);
	SERVICE_MAP(SID_GET_ORDER_LIST,AccountSvc,GetOrderList);
	SERVICE_MAP(SID_GET_DETAIL_OF_ORDER,AccountSvc,GetDetailOfOrder);
	SERVICE_MAP(SID_DEAL_AUTHORITY,AccountSvc,DealAuthority);
	SERVICE_MAP(SID_GET_USER_ID,AccountSvc,getUserId);
	SERVICE_MAP(SID_GET_TEL_BAL,AccountSvc,getTelBal);
	SERVICE_MAP(SID_GET_USER_INFO,AccountSvc,getUserInfo);
	SERVICE_MAP(SID_GET_USERID_FROM_THIRDID,AccountSvc,getUserIdFromThirdId);
	SERVICE_MAP(SID_GET_EXCHANGE,AccountSvc,getExchange);
	SERVICE_MAP(SID_GET_CONNECTCHARGE,AccountSvc,getConnectCharge);
	SERVICE_MAP(SID_CONFIRM_USER_PASSWD,AccountSvc,confirmUserPasswd);
	SERVICE_MAP(SID_LOGOUT,AccountSvc,logout);
	SERVICE_MAP(SID_AUTO_REGISTER,AccountSvc,autoRegister);
	SERVICE_MAP(SID_UPD_THIRDID,AccountSvc,updThirdId);
	SERVICE_MAP(SID_GET_DEP_NUM,AccountSvc,getDepNum);
	SERVICE_MAP(SID_GET_DEP_DETAIL,AccountSvc,getDepDetail);
	SERVICE_MAP(SID_QUE_USER_SERVICE,AccountSvc,queUserService);
	SERVICE_MAP(SID_GET_PORT_LIST,AccountSvc,getPortList);
	SERVICE_MAP(SID_GET_SHIP_LIST,AccountSvc,getShipList);
	SERVICE_MAP(SID_GET_PORT_BRIEF,AccountSvc,getPortBrief);
	SERVICE_MAP(SID_GET_SERVICENAME,AccountSvc,getServiceName);
	SERVICE_MAP(SID_UPD_PASSWD,AccountSvc,updPhoneUserPasswd);
	SERVICE_MAP(SID_IPHONE_REGISTER,AccountSvc,iPhoneRegister);
	SERVICE_MAP(SID_INSERT_IOS_DEVICE,AccountSvc,insertIOSDevice);
	SERVICE_MAP(SID_UDP_IOS_USER,AccountSvc,updIOSUser);
	SERVICE_MAP(SID_REGISTER_NOACTIVE,AccountSvc,registerStraight);
	SERVICE_MAP(SID_RELATE_IPHONE_PC_USER,AccountSvc,relateIphonePc);
	SERVICE_MAP(SID_GET_THIRD_PASSWORD,AccountSvc,getThirdPassword);
    DEBUG_LOG("[AccountSvc::Start] OK......................................");

	//stringstream out;
	//uplCompanyAuth("test","{cmpid:\"C09042029717\",pics:[{issystem:1,authtp:\"qiye1\",picid:\"00001721\"}]}",out);
	return true;
}

bool OnUserAddCompany(const char* UserId, const char* CmpyId, const char* CmpyName)
{
    if (strlen(CmpyId)==0||strlen(UserId)==0)
    {
        return false;
    }

    string _CmpId=string(CmpyId);
    int len=_CmpId.length()>9?_CmpId.length()-9:1;
    _CmpId=_CmpId.replace(0,len,"2");

    bool roomExist=false;
    int roomId=-1;
    string userJid=string(UserId)+"@boloomo.com";

    MySqlBj* psql = CREATE_MYSQL_BJ;

    char sql[1024];
    sprintf(sql, "select roomid from ofmucroom where name='%s'", _CmpId.c_str());
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql), false);
    if(psql->NextRow())
    {
        roomExist=true;
        READMYSQL_INT(roomid,roomId,-1);
    }

    if (!roomExist) //不存在,创建群并设用户为管理员
    {
        sprintf(sql, "select max(roomid) as maxid from ofmucroom");
		CHECK_MYSQL_BJ_STATUS(psql->Query(sql), false);
        if(psql->NextRow())
        {
            READMYSQL_INT(maxid,roomId,1000)
        }
        if (roomId<80000)
        {
            roomId=80000;
        }
        roomId++;

        sprintf(sql, "insert into ofmucroom(serviceId,roomId,creationDate,modificationDate,name,naturalName,description,lockedDate,emptyDate,canChangeSubject,maxUsers,publicRoom,moderated,membersOnly,canInvite,canDiscoverJID,logEnabled,rolesToBroadcast,useReservedNick,canChangeNick,canRegister,roomkeywords,roomblmauth,iscmpyRoom)"\
                " values(7,%d,'000000000000000','000000000000000','%s','%s','...','000000000000000','000000000000000',0,50,1,0,1,0,1,0,7,0,1,1,'',1,1)",
                roomId,_CmpId.c_str(),CmpyName);
        CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, false);
        
		sprintf(sql, "insert into ofmucaffiliation(roomID,jid,affiliation) values(%d,'%s',10)", roomId, userJid.c_str());
        CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, false);

        RELEASE_MYSQL_BJ_RETURN(psql, true);
    }

    sprintf(sql, "select * from ofmucaffiliation where roomId=%d and affiliation=10", roomId);
    CHECK_MYSQL_BJ_STATUS(psql->Query(sql), false);
	
	int count=psql->GetRowCount();
	if (count>0) //存在管理员
	{
		sprintf(sql, "insert into ofmucmember(roomID,jid,nickname) values(%d,'%s','%s')", roomId,userJid.c_str(),"0");
	}
	else //不存在管理员
	{
		sprintf(sql, "insert into ofmucaffiliation(roomID,jid,affiliation) values(%d,'%s',10)", roomId,userJid.c_str());
	}
	CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, false);

	RELEASE_MYSQL_BJ_RETURN(psql, true);
}

bool OnUserQuitCompany(const char* UserId, const char* CmpyId)
{
    if (strlen(CmpyId)==0||strlen(UserId)==0)
    {
        return false;
    }

    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "delete from t41_comp_contacts where user_Id='%s'", UserId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, false);

    string _CmpId=string(CmpyId);
    if (_CmpId.substr(0,1)=="C"||_CmpId.substr(0,1)=="c")
    {
        int len=_CmpId.length()>9?_CmpId.length()-9:1;
        _CmpId=_CmpId.replace(0,len,"2");
    }

    int roomId=-1;
    string userJid=string(UserId)+"@boloomo.com";
    sprintf(sql, "select roomid from ofmucroom where name='%s'", _CmpId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), false);
	if (psql->NextRow())
	{
		READMYSQL_INT(roomid,roomId,-1)
	}

    sprintf(sql, "select * from ofmucmember where roomId=%d and jid='%s'", roomId, userJid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), false);
	int count=psql->GetRowCount();
	if (count>0)//判断是否普通成员？是,直接删除
	{
		sprintf(sql, "delete from ofmucmember where roomId=%d and jid='%s'", roomId, userJid.c_str());
		psql->Execute(sql);
		RELEASE_MYSQL_RETURN(psql, true);
	}

	//不是普通成员
	sprintf(sql, "select jid from ofmucaffiliation where roomId=%d and affiliation=10", roomId);
	if(psql->Query(sql) && psql->NextRow())
	{
		char AdminJid[128]= {'\0'};
		READMYSQL_STR(jid,AdminJid);

		if (string(AdminJid)==userJid) //是群的管理员
		{
			sprintf(sql, "select jid as NadJid from ofmucmember where roomId=%d", roomId);
			CHECK_MYSQL_STATUS(psql->Query(sql), false);
			if (psql->NextRow())//选择其中一位普通成员做管理员
			{
				char NJid[128]= {'\0'};
				READMYSQL_STR(NadJid,NJid);
				sprintf(sql, "update ofmucaffiliation set jid='%s' where roomId=%d", NJid, roomId);
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, false);

				sprintf(sql, "delete from ofmucmember where jid='%s' where roomId=%d", NJid, roomId);
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, false);

				RELEASE_MYSQL_RETURN(psql, true);

			}
			else //没有可选的管理员
			{
				sprintf(sql, "delete from ofmucaffiliation where roomId=%d", roomId);
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, false);
				
				sprintf(sql, "delete from ofmucroom where roomId=%d and serviceId=7", roomId);
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, false);

				RELEASE_MYSQL_RETURN(psql, true);
			}
		}
	}

    RELEASE_MYSQL_RETURN(psql, false);
}

//zhuxj
int AccountSvc::getSummary(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getSummary]bad format:", jsonString, 1);
    string uid = root.getv("uid", "");
	MySql* psql = CREATE_MYSQL;

    char sql[6*1024];
	int apply_type;
    int  Integrities = 0;//完整度

    TAccountBaseInfo  m_UserInfo;
	
    COMP_FLAG  m_CompFlag;

	sprintf (sql, "SELECT t1.POINT, t1.USER_VERIFY, t2.COMPANY_KEY, t3.LocalFullName, t3.Name, t3.VERIFIED AS compVerify, t2.Isadmin, t2.FLAG, t2.APPLY_TYPE, t1.CITY_CODE, t1.PASSWORD, t1.BIRTHDAY, t1.GENDER, t1.SALUTATION, t1.REALNAME, t1.FirstName, t1.LastName, t1.CREATIONDATE, t1.MODIFICATIONDATE, t1.GETPWDQUESTION, 		\
							t1.GETPWDANSWER, t1.USERTYPE, t1.TELNO, t1.FAX, t1.MOBILE, t1.EMAIL, t1.ZIP, t1.ADDRESS, t1.SOURCE, t1.SOURCETYPE, t1.IS_FLAG, t1.MAINFLAG, t1.IP, t1.COUNTRY, t1.NICKNAME, t1.SIGN, t1.CURR_CITY, t1.AVATAR_ID, t1.MOBILE_VERIFIED, t1.USER_LEVEL, t1.EMAIL_VERIFIED	\
							FROM t00_user AS t1 LEFT JOIN t41_comp_contacts AS t2 ON t1.USER_ID = t2.USER_ID \
							LEFT JOIN t41_company AS t3 ON t2.COMPANY_KEY = t3.Company_Key \
							WHERE t1.USER_ID = '%s' ORDER BY t2.last_upd_dt DESC LIMIT 1", uid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		char tmp[256];

		READMYSQL_INT(POINT, m_UserInfo.m_nPoints, 0);
		Integrities += (m_UserInfo.m_nPoints != 0)?1:0;

		READMYSQL_INT(USER_VERIFY, m_UserInfo.m_bPersonAuth, 0);
		Integrities += ((m_UserInfo.m_bPersonAuth == 1)?1:0);

		READMYSQL_STR(COMPANY_KEY, m_UserInfo.m_strCompanyID);
		READMYSQL_STR(LocalFullName, m_UserInfo.m_strCmpName);
		READMYSQL_STR(Name, m_UserInfo.m_strCmpEngName);
		READMYSQL_INT(compVerify, m_UserInfo.m_bCmpAuth, 0);
		READMYSQL_INT(Isadmin, m_UserInfo.m_bAdmin, 0);
		READMYSQL_INT(FLAG, m_UserInfo.m_nRequest, 0);
		READMYSQL_INT(APPLY_TYPE, apply_type, -1);

		READMYSQL_INT(mobile_verified, m_UserInfo.mobile_vd, 0);;
		Integrities += (m_UserInfo.mobile_vd?1:0);

		READMYSQL_INT(EMAIL_VERIFIED, m_UserInfo.email_vd, 0);
		Integrities += (m_UserInfo.email_vd?1:0);

		READMYSQL_STR(CITY_CODE, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(PASSWORD, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(BIRTHDAY, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(GENDER, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(SALUTATION, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(REALNAME, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(FirstName, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(LastName, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(CREATIONDATE, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(MODIFICATIONDATE, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(GETPWDQUESTION, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(GETPWDANSWER, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(USERTYPE, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(TELNO, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(FAX, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(MOBILE, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(EMAIL, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(ZIP, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(ADDRESS, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(SOURCE, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(SOURCETYPE, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(IS_FLAG, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(MAINFLAG, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(IP, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(COUNTRY, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(NICKNAME, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(SIGN, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(CURR_CITY, tmp);
		Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(AVATAR_ID, tmp);
		Integrities += (strlen(tmp)?1:0);

		//READMYSQL_STR(VERIFIED, tmp);
		//Integrities += (strlen(tmp)?1:0);

		READMYSQL_STR(USER_LEVEL, tmp);
		Integrities += (strlen(tmp)?1:0);
	}
	m_UserInfo.m_bCmpAuth = (m_UserInfo.m_bCmpAuth == 1)?1:0;
	m_UserInfo.m_bPersonAuth = (m_UserInfo.m_bPersonAuth == 1)?1:0;
	m_UserInfo.m_nPercent = (Integrities / 39.0) * 100.0;

	m_UserInfo.m_bAdmin = (m_UserInfo.m_bAdmin == 1)?1:0;

	if (m_UserInfo.m_bAdmin)
	{
		sprintf (sql, "SELECT COUNT(1) as num FROM t41_comp_contacts WHERE company_key = '%s' AND flag = 0 AND apply_type = 0", m_UserInfo.m_strCompanyID);
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
		READMYSQL_INT(num, m_UserInfo.m_nQingqiu, 0);
	}
	else 
		m_UserInfo.m_nQingqiu = 0;

	if (!(m_UserInfo.m_nRequest == 1 || (!m_UserInfo.m_nRequest && !apply_type)))
	{
		m_UserInfo.m_strCompanyID[0] = '\0';
		m_UserInfo.m_strCmpName[0] = '\0';
		m_UserInfo.m_strCmpEngName[0] = '\0';
	}

	if(!strlen(m_UserInfo.m_strCmpName))
	{
		strcpy(m_UserInfo.m_strCmpName,m_UserInfo.m_strCmpEngName);
	}
	
	//查询被邀请的记录数
    sprintf(sql, "select count(1) as num from t41_comp_contacts where USER_ID = '%s' and (APPLY_TYPE = 1 or APPLY_TYPE = 2) and flag = '0'",uid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	READMYSQL_INT(num,m_UserInfo.m_nBeiYaoqing,0);

	//查询发出的邀请数
    sprintf(sql, "select count(1) as Num from t41_comp_contacts where INVITER = '%s'",uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	READMYSQL_INT(Num,m_UserInfo.m_nYaoqing,0);

	m_UserInfo.m_nBuyProductType = 0;			//已购买产品种类
	m_UserInfo.m_nRemindProductNum = 0;			//提醒产品数量
	m_UserInfo.m_strResumeID[0] = '\0';			//简历ID	
    m_UserInfo.m_strRemindProductID[0] = '\0';	//提醒产品ID，若无提醒，则为空串
    m_UserInfo.m_strRemindProductName[0] = '\0';

	char res[1024 * 10] = "";
    sprintf(res,"{pt:%d,npercent:%d,bpersonauth:%d,cmpid:\"%s\",cmplocna:\"%s\",cmpengna:\"%s\",cmpauth:%d,badmin:%d,reqst:%d,nprodty:%d,remindprodid:\"%s\",\
				 remindprodna:\"%s\",remindprodnum:%d,nbeiyq:%d,nyq:%d,nrequest:%d,resumeid:\"%s\",nlastdt:0,mobile_vd:%d,email_vd:%d}",
				m_UserInfo.m_nPoints, (int)m_UserInfo.m_nPercent, m_UserInfo.m_bPersonAuth, m_UserInfo.m_strCompanyID, m_UserInfo.m_strCmpName,
				m_UserInfo.m_strCmpEngName, m_UserInfo.m_bCmpAuth, m_UserInfo.m_bAdmin, m_UserInfo.m_nRequest, m_UserInfo.m_nBuyProductType,
				m_UserInfo.m_strRemindProductID, m_UserInfo.m_strRemindProductName, m_UserInfo.m_nRemindProductNum, m_UserInfo.m_nBeiYaoqing,
				m_UserInfo.m_nYaoqing,m_UserInfo.m_nQingqiu, m_UserInfo.m_strResumeID, m_UserInfo.mobile_vd,m_UserInfo.email_vd);

    out << res;
    RELEASE_MYSQL_RETURN(psql, 0);
}



//暂时不用实现
int AccountSvc::getResume(const char* pUid, const char* jsonString, std::stringstream& out)
{
    out << "{eid:1, emsg:\"not implements\"}";
    return 0;
}

//暂时不用实现
int AccountSvc::updResume(const char* pUid, const char* jsonString, std::stringstream& out)
{
    out << "{eid:1, emsg:\"not implements\"}";
    return 0;
}

//caiwj
//{uid:"blm",email:"blm@boloomo.com ",tel: "13810610657",cn:1,ipPhone:"",seq:""}
int AccountSvc::getVerifyCode(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getVerifyCode]bad format:", jsonString, 1);
    string uid    = root.getv("uid", "");
    string email  = root.getv("email", "");
    string tel	  = root.getv("tel", "");
    int	   nCn	  = root.getv("cn", 0);
	string ipPhone = root.getv("ipPhone", "");
	string seq = root.getv("seq", "");
	bool bSucc = false;

    if(uid.empty() || (email.empty() && tel.empty()))
		return 1;

	if(ipPhone.length() == 1 && ipPhone[0] == '1')
	{
		string t_tel = tel;
		Tokens tokens = StrSplit(t_tel, " ");
		if(tokens.size() == 2 && t_tel[0] == '+')
		{
			MySql *psql = CREATE_MYSQL;
			char sql[1024] = "";
			int num = 0;
			sprintf(sql, "SELECT COUNT(1) AS num FROM t00_user WHERE user_id = '%s%s'", tokens[0].substr(1).c_str(), tokens[1].c_str());
			CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
			READMYSQL_INT(num, num, 0)
			if(num)
			{
				out << FormatString("{\"eid\":100,\"seq\":\"%s\"}", seq.c_str());
				RELEASE_MYSQL_RETURN(psql,0);
			}
			RELEASE_MYSQL_NORETURN(psql);
			bSucc = g_UserRegSvc::instance()->sendCode(uid, tel);
		}		
	}
	else
	{
		bSucc = g_UserRegSvc::instance()->sendCode(uid, tel, email, (nCn==1), false);
	}

    if(bSucc)
	{
		out << FormatString("{\"eid\":0,\"seq\":\"%s\"}", seq.c_str());
        return 0;
	}

    return 2;
}

//caiwj
//{uid:"blm",email:"blm@boloomo.com ",tel:"13810610657",validcode_email: "123456", validcode_tel: "123456", seq:""}
int AccountSvc::verifyCode(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::verifyCode]bad format:", jsonString, 1);
    string uid    = root.getv("uid", "");
    string email  = root.getv("email", "");
    string tel	  = root.getv("tel", "");
    string emailCode = root.getv("validcode_email", "");
    string telCode	 = root.getv("validcode_tel", "");
	string seq = root.getv("seq", "");

    if(uid.empty() || (email.empty() && tel.empty()) || (emailCode.empty() && telCode.empty()))
		return 1;

    string code = (emailCode.empty() ? telCode : emailCode);

    string telno = "";
	string stremail = "";

    int ret = g_UserRegSvc::instance()->verifyCode(uid, code, telno, stremail);
	if(ret == 1)
	{
		out << FormatString("{\"eid\":0,\"seq\":\"%s\"}", seq.c_str());
		return 0;
	}

    return 2;
}

//conght  修改密码接口  测试过  还有问题
int AccountSvc::chgPassword(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::chgPassword]bad format:", jsonString, 1);
    string uid     = root.getv("uid", "");
    string oldPwd  = root.getv("oldpwd", "");
    string newPwd  = root.getv("newpwd", "");

    if(uid.empty()||oldPwd.empty())
        return 1;

    char sql[256];
    int  COUNT = 0;
    sprintf(sql,"select count(1) as Num from t00_user where USER_ID = '%s' and PASSWORD = '%s'",
				uid.c_str(),oldPwd.c_str());

    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	READMYSQL_INT(Num,COUNT,0);

    if(COUNT == 0)
    {
        out<<"{\"eid\":100}";
        RELEASE_MYSQL_RETURN(psql, 0);
    }

    sprintf(sql,"update t00_user set PASSWORD = '%s' where USER_ID = '%s'", newPwd.c_str(), uid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{\"eid\":0}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght   获取公司信息接口  测试过
int AccountSvc::getCompany(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getCompany]bad format:", jsonString, 1);
    string cmpid = root.getv("cmpid", "");
	if(cmpid.empty())
		return 1;

    TAccountDetailCmpInfo m_CmpInfo;

    char sql[512];
    char result[500];
    char text[COMPANY_REMARK_SIZE];
    char LogID[200];
    LogID[0] = '\0';
    int  auth = 0;
    //int  temp;
    sprintf(sql,"select t1.Name,t1.Address,t1.LocalFullName,t1.CountryName,\
				 t1.City, t1.TelNo,t1.Fax,t1.Website ,t1.COUNTRY_LOCATION, t1.EMAIL,\
				 t1.CITY_LOCATION , t1.ADDRESS_LOCATION , t1.NEW_LOGOID , t1.VERIFIED from \
				 t41_company t1 where t1.Company_Key = '%s' ", cmpid.c_str());
    MySql* psql = CREATE_MYSQL;

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{
		READMYSQL_STR(Name, result);
		m_CmpInfo.m_strName = string(result);

		READMYSQL_STR(CITY_LOCATION, result);
		m_CmpInfo.m_strCityLocation = string(result);

		READMYSQL_STR(ADDRESS_LOCATION, result);
		m_CmpInfo.m_strAddressLocation = string(result);

		READMYSQL_STR(Address, result);
		m_CmpInfo.m_strAddress = string(result);

		READMYSQL_STR(LocalFullName,result);
		m_CmpInfo.m_strLocalName = string(result);

		READMYSQL_STR(CountryName,result);
		m_CmpInfo.m_strCountry = string(result);

		READMYSQL_STR(City,result);
		m_CmpInfo.m_strCity = string(result);

		READMYSQL_STR(TelNo,result);
		m_CmpInfo.m_strTels = string(result);

		READMYSQL_STR(Fax,result);
		m_CmpInfo.m_strFaxs = string(result);

		READMYSQL_STR(Website,result);
		m_CmpInfo.m_strWebsite = string(result);

		READMYSQL_STR(EMAIL,result);
		m_CmpInfo.m_strEmail = string(result);

		READMYSQL_STR(NEW_LOGOID,LogID);

		READMYSQL_STR(VERIFIED,result);
		if(!strcmp(result,"1"))
			auth = 1;
	}

    sprintf(sql,"select DEP_ID,SEQ,DEP_NAME,REMARK,NAME_LOCATION,REMARK_LOCATION from t41_comp_department where Company_Key = '%s'" ,cmpid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{
		TAccountDetailDepartInfo DepInfo;

		READMYSQL_INT(DEP_ID,DepInfo.m_strDepartID,0);
		READMYSQL_INT(DEP_ID,DepInfo.m_nSeq,0);

		READMYSQL_STR(DEP_NAME,result);
		DepInfo.m_strName = string(result);

		READMYSQL_STR(NAME_LOCATION,result);
		DepInfo.m_strNameLocation = string(result);


		READMYSQL_STR(REMARK,text);
		DepInfo.m_strRemark = string(text);

		READMYSQL_STR(REMARK_LOCATION,text);
		DepInfo.m_strRemarkLocation = string(text);

		m_CmpInfo.m_vecDepart.push_back(DepInfo);
	}

    // modified by caiwj
    // 如果公司部门为空，则插入一个默认部门(depid=1)
    if(m_CmpInfo.m_vecDepart.empty())
    {
        sprintf(sql, "insert into t41_comp_department(company_key, dep_id, dep_name) values ('%s', 1, 'Others')", cmpid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>0, 3);

		TAccountDetailDepartInfo DepInfo;
		DepInfo.m_strDepartID = 1;
		DepInfo.m_strName = "Others";
		m_CmpInfo.m_vecDepart.push_back(DepInfo);
    }

    Tokens r = StrSplit(m_CmpInfo.m_strTels,string(";"));
    m_CmpInfo.m_strTels = ToStr(r,string(","));

    r = StrSplit(m_CmpInfo.m_strFaxs,";");
    m_CmpInfo.m_strFaxs = ToStr(r,",");

    char retjson[500];
    sprintf(retjson,"{logid:\"%s\",na_e:\"%s\",na_l:\"%s\",addr_e:\"%s\",cry:\"%s\",city_e:\"%s\",tels:\"%s\",\
					faxs:\"%s\",email:\"%s\",website:\"%s\",addr_l:\"%s\",city_l:\"%s\",auth:%d,",
            LogID,
            m_CmpInfo.m_strName.c_str(),
            m_CmpInfo.m_strLocalName.c_str(),
            m_CmpInfo.m_strAddress.c_str(),
            m_CmpInfo.m_strCountry.c_str(),
            m_CmpInfo.m_strCity.c_str(),
            m_CmpInfo.m_strTels.c_str(),
            m_CmpInfo.m_strFaxs.c_str(),
            m_CmpInfo.m_strEmail.c_str(),
            m_CmpInfo.m_strWebsite.c_str(),
            m_CmpInfo.m_strAddressLocation.c_str(),
            m_CmpInfo.m_strCityLocation.c_str(),auth);

    string strDep = "";
    char strTemp[56*1024];
	
	std::stringstream ss;
    ss << "departs:[";
    vector<TAccountDetailDepartInfo>::iterator iter = m_CmpInfo.m_vecDepart.begin();
    while(iter != m_CmpInfo.m_vecDepart.end())
    {
        sprintf(strTemp,"{id:\"%d\",seq:%d,na:\"%s\",rmk:\"%s\"}",
                iter->m_strDepartID,iter->m_nSeq,iter->m_strName.c_str(),
                iter->m_strRemark.c_str());
        if(iter == m_CmpInfo.m_vecDepart.begin())
            ss << strTemp;
        else
            ss << "," << strTemp;
        iter++;
    }
    ss << "]}";
    strDep = ss.str();

    out<<retjson<<strDep;
    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght   更新公司信息接口    测试过
int AccountSvc::updCompany(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::updCompany]bad format:", jsonString, 1);

	char gmt0now[20];
	GmtNow(gmt0now);

    TAccountDetailCmpInfo m_CmpInfo;

    string    uid            = root.getv("uid", "");
    m_CmpInfo.m_strCmpID     = root.getv("cmpid", "");
    m_CmpInfo.m_strName      = root.getv("na_e", "");
    m_CmpInfo.m_strLocalName = root.getv("na_l", "");
    m_CmpInfo.m_strAddress   = root.getv("addr_e", "");
    m_CmpInfo.m_strAddressLocation   = root.getv("addr_l", "");
    m_CmpInfo.m_strCountry   = root.getv("cry", "");
    m_CmpInfo.m_strCity      = root.getv("city_e", "");
    m_CmpInfo.m_strCityLocation     = root.getv("city_l", "");
    m_CmpInfo.m_strTels      = root.getv("tels", "");
    m_CmpInfo.m_strFaxs      = root.getv("faxs", "");
    m_CmpInfo.m_strWebsite   = root.getv("website", "");
	m_CmpInfo.m_strEmail	 = root.getv("email", "");

    char sql[1024 * 2];
    MySql* psql = CREATE_MYSQL;
    if(m_CmpInfo.m_strCmpID.empty())
    {
        struct tm * tblock;
        time_t t = time(NULL);
        tblock=localtime(&t);
        char cmpid[100];
        srand((unsigned int)t);
        int random = rand()%1000;
        int servID = rand()%SERVNUMBER;
        ++servID;
        sprintf(cmpid,"%c%02d%02d%02d%c%02d%02d%02d%03d",
                'C',(tblock->tm_year+1900)%100,tblock->tm_mon+1,tblock->tm_mday,servID+'0',tblock->tm_hour,tblock->tm_min,tblock->tm_sec,random%1000);
        
		sprintf(sql,"insert into t41_company (Company_Key,NAME,localfullname,countryname,address,city,\
					telno,fax,website,last_upd_dt,last_upd_user,CITY_LOCATION ,ADDRESS_LOCATION, email) \
					VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s', '%s')",
                cmpid,m_CmpInfo.m_strName.c_str(),m_CmpInfo.m_strLocalName.c_str(),
                m_CmpInfo.m_strCountry.c_str(),m_CmpInfo.m_strAddress.c_str(),
                m_CmpInfo.m_strCity.c_str(),m_CmpInfo.m_strTels.c_str(),
                m_CmpInfo.m_strFaxs.c_str(),m_CmpInfo.m_strWebsite.c_str(), gmt0now, uid.c_str(),
                m_CmpInfo.m_strCityLocation.c_str(), m_CmpInfo.m_strAddressLocation.c_str(), m_CmpInfo.m_strEmail.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
        
        sprintf(sql,"insert into t41_comp_contacts(user_id,company_key,flag,isadmin,last_upd_dt,dep_id,apply_type) \
					 VALUES ('%s','%s','%s','%s','%s',%d,%d)",uid.c_str(),cmpid,"1","1",gmt0now, 1,0);
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

        //插入默认部门，id为1.
        sprintf(sql,"insert into t41_comp_department(company_key,dep_id,dep_name,remark) values('%s',1,'Others','the default department.')",cmpid);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
        
        //创建公司群并加入
        OnUserAddCompany(pUid, cmpid, m_CmpInfo.m_strName.c_str());

        out<<"{eid:0,cmpid:\""<<cmpid<<"\"}";
        RELEASE_MYSQL_RETURN(psql, 0);
    }

    sprintf(sql, "UPDATE t41_company SET NAME = '%s', localfullname = '%s', countryname = '%s', address = '%s', city = '%s', telno = '%s', fax = '%s', website = '%s', \
				last_upd_dt = '%s', last_upd_user = '%s' , CITY_LOCATION='%s', ADDRESS_LOCATION ='%s', email = '%s' WHERE company_key = '%s'",
            m_CmpInfo.m_strName.c_str(), m_CmpInfo.m_strLocalName.c_str(), m_CmpInfo.m_strCountry.c_str(), m_CmpInfo.m_strAddress.c_str(), m_CmpInfo.m_strCity.c_str(), m_CmpInfo.m_strTels.c_str(), m_CmpInfo.m_strFaxs.c_str(), m_CmpInfo.m_strWebsite.c_str(),
            gmt0now, pUid, m_CmpInfo.m_strCityLocation.c_str(),m_CmpInfo.m_strAddressLocation.c_str(),m_CmpInfo.m_strEmail.c_str(), m_CmpInfo.m_strCmpID.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out<<"{eid:0,cmpid:\""<<m_CmpInfo.m_strCmpID<<"\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght  更新公司单个部门信息接口  测试过
int AccountSvc::updCompanyDept(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::updCompanyDept]bad format:", jsonString, 1);
    string cmpid = root.getv("cmpid", "");
	string na = root.getv("na", "");
	string rmk = root.getv("rmk", "");
    if(cmpid.empty())
        return 1;

    TAccountDetailDepartInfo m_DepInfo;
    string DepID = root.getv("id", "");
    m_DepInfo.m_strName    = StrReplace(na, "'", "\'");
    m_DepInfo.m_strRemark  = StrReplace(rmk, "'", "\'");
	
    MySql* psql = CREATE_MYSQL;
    char sql[256];
    int  DEPNUM;
    if(DepID == "") // modified by caiwj 新增
    {
        sprintf(sql,"select count(1) as num from t41_comp_department where company_key = '%s'",cmpid.c_str());
        CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
		READMYSQL_INT(num,DEPNUM,0);
		
        if(DEPNUM!=0)
        {
            sprintf(sql,"insert into t41_comp_department(COMPANY_KEY,DEP_NAME,REMARK) values('%s','%s','%s')",
						cmpid.c_str(),m_DepInfo.m_strName.c_str(),m_DepInfo.m_strRemark.c_str());
            CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
            m_DepInfo.m_strDepartID = psql->GetInsertId();
        }
        else
        {
            sprintf(sql,"insert into t41_comp_department(COMPANY_KEY,DEP_ID,DEP_NAME,REMARK) values('%s',2,'%s','%s')",
                    cmpid.c_str(),m_DepInfo.m_strName.c_str(),m_DepInfo.m_strRemark.c_str());
            CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
            m_DepInfo.m_strDepartID = 2;
        }
    }
    else
    {
        m_DepInfo.m_strDepartID = ACE_OS::atoi(DepID.c_str());
        sprintf(sql,"update t41_comp_department set DEP_NAME='%s',REMARK='%s' where COMPANY_KEY = '%s' and DEP_ID = %d",
					m_DepInfo.m_strName.c_str(),m_DepInfo.m_strRemark.c_str(),cmpid.c_str(), m_DepInfo.m_strDepartID);
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }

    out<<"{eid:0,id:\""<<m_DepInfo.m_strDepartID<<"\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght   删除公司单个部门信息接口     测试过
int AccountSvc::delCompanyDept(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::delCompanyDept]bad format:", jsonString, 1);
    string cmpid = root.getv("cmpid", "");
    string DepID = root.getv("id", "");

    if(cmpid.empty()||DepID.empty())
        return 1;

	char gmt0now[20];
	GmtNow(gmt0now);

    int  RowNum = 0;
    char sql[2*1024];
    MySql* psql = CREATE_MYSQL;

    if(DepID == "1")
        RELEASE_MYSQL_RETURN(psql, 1);

    //delete
    sprintf(sql,"delete from t41_comp_department where COMPANY_KEY='%s' and DEP_ID =%d",cmpid.c_str(), ACE_OS::atoi(DepID.c_str()));
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    //update
    vector<TAccountDetailCmpDepUser>  m_CmpUserInfo;
    char result[100];
    int  temp;
    sprintf(sql,"select USER_ID,POSITION,TELFAX,HEADOFFICE,TOLLFREE,EMCALL,Flag,Isadmin,\
				 last_upd_dt,USERTYPE ,APPLY_TYPE,INVITER,VERIFIER from t41_comp_contacts where COMPANY_KEY='%s' and DEP_ID=%d",
				cmpid.c_str(), ACE_OS::atoi(DepID.c_str()));
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{
		TAccountDetailCmpDepUser p;

		READMYSQL_STR(USER_ID,result);
		p.m_strUserID = string(result);

		READMYSQL_STR(POSITION,result);
		p.m_strPos = string(result);

		READMYSQL_STR(TELFAX,result);
		p.m_strTelFax = string(result);

		READMYSQL_STR(HEADOFFICE,result);
		p.m_strHeadOffice = string(result);

		READMYSQL_STR(TOLLFREE,result);
		p.m_strTollFree = string(result);

		READMYSQL_STR(EMCALL,result);
		p.m_strEmCall = string(result);

		READMYSQL_STR(Flag,result);
		p.m_strFlag = string(result);

		READMYSQL_STR(Isadmin,result);
		p.m_strIsAdmin = string(result);

		READMYSQL_STR(last_upd_dt,result);
		p.m_strUpd_Dt = string(result);

		READMYSQL_STR(USERTYPE,result);
		p.m_strUserType = string(result);

		READMYSQL_INT(APPLY_TYPE,temp,0);
		ACE_OS::itoa(temp,result,10);
		p.m_strApplyType = string(result);

		READMYSQL_STR(INVITER,result);
		p.m_strInviter = string(result);

		READMYSQL_STR(VERIFIER,result);
		p.m_strVerifier = string(result);

		m_CmpUserInfo.push_back(p);
	}

    //delete
    sprintf(sql,"delete from t41_comp_contacts where COMPANY_KEY='%s' and DEP_ID = %d", cmpid.c_str(), ACE_OS::atoi(DepID.c_str()));
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    //insert
    DepID = "1";
    vector<TAccountDetailCmpDepUser>::iterator iter = m_CmpUserInfo.begin();
    while(iter!=m_CmpUserInfo.end())
    {
        sprintf(sql,"insert into t41_comp_contacts (USER_ID,COMPANY_KEY,POSITION,TELFAX,HEADOFFICE,TOLLFREE,EMCALL,Flag,Isadmin,last_upd_dt,USERTYPE,DEP_ID,INVITER,VERIFIER)  \
					values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,'%s','%s')",
                iter->m_strUserID.c_str(),cmpid.c_str(),iter->m_strPos.c_str(),iter->m_strTelFax.c_str(),iter->m_strHeadOffice.c_str(),iter->m_strTollFree.c_str(),iter->m_strEmCall.c_str(),
                iter->m_strFlag.c_str(),iter->m_strIsAdmin.c_str(),gmt0now, iter->m_strUserType.c_str(),ACE_OS::atoi(DepID.c_str()),iter->m_strInviter.c_str(),iter->m_strVerifier.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
        
        iter++;
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght  个人改变所属公司部门接口  测试过
int AccountSvc::chgCompanyDept(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::chgCompanyDept]bad format:", jsonString, 1);
    string uid    = root.getv("userid", "");
    string DepID  = root.getv("depid", "");

    if(uid.empty()&&DepID.empty())
        return 1;

	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    char sql[256];

    sprintf(sql,"update t41_comp_contacts set DEP_ID = %d, last_upd_dt = '%s' where USER_ID = '%s'",
				ACE_OS::atoi(DepID.c_str()),gmt0now,uid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out <<"{userid:\""<<uid<<"\""<<",depid:\""<<DepID<<"\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght  上传个人认证资料接口  firstname\lastname 不知道用处  测试过--不给seq的话，无法插入值
int AccountSvc::uplAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::uplAuth]bad format:", jsonString, 1);
    string uid = root.getv("uid", "");
    if(uid.empty())
        return 1;

	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    Json* _jPics = root["pics"];
    for(int i=0; i<_jPics->size(); i++)
    {
		Json* jPics = _jPics->get(i);
        int issystem;
        int authtp;
        string typeDes;
        string IdNum;
        string fileId;
        char sql[256];
        IdNum = jPics->getv("authnum", "");
        fileId = jPics->getv("picid", "");
        issystem = jPics->getv("issystem", 0);
        if(issystem == 1)//
        {
            authtp = ACE_OS::atoi(jPics->getv("authtp", ""));
            sprintf(sql,"insert into  t00_user_verify_info(USER_ID,DATA_TYPE,ID_NO,UPDATE_DT,STATUS,OBJ_ID)values('%s',%d,'%s','%s',%d,'%s')",
						uid.c_str(),authtp,IdNum.c_str(),gmt0now,USER_VERIFY_STATUS,fileId.c_str());
        }
        else
        {
            typeDes = jPics->getv("authtp", "");
            sprintf(sql,"insert into t00_user_verify_info(USER_ID,ID_NO,USER_DES,UPDATE_DT,STATUS, OBJ_ID)values('%s','%s','%s','%s',%d,'%s')",
						uid.c_str(),IdNum.c_str(),typeDes.c_str(),gmt0now,USER_VERIFY_STATUS,fileId.c_str());
        }
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght  获取个人认证资料接口   测试过，运行到最后一句话报错  "{uid:\"caiwj\"}"
int AccountSvc::getAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getAuth]bad format:", jsonString, 1);
    string uid = root.getv("uid", "");

    int     authtp;
    string  userDes;
    int     status;
    string  fileID;
    string  authno;// ID No.
    Tokens  picInfo;//array of pic
    long    authTM;
    string  FirstName;
    string  LastName;
    string  reason;

    char sql[1024];
    char result[150];
    MySql* psql = CREATE_MYSQL;

    sprintf(sql,"select UNIX_TIMESTAMP(t1.UPDATE_DT) as authtm,t2.FirstName,t2.LastName,t1.DATA_TYPE,t1.USER_DES,t1.ID_NO,t1.OBJ_ID,t1.STATUS,t1.USER_ID,t1.CAUSE\
				 from t00_user t2 LEFT JOIN t00_user_verify_info t1 on t2.USER_ID = t1.USER_ID where t2.USER_ID = '%s'",uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    while(psql->NextRow())
	{
		READMYSQL_INT(DATA_TYPE,authtp,USER_DEFINE_DATATYPE);

		READMYSQL_STR(OBJ_ID,result);
		fileID = string(result);

		READMYSQL_64(authtm,authTM,0);

		READMYSQL_STR(USER_DES,result);
		userDes = string(result);
		READMYSQL_STR(CAUSE,result);
		reason = string(result);

		READMYSQL_INT(STATUS,status,USER_VERIFY_STATUS);
		READMYSQL_STR(ID_NO,result);
		authno = string(result);

		READMYSQL_STR(FirstName,result);
		FirstName = string(result);

		READMYSQL_STR(LastName,result);
		LastName = string(result);

		char strPic[256];
		READMYSQL_STR(USER_ID,result);

		string USER_ID  =  string(result);
		if( USER_ID  !=  "")
		{
			if(authtp != USER_DEFINE_DATATYPE)
				sprintf(strPic,"{issystem:1,authtp:\"%d\",authnum:\"%s\",picid:\"%s\",nst:%d,authtm:%ld,reason:\"%s\"}",
				authtp,authno.c_str(),fileID.c_str(),status,authTM,reason.c_str());
			else
				sprintf(strPic,"{issystem:0,authtp:\"%s\",authnum:\"%s\",picid:\"%s\",nst:%d,authtm:%ld,reason:\"%s\"}",
				userDes.c_str(),authno.c_str(),fileID.c_str(),status,authTM,reason.c_str());
			picInfo.push_back(string(strPic));
		}
	}

    string Pics = ToStr(picInfo,string(","));
    Pics = ",pics:[" + Pics + "]}";
    sprintf(sql,"{firstna:\"%s\",lastna:\"%s\"",FirstName.c_str(),LastName.c_str());

    out<<sql<<Pics;
    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght  上传公司认证资料接口
int AccountSvc::uplCompanyAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::uplCompanyAuth]bad format:", jsonString, 1);
    string cmpID = root.getv("cmpid", "");
    if(cmpID.empty())
        return 1;

	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    Json* _jPics = root["pics"];
    for(int i=0; i<_jPics->size(); i++)
    {
		Json* jPics = _jPics->get(i);
        int      issystem;
        int      authtp;
        string   typeDes;
        string   fileId;
        string   authnum;
        char	 sql[2048];

        fileId  = jPics->getv("picid", "");
        authnum = jPics->getv("authnum", "");
        issystem = jPics->getv("issystem", 0);
        if(issystem == 1)//
        {
            authtp = ACE_OS::atoi(jPics->getv("authtp", ""));
            sprintf(sql,"insert into t41_comp_file(COMPANY_KEY,DATA_TYPE,ID_NO, CREATE_DT,INFO_ID,USER_ID)values('%s',%d,'%s','%s','%s','%s')",
						cmpID.c_str(), authtp,authnum.c_str(),gmt0now,fileId.c_str(),pUid);
        }
        else
        {
            typeDes = jPics->getv("authtp", "");
            sprintf(sql,"insert into t41_comp_file(COMPANY_KEY,ID_NO,USER_DES,CREATE_DT,INFO_ID,USER_ID)values('%s','%s','%s','%s','%s','%s')",
						cmpID.c_str(),authnum.c_str(),typeDes.c_str(),gmt0now,fileId.c_str(),pUid);
        }
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght 上传公司LOGO接口
int AccountSvc::uplCompanyLogo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::uplCompanyLogo]bad format:", jsonString, 1);
    string cmpID = root.getv("cmpid", "");
    if(cmpID.empty())
        return 1;

    string strPicID = root.getv("picd", "");
    Tokens tokPicID = StrSplit(strPicID,string("|"));
    strPicID        = ToStr(tokPicID,string(";"));
    string oldLogoId;

    char sql[2*1024];
    char result[2*1024];
    MySql* psql = CREATE_MYSQL;
    sprintf(sql,"select NEW_LOGOID from t41_company where Company_Key = '%s'",cmpID.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{
		READMYSQL_STR(NEW_LOGOID,result);
		oldLogoId = string(result);
	}

    if(oldLogoId != "")
        strPicID = oldLogoId + ";" + strPicID;

    sprintf(sql,"update t41_company set OLD_LOGOID = '%s',NEW_LOGOID = '%s' where Company_Key = '%s'",oldLogoId.c_str(),strPicID.c_str(),cmpID.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght  获取公司认证资料接口
int AccountSvc::getCompanyAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getCompanyAuth]bad format:", jsonString, 1);
    string cmpID = root.getv("cmpid", "");
    if(cmpID.empty())
        return 1;

    int      authtp;
    string   userDes;
    int      status;
    string   fileID;
    Tokens   picInfo;
    long     authTM;
    string   authnum;
    string   reason;

    char     sql[256];
    char     result[150];
    MySql* psql = CREATE_MYSQL;
    sprintf(sql,"select UNIX_TIMESTAMP(CREATE_DT) as authtm,DATA_TYPE,USER_DES,INFO_ID,STATUS,ID_NO,CAUSE from t41_comp_file where COMPANY_KEY = '%s'",cmpID.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	while(psql->NextRow())
    {
		READMYSQL_INT(DATA_TYPE,authtp,COMP_DEFINE_DATATYPE);

		READMYSQL_STR(INFO_ID,result);
		fileID = string(result);

		READMYSQL_64(authtm,authTM,0);

		READMYSQL_STR(USER_DES,result);
		userDes = string(result);

		READMYSQL_STR(ID_NO,result);
		authnum = string(result);
		READMYSQL_STR(CAUSE,result);
		reason = string(result);

		READMYSQL_INT(STATUS,status,COMP_VERIFY_STATUS);

		char strPic[256];
		if(authtp != COMP_DEFINE_DATATYPE && userDes.empty())
			sprintf(strPic,"{issystem:1,authtp:\"%d\",picid:\"%s\",nst:%d,authtm:%ld,authnum:\"%s\",reason:\"%s\"}",
			authtp,fileID.c_str(),status,authTM,authnum.c_str(),reason.c_str());
		else
			sprintf(strPic,"{issystem:0,authtp:\"%s\",picid:\"%s\",nst:%d,authtm:%ld,authnum:\"%s\",reason:\"%s\"}",
			userDes.c_str(),fileID.c_str(),status,authTM,authnum.c_str(),reason.c_str());
		picInfo.push_back(string(strPic));
	}

    string Pics = ToStr(picInfo,string(","));
    out<<"{pics:["<<Pics<<"]}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght  获取公司LOGO接口
int AccountSvc::getCompanyLogo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getCompanyLogo]bad format:", jsonString, 1);
    string cmpID = root.getv("cmpid", "");
    if(cmpID.empty())
        return 1;

    string strPicID;
    char sql[256];
    char result[256];
    MySql* psql = CREATE_MYSQL;

    sprintf(sql,"select NEW_LOGOID from t41_company where Company_Key = '%s'",cmpID.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{
		READMYSQL_STR(NEW_LOGOID,result);
		strPicID = string(result);
	}

    Tokens tokPicID = StrSplit(strPicID,string(";"));
    strPicID = ToStr(tokPicID,string("|"));

    out<<"{logoids:\""<<strPicID<<"\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght  获取公司简介信息接口 测试过
int AccountSvc::getCompanySummary(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getCompanySummary]bad format:", jsonString, 1);
	string cmpID = root.getv("cmpid", "");
	if(cmpID.empty())
		return 1;

    string cmpSvcID;
    string cmpPorts;
    string cmpKey;
    string cmpAdv;
    string cmpBrief;
    string cmpAdvlocal;
    string cmpBrieflocal;

    char sql[11*1024];
    MySql* psql = CREATE_MYSQL;
    char result[260];
    char text[5*1024];
    int  port;
    sprintf(sql,"select t1.ServiceType,t2.PORTID,t1.KEY_WORD,t1.ADV_DETAIL,t1.Detail,t1.ADV_DETAIL_LOCATION,t1.DETAIL_LOCATION\
				from t41_company t1 LEFT JOIN t41_comp_port t2 on t1.Company_Key = t2.COMPANY_KEY where t1.Company_Key = '%s'",cmpID.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	bool DataLock = false;
	Tokens tok_Ports;
	while(psql->NextRow())
	{
		if(!DataLock)//
		{
			READMYSQL_STR(ServiceType,result);
			cmpSvcID = string(result);

			READMYSQL_STR(KEY_WORD,result);
			cmpKey = string(result);

			READMYSQL_STR(ADV_DETAIL,text);
			cmpAdv = string(text);

			READMYSQL_STR(Detail,text);
			cmpBrief = string(text);

			READMYSQL_STR(ADV_DETAIL_LOCATION,text);
			cmpAdvlocal = string(text);
			if (cmpAdvlocal.empty())
				cmpAdvlocal = cmpAdv;

			READMYSQL_STR(DETAIL_LOCATION,text);
			cmpBrieflocal = string(text);

			DataLock = true;
		}

		//
		READMYSQL_INT(PORTID,port,-1);
		if(port != -1)
		{
			char des[20];
			ACE_OS::itoa(port,des,10);
			tok_Ports.push_back(string(des));
		}
	}
	Tokens tok_Svcs;
	if(!tok_Ports.empty())
	{
		cmpPorts = ToStr(tok_Ports,string("|"));
		cmpPorts = "|" + cmpPorts + "|";
	}
	else
		cmpPorts = "";

    sprintf(sql,"{sts:\"%s\",ports:\"%s\",keys:\"%s\",adv_e:\"%s\",brief_e:\"%s\",adv_l:\"%s\",brief_l:\"%s\"}",
            cmpSvcID.c_str(),cmpPorts.c_str(),cmpKey.c_str(),cmpAdv.c_str(),cmpBrief.c_str(),cmpAdvlocal.c_str(),cmpBrieflocal.c_str());
    out<<sql;

    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght  更新公司简介信息接口
int AccountSvc::updCompanySummary(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::updCompanySummary]bad format:", jsonString, 1);
	string cmpID = root.getv("cmpid", "");
	if(cmpID.empty())
		return 1;

    string cmpSvcID;
    string cmpPorts;
    string cmpKey;
    string cmpAdv;
    string cmpBrief;
    string cmpAdvlocal;
    string cmpBrieflocal;
    cmpSvcID = root.getv("sts", "");
    cmpPorts = root.getv("ports", "");
    cmpKey   = root.getv("keys", "");
    cmpAdv   = root.getv("adv_e", "");
    cmpAdvlocal   = root.getv("adv_l", "");
    cmpBrief = root.getv("brief_e", "");
    cmpBrieflocal = root.getv("brief_l", "");

    Tokens r;
    char sql[1024*6];
    MySql* psql = CREATE_MYSQL;
    sprintf(sql,"update t41_company set ServiceType = '%s',KEY_WORD = '%s',ADV_DETAIL = '%s',Detail = '%s',ADV_DETAIL_LOCATION='%s', DETAIL_LOCATION='%s' where Company_Key = '%s'",
				cmpSvcID.c_str(),cmpKey.c_str(),cmpAdv.c_str(),cmpBrief.c_str(),cmpAdvlocal.c_str(),cmpBrieflocal.c_str(),cmpID.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    //先删除，在插入
    sprintf(sql,"delete from t41_comp_port where COMPANY_KEY = '%s'",cmpID.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    if(!cmpPorts.empty())
    {
        r = StrSplit(cmpPorts,string("|"));
        Tokens::iterator iter = r.begin();
        int EmptyNum = 0;
        for(; iter!=r.end(); iter++)
            if((*iter).empty())
                ++EmptyNum;
        iter = r.begin();
        if(!iter->empty()||EmptyNum != 1||r.size()<=1)
			RELEASE_MYSQL_RETURN(psql, 1);

        r.erase(iter);
        iter = r.begin();
        stringstream ss;
        ss<<"insert into t41_comp_port (company_key,portid) values ";
        while(iter!=r.end())
        {
            int portID = ACE_OS::atoi((*iter).c_str());
            sprintf(sql,"('%s',%d)",cmpID.c_str(),portID);
            if(iter == r.begin())
                ss<<sql;
            else
                ss<<","<<sql;
            iter++;
        }
        CHECK_MYSQL_STATUS(psql->Execute(ss.str().c_str())>=0, 3);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
int AccountSvc::getCompanyPersonal(const char* pUid, const char* jsonString, std::stringstream& out)
{
    int nworks = 0;
    char ntype[10] = "";
    MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql, "SELECT COUNT(1) AS nworks FROM t41_comp_contacts AS t1, t41_comp_contacts AS t2 WHERE t1.company_key = t2.company_key AND t1.user_id = '%s' AND t2.flag = 1", pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()
	if (psql->NextRow())
		READMYSQL_INT(nworks, nworks, 0);

	sprintf(sql, "SELECT usertype FROM t41_comp_contacts WHERE user_id = '%s'", pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()
	if (psql->NextRow())
		READMYSQL_STR(usertype, ntype);

    out << "{ntype:" << ntype << ",nworks:" << nworks << "}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{cmpid:"2011120800", uid:"blm "}
int AccountSvc::getCompanyCowoker(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getCompanyCowoker]bad format:", jsonString, 1);
    string cmpid =	root.getv("cmpid", "");
    string usrid   =	root.getv("uid", "");

    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;
    CWVec cwVec;

    sprintf(sql, "SELECT t00_user.firstname, t00_user.lastname, t00_user.nickname, t41_comp_contacts.user_id,  t41_comp_contacts.Isadmin, t41_comp_contacts.DEP_ID,											\
					t41_comp_department.DEP_NAME, t00_user.MOBILE, t00_user.email																															\
					FROM t41_comp_contacts LEFT JOIN t00_user ON t41_comp_contacts.USER_ID = t00_user.USER_ID																								\
					LEFT JOIN t41_comp_department ON t41_comp_contacts.DEP_ID = t41_comp_department.DEP_ID	AND t41_comp_contacts.COMPANY_KEY = t41_comp_department.COMPANY_KEY		\
					WHERE t41_comp_contacts.COMPANY_KEY = '%s' AND t41_comp_contacts.Flag = 1", cmpid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{
		CompanyCoWorker coWorker;

		READMYSQL_STR(firstname, coWorker.firstna);
		READMYSQL_STR(lastname, coWorker.lastna);
		READMYSQL_STR(nickname, coWorker.nickna);
		READMYSQL_STR(user_id, coWorker.uid);
		READMYSQL_INT(Isadmin, coWorker.badmin, 1);
		READMYSQL_STR(DEP_ID, coWorker.departid);
		READMYSQL_STR(DEP_NAME, coWorker.departna);
		READMYSQL_STR(MOBILE, coWorker.tel);
		READMYSQL_STR(email, coWorker.email);

		cwVec.push_back(coWorker);
	}

    out << "[";
    for (int i = 0; i < (int)cwVec.size(); i++)
    {
        if (i)
            out << cwVec[i].toJson2();
        else
            out << cwVec[i].toJson1();
    }
    out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"2011120800",yuid:"blm，gsf@boloomo.com",did: "blmusr"}
int AccountSvc::inviteCoworker(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::inviteCoworker]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");
    string yuid = root.getv("yuid", "");
    string did = root.getv("did", "");

    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;
    char yUid[1024] = "";
    char yEmail[1024] = "";
    int lU = 0;
    int lE = 0;

    Tokens succ, invited, hcomp, invalid;
    Tokens inviteIDOrEmail = StrSplit(yuid, ",");
    for (Tokens::iterator tIter = inviteIDOrEmail.begin(); tIter != inviteIDOrEmail.end(); tIter++)
    {
        if ((*tIter).find('@') != -1)
        {
            if (lE)
                lE += sprintf (yEmail + lE, ",'%s'", (*tIter).c_str());
            else
                lE = sprintf (yEmail, "'%s'", (*tIter).c_str());
        }
        else
        {
            if (lU)
                lU += sprintf (yUid + lU, ",'%s'", (*tIter).c_str());
            else
                lU = sprintf (yUid, "'%s'", (*tIter).c_str());
        }
    }

	char cmpid[64] = "";
	char isAdmin[10] = "";
	sprintf (sql, "SELECT company_key, isadmin FROM t41_comp_contacts WHERE user_id = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{
		READMYSQL_STR(company_key, cmpid);
		READMYSQL_STR(isadmin, isAdmin);
	}

	if (strlen(yUid) && strlen(yEmail))
		sprintf (sql, "SELECT user_id, email FROM t00_user WHERE user_id IN (%s) OR email IN (%s) ", yUid, yEmail);
	else if (strlen(yUid))
		sprintf (sql, "SELECT user_id, email FROM t00_user WHERE user_id IN (%s)", yUid);
	else if (strlen(yEmail))
		sprintf (sql, "SELECT user_id, email FROM t00_user WHERE email IN (%s) ", yEmail);
	else
		RELEASE_MYSQL_RETURN(psql, 1);

	StrStrMap uMap;
	StrSet mailSet;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql ->NextRow())
	{
		char usrid[30];
		char email[64];
		READMYSQL_STR(user_id, usrid);
		READMYSQL_STR(email, email);
		uMap.insert(make_pair(string(usrid),string(email)));
		mailSet.insert(string(email));
	}

	/*增加判断没法读取email 和 userid 的判断*/
	if (uMap.empty())
	{
		out << "{eid:0,succ:\"\",invited:\"\",hcomp:\"\",invalid:\"" << yuid << "\"}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}
	if (uMap.size() < inviteIDOrEmail.size())
	{
		for (Tokens::iterator iTer = inviteIDOrEmail.begin(); iTer != inviteIDOrEmail.end(); iTer++)
		{
			if (uMap.find((*iTer)) == uMap.end() && mailSet.find((*iTer)) == mailSet.end())
			{
				invalid.push_back((*iTer));
			}
		}
	}

	char upSql[1024*100] = "";
	int len = 0;
	//查出已发送公司邀请或者已有公司的用户
	for (StrStrMapIter iTer = uMap.begin(); iTer != uMap.end(); iTer++)
	{
		if (len)
			len += sprintf (upSql + len, ", '%s'", iTer->first.c_str());
		else
			len += sprintf (upSql, "SELECT user_id, flag FROM t41_comp_contacts WHERE user_id IN ('%s'", iTer->first.c_str());
	}
	sprintf (upSql + len, ") AND (flag = '1' OR (company_key = '%s' AND (apply_type = 1 or apply_type = 2)))", cmpid);

	//删除已发送邀请或者加入其它公司的用户
	CHECK_MYSQL_STATUS(psql->Query(upSql), 3);
	while(psql->NextRow())
	{
		StrStrMapIter iTer;
		char usrid[30];
		char flag[2];
		READMYSQL_STR(user_id, usrid);
		READMYSQL_STR(flag, flag);
		if ((iTer = uMap.find(string(usrid))) != uMap.end())
			uMap.erase(iTer);
		if (flag[0] == '1')
			hcomp.push_back(usrid);
		else
			invited.push_back(usrid);
	}

	if (uMap.empty())
	{
		out << "{eid:0,succ:\"\",invited:\"";
		for (Tokens::iterator iTer = invited.begin(); iTer != invited.end(); iTer++)
		{
			if (iTer == invited.begin())
				out << (*iTer);
			else
				out << "," << (*iTer);
		}
		out << "\",hcomp:\"";
		for (Tokens::iterator iTer = hcomp.begin(); iTer != hcomp.end(); iTer++)
		{
			if (iTer == hcomp.begin())
				out << (*iTer);
			else
				out << "," << (*iTer);
		}
		out << "\",invalid:\"";
		for (Tokens::iterator iTer = invalid.begin(); iTer != invalid.end(); iTer++)
		{
			if (iTer == invalid.begin())
				out << (*iTer);
			else
				out << "," << (*iTer);
		}
		out << "\"}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	len = 0;
	int isAdFlag = (strcmp(isAdmin,"1")?2:1);
	int updateT = (int)time(NULL);
	for (StrStrMapIter iTer = uMap.begin(); iTer != uMap.end(); iTer++)
	{
		succ.push_back(iTer ->first.c_str());
		if (iTer != uMap.begin())
			len += sprintf (upSql + len, ",('%s', '%s', %d, 0, 2, '%s', FROM_UNIXTIME(%d), '%s')", iTer ->first.c_str(), cmpid, isAdFlag, did.c_str(), updateT, uid.c_str());
		else
			len += sprintf (upSql, "INSERT INTO t41_comp_contacts (USER_ID, COMPANY_KEY, APPLY_TYPE, FLAG, ISADMIN, dep_id, LAST_UPD_DT, INVITER) VALUES ('%s', '%s', %d, 0, 2, '%s', FROM_UNIXTIME(%d), '%s')",
			iTer ->first.c_str(), cmpid, isAdFlag, did.c_str(), updateT, uid.c_str());
	}
	CHECK_MYSQL_STATUS(psql->Execute(upSql)>=0, 3);

    out << "{eid:0,succ:\"";
    for (Tokens::iterator iTer = succ.begin(); iTer != succ.end(); iTer++)
    {
        if (iTer == succ.begin())
            out << (*iTer);
        else
            out << "," << (*iTer);
    }
    out << "\",invited:\"";
    for (Tokens::iterator iTer = invited.begin(); iTer != invited.end(); iTer++)
    {
        if (iTer == invited.begin())
            out << (*iTer);
        else
            out << "," << (*iTer);
    }
    out << "\",hcomp:\"";
    for (Tokens::iterator iTer = hcomp.begin(); iTer != hcomp.end(); iTer++)
    {
        if (iTer == hcomp.begin())
            out << (*iTer);
        else
            out << "," << (*iTer);
    }
    out << "\",invalid:\"";
    for (Tokens::iterator iTer = invalid.begin(); iTer != invalid.end(); iTer++)
    {
        if (iTer == invalid.begin())
            out << (*iTer);
        else
            out << "," << (*iTer);
    }
    out << "\"}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"blm", cmpid:"12345",depid:"",lang:"en"}
//return true and did not get mail
int AccountSvc::apply(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::apply]bad format:", jsonString, 1);
    string uid = root.getv("uid", "");
    string cmpid = root.getv("cmpid", "");
    string depid = root.getv("depid", "");
    string lang = root.getv("lang", "");

    int isadmin = 1;
    int flag = 0;

    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;

	sprintf (sql, "SELECT t41_company.LocalFullName, t41_company.Name FROM t41_company WHERE company_key = '%s'", cmpid.c_str());
	char localname[100];
	char name[100];
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		READMYSQL_STR(LocalFullName, localname);
		READMYSQL_STR(Name, name);
	}

	bool ctyFlg = (lang.compare("en")?true:false);
	sprintf(sql, "SELECT t00_user.email,  t41_comp_contacts.USER_ID FROM t00_user LEFT JOIN t41_comp_contacts ON t00_user.USER_ID = t41_comp_contacts.USER_ID WHERE company_key = '%s' AND isadmin = 1", cmpid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{
		isadmin = 2;   //普通用户
		char email[64] = "";
		char admin[20] = "";
		READMYSQL_STR(email, email);
		READMYSQL_STR(user_id, admin);

		char title[256];
		char content[2048];
		sprintf(title, "%s", ctyFlg?APPLY_TITLE_CN:APPLY_TITLE_EN);
		sprintf(content, ctyFlg?EMAIL_CONTAINT_CN:EMAIL_CONTAINT_EN, uid.c_str(), strlen(localname)?CodeConverter::Utf8ToGb2312(localname).c_str():CodeConverter::Utf8ToGb2312(name).c_str());

		CNotifyMsg* pMsg = new CNotifyMsg;
		pMsg->m_szType = "mail";
		pMsg->m_MailFromUser = "blm_system";
		pMsg->m_MailFromAddr = "supports@boloomo.com";
		pMsg->m_MailToUser = admin;
		pMsg->m_MailToAddr = email;
		pMsg->m_MailSubject = title;
		pMsg->m_MailMsg = content;
		pMsg->m_MailType = 5;

		if(!g_NotifyService::instance()->PutMessage(pMsg))
		{
			RELEASE_MYSQL_RETURN(psql, 2);
		}
	}
	if (isadmin == 1) 
	{
		flag = 1;			
		OnUserAddCompany(uid.c_str(), cmpid.c_str(), strlen(localname)?localname:name);
	}

	sprintf(sql, "INSERT IGNORE INTO t41_comp_contacts (USER_ID, COMPANY_KEY, APPLY_TYPE, FLAG, ISADMIN, dep_id, LAST_UPD_DT) VALUES ('%s', '%s', 0, %d, %d, '%s', FROM_UNIXTIME(%d))", uid.c_str(), cmpid.c_str(), flag, isadmin, depid.c_str(), (int)time(NULL));
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"2011120800"}
int AccountSvc::getAllSendInvite(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getAllSendInvite]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");
    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;
    SIVec siVec;

	sprintf(sql, "SELECT user_id, flag FROM t41_comp_contacts WHERE INVITER = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{
		SendInvite siInfo;
		READMYSQL_STR(user_id, siInfo.uid);
		READMYSQL_STR(flag, siInfo.nst);
		siVec.push_back(siInfo);
	}

    out << "[";
    for (int i = 0; i < (int)siVec.size(); i++)
    {
        if (i)
            out << ",{uid:\"" << siVec[i].uid << "\",nst: " << siVec[i].nst << "}";
        else
            out << "{uid:\"" << siVec[i].uid << "\",nst: " << siVec[i].nst << "}";
    }
    out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"2011120800"}
//admin
int AccountSvc::getAllUnDeal(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getAllUnDeal]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");
    char cmpid[CMPLEN];
    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;
    std::vector<std::string> uVec;

	sprintf (sql, "SELECT isadmin, company_key FROM t41_comp_contacts WHERE user_id = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char isAdmin[10] = "";
	if (psql ->NextRow())
	{
		READMYSQL_STR(isadmin, isAdmin);
		READMYSQL_STR(company_key, cmpid);
	}

	if (strcmp(isAdmin, "1"))
		RELEASE_MYSQL_RETURN(psql, 1);

	sprintf(sql, "SELECT user_id FROM t41_comp_contacts WHERE company_key = '%s' and flag = 0 AND apply_type = 0 and user_id != '%s'", cmpid, uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{
		char inviteId[20];
		READMYSQL_STR(user_id, inviteId);
		std::string inviterString = inviteId;
		uVec.push_back(inviterString);
	}

    out << "[";
    for (int i = 0; i < (int)uVec.size(); i++)
    {
        if (i)
            out << ",{uid:\"" << uVec[i] << "\"}";
        else
            out << "{uid:\"" << uVec[i] << "\"}";
    }
    out << "]";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"2011120800"}
int AccountSvc::getAllRecvInvite(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getAllRecvInvite]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");
    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;
    RIVec riVec;

	sprintf(sql, "SELECT t41_comp_contacts.INVITER, t41_comp_contacts.company_key, t41_company.LocalFullName, t41_company.name  \
				 FROM t41_comp_contacts LEFT JOIN t41_company ON t41_comp_contacts.COMPANY_KEY = t41_company.Company_Key		\
				 WHERE t41_comp_contacts.USER_ID = '%s' and (APPLY_TYPE = 1 or APPLY_TYPE = 2) and flag = '0'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{
		char companyFName[64];
		char companyName[64];
		RecvInvite riInfo;

		READMYSQL_STR(INVITER, riInfo.inviter);
		READMYSQL_STR(company_key, riInfo.companyID);
		READMYSQL_STR(LocalFullName, companyFName);
		READMYSQL_STR(name, companyName);

		if (strcmp(companyFName, ""))
			strcpy(riInfo.companyName, companyFName);
		else
			strcpy(riInfo.companyName, companyName);

		if (strlen(riInfo.inviter))
			riVec.push_back(riInfo);
	}

    out << "[";
    for (int i = 0; i < (int)riVec.size(); i++)
    {
        if (i)
            out << ",{uid:\"" << riVec[i].inviter << "\", cmpid:\"" << riVec[i].companyID << "\",cmpna:\"" <<  riVec[i].companyName << "\"}";
        else
            out << "{uid:\"" << riVec[i].inviter << "\", cmpid:\"" << riVec[i].companyID << "\",cmpna:\"" <<  riVec[i].companyName << "\"}";
    }
    out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"caiwj", cmpid:"C1106271309163016773",nst:1}
int AccountSvc::dealInvite(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::dealInvite]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");
    string cmpid = root.getv("cmpid", "");
    int nst = root.getv("nst", 0);

    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;

	if (nst == 0)
		nst = 3;

	sprintf(sql, "UPDATE t41_comp_contacts SET flag = %d WHERE user_id = '%s' AND company_key = '%s' ", nst, uid.c_str(), cmpid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	if (nst == 1)
	{
		sprintf (sql, "UPDATE t41_comp_contacts SET flag = '3' WHERE user_id = '%s' AND company_key <> '%s'", uid.c_str(), cmpid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"blm", cmpid:"12345", yid:"blm1,blm2",nst:1}
//admin
int AccountSvc::acceptApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::acceptApply]bad format:", jsonString, 1);

    string uid   =	root.getv("uid", "");
    string cmpid = root.getv("cmpid", "");
    string yid = root.getv("yid", "");
    int nst = root.getv("nst", 0);

    char sql[1024] = "";
    char yUid[1024] = "";
    MySql* psql = CREATE_MYSQL;

	sprintf (sql, "SELECT isadmin FROM t41_comp_contacts WHERE user_id = '%s'", uid.c_str());
	char isAdmin[10]="";
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql ->NextRow())
	{
		READMYSQL_STR(isadmin, isAdmin);
	}

	if (strcmp(isAdmin, "1"))
		RELEASE_MYSQL_RETURN(psql, 1);

	int l = 0;
	Tokens inviteID = StrSplit(yid, ",");
	for (Tokens::iterator tIter = inviteID.begin(); tIter != inviteID.end(); tIter++)
	{
		if (l)
			l += sprintf (yUid + l, ",'%s'", (*tIter).c_str());
		else
			l = sprintf (yUid, "'%s'", (*tIter).c_str());

		//加入公司群
		if (nst)	//zhuxj modify, it did not judge if accept or not before
			OnUserAddCompany((*tIter).c_str(), cmpid.c_str(), "");		
	}

	if (nst == 0)
		nst = 3;

	sprintf(sql, "UPDATE t41_comp_contacts SET flag = %d WHERE user_id IN (%s) AND company_key = '%s' AND flag = 0", nst, yUid, cmpid.c_str(), nst);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"blm", cmpid:"12345"}
int AccountSvc::quitCompany(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::quitCompany]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");
    string cmpid = root.getv("cmpid", "");

    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;

	sprintf (sql, "SELECT COUNT(1) AS num FROM t41_comp_contacts WHERE company_key = '%s' and (flag = 0 or flag = 1)", cmpid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()
	int num=-1;
	if (psql->NextRow())
	{
		READMYSQL_INT(num, num, -1);
	}

	if (num == 1)
	{
		sprintf (sql, "DELETE FROM t41_comp_contacts WHERE company_key = '%s' AND user_id = '%s'", cmpid.c_str(), uid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else
	{
		sprintf (sql, "SELECT isadmin FROM t41_comp_contacts WHERE user_id = '%s'", uid.c_str());
		char isAdmin[10]="";
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if (psql ->NextRow())
		{
			READMYSQL_STR(isadmin,isAdmin);
		}

		if (strcmp(isAdmin, "1"))
		{
			sprintf (sql, "DELETE FROM t41_comp_contacts WHERE company_key = '%s' AND user_id = '%s'", cmpid.c_str(), uid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
		else
		{
			sprintf(sql, "SELECT COUNT(1) AS num FROM t41_comp_contacts WHERE company_key = '%s' AND isadmin = '1'", cmpid.c_str());
			int adminNum=-1;
			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
			if (psql->NextRow())
			{
				READMYSQL_INT(num, adminNum, -1);
			}

			if (adminNum == 1)
			{
				out << "{eid:100}";
				RELEASE_MYSQL_RETURN(psql, 0);
			}
			else
			{
				sprintf (sql, "DELETE FROM t41_comp_contacts WHERE company_key = '%s' AND user_id = '%s'", cmpid.c_str(), uid.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
			}
		}
	}

    //退出公司群
    OnUserQuitCompany(pUid, cmpid.c_str());

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"blm", cmpid:"12345",yid:"12345,34566"}
int AccountSvc::setAdmin(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::setAdmin]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");
    string cmpid = root.getv("cmpid", "");
    string yid = root.getv("yid", "");

    char sql[1024] = "";
    char yUid[1024] = "";
    MySql* psql = CREATE_MYSQL;

	sprintf (sql, "SELECT isadmin FROM t41_comp_contacts WHERE user_id = '%s'", uid.c_str());
	char isAdmin[10]="";
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql ->NextRow())
	{
		READMYSQL_STR(isadmin, isAdmin);
	}

	if (strcmp(isAdmin, "1"))
		RELEASE_MYSQL_RETURN(psql, 1);

	int l = 0;
	Tokens inviteID = StrSplit(yid, ",");
	for (Tokens::iterator tIter = inviteID.begin(); tIter != inviteID.end(); tIter++)
	{
		if (l)
			l += sprintf (yUid + l, ",'%s'", (*tIter).c_str());
		else
			l = sprintf (yUid, "'%s'", (*tIter).c_str());
	}

	sprintf(sql, "UPDATE t41_comp_contacts SET isadmin = 1 WHERE user_id IN (%s) AND company_key = '%s'", yUid, cmpid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"blm" ,yid:"12345，67890"}
//admin
int AccountSvc::kickAss(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::kickAss]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");
    string yid = root.getv("yid", "");

    char sql[1024] = "";
    char yUid[1024] = "";
    MySql* psql = CREATE_MYSQL;

	sprintf (sql, "SELECT isadmin FROM t41_comp_contacts WHERE user_id = '%s'", uid.c_str());
	char isAdmin[10]="";
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		READMYSQL_STR(isadmin, isAdmin);
	}

	if (strcmp(isAdmin, "1"))
		RELEASE_MYSQL_RETURN(psql, 1);

	int l = 0;
	Tokens inviteID = StrSplit(yid, ",");
	for (Tokens::iterator tIter = inviteID.begin(); tIter != inviteID.end(); tIter++)
	{
		if (l)
			l += sprintf (yUid + l, ",'%s'", (*tIter).c_str());
		else
			l = sprintf (yUid, "'%s'", (*tIter).c_str());
	}

	sprintf(sql, "DELETE FROM t41_comp_contacts WHERE user_id IN (%s)", yUid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

// 暂不实现
int AccountSvc::getVessel(const char* pUid, const char* jsonString, std::stringstream& out)
{
    out << "{eid:1, emsg:\"not implements\"}";
    return 0;
}

// 暂不实现
int AccountSvc::updVessel(const char* pUid, const char* jsonString, std::stringstream& out)
{
    out << "{eid:1, emsg:\"not implements\"}";
    return 0;
}

// 暂不实现
int AccountSvc::delVessel(const char* pUid, const char* jsonString, std::stringstream& out)
{
    out << "{eid:1, emsg:\"not implements\"}";
    return 0;
}

// 暂不实现
int AccountSvc::getPort(const char* pUid, const char* jsonString, std::stringstream& out)
{
    out << "{eid:1, emsg:\"not implements\"}";
    return 0;
}

//{pid:"",name:"",uid:"",lon:"",lat:"",iso:"",tmzone:"",website:""}
int AccountSvc::updPort(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getProduct]bad format:", jsonString, 1);
    string pid = root.getv("pid", "");
	string name = root.getv("name", "");
	string uid = root.getv("uid", "");
	string lon = root.getv("lon", "");
	string lat = root.getv("lat", "");
	string iso = root.getv("iso", "");
	string tmzone = root.getv("tmzone", "");
	string website = root.getv("website", "");

	char gmt0now[20];
	GmtNow(gmt0now);

    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;
	time_t t = time(0);
	struct tm *ptr = localtime(&t);
	if (pid.empty())
	{
		sprintf (sql, "SELECT COUNT(1) AS num FROM blmdtmaintain.t41_port WHERE portid LIKE '%d%02d%02d%%'", ptr->tm_year+1900, ptr->tm_mon+1, ptr->tm_mday);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		int num = 0;
		if (psql->NextRow())
		{
			READMYSQL_INT(num, num, 0);
		}
		num++;
		sprintf (sql, "INSERT INTO blmdtmaintain.t41_port (PortID, user_id, `NAME`, ISO3, LATITUDE, LONGITUDE, TIMEZONE, WEBSITE, LAST_UPD_DT, flag)\
					  VALUES ('%d%02d%02d%d', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '0')", ptr->tm_year+1900, ptr->tm_mon+1, ptr->tm_mday, num,
					  uid.c_str(), name.c_str(), iso.c_str(), SqlReplace(lat).c_str(), SqlReplace(lon).c_str(), tmzone.c_str(), website.c_str(), gmt0now);
	}
	else
	{
		sprintf (sql, "UPDATE blmdtmaintain.t41_port SET user_id = '%s', `NAME` = '%s', ISO3 = '%s', LATITUDE = '%s', LONGITUDE = '%s', \
					  TIMEZONE = '%s', WEBSITE = '%s', LAST_UPD_DT = '%s' WHERE portid = '%s'", uid.c_str(), name.c_str(), iso.c_str(), SqlReplace(lat).c_str(), SqlReplace(lon).c_str(), tmzone.c_str(), website.c_str(), gmt0now);
	}
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	RELEASE_MYSQL_RETURN(psql, 0);
    return 0;
}

// 暂不实现
int AccountSvc::delPort(const char* pUid, const char* jsonString, std::stringstream& out)
{
    out << "{eid:1, emsg:\"not implements\"}";
    return 0;
}

//caiwj
//{uid:"blm"}
int AccountSvc::getProduct(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getProduct]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");

    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;
    out << "[]";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//caiwj
//{uid:"blm"}
int AccountSvc::getJifen(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getJifen]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");

    char sql[1024] = "";
    JiFenVEC jiFenVec;
    MySql* psql = CREATE_MYSQL;

    sprintf (sql, "SELECT num, UNIX_TIMESTAMP(change_dt) AS changedt, UNIX_TIMESTAMP(expire_dt) AS expiredt FROM t00_user_point_his WHERE user_id = '%s'", uid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while (psql->NextRow())
	{
		JIFEN jiFen;
		READMYSQL_INT(num, jiFen.num, 0);
		READMYSQL_INT(changedt, jiFen.change_dt, 0);
		READMYSQL_INT(expiredt, jiFen.expire_dt, 0);

		jiFenVec.push_back(jiFen);
	}

    out << "[";
    for (JiFenVEC::iterator iTer = jiFenVec.begin(); iTer != jiFenVec.end(); iTer++)
    {
        out << (iTer == jiFenVec.begin()?"":",");
        out << "{num:\"" << (*iTer).num << "\",src: \"" << (*iTer).src << "\" ,validdt:\"" << (*iTer).change_dt << "\",invaliddt:\"" << (*iTer).expire_dt << "\"}";
    }
    out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//caiwj
//{orderid: "",uid: "blm1",pid:"blm1",unitprice:10.2,num:5,totalprice:100,discount:95,points:100}
int AccountSvc::updOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::updOrder]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string orderid   =	root.getv("orderid", "");
    string pid = root.getv("pid", "");
    double unitprice = root.getv("unitprice", 0.0);
    int num = root.getv("num", 0);
    double totalprice = root.getv("totalprice", 0.0);
    int discount = root.getv("discount", 0);
    int point = root.getv("point", 0);

    char sql[1024] = "";
    MySql* psql = CREATE_MYSQL;

    out << "{eid:1, emsg:\"not implements\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//caiwj
int AccountSvc::allProduct(const char* pUid, const char* jsonString, std::stringstream& out)
{
    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "SELECT product_id, name_en, name_cn FROM t02_product";
    ProductNameVEC pNameVec;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while (psql->NextRow())
	{
		PRODUCTNAME pName;
		READMYSQL_STR(product_id, pName.id);
		READMYSQL_STR(name_en, pName.name_en);
		READMYSQL_STR(name_cn, pName.name_cn);

		pNameVec.push_back(pName);
	}

    out << "[";
    for (ProductNameVEC::iterator iTer = pNameVec.begin(); iTer != pNameVec.end(); iTer++)
    {
        out << (iTer == pNameVec.begin()?"":",");
        out << "{pid:\"" << (*iTer).id << "\",pna_en:\"" << (*iTer).name_en << "\",pna_cn:\"" << (*iTer).name_cn << "\"}";
    }
    out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

// 暂不实现
int AccountSvc::productDesc(const char* pUid, const char* jsonString, std::stringstream& out)
{
    out << "{eid:1, emsg:\"not implements\"}";
    return 0;
}

//zhuxj
//{uid:"zhuxj"}
int AccountSvc::getUserOrders(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getUserOrders]bad format:", jsonString, 1);
    string uid   =	root.getv("uid", "");
    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";

    orderInfoVEC oInfoVec;
    sprintf (sql, "SELECT t02_order.order_id, t02_order.STATUS_CD, UNIX_TIMESTAMP(t02_order.ORDER_TIME) AS ORDER_TIME, UNIX_TIMESTAMP(t02_order.PAY_TIME) AS PAY_TIME, t02_product_scheme.PRICE,\
				UNIX_TIMESTAMP(t02_order.EXPIRE_TIME) AS EXPIRE_TIME, t02_product.NAME_CN, t02_product.NAME_EN, t02_product_scheme.CURRENCY,  t02_product_scheme.LIMIT_AMOUNT \
				FROM t02_product_scheme RIGHT JOIN t02_order ON t02_product_scheme.SCHEME_ID = t02_order.SCHEME_ID \
				LEFT JOIN t02_product ON t02_product_scheme.PRODUCT_ID = t02_product.PRODUCT_ID \
				WHERE t02_order.USER_ID = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while (psql->NextRow())
	{
		MYORDERINFO t_myorderinfo;
		orderInfoVEC::iterator iTer;

		READMYSQL_STR(order_id, t_myorderinfo.order_id);
		READMYSQL_INT(STATUS_CD, t_myorderinfo.status, -1);
		READMYSQL_STR(NAME_CN, t_myorderinfo.pro_name);
		READMYSQL_DOUBLE(PRICE, t_myorderinfo.price, 0.0);
		READMYSQL_DOUBLE(LIMIT_AMOUNT, t_myorderinfo.amount, 0.0);
		READMYSQL_STR(CURRENCY, t_myorderinfo.currency);
		READMYSQL_INT(ORDER_TIME, t_myorderinfo.order_time, 0);
		READMYSQL_INT(PAY_TIME, t_myorderinfo.pay_time, 0);
		READMYSQL_INT(EXPIRE_TIME, t_myorderinfo.expire_time, 0);

		oInfoVec.push_back(t_myorderinfo);
	}

    out << "[";
    for (orderInfoVEC::iterator iTer = oInfoVec.begin(); iTer != oInfoVec.end(); iTer++)
    {
        out << (iTer == oInfoVec.begin()?"":",");
        out << "{id:\"" << (*iTer).order_id << "\", nst:" << (*iTer).status << ", na:\"" << (*iTer).pro_name << "\", dbunit:" << (*iTer).price << ",num:" << (*iTer).amount
            << ",dbprice:" << (*iTer).price * (*iTer).amount << ",nmoney:" << (*iTer).currency << ",dt:" << (*iTer).order_time << ",paydt:" << ((*iTer).pay_time?(*iTer).pay_time:(*iTer).expire_time)<< "}";
    }
    out << "]";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{pid:"blm",paratype: "1|2",para: "pa10,pa11|pa20,pa21"}
int AccountSvc::updateProductParam(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getUserOrders]bad format:", jsonString, 1);

    string uid   =	root.getv("uid", "");

    MySql* psql = CREATE_MYSQL;

    char sql[1024] = "";
    out << "{eid:1, emsg:\"not implements\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//conght
int AccountSvc::getAllUserAuthType(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getAllUserAuthType]bad format:", jsonString, 1);
    int NTP   =	root.getv("ntp", 0);

    char sql[1024] = "";
    char result[50] = "";
    MySql* psql = CREATE_MYSQL;
    Tokens Ntype;
    if(NTP == 0)
    {
        sprintf (sql, "SELECT  DISTINCT USER_DES FROM t00_user_verify_info WHERE USER_DES!= ''");
        CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		while (psql->NextRow())
		{
			READMYSQL_STR(USER_DES,result);
			Ntype.push_back(result);
		}
    }

    else if(NTP == 1)
    {
        sprintf (sql, "SELECT  DISTINCT USER_DES FROM t41_comp_file WHERE USER_DES!= ''");
        CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		while (psql->NextRow())
		{
			READMYSQL_STR(USER_DES,result);
			Ntype.push_back(result);
		}
    }
    out << "[";
    Tokens::iterator iter = Ntype.begin();
    while(iter != Ntype.end())
    {
        if(iter == Ntype.begin())
            out<< "\"" << (*iter) << "\"";
        else
            out<< "," << "\""<<(*iter) << "\"";
        ++iter;
    }
    out << "]";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",seq:"",nn:"",stdt:13000,edt:13000,ntp:0,reviewer:"",rstdt:13000,redt:13000,pn:1}
int AccountSvc::applyVerifyPersonal(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::applyVerifyPersonal]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string seq = root.getv("seq", "");
    string nn = root.getv("nn", "");
    string stdt = root.getv("stdt", "");
    string edt = root.getv("edt", "");
    string ntp = root.getv("ntp", "");
    string reviewer = root.getv("reviewer", "");
    string rstdt = root.getv("rstdt", "");
    string redt = root.getv("redt", "");
    string pnString = root.getv("pn", "");

    int pn = atoi(pnString.c_str());
	if(pn <= 0)
		return 1;

    MySql* psql = CREATE_MYSQL;

    int len = 0;
    char sql[1024 * 4] = "";
    bool conFlg = false;

    len = sprintf(sql, "SELECT t1.user_id, t1.seq, t2.nickname, t1.status, UNIX_TIMESTAMP(t1.update_dt) AS applydt, UNIX_TIMESTAMP(t1.review_dt) AS reviewdt, t1.reviewer, t1.cause FROM t00_user_verify_info AS t1 LEFT JOIN t00_user AS t2 ON t1.user_id = t2.user_id ");
    if (uid.length())
    {
        len += sprintf (sql + len, "WHERE t1.user_id = '%s' ", uid.c_str());
        conFlg = true;
    }

    if (seq.length())
    {
        len += sprintf (sql + len, conFlg?"AND  t1.seq = '%s' ":"WHERE t1.seq = '%s' ", seq.c_str());
        conFlg = true;
    }

    if (nn.length())
    {
        len += sprintf (sql + len, conFlg?"AND  t2.nickname like '%%%s%%' ":"WHERE t2.nickname like '%%%s%%' ", nn.c_str());
    }

    if (stdt.length())
    {
        len += sprintf (sql + len, conFlg?"AND  UNIX_TIMESTAMP(t1.update_dt) >= '%s' ":"WHERE UNIX_TIMESTAMP(t1.update_dt) >= '%s' ", stdt.c_str());
        conFlg = true;
    }

    if (edt.length())
    {
        len += sprintf (sql + len, conFlg?"AND  UNIX_TIMESTAMP(t1.update_dt) <= '%s' ":"WHERE UNIX_TIMESTAMP(t1.update_dt) <= '%s' ", edt.c_str());
        conFlg = true;
    }

    if (ntp.length())
    {
        if (ntp[0] == '0')
			len += sprintf (sql + len, conFlg?"AND  (t1.status = '%s' or t1.status  IS NULL ) ":"WHERE (t1.status = '%s'  or t1.status  IS NULL ) ", ntp.c_str());
		else
			len += sprintf (sql + len, conFlg?"AND  t1.status = '%s' ":"WHERE t1.status = '%s' ", ntp.c_str());
        conFlg = true;
    }

    if (reviewer.length())
    {
        len += sprintf (sql + len, conFlg?"AND t1.reviewer = '%s' ":"WHERE t1.reviewer = '%s' ", reviewer.c_str());
        conFlg = true;
    }

    if (rstdt.length())
    {
        len += sprintf (sql + len, conFlg?"AND  UNIX_TIMESTAMP(t1.review_dt) >= '%s' ":"WHERE UNIX_TIMESTAMP(t1.review_dt) >= '%s' ", rstdt.c_str());
        conFlg = true;
    }

    if (redt.length())
    {
        len += sprintf (sql + len, conFlg?"AND  UNIX_TIMESTAMP(t1.review_dt) <= '%s' ":"WHERE UNIX_TIMESTAMP(t1.review_dt) <= '%s' ", redt.c_str());
        conFlg = true;
    }

    sprintf (sql + len, " ORDER BY t1.update_dt DESC ");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    int pcnt = 0;
    VerifyPInfoVEC vpInfoVec;
	VERIFYPERSONALINFO vpInfo;
	while (psql->NextRow())
	{
		READMYSQL_STR(user_id, vpInfo.uid);//
		READMYSQL_INT(seq, vpInfo.seq, -1);//
		READMYSQL_STR(nickname, vpInfo.nickname);//
		READMYSQL_INT(status, vpInfo.status, 0);//状态为null改为0
		READMYSQL_INT(applydt, vpInfo.update_dt, -1);//
		READMYSQL_INT(reviewdt, vpInfo.review_dt, -1);//
		READMYSQL_STR(reviewer, vpInfo.reviewer);//
		READMYSQL_STR(cause, vpInfo.cause);//

		vpInfoVec.push_back(vpInfo);
	}

    out << "{eid:0, total:" << (int)vpInfoVec.size() << ",pn:" << ceil(vpInfoVec.size()/10.0) << ",cpn:" << pn << ",ss:[";
    for (int i = (pn - 1) * 10, cnt = 0; i < (int)vpInfoVec.size() && cnt < 10; i++, cnt++)
    {
        out << ((i == (pn - 1) * 10)?"":",");
        out << "{uid:\"" << vpInfoVec[i].uid << "\",seq:\"" << vpInfoVec[i].seq << "\",nn:\"" << vpInfoVec[i].nickname << "\",appdt:" << vpInfoVec[i].update_dt
            << ",status:" << vpInfoVec[i].status <<",reviewer:\"" << vpInfoVec[i].reviewer << "\",rdt:" << vpInfoVec[i].review_dt <<",cause:\"" << vpInfoVec[i].cause << "\"}";
    }
    out<< "]}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",seq:"",cause:"",reviewer:"",ntp:"0",expiredt:""}
int AccountSvc::dealPersonalApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::dealPersonalApply]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string seq = root.getv("seq", "");
    string cause = root.getv("cause", "");
    string reviewer = root.getv("reviewer", "");
    string ntpString = root.getv("ntp", "");
	string expiredtString = root.getv("expiredt", "");

    int ntp = atoi(ntpString.c_str());
	int expiredt = atoi(expiredtString.c_str());

	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";

    ntp = ntp?1:2;
    sprintf (sql, "UPDATE t00_user_verify_info SET t00_user_verify_info.STATUS = %d, reviewer = '%s', review_dt = '%s', cause = '%s', EXPIRE_DT = FROM_UNIXTIME(%d)	\
					WHERE user_id = '%s' AND seq = '%s'", ntp, reviewer.c_str(), gmt0now,cause.c_str(), expiredt, uid.c_str(), seq.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	//增加往t00_user表里设置
	int verify = 0;
	sprintf (sql, "SELECT COUNT(1) as num FROM t00_user_verify_info WHERE user_id = '%s' AND t00_user_verify_info.STATUS = 1", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	int num=0;
	READMYSQL_INT(num, num, 0);
	verify = num?1:0;

	sprintf (sql, "UPDATE t00_user SET t00_user.USER_VERIFY = '%d' WHERE user_id = '%s'", verify, uid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",seq:""}
//{uid:"", na:"", birth:"", gender:"", issystem:1, cretp:1, creid:1300000, cdldt:130000, picid:"20111208,20111209", nst:0, appdt:130000, cause:""}
int AccountSvc::getPersonalDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getPersonalDetail]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string seq = root.getv("seq", "");

    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";

    int len = sprintf(sql, "SELECT t2.firstname, UNIX_TIMESTAMP(t2.birthday) as birthday, t2.gender, t1.DATA_TYPE, t1.ID_NO, t1.user_des, UNIX_TIMESTAMP(t1.EXPIRE_DT) AS cdldt, t1.obj_id, t1.status, UNIX_TIMESTAMP(t1.update_dt) AS appdt, t1.cause	\
							FROM t00_user_verify_info AS t1 LEFT JOIN t00_user AS t2 ON t1.user_id = t2.user_id WHERE t1.user_id = '%s' AND t1.seq = '%s'", uid.c_str(), seq.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    int birthday;
    int gender;
    //int issystem;
    int type;
    int expiredt;
    int status;
    int appdt;
    char name[64];
    char pcid[48];
    char user_des[32];
    char idno[32];
    char cause[128];
    int flg = false;
	while (psql->NextRow())
	{
		READMYSQL_STR(firstname, name);//
		READMYSQL_INT(birthday, birthday, -1);//
		READMYSQL_INT(gender, gender, -1);//
		READMYSQL_INT(DATA_TYPE, type, -1);//
		READMYSQL_STR(ID_NO, idno);//
		READMYSQL_STR(user_des, user_des);//
		READMYSQL_INT(cdldt, expiredt, -1);//
		READMYSQL_STR(obj_id, pcid);//
		READMYSQL_INT(status, status, -1);//
		READMYSQL_INT(appdt, appdt, -1);//
		READMYSQL_STR(cause, cause);//
	}

    out << "{eid:0, uid:\"" << uid << "\",seq:\"" << seq << "\", na:\"" << name << "\", birth:\"" << birthday << "\", gender:\"" << gender
        << "\",issystem:" << ((type==-1)?0:1) << ", cretp:" << type << ", creid:\"" << idno << "\",userDes:\"" << user_des << "\", cdldt:" << expiredt
        << ", picid:\"" << pcid << "\", nst:" << status << ", appdt:" << appdt << ", cause:\"" << cause << "\"}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{cmpid:"",seq:"", cmpna:"", appid:"", ntp:0, stdt:130000, edt:130000, reviewer:"", rstdt:"", redt:"", pn:1}
int AccountSvc::applyVerifyCompany(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::applyVerifyCompany]bad format:", jsonString, 1);

    string cmpid = root.getv("cmpid", "");		//
	string seq = root.getv("seq", "");			//
    string cmpna = root.getv("cmpna", "");		//
    string appid = root.getv("appid", "");		//
    string reviewer = root.getv("reviewer", "");	//
    string ntp = root.getv("ntp", "");			//		
    string stdt = root.getv("stdt", "");			//
    string edt = root.getv("edt", "");			//
    string rstdt = root.getv("rstdt", "");		//
    string redt = root.getv("redt", "");			//
    string pnString = root.getv("pn", "");

    int pn = atoi(pnString.c_str());
	if(pn <= 0)
		return 1;

    MySql* psql = CREATE_MYSQL;

    char sql[1024 * 4] = "";
    bool conFlg = false;
    int len = 0;

    len = sprintf(sql, "SELECT t1.company_key, t1.seq, t1.user_id, UNIX_TIMESTAMP(t1.create_dt) AS appdt, t1.status, t1.reviewer, UNIX_TIMESTAMP(t1.review_dt) AS rdt, t1.cause, t2.Name, t2.Address, t2.Website FROM t41_comp_file AS t1 LEFT JOIN t41_company AS t2 ON t1.COMPANY_KEY = t2.Company_Key ");
    if (cmpid.length())
    {
        len += sprintf (sql + len, "WHERE t1.COMPANY_KEY = '%s' ", cmpid.c_str());
        conFlg = true;
    }

    if (seq.length())
    {
        len += sprintf (sql + len, conFlg?"AND t1.seq = '%s' ":"WHERE t1.seq = '%s' ", seq.c_str());
        conFlg = true;
    }

    if (cmpna.length())
    {
        len += sprintf (sql + len, conFlg?"AND  t2.name like '%%%s%%' ":"WHERE t2.name like '%%%s%%' ", cmpna.c_str());
        conFlg = true;
    }

	if (appid.length())
	{
		len += sprintf (sql + len, conFlg?"AND  t1.user_id = '%s' ":"WHERE t1.user_id = '%s' ", appid.c_str());
		conFlg = true;
	}

	if (reviewer.length())
    {
        len += sprintf (sql + len, conFlg?"AND t1.reviewer = '%s' ":"WHERE t1.reviewer = '%s' ", reviewer.c_str());
        conFlg = true;
    }

	if (ntp.length())
    {
		if (ntp[0] == '0')
			len += sprintf (sql + len, conFlg?"AND  (t1.status = '%s' or t1.status is null) ":"WHERE (t1.status = '%s' or t1.status is null) ", ntp.c_str());
		else
			len += sprintf (sql + len, conFlg?"AND  t1.status = '%s' ":"WHERE t1.status = '%s' ", ntp.c_str());
        conFlg = true;
    }

    if (stdt.length())
    {
        len += sprintf (sql + len, conFlg?"AND  UNIX_TIMESTAMP(t1.create_dt) >= '%s' ":"WHERE UNIX_TIMESTAMP(t1.create_dt) >= '%s' ", stdt.c_str());
        conFlg = true;
    }

    if (edt.length())
    {
        len += sprintf (sql + len, conFlg?"AND  UNIX_TIMESTAMP(t1.create_dt) <= '%s' ":"WHERE UNIX_TIMESTAMP(t1.create_dt) <= '%s' ", edt.c_str());
        conFlg = true;
    }

    if (rstdt.length())
    {
        len += sprintf (sql + len, conFlg?"AND  UNIX_TIMESTAMP(t1.review_dt) >= '%s' ":"WHERE UNIX_TIMESTAMP(t1.review_dt) >= '%s' ", rstdt.c_str());
        conFlg = true;
    }

    if (redt.length())
    {
        len += sprintf (sql + len, conFlg?"AND  UNIX_TIMESTAMP(t1.review_dt) <= '%s' ":"WHERE UNIX_TIMESTAMP(t1.review_dt) <= '%s' ", redt.c_str());
        conFlg = true;
    }

    sprintf (sql + len, " ORDER BY t1.create_dt DESC ");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    CSInfoVEC csInfoVec;
	while (psql->NextRow())
	{
		COMPANYSEARCHINFO csInfo;

		READMYSQL_STR(company_key, csInfo.id);//
		READMYSQL_INT(seq, csInfo.seq, -1);//
		READMYSQL_STR(user_id, csInfo.appid);//
		READMYSQL_STR(Address, csInfo.addr);//
		READMYSQL_STR(reviewer, csInfo.reviewer);//
		READMYSQL_STR(cause, csInfo.cause);//
		READMYSQL_STR(Name, csInfo.name);//
		READMYSQL_STR(Website, csInfo.web);//
		READMYSQL_INT(rdt, csInfo.rdt, -1);//
		READMYSQL_INT(status, csInfo.ntp, 0);//状态改为0
		READMYSQL_INT(appdt, csInfo.appdt, -1);//

		csInfoVec.push_back(csInfo);
	}

    out << "{eid:0, total:" << csInfoVec.size() << ",pn:" << ceil(csInfoVec.size()/10.0) << ",cpn:" << pn << ",ss:[";
    for (int i = (pn - 1) * 10, cnt = 0; i < (int)csInfoVec.size() && cnt < 10; i++, cnt++)
    {
        out << ((i == (pn - 1) * 10)?"":",");
        out << "{cmpid:\"" << csInfoVec[i].id << "\",seq:\"" << csInfoVec[i].seq <<"\",cmpna:\"" << csInfoVec[i].name << "\", addr:\"" << csInfoVec[i].addr << "\", web:\"" << csInfoVec[i].web << "\", uid:\"" << csInfoVec[i].appid
            << "\", appdt:" << csInfoVec[i].appdt << ", status:" <<csInfoVec[i].ntp << ", reviewer:\"" << csInfoVec[i].reviewer << "\", rdt:" << csInfoVec[i].rdt << ", cause:\"" << csInfoVec[i].cause <<"\"}";
    }
    out << "]}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{cmpid:"",seq:"",cause:"",reviewer:"",ntp:"0", expiredt:"130000"}
int AccountSvc::dealCompanyApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::dealCompanyApply]bad format:", jsonString, 1);

    string cmpid = root.getv("cmpid", "");
    string seq = root.getv("seq", "");
    string cause = root.getv("cause", "");
    string reviewer = root.getv("reviewer", "");
    string ntpString = root.getv("ntp", "");
	string expiredtString = root.getv("expiredt", "");

    int ntp = atoi(ntpString.c_str());
	int expiredt = atoi(expiredtString.c_str());

	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";

    ntp = ntp?1:2;
    sprintf (sql, "UPDATE t41_comp_file SET t41_comp_file.STATUS = %d, REVIEWER = '%s', CAUSE = '%s', REVIEW_DT = '%s', EXPIRE_DT = FROM_UNIXTIME(%d) WHERE company_key = '%s' AND seq = '%s'",
             ntp, reviewer.c_str(), cause.c_str(), gmt0now,expiredt, cmpid.c_str(), seq.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	//增加设置t41_company中VERIFIED
	int verify = 0;
	sprintf (sql, "SELECT COUNT(1) AS num FROM t41_comp_file WHERE COMPANY_KEY = '%s' AND t41_comp_file.STATUS = 1", cmpid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()
	int num=0;
	if (psql->NextRow())
	{
		READMYSQL_INT(num, num, 0);
	}
	verify = num?1:0;

	sprintf (sql, "UPDATE t41_company SET VERIFIED = '%d' WHERE company_key = '%s'", verify, cmpid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{cmpid:"", seq:""}
//{eid:0, cmpid:"", seq:"", cmpna:"", addr:"", tel:"", fax:"", web:"",longi:"", lati:"", issystem:1, cretp:0, creid:"", user_des:"", cdldt:1300000, uid:"", picid:"20111208,20111209", nst:0, appdt:1300000, cause:""}
int AccountSvc::getCompanyDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::getCompanyDetail]bad format:", jsonString, 1);

    string cmpid = root.getv("cmpid", "");
    string seq = root.getv("seq", "");

    MySql* psql = CREATE_MYSQL;

    char sql[1024] = "";
    sprintf (sql, "SELECT t2.name, t2.address, t2.telno, t2.fax, t2.website, t2.Longitude, t2.Latitude, t1.data_type, t1.id_no, t1.user_id, t1.info_id, t1.status, t1.user_des, UNIX_TIMESTAMP(t1.create_dt) AS appdt, UNIX_TIMESTAMP(t1.EXPIRE_DT) AS exptredt, t1.cause		\
					FROM t41_comp_file AS t1 LEFT JOIN t41_company AS t2 ON t1.COMPANY_KEY = t2.Company_Key WHERE t1.COMPANY_KEY = '%s' AND t1.seq = '%s'", cmpid.c_str(), seq.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    int type;
    int status;
    int appdt;
    int cdldt;
	char longi[20];
	char lati[20];
    char idno[32];
    char user_des[32];
    char appid[32];
    char infoId[48];
    char cause[128];
    char name[100];
    char addr[256];
    char web[256];
    char telno[400];
    char fax[400];
    bool flg = false;

	while (psql->NextRow())
	{
		READMYSQL_STR(name, name);//
		READMYSQL_STR(address, addr);//
		READMYSQL_STR(telno, telno);//
		READMYSQL_STR(user_des, user_des);//
		READMYSQL_STR(fax, fax);//
		READMYSQL_STR(website, web); //   
		READMYSQL_STR(Longitude, longi);
		READMYSQL_STR(Latitude, lati);
		READMYSQL_INT(data_type, type, -1);//
		READMYSQL_STR(id_no, idno);//
		READMYSQL_STR(user_id, appid);//
		READMYSQL_STR(info_id, infoId);//
		READMYSQL_STR(cause, cause);//
		READMYSQL_INT(status, status, -1);//
		READMYSQL_INT(appdt, appdt, -1);//
		READMYSQL_INT(exptredt, cdldt, -1);//
	}

	char longiEW[2];
	char latiNS[2];

	longiEW[0] = 0;
	latiNS[0] = 0;

	if (strlen(longi) && longi[strlen(longi) - 2] == '"')
	{
		longiEW[0] = longi[strlen(longi) - 1];
		latiNS[0] = lati[strlen(lati) - 1];
		
		longi[strlen(longi) - 2] = '\0';
		lati[strlen(lati) - 2] = '\0';
	}

	out << "{eid:0, cmpid:\"" << cmpid << "\",seq:\"" << seq << "\", cmpna:\"" << name << "\", addr:\"" << addr << "\", tel:\"" << telno << "\", fax:\"" << fax << "\", web:\"" << web << "\",longi:\"" << longi
		<< (longiEW[0]?"\\\\\\\"":"") << (longiEW[0]?longiEW:"") << "\",lati:\"" << lati << (latiNS[0]?"\\\\\\\"":"") << (latiNS[0]?latiNS:"") << "\", issystem:" << ((type == -1)?0:1) << ", cretp:" << type << ", creid:\"" << idno 
		<< "\",userDes:\"" << user_des << "\", cdldt:" << cdldt << ", uid:\"" << appid << "\", picid:\"" << infoId << "\", nst:" << status << ", appdt:" << appdt << ", cause:\"" << cause << "\"}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",pwd:""}
int AccountSvc::interMember(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AccountSvc::interMember]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string pwd = root.getv("pwd", "");

    if (!uid.length() || !pwd.length())
        return 1;

    MySql* psql = CREATE_MYSQL;

    char sql[1024] = "";
    int num = 0;
    sprintf (sql, "SELECT COUNT(1) as num FROM t00_user WHERE user_id = '%s' AND t00_user.PASSWORD = '%s' AND usertype = 1", uid.c_str(), pwd.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	READMYSQL_INT(num, num, 0);

    out << (num?"{eid:0}":"{eid:100}");
    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
int AccountSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	SYSTEM_LOG("[AccountSvc::handle_timeout] begin deal company apply auto expired ==============");
	MySql* psql = CREATE_MYSQL;

	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
	sprintf(sql, "UPDATE t41_comp_contacts SET flag = 2, last_upd_dt = '%s' WHERE apply_type = 0 AND TO_DAYS('%s') - TO_DAYS(last_upd_dt) > 3 AND flag = 0", gmt0now,gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 0);

	SYSTEM_LOG("[AccountSvc::handle_timeout] end deal company apply auto expired, count:%d ==============", psql->GetAffectedRows());
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{ordertype:"",ordid:"",strdt:"",enddt:"",payid:"",status:"",uid:"",invsts:"",operid:"",pn:""}
int  AccountSvc::GetOrderList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::GetOrderList]bad format:", jsonString, 1);

	string ordertype = root.getv("ordertype", "");
    string ordid = root.getv("ordid", "");
    string strdt = root.getv("strdt", "");
	string enddt = root.getv("enddt", "");
    string payid = root.getv("payid", "");
	string status = root.getv("status", "");
	string uid = root.getv("uid", "");    
	string invsts = root.getv("invsts", "");
    string operid = root.getv("operid", "");
	string pnString = root.getv("pn", "");
	int pn = atoi(pnString.c_str());
	if(pn <= 0)
		return 1;

    MySql* psql = CREATE_MYSQL;

    char sql[1024 * 2] = "";
	int len = 0;
	int cnt = 0;

	char gmt0now[20];
	GmtNow(gmt0now);

	len = sprintf (sql, "SELECT t1.order_id, t1.user_id, t1.status_cd, t1.remark, t2.STATUS, UNIX_TIMESTAMP(t1.last_upd_dt) AS paydt,												\
								t1.amout, t3.user_id AS operid, t4.BALANCE, DATEDIFF('%s', t4.Last_udp_dt) AS lastdt																\
									FROM blmdtmaintain.t02_buy_detail AS t1 LEFT JOIN t02_invoices AS t2 ON t1.order_id = t2.order_id																	\
									LEFT JOIN t00_operation_log AS t3 ON t1.order_id = t3.OPERATION_TARGET																			\
									LEFT JOIN t02_product_balance AS t4 ON t1.user_id = t4.user_id AND t1.product_id = t4.PRODUCT_ID WHERE t1.product_id = 'BLMPSATELLITEAIS'", gmt0now);
	if (ordid.length())
		len += sprintf (sql + len, " and t1.order_id = '%s'", ordid.c_str());

	if (strdt.length())
		len += sprintf (sql + len, " and UNIX_TIMESTAMP(t1.order_time) >= '%s'", strdt.c_str());

	if (enddt.length())
		len += sprintf (sql + len, " and UNIX_TIMESTAMP(t1.order_time) <= '%s'", enddt.c_str());

	if (payid.length())
		len += sprintf (sql + len, " and t1.user_id = '%s'", payid.c_str());

	if (status.length())
		len += sprintf (sql + len, " AND t1.status_cd = '%s'", status.c_str());

	if (uid.length())
		len += sprintf (sql + len, " AND t1.remark LIKE '%%;%s%%' OR t1.remark LIKE '%%,%s%%'", uid.c_str(), uid.c_str());

	if (invsts.length())
		len += sprintf (sql + len, " and t2.status = '%s'", invsts.c_str());

	if (operid.length())
		len += sprintf (sql + len, " and t3.user_id = '%s'", operid.c_str());

	if(ordertype.length())
		len += sprintf(sql + len, " AND t1.product_id = '%s'", ordertype.c_str());

	sprintf (sql + len, " ORDER BY t1.order_time DESC, t3.update_dt DESC");

	//printf("%s",sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	OrderInfos orderInfos;
	std::set<string> ids;
	while (psql->NextRow())
	{
		OrderInfo orderInfo;
		char text[1024 * 3];
		double balance;
		int lastdt;
		int yn, pn;
		READMYSQL_STR(order_id, orderInfo.ordid);
		READMYSQL_INT(status_cd, orderInfo.status, -1);
		READMYSQL_STR(user_id, orderInfo.payid);
		READMYSQL_STR(remark, text);
		bool f1 = false, f2 = false;
		for (size_t i = 0; i < strlen(text); i++)
		{
			if (text[i] == 'X')
				f1 = true;
			if (text[i] == ';')
				f2 = true;
		}
		if (f1 && f2)
			sscanf(text, "%dX%d;%s", &pn, &yn, orderInfo.uids);
		else 
		{
			pn = 0;
			yn = 0;
			orderInfo.uids[0] = '\0';
		}
		READMYSQL_INT(status, orderInfo.invsts, -1);
		READMYSQL_INT(paydt, orderInfo.paydt, -1);
		if ((orderInfo.status == 2 || orderInfo.status == 3) && orderInfo.paydt != -1)
		{
			time_t tt = (time_t)(orderInfo.paydt);
			struct tm *tmpTm = localtime(&tt);
			tmpTm->tm_year += yn;
			orderInfo.expiredt = (int)mktime(tmpTm);
			//orderInfo.days = (orderInfo.expiredt - orderInfo.paydt) / 3600 / 24;
		}
		else
		{
			orderInfo.expiredt = -1;
			//orderInfo.days = -1;
		}
		READMYSQL_DOUBLE(amout, orderInfo.amout, 0.0);
		READMYSQL_STR(operid, orderInfo.operid);			
		READMYSQL_DOUBLE(BALANCE, balance, 0.0);
		READMYSQL_INT(lastdt, lastdt, 0);
		orderInfo.days = (int)balance - lastdt;
		if (orderInfo.days < 0)
			orderInfo.days = 0;
		if (ids.find(string(orderInfo.ordid)) == ids.end())
		{
			orderInfos.push_back(orderInfo);
			ids.insert(string(orderInfo.ordid));
			cnt++;
		}
	}

	OrderInfos::iterator iTer = orderInfos.begin();
	int totalPn = (int)ceil(cnt / 10.0);
	int tmpN = 0;
	while (tmpN < (pn - 1) * 10 && tmpN++ < cnt) iTer++;
	out << "{eid:0,total:" << cnt << ",pn:" << totalPn << ",cpn:" << pn << ",ss:[";
	for (tmpN = 0; iTer != orderInfos.end() && tmpN < 10; iTer++, tmpN++)
	{
		out << (tmpN?",":"");
		out << "{ordid:\"" << (*iTer).ordid << "\",paydt:\"" << (*iTer).paydt << "\",payid:\"" << (*iTer).payid << "\",status:\"" << (*iTer).status << "\",uid:\"" << (*iTer).uids << "\",ktsts:\"\",amout:\"" << (*iTer).amout << "\",expiredt:\"" << (*iTer).expiredt << "\",days:\"" << (*iTer).days << "\",invsts:\"" << (*iTer).invsts << "\",proid:\"BLMPSATELLITEAIS\",operid:\"" << (*iTer).operid << "\"}";
	}
	out << "]}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:""}
int  AccountSvc::GetDetailOfOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::GetDetailOfOrder]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    //string ordid = root.getv("ordid", "");

    MySql* psql = CREATE_MYSQL;

    char sql[1024] = "";

	sprintf (sql, "SELECT t1.REALNAME, t1.COUNTRY, t1.USER_VERIFY, t3.verified \
					FROM t00_user AS t1 LEFT JOIN t41_comp_contacts AS t2 ON t1.user_id = t2.user_id \
					LEFT JOIN t41_company AS t3 ON t2.company_key = t3.company_key WHERE t1.user_id = '%s'", uid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char name[100] = "";
	char country[10] = "";
	int usrVerify = -1, cmpVerify = -1;
	if (psql->NextRow())
	{
		READMYSQL_STR(REALNAME, name);
		READMYSQL_STR(COUNTRY, country);
		READMYSQL_INT(USER_VERIFY, usrVerify, -1);
		READMYSQL_INT(verified, cmpVerify, -1);
	}

	out << "{eid:0,uid:\"" << uid << "\",name:\"" << name << "\",cny:\"" << country << "\",usrverify:\"" << usrVerify << "\",cmpverify:\"" << cmpVerify << "\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",proid:"",balance:"",ntp:"",cause:"",operid:""}
int  AccountSvc::DealAuthority(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::DealAuthority]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string proid = root.getv("proid", "");
	string ntp = root.getv("ntp", "");
	string balance = root.getv("balance", "");
	string cause = root.getv("cause", "");
	string operid = root.getv("operid", "");
	string ordid = root.getv("ordid", "");

	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";
    
	if (ntp[0] == '1')
		sprintf (sql, "REPLACE INTO t02_product_balance (USER_ID, PRODUCT_ID, BALANCE_TYPE, BALANCE, Last_udp_dt) VALUES ('%s', '%s', 'BLMPSATELLITEAIS', '%s', '%s')", uid.c_str(), proid.c_str(), balance.c_str(),gmt0now);
	else
		sprintf (sql, "UPDATE t02_product_balance SET BALANCE = 0, Last_udp_dt = '%s' WHERE user_id = '%s' AND PRODUCT_ID = '%s'", gmt0now, uid.c_str(), proid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	if (ntp[0] == '0')
		sprintf (sql, "INSERT INTO t00_operation_log (user_id, bus_type, `action`, detail, update_dt, OPERATION_TARGET) VALUES ('%s', 'BLMPSATELLITEAIS', 'cancel business', '%s', '%s', '%s')", operid.c_str(), cause.c_str(), gmt0now,ordid.c_str());
	else
		sprintf (sql, "INSERT INTO t00_operation_log (user_id, bus_type, `action`, update_dt, OPERATION_TARGET) VALUES ('%s', 'BLMPSATELLITEAIS', 'update business', '%s', '%s')", operid.c_str(), gmt0now,ordid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	if (ntp[0] == '1')
	{
		sprintf(sql, "UPDATE t02_order SET status_cd = '3' WHERE order_id = '%s'", ordid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{email:""}
int AccountSvc::getUserId(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getUserId]bad format:", jsonString, 1);
	string email = root.getv("email", "");

	if (email.empty())
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char uid[64] = "";
	sprintf (sql, "SELECT USER_ID FROM t00_user WHERE EMAIL = '%s'", email.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{
		READMYSQL_STR(USER_ID, uid);
	}
	
	out << "{uid:\""  << uid << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:""}
int AccountSvc::getTelBal(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getTelBal]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char curCode[4] = "";
	double telBal = 0.0;
	sprintf (sql, "SELECT curcode, bal FROM t02_user_tel_bal WHERE User_id = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{
		READMYSQL_STR(curcode, curCode);
		READMYSQL_DOUBLE(bal, telBal, 0.0);
	}
	
	out << "{curcode:\""  << curCode << "\",bal:" << telBal << "}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:""}
int AccountSvc::getUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getTelBal]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char fn[64] = "";
	char ln[64] = "";
	char tel[300] = "";
	char email[100] = "";
	sprintf (sql, "SELECT firstname, lastname, mobile, email FROM t00_user WHERE user_id = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{
		READMYSQL_STR(firstname, fn);
		READMYSQL_STR(lastname, ln);
		READMYSQL_STR(mobile, tel);
		READMYSQL_STR(email, email);
	}
	else
	{
		out << "{eid:0, msg:\"this user do not exist\"}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}
	
	out << "{uid:\""  << uid << "\",fn:\"" << fn << "\",ln:\"" << ln << "\",tel:\"" << tel << "\",email:\"" << email << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{thirduid:"", webid:""}
int AccountSvc::getUserIdFromThirdId(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getUserIdFromThirdId]bad format:", jsonString, 1);
	string thrid = root.getv("thirduid", "");
	string id = root.getv("webid", "");

	if (thrid.empty() || id.empty())
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char uid[32] = "";
	sprintf (sql, "SELECT USER_ID FROM t00_user_third_rel WHERE THIRD_ID = '%s' AND ID = '%s'", thrid.c_str(), id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{
		READMYSQL_STR(USER_ID, uid);
	}
	else
	{
		out << "{eid:0, msg:\"this user do not exist\"}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}
	
	out << "{uid:\""  << uid << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",thirduid:"",webid:""}
int AccountSvc::updThirdId(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::updThirdId]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string thrid = root.getv("thirduid", "");
	string id = root.getv("webid", "");

	if (uid.empty() || thrid.empty() || id.empty())
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char gmt0now[20];
	GmtNow(gmt0now);
	sprintf (sql, "INSERT IGNORE INTO t00_user_third_rel (USER_ID, THIRD_ID, ID, UPDATE_DT) VALUES ('%s', '%s', '%s', '%s')", uid.c_str(), thrid.c_str(), id.c_str(), gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

// zhuxj
// {from:"", to:"", dt:}
int AccountSvc::getExchange(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getExchange]bad format:", jsonString, 1);
	string from = root.getv("from", "");
	string to = root.getv("to", "");
	int dt  = root.getv("dt", 0);

	if (from.empty() || to.empty())
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT exchange FROM t41_exchange WHERE (UNIX_TIMESTAMP(FROM_UNIXTIME('%d')) BETWEEN UNIX_TIMESTAMP(start_dt) \
		AND UNIX_TIMESTAMP(end_dt) OR end_dt = '3000-01-01 00:00:00') AND curcode = '%s' AND to_cur = '%s' ORDER BY end_dt ", dt, from.c_str(), to.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	double exchange = -1.0;
	if (psql->NextRow())
	{
		READMYSQL_DOUBLE(exchange, exchange, -1.0);
	}
	out << "{exchange:" << exchange << "}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{dprefix:"",pn:}
int AccountSvc::getConnectCharge(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getConnectCharge]bad format:", jsonString, 1);
	string dialprefix = root.getv("dprefix", "");
	int pn = root.getv("pn", 0);
	if (pn <= 0)
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT dialprefix, connectcharge FROM astercc.myrate WHERE dialprefix like '%s%%'", dialprefix.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	ConnectCharges connectCharges;
	
	while (psql->NextRow())
	{
		ConnectCharge oneConnectCharge;
		READMYSQL_DOUBLE(connectcharge, oneConnectCharge.connectcharge, -1.0);
		READMYSQL_STR(dialprefix, oneConnectCharge.dialpre);
		connectCharges.push_back(oneConnectCharge);
	}
	out << "{total:" << connectCharges.size() << ",pagen:" << ceil(connectCharges.size()/10.0) << ",ConnectCharges:[";
	for (int i = (pn-1)*10, cnt = 0; i < (int)connectCharges.size() && cnt < 10; i++,cnt++)
	{
		if(cnt)
			out << ",";
		out << "{dialprefix:\"" << connectCharges[i].dialpre << "\",ccharge:" << connectCharges[i].connectcharge << "}";
	}	
	out << "]}";
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",passwd:""}
int AccountSvc::confirmUserPasswd(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::confirmUserPasswd]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string passwd = root.getv("passwd", "");
	unsigned char tpwd[1024] = "";
	for (int i = 0 ; i < (int)passwd.length() && i < 1024; i++)
		tpwd[i] = (unsigned char)passwd[i];


	MD5 tMd5;
	tMd5.update(tpwd, passwd.length());
	tMd5.finalize();
	//cout << tMd5->hex_digest() << endl;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT COUNT(1) as num, is_flag FROM t00_user WHERE user_id = '%s' AND PASSWORD = '%s'", uid.c_str(), tMd5.hex_digest());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int num = 0, is_flag;
	if (psql->NextRow())
	{
		READMYSQL_INT(num, num, 0);
		READMYSQL_INT(is_flag, is_flag, 0);
	}
	if (is_flag == 2)
		is_flag = 0;
	out << "{\"valid\":" << num << ",\"active\":" << is_flag << "}";
	RELEASE_MYSQL_RETURN(psql, 0);
}


int AccountSvc::logout(const char* pUid, const char* jsonString, std::stringstream& out)
{
	return 0;
}

//zhuxj
//{tel:"", vcode:"",src:""}
int AccountSvc::autoRegister(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::autoRegister]bad format:", jsonString, 1);
	string tel = root.getv("tel", "");
	string vcode = root.getv("vcode", "");
	string src = root.getv("src", "");

	string szTel = tel;
	string iso2;
	int usertype = 21;
	if (src.compare("android") == 0)
		usertype = 31;
	if(!g_MobileIllegal(szTel, iso2))
	{
		DEBUG_LOG("[AccountSvc::autoRegister] illegal mobile:%s", szTel.c_str());
		return 1;
	}
	if(iso2 == "CN")
	{
		Tokens tokens = StrSplit(tel, " ");
		szTel = tokens[0].substr(1) + szTel;
	}

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf(sql, "SELECT COUNT(1) as num FROM t00_user WHERE user_id = '%s'", szTel.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int num = 0;
	if (psql->NextRow())
		READMYSQL_INT(num, num, 0);
	if(num)
	{
		out << "{\"eid\":100}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	//verify code
	string telno = "";
	string email = "";
    int ret = g_UserRegSvc::instance()->verifyCode(tel, vcode, telno, email);
	DEBUG_LOG("[AccountSvc::autoRegister] verifyCode: ret : %d", ret);
	if (!ret)
		RELEASE_MYSQL_RETURN(psql, 1);

	//生成随机6位密码
	srand((unsigned int)time(0));
	static int nbegin = 100000;
	static int nend = 999999;
	int nsecr = rand()%(nend-nbegin+1)+nbegin;
	DEBUG_LOG("[AccountSvc::autoRegister]nsecr : %d", nsecr);
	unsigned char tpwd[7];
	for (int i = 5, tnsecr = nsecr; i>=0; i--)
	{
		tpwd[i] = tnsecr%10 + '0';
		tnsecr /= 10;
	}

	MD5 tMd5;
	tMd5.update(tpwd, 6);
	tMd5.finalize();

	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "INSERT IGNORE INTO t00_user(user_id, PASSWORD, mobile, is_flag, mobile_verified, ACTIVATE_DT, REGISTER_FROM, USERTYPE, CREATIONDATE) \
					VALUES ('%s', '%s', '%s', '1', '1', '%s', '%s', '%d', CURRENT_TIMESTAMP())", szTel.c_str(), tMd5.hex_digest(), tel.c_str(), gmt0now, src.c_str(), usertype);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	if (usertype == 31)
	{
		sprintf (sql, "INSERT IGNORE INTO t02_user_tel_bal (User_id, curcode, bal, valid_dt, Last_upd_dt, total_given, last_given, last_given_dt) VALUES ('%s', 'CNY', '9', DATE_ADD(%s,INTERVAL 1 MONTH), %s, '9', '9', %s)", szTel.c_str(), gmt0now, gmt0now, gmt0now);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	std::stringstream tout;
	char req[1024] = "";
	sprintf(req,  "{userid:\"%s\",money:0}", szTel.c_str());
	ret = g_PhoneSvc::instance()->GetSipNo(szTel.c_str(), req, tout);		//xiugai
	DEBUG_LOG("[AccountSvc::autoRegister] GetSipNo: ret : %d", ret);
	if (ret)
		RELEASE_MYSQL_RETURN(psql, 2);

	char content[1024] = "";
	if (iso2 == "CN")
		sprintf (content, "BLM打遍天下注册成功，用户名:%s,密码:%d。直接拨打区号-号码，如8613908888888。[BLM] ", szTel.c_str(), nsecr);
	else
		sprintf (content, "Register Successfully! User Name:%s, Passwd:%d. Please dial: Area code-Number.[BLM] ", szTel.c_str(), nsecr);

	CNotifyMsg* pMsg = new CNotifyMsg;
    pMsg->m_szType = "sms";
    pMsg->m_SmsFromUser = "blm_system";
    pMsg->m_SmsToUser = tel;
    pMsg->m_SmsToTel = tel;
    pMsg->m_SmsMsg = content;
    pMsg->m_SmsType = 0;
    g_NotifyService::instance()->PutMessage(pMsg);
	DEBUG_LOG("[AccountSvc::autoRegister] sendSmsSS: ret : %d", ret);
	//out << "{\"eid\":0}";
	out << "{\"eid\":0,\"uid\":\"" << szTel << "\",\"passwd\":\"" << nsecr << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:""}
int AccountSvc::getDepNum(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getDepNum]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	int nworks = 0;
	char cmpid[64] = "";
	int depid = 0;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql, "SELECT COUNT(1) AS nworks, t1.company_key, t1.dep_id FROM t41_comp_contacts AS t1, t41_comp_contacts AS t2 WHERE t1.company_key = t2.company_key AND t1.dep_id = t2.dep_id AND t1.user_id = '%s' AND t2.flag = 1", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()
	if (psql->NextRow())
	{
		READMYSQL_INT(nworks, nworks, 0);
		READMYSQL_STR(company_key, cmpid);
		READMYSQL_INT(dep_id, depid, 0);
	}

	out << "{nworks:" << nworks << ",cmpid:\"" << cmpid << "\",depid:" << depid << "}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{cmpid:"",depid:"",uid:""}
int AccountSvc::getDepDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getDepDetail]bad format:", jsonString, 1);
	string cmpid =	root.getv("cmpid", "");
	string depid =	root.getv("depid", "");
	string usrid =	root.getv("uid", "");

	char sql[1024] = "";
	MySql* psql = CREATE_MYSQL;
	CWVec cwVec;

	sprintf(sql, "SELECT t2.firstname, t2.lastname, t2.nickname, t1.user_id,  t1.Isadmin, t1.DEP_ID, t3.DEP_NAME, t2.MOBILE, t2.email	\
				 FROM t41_comp_contacts t1 LEFT JOIN t00_user t2 ON t1.USER_ID = t2.USER_ID												\
				 LEFT JOIN t41_comp_department t3 ON t1.DEP_ID = t3.DEP_ID AND t1.COMPANY_KEY = t3.COMPANY_KEY							\
				 WHERE t1.COMPANY_KEY = '%s' AND t1.dep_id = '%s' AND t1.Flag = 1", cmpid.c_str(), depid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow())
	{
		CompanyCoWorker coWorker;

		READMYSQL_STR(firstname, coWorker.firstna);
		READMYSQL_STR(lastname, coWorker.lastna);
		READMYSQL_STR(nickname, coWorker.nickna);
		READMYSQL_STR(user_id, coWorker.uid);
		READMYSQL_INT(Isadmin, coWorker.badmin, 1);
		READMYSQL_STR(DEP_ID, coWorker.departid);
		READMYSQL_STR(DEP_NAME, coWorker.departna);
		READMYSQL_STR(MOBILE, coWorker.tel);
		READMYSQL_STR(email, coWorker.email);

		cwVec.push_back(coWorker);
	}

	out << "[";
	for (int i = 0; i < (int)cwVec.size(); i++)
	{
		if (i)
			out << cwVec[i].toJson2();
		else
			out << cwVec[i].toJson1();
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",serv:}:0表示AIS业务，1:表示亲情版业务
int AccountSvc::queUserService(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::queUserService]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	int serv = root.getv("serv", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	int num = 0;

	sprintf(sql, "SELECT COUNT(1) AS num FROM t02_product_balance WHERE user_id = '%s' AND product_id %s", uid.c_str(), service[serv].c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()
	if (psql->NextRow())
	{		
		READMYSQL_INT(num, num, 0);
	}

	if(num > 0)
		out << "{eid:0,flag:1}";
	else
		out << "{eid:0,flag:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",ntp:2,pn:1}1:审核通过，2:待审核，0:审核未通过
int AccountSvc::getPortList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getPortList]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	int ntp = root.getv("ntp", 0);
	int pn = root.getv("pn", 0);

	if(pn <= 0)
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	UserPortInfoList portInfos;

	sprintf(sql, "SELECT portid FROM blmdtmaintain.t41_port WHERE user_id = '%s' AND flag = '%d'", uid.c_str(), ntp);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()
	while (psql->NextRow())
	{	
		UserPortInfo portInfo;
		READMYSQL_INT(portid, portInfo.portid, 0);
		portInfos.push_back(portInfo);
	}

	out << "{total:" << portInfos.size() << ",pagen:" << ceil(portInfos.size()/10.0) << ",ports:[";
	for(int i = (pn - 1)*10, cnt = 0; i < (int)portInfos.size() && cnt < 10; i++, cnt++)
	{
		if(cnt)
			out << ",";
		out << "{portid:\"" << portInfos[i].portid << "\"}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"",ntp:2,pn:1}1:审核通过，2:待审核，0:审核未通过
int AccountSvc::getShipList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getShipList]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	int ntp = root.getv("ntp", 0);
	int pn = root.getv("pn", 0);
	
	if(pn <= 0)
		return 0;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	UserShipInfoList shipInfos;

	sprintf(sql, "SELECT shipid FROM blmdtmaintain.t41_ship WHERE user_id = '%s' AND flag = '%d'", uid.c_str(), ntp);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()
	while (psql->NextRow())
	{		
		UserShipInfo shipInfo;
		READMYSQL_STR(shipid, shipInfo.shipid);
		shipInfos.push_back(shipInfo);
	}

	out << "{total:" << shipInfos.size() << ",pagen:" << ceil(shipInfos.size()/10.0) << ",ships:[";
	for(int i = (pn - 1)*10, cnt = 0; i < (int)shipInfos.size() && cnt < 10; i++, cnt++)
	{
		if(cnt)
			out << ",";
		out << "{shipid:\"" << shipInfos[i].shipid << "\"}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{portid:""}
int AccountSvc::getPortBrief(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getPortBrief]bad format:", jsonString, 1);
	string portid = root.getv("portid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char name[100] = "";
	char name_cn[100] = "";
	char brief[1024*10] = "";

	sprintf(sql, "SELECT name, NAMECN, remark FROM t41_port WHERE portid = '%s'", portid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()
	if (psql->NextRow())
	{		
		READMYSQL_STR(name, name);
		READMYSQL_STR(NAMECN, name_cn);
		READMYSQL_STR(remark, brief);
	}

	out << "{name:\"" << JsonReplace(name) << "\",namecn:\"" << name_cn << "\",brief:\"" << JsonReplace(brief) << "\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{servid:""}
int AccountSvc::getServiceName(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getServiceName]bad format:", jsonString, 1);
	string servid = root.getv("servid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	

	int len = sprintf(sql, "SELECT key1, DESC_CN, DESC_EN FROM t91_code WHERE class_code = '2'");
	if (!servid.empty())
	{
		sprintf (sql + len, " AND key1 = '%s'", servid.c_str());
	}
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{		
		char servid[4] = "";
		char desCn[255] = "";
		char desEn[255] = "";
		READMYSQL_STR(key1, servid);
		READMYSQL_STR(DESC_CN, desCn);
		READMYSQL_STR(DESC_EN, desEn);
		if (cnt++)
			out << ",";
		out << "{servid:\"" << servid << "\",desCn:\"" << desCn << "\",desEn:\"" << desEn << "\"}";
	}
	out << "]";	

	RELEASE_MYSQL_RETURN(psql, 0);
}
//zhuxj
//{uid:"",telemail:""}
int AccountSvc::updPhoneUserPasswd(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::updPhoneUserPasswd]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");	
	string telemail = root.getv("telemail", "");
	
	if (uid.empty() || telemail.empty())
		return 1;

	if (telemail.find("@") == -1)
	{
		//size_t pos = telemail.find("+");
		//if (pos != -1)
		//	telemail = telemail.erase(pos,1);
		size_t pos = telemail.find(" ");
		if (pos != -1)
			telemail = telemail.erase(0, pos+1);
	}

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT mobile, email FROM t00_user WHERE user_id = '%s' AND (email = '%s' OR mobile LIKE '%%%s')", uid.c_str(), telemail.c_str(), telemail.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	string mobile = "";
	string email = "";
	if(psql->NextRow())
	{
		char _tmpS[400];
		READMYSQL_STR(mobile, _tmpS);
		mobile = _tmpS;
		READMYSQL_STR(email, _tmpS);
		email = _tmpS;
	}
	else
	{
		out << "{\"eid\":100}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	//生成随机6位密码
	srand((unsigned int)time(0));
	static int nbegin = 100000;
	static int nend = 999999;
	int nsecr = rand()%(nend-nbegin+1)+nbegin;
	DEBUG_LOG("[AccountSvc::autoRegister]nsecr : %d", nsecr);
	unsigned char tpwd[7];
	for (int i = 5, tnsecr = nsecr; i>=0; i--)
	{
		tpwd[i] = tnsecr%10 + '0';
		tnsecr /= 10;
	}

	MD5 tMd5;
	tMd5.update(tpwd, 6);
	tMd5.finalize();

	
	sprintf (sql, "UPDATE t00_user SET PASSWORD = '%s' WHERE user_id = '%s'", tMd5.hex_digest(), uid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	char smsMsg[1024] = "";
	bool bsucc1 = false, bsucc2 = false;
	if (email.length())
	{
		sprintf(smsMsg, "您的新密码已经被系统自动重置为%d，请妥善保存。", nsecr);
		char _jsonString[1024] = "";
		sprintf (_jsonString, "{id:\"blm_system\",to:\"%s\",cc:\"\",subject:\"%s\",msg:\"%s\",type:\"0\"}", email.c_str(), CodeConverter::Gb2312ToUtf8(smsMsg).c_str(), CodeConverter::Gb2312ToUtf8(smsMsg).c_str());
		int tmp_flag = g_SmsMailSvc::instance()->sendMail("blm_system", _jsonString, out);
		DEBUG_LOG("[AccountSvc::updPhoneUserPasswd] sendemail: tmp_flag : %d", tmp_flag);
		bsucc1 = (tmp_flag == 0);
	}

	if (mobile.length())
	{
		sprintf(smsMsg, "您的新密码已经被系统自动重置为%d，请妥善保存。(来自 博懋国际)", nsecr);
		CNotifyMsg* pSmsMsg = new CNotifyMsg;
		pSmsMsg->m_szType = "sms";
		pSmsMsg->m_SmsFromUser = "";
		pSmsMsg->m_SmsToUser = uid;
		pSmsMsg->m_SmsToTel = mobile;
		pSmsMsg->m_SmsMsg = smsMsg;
		pSmsMsg->m_SmsType = 0;
		bsucc2 = g_NotifyService::instance()->PutMessage(pSmsMsg);
		DEBUG_LOG("[AccountSvc::updPhoneUserPasswd] sendmsg: bsucc2 : %d", bsucc2);
	}

	int msgFlg = bsucc1?(bsucc2?3:2):(bsucc2?1:4);
	out.str("");
	out << "{\"eid\":0,\"msg\":" << msgFlg << "}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:\"%@\",pwd:\"%@\",telemail:\"%@\",src:\"iphone\"}
int AccountSvc::iPhoneRegister(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::updPhoneUserPasswd]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");	
	string pwd = root.getv("pwd", "");	
	string telemail = root.getv("telemail", "");
	string src = root.getv("src", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	int tp = 0;
	if (telemail.find("@") == -1)
		tp = 1;//telno
	string ziduan[] = {"email", "mobile"};
	sprintf (sql, "SELECT COUNT(1) as num FROM t00_user WHERE user_id = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	int num = 0;
	READMYSQL_INT(num, num, 0);
	if (num)
	{
		out << "{\"eid\":101}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	sprintf (sql, "SELECT COUNT(1) as num FROM t00_user WHERE %s = '%s'", ziduan[tp].c_str(), telemail.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	READMYSQL_INT(num, num, 0);
	if (num)
	{
		out << "{\"eid\":100}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}
	string ziduan2[] = {"EMAIL_VERIFIED", "MOBILE_VERIFIED"};
	sprintf (sql, "INSERT INTO t00_user (USER_ID, PASSWORD, %s,  %s, REGISTER_FROM, usertype) VALUES ('%s', '%s', '%s', '1', '%s', '21')",
		ziduan[tp].c_str(), ziduan2[tp].c_str(), uid.c_str(), pwd.c_str(), telemail.c_str(), src.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql), 3);
	out << "{\"eid\":0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{"token":"111111-1122222-33333-44444","version":"1.0",src:"blmphone"}
int AccountSvc::insertIOSDevice(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::insertIOSDevice]bad format:", jsonString, 1);
	string token = root.getv("token", "");	
	string version = root.getv("version", "");	
	string src = root.getv("src", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "REPLACE INTO t41_phone_device VALUES ('%s','%s','%s','%s','')", 
		token.c_str(), version.c_str(), src.c_str(), gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{userid:\"%s\",token:\"%s\",src:\"%s\"}
int AccountSvc::updIOSUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::updIOSUser]bad format:", jsonString, 1);
	string token = root.getv("token", "");	
	string userid = root.getv("userid", "");	
	string source = root.getv("src", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "UPDATE t41_phone_device SET USER_ID = '%s',update_dt = '%s' WHERE DEVICE_ID = '%s' and source = '%s'", userid.c_str(), gmt0now, token.c_str(), source.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:"",pwd:"",active:"",regfrom:""}
int AccountSvc::registerStraight(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::registerStraight]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");	
	string pwd = root.getv("pwd", "");	
	string active = root.getv("active", "");
	string regfrom = root.getv("regfrom", "");
	if (uid.empty() || pwd.empty())
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "INSERT INTO t00_user (USER_ID, `PASSWORD`, IS_FLAG, ACTIVATE_DT, REGISTER_FROM) VALUES ('%s','%s','%s','%s','%s')", 
		uid.c_str(), pwd.c_str(), active[0]=='1'?"1":"0", active[0]=='1'?gmt0now:"NULL", regfrom.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{duid:"",uid:""}
int AccountSvc::relateIphonePc(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::relateIphonePc]bad format:", jsonString, 1);
	string duid = root.getv("duid", "");	
	string uid = root.getv("uid", "");

	if (duid.empty() || uid.empty())
		return 1;
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT COUNT(1) AS num FROM t00_user_deviceus WHERE DEVICEUSERID = '%s'", duid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int num = 0;

	if (psql->NextRow())
	{
		READMYSQL_INT(num, num, 0);	
	}

	char gmt0now[20];
	GmtNow(gmt0now);
	if (!num)
		sprintf (sql, "INSERT INTO t00_user_deviceus (DEVICEUSERID, USERID, UPDATE_DT) VALUES ('%s','%s','%s')", duid.c_str(), uid.c_str(), gmt0now);
	else
		sprintf (sql, "UPDATE t00_user_deviceus SET USERID = '%s', UPDATE_DT = '%s' WHERE DEVICEUSERID = '%s'", uid.c_str(), gmt0now, duid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int AccountSvc::getThirdPassword(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AccountSvc::getThirdPassword]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");

	if (uid.empty())
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.password FROM t00_user t1, t00_user_third_rel t2 WHERE t1.USER_ID = t2.USER_ID AND t2.user_id = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql) && psql->NextRow(), 3);
	
	out << "{\"password\":\"" << psql->GetField("password") << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

#include "UserRegSvc.h"
#include "LogMgr.h"
#include "Util.h"
#include "SmsMail.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "xbase64.h"
#include "NotifyService.h"
#include "SmsMailSvc.h"
#include "MainConfig.h"

using namespace std;

#define ACTIVATE_TITLE_CN "激活BLM_Shipping账户"
#define ACTIVATE_CONTENT_CN "亲爱的 %s，\n \
感谢您注册BLM-Shipping账户！请您尽快激活您的账户，并使用博懋国际(BoLooMo International)的服务。您可以通过以下方式激活您的账户：\n \
1.在激活页面中入以下激活码，点击激活即可激活您的账户。\n \
%d\n \
2.点击以下链接\n \
http://www.boloomo.com/myaccount/register!activateCGI.action?param=%s\n \
（如果以上链接点击不能弹出浏览器或不能激活，请将您将链接地址复制到浏览器的地址栏中，直接访问进行激活。）\n \
激活成功后，请您通过BLM-Shipping软件http://www.boloomo.com/downloading/download_cn.html使用BoLooMo服务。\n\n\n \
******************************************************************************\n \
BLM-Shipping客户服务部 敬上\n \
地址：北京市海淀区中关村大街11号e世界A座1502-1503\n \
客户服务热线：8610-62682266\n \
*******************************************************************************\n"

#define ACTIVATE_TITLE_EN "Activation of the BLM-Shipping's account"
#define ACTIVATE_CONTENT_EN "Dear %s,\n \
Thank you for registering BLM-Shipping account! Please choose your favorite way to activate your account:\n \
1. Activate by verification code\n \
Enter the following verification code on activation page:\n \
%d  (verification code)\n \
2. Activate by link\n \
Click the following link to activate your account:\n \
http://www.boloomo.com/myaccount/register!activateCGI.action?param=%s  \n \
If the above URL can not be clicked or unable to complete the activation, please copy it to your web browser address bar, and visit the address directly.\n\n \
You will be able to login BLM-Shipping client with your registered ID after you activate your account successfully. If you have not downloaded BLM-Shipping client yet, please visit the following link address to download, and install it by following the wizard.\n \
http://www.boloomo.com/downloading/download_en.html\n\n\n \
*******************************************************************************\n \
Sincerely,\n \
The BLM-Shipping Supports Team\n \
ADD：Room 1502-1503, ZhongGuanCun e-Plaza fortune center , ZhongGuanCun Street No.11, Haidian District , Beijing , China\n \
Service Hotline：8610-62682266\n \
*******************************************************************************\n"

#define VERIFY_TITLE_CN "BLM_Shipping账户验证码"
#define VERIFY_CONTENT_CN "亲爱的 %s，\n \
感谢您使用BLM-Shipping服务！您的验证码是：%d\n\n\n \
******************************************************************************\n \
BLM-Shipping客户服务部 敬上\n \
地址：北京市海淀区中关村大街11号e世界A座1502-1503\n \
客户服务热线：8610-62682266\n \
*******************************************************************************\n"

#define VERIFY_TITLE_EN "BLM-Shipping's verification code"
#define VERIFY_CONTENT_EN "Dear %s,\n \
Thank you for using BLM-Shipping service! Your verification code is: %d\n\n\n \
*******************************************************************************\n \
Sincerely,\n \
The BLM-Shipping Supports Team\n \
ADD：Room 1502-1503, ZhongGuanCun e-Plaza fortune center , ZhongGuanCun Street No.11, Haidian District , Beijing , China\n \
Service Hotline：8610-62682266\n \
*******************************************************************************\n"

UserRegSvc::UserRegSvc()
{

}

UserRegSvc::~UserRegSvc()
{

}

bool UserRegSvc::Start()
{
    if(!loadEmailCode())
        return false;

    return true;
}

void UserRegSvc::ClearExpired()
{
	SYSTEM_LOG("[UserRegSvc::ClearExpired] begin ===============================");
	int count = 0;
    //清理过期的手机激活码(1800)
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadLock);

    for(MapCodeIter iter = m_mapCode.begin(); iter != m_mapCode.end(); )
    {
        if(iter->second->type==1 && time(NULL) - iter->second->tt >= 1800)
        {
            deleteFromUserMap(iter->second);
            delete iter->second;
            m_mapCode.erase(iter++);
			count++;
        }
        else
            iter++;
    }
	SYSTEM_LOG("[UserRegSvc::ClearExpired] end, total expired:%d ===============================", count);
}

void UserRegSvc::addTelCode(const std::string& uid, int nCode, const std::string& telno)
{
    MapUserTel::iterator iter = m_mapUserTel.find(uid);

    //新增
    if(iter == m_mapUserTel.end())
    {
        TCode* pCode = new TCode(uid, nCode, 1);
        pCode->tel = telno;

        m_mapCode.insert(make_pair(nCode, pCode));
        addToUserMap(pCode);
        return;
    }

    //更新
    iter->second->code = nCode;
    iter->second->tel = telno;
}

void UserRegSvc::addEmailCode(const std::string& uid, int nCode, const std::string& email)
{
    MapUserEmail::iterator iter = m_mapUserEmail.find(uid);
    if(iter == m_mapUserEmail.end())
    {
        TCode* pCode = new TCode(uid, nCode);
		pCode->email = email;

        m_mapCode.insert(make_pair(nCode, pCode));
        addToUserMap(pCode);
        return;
    }
}

void UserRegSvc::deleteFromUserMap(TCode* pCode)
{
    string& uid = pCode->uid;

    if(pCode->type == 0)
    {
        MapUserEmail::iterator iter = m_mapUserEmail.find(uid);
        if(iter != m_mapUserEmail.end())
            m_mapUserEmail.erase(iter);
    }
    else if(pCode->type == 1)
    {
        MapUserTel::iterator iter = m_mapUserTel.find(uid);
        if(iter != m_mapUserTel.end())
            m_mapUserTel.erase(iter);
    }
}

void UserRegSvc::addToUserMap(TCode* pCode)
{
    string& uid = pCode->uid;

    if(pCode->type == 0) // email
    {
        MapUserEmail::iterator iter = m_mapUserEmail.find(uid);
        if(iter == m_mapUserEmail.end())
            m_mapUserEmail.insert(make_pair(uid, pCode));
    }
    else if(pCode->type == 1) // tel
    {
        MapUserTel::iterator iter = m_mapUserTel.find(uid);
        if(iter == m_mapUserTel.end())
            m_mapUserTel.insert(make_pair(uid, pCode));
    }
}

int UserRegSvc::getEmailCode(const std::string& uid)
{
    MapUserEmail::iterator iter = m_mapUserEmail.find(uid);
    if(iter == m_mapUserEmail.end())
        return 0;

    return iter->second->code;
}

bool UserRegSvc::loadEmailCode()
{
    bool bSucc = true;
    MySql* psql = CREATE_MYSQL;

    char sql[1024];
    sprintf(sql, "select atv_code, user_id, UNIX_TIMESTAMP(create_dt) as tm, email from t00_email_code");
    if(psql->Query(sql))
    {
        char uid[20];
        while(psql->NextRow())
        {
            TCode* pCode = new TCode;
            READMYSQL_INT(atv_code, pCode->code, 0);
            READMYSQL_STR(user_id, uid);
            READMYSQL_64(tm, pCode->tt, 0);
            pCode->uid = string(uid);
			pCode->email = NOTNULL(psql->GetField("email"));

            m_mapCode.insert(make_pair(pCode->code, pCode));
            addToUserMap(pCode);
        }
    }
    else
        bSucc = false;

    RELEASE_MYSQL_RETURN(psql, bSucc);
}

int UserRegSvc::getRandomCode()
{
    srand((unsigned int)time(NULL));
    static int nBegin = 100000;
    static int nEnd = 999999;
    int nCode;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadLock);
    while(true)
    {
        nCode = rand() % (nEnd - nBegin + 1) + nBegin;
        if(m_mapCode.find(nCode) == m_mapCode.end())
            break;
    }

    return nCode;
}

bool UserRegSvc::sendCode(const std::string& uid, const std::string& tel, const std::string& email, bool cnOrEn, bool bIsActivation)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadLock);

	//StrIntMapIter botIter = m_mapBot.find(uid);
	//if(botIter != m_mapBot.end())
	//{
	//	DEBUG_LOG("[UserRegSvc::sendCode] illegal first:%s, second:%d", botIter->first.c_str(), botIter->second);
	//	if(botIter->second >= 3)
	//		return false;

	//	botIter->second++;
	//}
	//else
	//{
	//	m_mapBot.insert(StrIntMap::value_type(uid, 1));
	//}

    bool bSucc1 = false, bSucc2 = false;

    //生成手机码并发送(判断用户注册的国籍，取得ISO2)
    string szTel = tel;
	string iso2;
	if(!g_MobileIllegal(szTel, iso2))
	{
		DEBUG_LOG("[UserRegSvc::sendCode] illegal mobile:%s", szTel.c_str());
	}
	else
    {
        int nCode = getRandomCode();

        char content[1024];
		if(iso2 == "CN")
		{
			if(bIsActivation)
				sprintf(content, "BLM-Shipping账户激活码：%d，请您在30分钟内输入激活码！\n博懋国际", nCode);
			else
				sprintf(content, "BLM-Shipping账户验证码：%d，请您在30分钟内输入验证码！\n博懋国际", nCode);
		}
		else
		{
			if(bIsActivation)
				sprintf(content, "BLM-Shipping Activation code：%d, please input within 30 minutes\nBoLooMo", nCode);
			else
				sprintf(content, "BLM-Shipping Verification code：%d, please input within 30 minutes\nBoLooMo", nCode);
		}
		DEBUG_LOG("[UserRegSvc::sendCode]  mobile:%s", tel.c_str());
        CNotifyMsg* pMsg = new CNotifyMsg;
        pMsg->m_szType = "sms";
        pMsg->m_SmsFromUser = "blm_system";
        pMsg->m_SmsToUser = uid;
        pMsg->m_SmsToTel = tel;
        pMsg->m_SmsMsg = content;
        pMsg->m_SmsType = 5;
        g_NotifyService::instance()->PutMessage(pMsg);

        addTelCode(uid, nCode, tel);
		DEBUG_LOG("[UserRegSvc::sendCode] user:%s, ncode:%d", uid.c_str(), nCode);
        bSucc1 = true;
    }
	
    //生成邮件码并发送
    if(!email.empty())
    {
        //判断是否已经生成过邮件码
        bool bCreated = true;
        int nCode = getEmailCode(uid);
        if(nCode <= 0)
        {
            bCreated = false;
            nCode = getRandomCode();
        }

        char activateParam[1024];
        sprintf(activateParam, "{uid:\"%s\",active:\"%d\"}", uid.c_str(), nCode);
		string activateBase64Param = base64_encode((unsigned char const*)activateParam, (unsigned int)strlen(activateParam));

        char title[256];
        char content[2048];
        if(bIsActivation)
        {
            sprintf(title, "%s", cnOrEn?ACTIVATE_TITLE_CN:ACTIVATE_TITLE_EN);
            sprintf(content, cnOrEn?ACTIVATE_CONTENT_CN:ACTIVATE_CONTENT_EN, uid.c_str(), nCode, activateBase64Param.c_str());
        }
        else
        {
            sprintf(title, "%s", cnOrEn?VERIFY_TITLE_CN:VERIFY_TITLE_EN);
            sprintf(content, cnOrEn?VERIFY_CONTENT_CN:VERIFY_CONTENT_EN, uid.c_str(), nCode);
        }

        CNotifyMsg* pMsg = new CNotifyMsg;
        pMsg->m_szType = "mail";
        pMsg->m_MailFromUser = "blm_system";
        pMsg->m_MailFromAddr = "supports@boloomo.com";
        pMsg->m_MailToUser = uid;
        pMsg->m_MailToAddr = email;
        pMsg->m_MailSubject = title;
        pMsg->m_MailMsg = content;
        pMsg->m_MailType = 5;
        g_NotifyService::instance()->PutMessage(pMsg);

        MySql* psql = CREATE_MYSQL;;
        char sql[256];
        if(!bCreated)
        {
            sprintf(sql, "insert into t00_email_code(atv_code, user_id, create_dt, email) values (%d, '%s', CURRENT_TIMESTAMP(), '%s')", nCode, uid.c_str(), email.c_str());
            int tmplen = strlen(sql);
			if(psql->Execute(sql) <= 0)
            {
                bSucc2 = false;
            }
            else
            {
                addEmailCode(uid, nCode, email);
                bSucc2 = true;
            }
        }
        else
        {
            bSucc2 = true;
        }
		RELEASE_MYSQL_NORETURN(psql);
    }

    return bSucc1 || bSucc2;
}

bool UserRegSvc::sendCode(const std::string& uid, const std::string& tel)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadLock);

    //生成手机码并发送(判断用户注册的国籍，取得ISO2)
    string szTel = tel;
	string iso2;
	if(!g_MobileIllegal(szTel, iso2))
	{
		DEBUG_LOG("[UserRegSvc::sendCode Phone] illegal mobile:%s", szTel.c_str());
		return false;
	}
	else
    {
        int nCode = getRandomCode();

        char content[1024];
		if(iso2 == "CN")
		{
			sprintf(content, "欢迎使用BLM打遍天下网络电话! 帐户验证码为：%d，请在30分钟内使用。详情：http://t.cn/zlZau40[BLM]", nCode);
		}
		else
		{
			sprintf(content, "Welcom to BLM Call the world! Verification Code:%d. Please use it in 30 minutes. Click Here for details.[BLM]", nCode);
		}
		DEBUG_LOG("[UserRegSvc::sendCode Phone]  mobile:%s", tel.c_str());
        CNotifyMsg* pMsg = new CNotifyMsg;
        pMsg->m_szType = "sms";
        pMsg->m_SmsFromUser = "blm_system";
        pMsg->m_SmsToUser = uid;
        pMsg->m_SmsToTel = tel;
        pMsg->m_SmsMsg = content;
        pMsg->m_SmsType = 5;
        g_NotifyService::instance()->PutMessage(pMsg);

        addTelCode(uid, nCode, tel);
		DEBUG_LOG("[UserRegSvc::sendCode Phone] user:%s, ncode:%d", uid.c_str(), nCode);
    }
    return true;
}

int UserRegSvc::verifyCode(const std::string& uid, const std::string& code, std::string& telno, std::string& email)
{
    int nCode = atoi(code.c_str());

    //如果验证码合法,则删除内存中该用户全部的验证码
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadLock);

        MapCodeIter codeIter = m_mapCode.find(nCode);
        if(codeIter == m_mapCode.end())
		{
			DEBUG_LOG("[UserRegSvc::verifyCode] codeIter == m_mapCode.end()");
            return 0;
		}

        if(codeIter->second->type == 1) //手机验证码
            telno = codeIter->second->tel;
		else
			email = codeIter->second->email; //邮件验证码

        MapUserEmail::iterator emailIter = m_mapUserEmail.find(uid);
        MapUserTel::iterator telIter = m_mapUserTel.find(uid);

        if(emailIter != m_mapUserEmail.end())
            m_mapUserEmail.erase(emailIter);
        if(telIter != m_mapUserTel.end())
            m_mapUserTel.erase(telIter);

        delete codeIter->second;
        m_mapCode.erase(codeIter);
    }

    //验证成功,并删除数据库中该用户全部的验证码;同时如果是手机验证,则自动验证接收验证码的手机
    {
        MySql* psql = CREATE_MYSQL;
        char sql[1024];
		sprintf(sql, "DELETE FROM t00_email_code WHERE user_id = '%s'", uid.c_str());
		psql->Execute(sql);

		if(!telno.empty()) {
			sprintf(sql, "UPDATE t00_user SET mobile = '%s', mobile_verified = 1, is_flag = '1' WHERE user_id = '%s'", telno.c_str(), uid.c_str());
		}
        else
		{
            sprintf(sql, "UPDATE t00_user SET email = '%s', email_verified = 1, is_flag = '1' WHERE user_id = '%s'", email.c_str(), uid.c_str());
		}
        psql->Execute(sql);

		RELEASE_MYSQL_NORETURN(psql);
    }

    return 1;
}

int UserRegSvc::userEmailExists(const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[UserRegSvc::userEmailExists]bad format:", jsonString, 1);

    string uid   =	root.getv("uid", "");
    string email =	root.getv("email", "");
    if(uid.empty() && email.empty())
		return 1;

    int nExists = 0;
    int nRet = 0;

    char sql[512];
    if(!uid.empty())
        sprintf(sql, "select count(1) as Num from t00_user where user_id = '%s'", uid.c_str());
    else
        sprintf(sql, "select count(1) as Num from t00_user where email = '%s' and email_verified = '1'", email.c_str());

    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	READMYSQL_INT(Num, nExists, 0);

    out << "{eid:0,status:" << nExists << "}";
    RELEASE_MYSQL_RETURN(psql, nRet);
}

//{uid:"blmuser",nn:"dodobird",fn:"cai",ln:"wenjian",pwd:"e1414141feafa",email:"caiwj@boloomo.com",zip:"",tel:"+86 13581619514",country:"CHN",gender:"1",ip:"192.168.0.3",cn:1}
int UserRegSvc::registerUser(const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[UserRegSvc::registerUser]bad format:", jsonString, 1);

    string uid		= root.getv("uid", "");
    string nn		= root.getv("nn", "");
    string fn		= root.getv("fn", "");
    string ln		= root.getv("ln", "");
    string pwd		= root.getv("pwd", "");
    string email	= root.getv("email", "");
    string zip		= root.getv("zip", "");
    string tel		= root.getv("tel", "");
    string country	= root.getv("country", "");
    string gender	= root.getv("gender", "");
    string ip		= root.getv("ip", "");
    int cn			= root.getv("cn", 0);
	string active	= root.getv("active", "");
	string eOt		= root.getv("EmailorTel", "");
	string regfrom  = root.getv("regfrom", "");

    if(uid.empty() || pwd.empty())
		return 1;
	if (eOt.empty() && email.empty())
		return 1;

	// 手机号码格式加判断
	string szMobile = tel;
	string iso2;
	if(szMobile.length() > 2 && !g_MobileIllegal(szMobile, iso2))
		return 1;

    MySql* psql = CREATE_MYSQL;
    {
		// 临时逻辑(等郭善飞通知再去掉这个逻辑)
		if(regfrom == "ipad_blmshipping")
			country = "CHN";

        char sql[1024];

		
		sprintf (sql, "SELECT count(1) AS num FROM t00_user WHERE user_id = '%s'", uid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		int uidnum = 0, emailnum = 0;
		if (psql->NextRow())
		{			
			READMYSQL_INT(num, uidnum, 0);
		}
		
		if (!email.empty())
		{
			sprintf (sql, "SELECT count(1) as num FROM t00_user WHERE email = '%s'", email.c_str());
			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		
			if (psql->NextRow())
			{			
				READMYSQL_INT(num, emailnum, 0);
			}
		}
		out << "{eid:0,uid:" << uidnum << ", email:" << emailnum << "}";
		if (uidnum || emailnum)
			RELEASE_MYSQL_RETURN(psql, 0);


		if (!active.length() || active[0] == '0')
		{
			sprintf(sql, "INSERT IGNORE INTO t00_user(user_id, PASSWORD, gender, firstname, lastname, creationdate, zip, mobile, email, is_flag, ip, country, nickname, REGISTER_FROM) \
						VALUES ('%s', '%s', '%s', '%s', '%s', CURRENT_TIMESTAMP(), '%s', '%s', '%s', '2', '%s', '%s', '%s', '%s')",
						uid.c_str(), pwd.c_str(), gender.c_str(), fn.c_str(), ln.c_str(), zip.c_str(), tel.c_str(), email.c_str(), ip.c_str(), country.c_str(), nn.c_str(), regfrom.c_str());
		}
		else
		{
			char gmt0now[20];
			GmtNow(gmt0now);

			if (eOt[0] == '0')
				sprintf(sql, "INSERT IGNORE INTO t00_user(user_id, PASSWORD, gender, firstname, lastname, creationdate, zip, mobile, email, is_flag, ip, country, nickname, email_verified, ACTIVATE_DT, REGISTER_FROM) \
							 VALUES ('%s', '%s', '%s', '%s', '%s', CURRENT_TIMESTAMP(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '%s', '%s')",
						uid.c_str(), pwd.c_str(), gender.c_str(), fn.c_str(), ln.c_str(), zip.c_str(), tel.c_str(), email.c_str(), active.c_str(), ip.c_str(), country.c_str(), nn.c_str(), gmt0now, regfrom.c_str());
			else if (eOt[0] == '1')
				sprintf(sql, "INSERT IGNORE INTO t00_user(user_id, PASSWORD, gender, firstname, lastname, creationdate, zip, mobile, email, is_flag, ip, country, nickname, mobile_verified, ACTIVATE_DT, REGISTER_FROM) \
							 VALUES ('%s', '%s', '%s', '%s', '%s', CURRENT_TIMESTAMP(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '%s', '%s')",
						uid.c_str(), pwd.c_str(), gender.c_str(), fn.c_str(), ln.c_str(), zip.c_str(), tel.c_str(), email.c_str(), active.c_str(), ip.c_str(), country.c_str(), nn.c_str(), gmt0now, regfrom.c_str());
			else 
				sprintf(sql, "INSERT IGNORE INTO t00_user(user_id, PASSWORD, gender, firstname, lastname, creationdate, zip, mobile, email, is_flag, ip, country, nickname, mobile_verified, ACTIVATE_DT, email_verified, REGISTER_FROM) \
							 VALUES ('%s', '%s', '%s', '%s', '%s', CURRENT_TIMESTAMP(), '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '%s', '1', '%s')",
						uid.c_str(), pwd.c_str(), gender.c_str(), fn.c_str(), ln.c_str(), zip.c_str(), tel.c_str(), email.c_str(), active.c_str(), ip.c_str(), country.c_str(), nn.c_str(), gmt0now, regfrom.c_str());
		}
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:"caiwj",active:"1415141415135"}
int UserRegSvc::activateUser(const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[UserRegSvc::activateUser]bad format:", jsonString, 1);

    char email[64];
    string uid	= root.getv("uid", "");
    string code = root.getv("active", "");
    if(uid.empty() || code.empty())
		return 1;

    MySql* psql = CREATE_MYSQL;
    //判断是否已经激活
    {
        char sql[128];
        sprintf(sql, "select is_flag, email from t00_user where user_id = '%s'", uid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

		int flag = 0;
		READMYSQL_INT(is_flag, flag, 0);
		READMYSQL_STR(email, email);
		if(flag == 1)
		{
			out << "{eid:0,code:0,uid:\"" << uid << "\",email:\"" << email << "\"}";
			RELEASE_MYSQL_RETURN(psql, 0);
		}
    }

    string telno = "";
	string strEmail = "";
	int ret = verifyCode(uid, code, telno, strEmail);
	if(ret == 0) {
        RELEASE_MYSQL_RETURN(psql, 2);
	}
	else if(ret == 1) {

	}
	else if(ret == 2) {
		out << "{eid:0,code:3,uid:\"" << uid << "\",email:\"" << email << "\"}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}

    //激活成功，送5条短信
    {
        char sql[1024];
        sprintf(sql, "UPDATE t00_user SET is_flag = 1, activate_dt = CURRENT_TIMESTAMP() WHERE user_id = '%s'", uid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

        if(!telno.empty())
        {
			sprintf(sql, "INSERT ignore INTO t02_user_sms_bal(user_id, homecode, local_bal, inter_bal, last_upd_dt) VALUES ('%s', '86', %d, %d, CURRENT_TIMESTAMP())", uid.c_str(), g_MainConfig::instance()->GetFreeSms(), g_MainConfig::instance()->GetFreeSms());
           CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
        }
    }

    out << "{eid:0,code:" << (telno.empty()?2:1) << ",uid:\"" << uid << "\",email:\"" << email << "\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:"caiwj",tel:"13581619514",email:"redcat.cai@gmail.com",cn:1}
int UserRegSvc::sendActivation(const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[UserRegSvc::activateUser]bad format:", jsonString, 1);

    string uid		= root.getv("uid", "");
    string tel		= root.getv("tel", "");
    string email	= root.getv("email", "");
    int cn			= root.getv("cn", 0);

    if(uid.empty() || (tel.empty() && email.empty()))
		return 1;

	//手机格式判断
	string szMobile = tel;
	string iso2;
	if(!tel.empty() && !g_MobileIllegal(szMobile, iso2))
		return 1;

    if(!sendCode(uid, tel, email, cn==1))
        return 2;

    return 0;
}

//{uid:"caiwj"}
int UserRegSvc::registerInfo(const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[UserRegSvc::registerInfo]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    if(uid.empty())
		return 1;

    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "select mobile, email from t00_user where user_id = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(!psql->NextRow())
		RELEASE_MYSQL_RETURN(psql, 2);

    char mobile[64];
    char email[64];
    READMYSQL_STR(mobile, mobile);
    READMYSQL_STR(email, email);

    out << "{uid:\"" << uid << "\",tel:\"" << mobile << "\",email:\"" << email << "\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int UserRegSvc::mobileBound(const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[UserRegSvc::mobileBound]bad format:", jsonString, 1);

	string tel = root.getv("tel", "");
	if(tel.empty())
		return 1;


	//手机格式判断
	string szMobile = tel;
	string iso2;
	if(!g_MobileIllegal(szMobile, iso2))
		return 1;

	char sql[1024];
	sprintf(sql, "select count(1) as num from t00_user where mobile = '%s' and mobile_verified = '1'", tel.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql) && psql->NextRow(), 3);

	out << FormatString("{eid:0,status:%d}", atoi(psql->GetField("num")));
	RELEASE_MYSQL_RETURN(psql, 0);
}

int UserRegSvc::userEmailPwd(const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[UserRegSvc::userEmailPwd]bad format:", jsonString, 1);

    string uid   =	root.getv("uid", "");
    string email =	root.getv("email", "");
    string pwd   =  root.getv("pwd", "");
    if((uid.empty() && email.empty()) || pwd.empty())
		return 1;

    int nExists = 0;

    char sql[512];
    if(!uid.empty())
        sprintf(sql, "select count(1) as Num from t00_user where user_id = '%s' and password = '%s'", uid.c_str(), pwd.c_str());
    else
        sprintf(sql, "select count(1) as Num from t00_user where email = '%s' and password = '%s'", email.c_str(), pwd.c_str());

    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
    READMYSQL_INT(Num, nExists, 0);

    out << "{eid:0,status:" << nExists << "}";
    RELEASE_MYSQL_RETURN(psql, 0);
}


int UserRegSvc::userResetPwd(const char* jsonString, std::stringstream& out)
{
	
	JSON_PARSE_RETURN("[UserRegSvc::userResetPwd]bad format:", jsonString, 1);

	 string uid   =	root.getv("uid", "");
	 string email =	root.getv("email", "");
	 string tel   =  root.getv("tel", "");
	 trimstr(email);
	 trimstr(tel);

	
	 if (uid.empty())
	 {
		 out<<"{eid:-1,code:1}"; //用户不存在
		 return 1;
	 }


	 if (email.empty()&&tel.empty())
	 {
		 out<<"{eid:-1,code:2}"; 
		 return 1;
	 }
	 

	 char sql[512];
		sprintf(sql, "select user_id,EMAIL,MOBILE,EMAIL_VERIFIED,MOBILE_VERIFIED from t00_user where user_id = '%s'", uid.c_str());
   
	 MySql* psql = CREATE_MYSQL;
	 CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	 if (psql->GetRowCount()<=0)
	 {
		 out<<"{eid:-1,code:1}"; //用户不存在
		 return 1;

	 }
	 else
	 {
		 if (psql->NextRow())
		 {
              
			 string usremail;
			 string usrmobile;
			 string usremailverify;
			 string usrmobileverify;


			 READMYSQL_STRING(EMAIL, usremail)
			 READMYSQL_STRING(MOBILE, usrmobile)
             READMYSQL_STRING(EMAIL_VERIFIED, usremailverify)
			 READMYSQL_STRING(MOBILE_VERIFIED, usrmobileverify)

			 trimstr(usremail);
			 trimstr(usrmobile);

             int updflag=0; //0 不重置密码 1Email重置 2Mobile重置

			 if (!email.empty())
			 {

				 if (email==usremail&&usremailverify=="1")
				 {
                     
                     updflag=1;

				 }
				 else
				 {
					 updflag=0;
					 out<<"{eid:-1,code:2}"; 
				 }


			 }
			 else
			 {
				 if (!tel.empty())
				 {
					 char telStr[64];
					 sprintf(telStr, "%s", tel.c_str());

					 char mobilStr[64];
					 sprintf(mobilStr,"%s",usrmobile.c_str());

					 if (strstr(mobilStr,telStr)!=NULL&&usrmobileverify=="1")
					 {
                        updflag=2;
					 }
					 else
					 {
						 updflag=0;
                         out<<"{eid:-1,code:3}"; 
					 }
				 }
			 }


			 if (updflag>0)
			 {
                 string pwd=IntToStr(getRandomCode()); //"123456"
				 string pwdmd5=MD5EncryptHex(pwd); //"e10adc3949ba59abbe56e057f20f883e";
				 char sql1[512];
				 sprintf (sql1, "update t00_user SET PASSWORD='%s' where user_id='%s'",pwdmd5.c_str(),uid.c_str());
				 CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);
                 
                out<<"{eid:0,code:0}"; 

		
				char content[1024]="";
				sprintf(content, "BLM重置密码成功,用户名：%s  新密码：%s",uid.c_str(),pwd.c_str());

				char title[256]="BLM重置密码";
                
				if (updflag==1)//邮件通知
				{

					CNotifyMsg* pMsg = new CNotifyMsg;
					pMsg->m_szType = "mail";
					pMsg->m_MailFromUser = "blm_system";
					pMsg->m_MailFromAddr = "supports@boloomo.com";
					pMsg->m_MailToUser = uid;
					pMsg->m_MailToAddr = email;
					pMsg->m_MailSubject = string(title);
					pMsg->m_MailMsg = string(content);
					pMsg->m_MailType = 5;
					g_NotifyService::instance()->PutMessage(pMsg);
					            
 
				}
				else if (updflag==2)//短信通知
				{
					CNotifyMsg* pMsg = new CNotifyMsg;
					pMsg->m_szType = "sms";
					pMsg->m_SmsFromUser = "blm_system";
					pMsg->m_SmsToUser = uid;
					pMsg->m_SmsToTel = usrmobile;
					pMsg->m_SmsMsg = string(content);
					pMsg->m_SmsType = 5;
					g_NotifyService::instance()->PutMessage(pMsg);  

				}

			 }

		 }
	 }

    RELEASE_MYSQL_RETURN(psql, 0);

}

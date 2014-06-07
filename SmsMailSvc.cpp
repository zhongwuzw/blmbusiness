#include "blmcom_head.h"
#include "SmsMailSvc.h"
#include "SmsMail.h"
#include "MessageService.h"
#include "NotifyService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"
#include "json.h"
#include "ClientSocketMgr.h"

typedef enum
{
    CODE_SUCC = 0,
    CODE_INVALID,  
    CODE_FAIL,
    CODE_DBEXCEPTION,
    CODE_INVALID_OPERATION,
    CODE_NOMONEY,
    CODE_BUSY
} SMSCODE;

const char* CODE_DESC[] =
{
    "emsg:\"ok\"",
    "emsg:\"Invalid data\"",
    "emsg:\"Fail operation\"",
    "emsg:\"Database Exception\"",
    "emsg:\"Invalid operation\"",
    "emsg:\"Not enough money\"",
    "emsg:\"Server busy\""
};

void GetCodeDesc(int nCode, char* result, const string& __jobid="")
{
    sprintf(result, "{eid:%d,%s", nCode, CODE_DESC[nCode]);
    if(__jobid.empty())
        strcat(result, "}");
    else
    {
        char jobid[256];
        sprintf(jobid, ",jobid:\"%s\"}", __jobid.c_str());
        strcat(result, jobid);
    }
}

IMPLEMENT_SERVICE_MAP(SmsMailSvc)

SmsMailSvc::SmsMailSvc()
{
    
}

SmsMailSvc::~SmsMailSvc()
{
    
}

bool SmsMailSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_SMS_MAIL, this))
        return false;

    SERVICE_MAP(SID_SMS_SS,SmsMailSvc,sendSms);
    SERVICE_MAP(SID_SMS_MAIL,SmsMailSvc,sendMail);
	SERVICE_MAP(SID_SMS_CHAT,SmsMailSvc,sendSmsChat);
    SERVICE_MAP(SID_SMS_BAL,SmsMailSvc,getSmsBal);
	SERVICE_MAP(SID_SMS_USE,SmsMailSvc,useSmsBal);
	SERVICE_MAP(SID_SMS_FREE,SmsMailSvc,sendFreeSms);

    DEBUG_LOG("[SmsMailSvc::Start] OK......................................");
	
    return true;
}

// -1: 手机号码格式不规范
//  0: 正常
//  1: 出错
//  2: 余额不足
int SmsMailSvc::CheckAndBilling(const string& userid, const string& mobile, int nCount)
{
	string szMobile = mobile;
	string iso2;
	if(!g_MobileIllegal(szMobile, iso2))
		return -1;

	int local_bal = 0, inter_bal = 0;
	int nRet = 0;

	char sql[1024];
	sprintf(sql, "select local_bal, inter_bal from t02_user_sms_bal where user_id = '%s'", userid.c_str());
	MySql* psql = CREATE_MYSQL;

	char gmt0now[20];
	GmtNow(gmt0now);

	CHECK_MYSQL_STATUS(psql->Query(sql), 1);
	
	if(psql->NextRow())
	{
		READMYSQL_INT(local_bal, local_bal, 0);
		READMYSQL_INT(inter_bal, inter_bal, 0);
	}

	//国内的走8优（扣费按照local_bal优先扣，inter_bal次之，都没值的话不发送）
	//国外的走gateway160（只判断inter_bal是否有值，有值则发送并扣费，否则不发送）
	if(iso2 == "CN")
	{
		if(local_bal >= nCount)
		{
			local_bal -= nCount;
		}
		else if(local_bal + inter_bal >= nCount)
		{
			local_bal = 0;
			inter_bal -= (nCount - local_bal);
		}
		else
		{
			nRet = 2;
		}
	}
	else
	{
		if(inter_bal >= nCount)
		{
			inter_bal -= nCount;
		}
		else
		{
			nRet = 2;
		}
	}

	if(nRet == 0)
	{
		sprintf(sql, "update t02_user_sms_bal set local_bal='%d', inter_bal='%d', last_upd_dt='%s' where user_id = '%s'",
			local_bal, inter_bal, gmt0now, userid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql), 1);
	}
	
	RELEASE_MYSQL_RETURN(psql, nRet);
}

//{id:"wangjun",tel:"+86 13811304835",msg:"Hello"}
int SmsMailSvc::sendSms(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[SmsMailSvc::sendSms]bad format:", jsonString, 1);

    string userid = root.getv("id", "");
    string mobile = root.getv("tel", "");
    string msg = CodeConverter::Utf8ToGb2312(root.getv("msg", ""));

    if(userid.empty() || msg.empty())
	{
		DEBUG_LOG("[SmsMailSvc::sendSms]bad sendSms:", jsonString);
		return 1;
	}

    int nRet = 0;
	CNotifyMsg* pMsg;

	switch(CheckAndBilling(userid, mobile))
	{
	case -1:
		{
			DEBUG_LOG("[SmsMailSvc::sendSms]bad sendSms:", jsonString);
			return 1;
		}
		break;
	case 0:
		{
			pMsg = new CNotifyMsg;
			pMsg->m_szType = "sms";
			pMsg->m_SmsFromUser = userid;
			pMsg->m_SmsToUser = "";
			pMsg->m_SmsToTel = mobile;
			pMsg->m_SmsMsg = msg;
			pMsg->m_SmsType = 0;
			g_NotifyService::instance()->PutMessage(pMsg);
		}
		break;
	case 1:
		{
			nRet = CODE_FAIL;
		}
		break;
	case 2:
		{
			nRet = CODE_NOMONEY;
		}
		break;
	}

    char retString[MAX_BUFF_1024] = {'\0'};
    GetCodeDesc(nRet, retString);

    out << retString;
    return 0;
}

//{"jobid":"123","id":"caiwj","to":"+86 13581619514,caiwj|+86 13581619515|+86 13581619516,bird","msg":"hell world","type":"0","cost":"1"}
int SmsMailSvc::sendSmsChat(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[SmsMailSvc::sendSmsChat]bad format:", jsonString, 1);

    string jobid = root.getv("jobid", "");
    string userid = root.getv("id", "");
    string to = root.getv("to", "");
	string msg = root.getv("msg", "");
	StrReplace(msg, "—", "-");
    msg = CodeConverter::Utf8ToGb2312(msg.c_str());
    int type = atoi(root.getv("type", ""));
    int cost = atoi(root.getv("cost", ""));

    Tokens toMobiles;
    Tokens toUsers;

    Tokens users = StrSplit(to, "|");
    if(!users.empty())
    {
        for(int i=0; i<(int)users.size(); i++)
        {
            Tokens userTels = StrSplit(users[i], ",");
            if(!userTels.empty())
            {
				toMobiles.push_back(userTels[0]);
				toUsers.push_back((userTels.size()==2)?userTels[1]:"");
            }
        }
    }

    if(userid.empty() || msg.empty() || toMobiles.empty())
		return 1;

    int nRet = 0;
	for(int i=0; i<(int)toMobiles.size(); i++)
	{
		nRet = CheckAndBilling(userid, toMobiles[i]);
		if(nRet != 0)
			break;

		CNotifyMsg* pMsg = new CNotifyMsg;
		pMsg->m_szType = "sms";
		pMsg->m_SmsFromUser = userid;
		pMsg->m_SmsToUser = toUsers[i];
		pMsg->m_SmsToTel = toMobiles[i];
		pMsg->m_SmsMsg = msg;
		pMsg->m_SmsType = 0;
		g_NotifyService::instance()->PutMessage(pMsg);
	}

    char retString[MAX_BUFF_1024] = {'\0'};
    GetCodeDesc(nRet, retString, jobid);

    out << retString;
    return 0;
}

int SmsMailSvc::sendFreeSms(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SmsMailSvc::sendFreeSms]bad format:", jsonString, 1);

    string to = root.getv("to", "");
	string msg = root.getv("msg", "");
	StrReplace(msg, "—", "-");
    //msg = CodeConverter::Utf8ToGb2312(msg.c_str());

    Tokens toMobiles;
    Tokens toUsers;

    Tokens users = StrSplit(to, "|");
    if(!users.empty())
    {
        for(int i=0; i<(int)users.size(); i++)
        {
            Tokens userTels = StrSplit(users[i], ",");
            if(!userTels.empty())
            {
				toMobiles.push_back(userTels[0]);
				toUsers.push_back((userTels.size()==2)?userTels[1]:"");
            }
        }
    }

    if(msg.empty() || toMobiles.empty())
		return 1;

    int nRet = 0;
	for(int i=0; i<(int)toMobiles.size(); i++)
	{
		CNotifyMsg* pMsg = new CNotifyMsg;
		pMsg->m_szType = "sms";
		pMsg->m_SmsFromUser = "blm_system";
		pMsg->m_SmsToUser = toUsers[i];
		pMsg->m_SmsToTel = toMobiles[i];
		pMsg->m_SmsMsg = msg;
		pMsg->m_SmsType = 0;
		g_NotifyService::instance()->PutMessage(pMsg);
	}

    char retString[MAX_BUFF_1024] = {'\0'};
    GetCodeDesc(nRet, retString, "");

    out << retString;
    return 0;
}

//{"id":"caiwj","to":"jiangtao@boloomo.com,caiwj|caiwj@boloomo.com|niechao@boloomo.com,niechao","cc:":"caiwj@boloomo.com|ais@boloomo.com","subject":"来自博懋的邮件通知","msg":"JIANG YIN HAO 2011-12-28 11:04:21在Shih-Peng Chiang港113度方向17海里处(29.6495N&122.573E)离开舟山区域","type":"0"}
int SmsMailSvc::sendMail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[SmsMailSvc::sendMail]bad format:", jsonString, 1);

    string	userid = root.getv("id", "");
    string	to = root.getv("to", "");
	string  cc = root.getv("cc", "");
    string	msg = CodeConverter::Utf8ToGb2312(root.getv("msg", ""));
    string	subject = CodeConverter::Utf8ToGb2312(root.getv("subject", ""));
	string  attach = "";
    int		type = atoi(root.getv("type", ""));

    if(userid.empty() || to.empty())
		return 1;

    msg.append("\nFrom BLM_Shipping( http://www.boloomo.com ) ");
    msg.append(userid);

    Tokens toMails, ccMails, toUsers;
    Tokens mailUsers = StrSplit(to, "|");
	ccMails = StrSplit(cc, "|");
    if(!mailUsers.empty())
    {
        for(int i=0; i<(int)mailUsers.size(); i++)
        {
            Tokens mailUser = StrSplit(mailUsers[i], ",");
            if(!mailUser.empty())
            {
                toMails.push_back(mailUser[0]);
                toUsers.push_back((mailUser.size()==2)?mailUser[1]:"");
            }
        }
    }

	for(int i=0; i<(int)toMails.size(); i++)
	{
		CNotifyMsg* pMsg = new CNotifyMsg;
		pMsg->m_szType = "mail";
		pMsg->m_MailFromUser = userid;
		pMsg->m_MailFromAddr = "subscribe@boloomo.com";
		pMsg->m_MailToUser = toUsers[i];
		pMsg->m_MailToAddr = toMails[i];
		if(i==0)
			pMsg->m_MailCcAddrs = ccMails;
		pMsg->m_MailSubject = subject;
		pMsg->m_MailMsg = msg;
		pMsg->m_MailAttach = attach;
		pMsg->m_MailType = type;
		g_NotifyService::instance()->PutMessage(pMsg);
	}

    out << MSG_SUCC;
    return 0;
}

int SmsMailSvc::getSmsBal(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[SmsMailSvc::getSmsBal]bad format:", jsonString, 1);

    string userid = root.getv("id", "");
    if(userid.empty())
		return 1;

    int balance = 0, inter_bal = 0;
    char sql[1024];
    sprintf(sql, "select local_bal, inter_bal from t02_user_sms_bal where user_id = '%s'", userid.c_str());
    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
	{
		balance = atoi(NOTNULL(psql->GetField("local_bal")));
		inter_bal = atoi(NOTNULL(psql->GetField("inter_bal")));
	}

	out << FormatString("{eid:0,emsg:\"ok\",bal:\"%d\"}", (balance+inter_bal));
    RELEASE_MYSQL_RETURN(psql, 0);
}

int SmsMailSvc::useSmsBal(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SmsMailSvc::useSmsBal]bad format:", jsonString, 1);

	string userid = root.getv("id", "");
	if(userid.empty())
		return 1;

	int count = root.getv("count", 0);
	if(count == 0)
		return 1;

	int balance = 0;
	char sql[1024];
	sprintf(sql, "select local_bal from t02_user_sms_bal where user_id = '%s'", userid.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
	{
		balance = atoi(NOTNULL(psql->GetField("local_bal")));
		balance -= count;
		if(balance < 0)
			balance = 0;

		sprintf(sql, "update t02_user_sms_bal set local_bal = '%d' where user_id = '%s'", balance, userid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	out << FormatString("{eid:0,emsg:\"ok\",bal:\"%d\"}", balance);
	RELEASE_MYSQL_RETURN(psql, 0);
}

#include "MobileSvc.h"
#include "blmcom_head.h"
#include "MessageService.h"
#include "UserRegSvc.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "ObjectPool.h"
#include "Util.h"
#include "kSQL.h"
#include "json.h"

IMPLEMENT_SERVICE_MAP(MobileSvc)

MobileSvc::MobileSvc(void)
{
}

MobileSvc::~MobileSvc(void)
{
}

bool MobileSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_MOBILEBIND, this))
        return false;

	SERVICE_MAP(SID_GETUSERBINDMOBILE,MobileSvc,GetUserMobileBindInfo);	//获取用户绑定手机信息
    SERVICE_MAP(SID_MSGUNIT,MobileSvc,GetMsgUnit);						//获取每条短信价格
    SERVICE_MAP(SID_EXECHANGE,MobileSvc,GetExechange);					//获取汇率

    DEBUG_LOG("[MobileSvc::Start] OK......................................");
    return true;
}

int MobileSvc::GetUserMobileBindInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[MobileSvc::GetUserMobileBindInfo]bad format:", jsonString, 1);

	char sql[1024];
	sprintf(sql, "SELECT T.USER_ID,T.mobile ,T.country, T.mobile_verified ,T2.LOCAL_BAL, t2.inter_bal FROM T00_USER T"\
		" LEFT OUTER JOIN t02_user_sms_bal T2 "\
		" ON T.USER_ID=T2.USER_ID"
		" WHERE T.USER_ID = '%s'", pUid);
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	const char* userid = psql->GetField("USER_ID");
	const char* telno = psql->GetField("mobile");
	const char* countryno = psql->GetField("country");
	const char* smsstatus = psql->GetField("mobile_verified");
	const char* smscount = psql->GetField("LOCAL_BAL");
	const char* inter = psql->GetField("inter_bal");

	out<<FormatString("{userid:\"%s\",smsstatus:%d,countryno:\"%s\",telno:\"%s\",nsms:%d}",
		NOTNULL(userid),
		atoi(NOTNULL(smsstatus)),
		NOTNULL(countryno),
		NOTNULL(telno),
		atoi(NOTNULL(smscount))+atoi(NOTNULL(inter)));

	RELEASE_MYSQL_RETURN(psql, 0);
}

int MobileSvc::GetMsgUnit(const char* pUid, const char* jsonString, std::stringstream& out)
{
	MySql *psql = CREATE_MYSQL;

	out<<"{cn:\"1.00\",en:\"0.80\"";

	const char* sql1 = "select curcode, exchange from t41_exchange where curcode = 'usd' order by start_dt desc limit 1";
	const char* sql2 = "select curcode, exchange from t41_exchange where curcode = 'eur' order by start_dt desc limit 1";

	CHECK_MYSQL_STATUS(psql->Query(sql1)&&psql->NextRow(), 3);
	const char* exchange = psql->GetField("exchange");
	out<<FormatString(",exc_usd:\"%s\"", exchange);

	CHECK_MYSQL_STATUS(psql->Query(sql2)&&psql->NextRow(), 3);
	exchange = psql->GetField("exchange");
	out<<FormatString(",exc_eur:\"%s\"", exchange);

	out<<"}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MobileSvc::GetExechange(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MobileSvc::GetExechange]bad format:", jsonString, 1);

    MySql *psql = CREATE_MYSQL;
    const char* sql1 = "select curcode, exchange from t41_exchange where curcode = 'usd' order by start_dt desc limit 1";
	const char* sql2 = "select curcode, exchange from t41_exchange where curcode = 'eur' order by start_dt desc limit 1";
	
	out<<"{bsucc:1";

	CHECK_MYSQL_STATUS(psql->Query(sql1)&&psql->NextRow(), 3);
	const char* exchange = psql->GetField("exchange");
	out<<FormatString(",exc_usd:\"%s\"", exchange);

	CHECK_MYSQL_STATUS(psql->Query(sql2)&&psql->NextRow(), 3);
	exchange = psql->GetField("exchange");
	out<<FormatString(",exc_eur:\"%s\"", exchange);
	out<<"}";
    
    RELEASE_MYSQL_RETURN(psql, 0);
}

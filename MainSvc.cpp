#include "blmcom_head.h"
#include "MainSvc.h"
#include "MessageService.h"
#include "NotifyService.h"
#include "SmsMail.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "kSQL.h"
#include "Util.h"
#include "GeoipHandler.h"
#include "json.h"

using namespace std;

IMPLEMENT_SERVICE_MAP(MainSvc)

MainSvc::MainSvc()
{
}

MainSvc::~MainSvc()
{
}

bool MainSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_MAINBLM, this))
        return false;

	int interval = 3600;
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
	if(timerId <= 0)
		return false;

    SERVICE_MAP(SID_REQGETUSERINFO,MainSvc,getUserInfo);
    SERVICE_MAP(SID_UPDATEUSERINFO,MainSvc,updateUserInfo);
    SERVICE_MAP(SID_REMARKMANAGE,MainSvc,manageRemark);
    SERVICE_MAP(SID_REQDAILYTIP,MainSvc,getDaylayTip);
    SERVICE_MAP(SID_IP_INFO,MainSvc,getIpInfo);
    SERVICE_MAP(SID_USER_IPINFO,MainSvc,getUserIpinfo);
    SERVICE_MAP(SID_REQNOTICE2,MainSvc,getNotice2);
    SERVICE_MAP(SID_REQUSERAUTHORITY,MainSvc,getUserAuthority);
    SERVICE_MAP(SID_GETUPDATE,MainSvc,getUpdate);
	SERVICE_MAP(SID_USER_COUNTRY,MainSvc,getUserCountry);

	stringstream out;
	getUserAuthority("sunliang", "", out);

	return true;
}

int MainSvc::getUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	string uid;
	if(!strlen(jsonString))
		uid = pUid;
	else
	{
		JSON_PARSE_RETURN("[MainSvc::getUserInfo]bad format:", jsonString, 1);
		uid = root.getv("uid", "");
	}
	
    Personality person;

    // 1.获取用户基本信息
    char sql[1024];
    sprintf(sql, "select city_code,password,birthday,gender,salutation,firstname,lastname,creationdate,modificationdate,telno,fax,mobile,email,zip,address,ip,country,nickname,sign,curr_city,country,avatar_id,usertype,email_verified,mobile_verified from t00_user where user_id = '%s' and is_flag = '1'", uid.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);//&&psql->NextRow()

	if (psql->NextRow())
	{
		person.nickname=NOTNULL(psql->GetField("nickname"));
		person.firstname=NOTNULL(psql->GetField("firstname"));;
		person.lastname = NOTNULL(psql->GetField("lastname"));
		person.birthday = NOTNULL(psql->GetField("birthday"));
		READMYSQL_STR(gender,person.gender);
		person.salutation = NOTNULL(psql->GetField("salutation"));
		person.sign = NOTNULL(psql->GetField("sign"));
		READMYSQL_STR(country,person.country);
		person.email = NOTNULL(psql->GetField("email"));
		person.telno = NOTNULL(psql->GetField("telno"));
		person.mobile = NOTNULL(psql->GetField("mobile"));
		person.zip = NOTNULL(psql->GetField("zip"));
		person.fax = NOTNULL(psql->GetField("fax"));
		person.address = NOTNULL(psql->GetField("address"));
		person.city = NOTNULL(psql->GetField("curr_city"));
		READMYSQL_INT(email_verified, person.email_verified, 0);
		READMYSQL_INT(mobile_verified, person.mobile_verified, 0);
		READMYSQL_INT(avatar_id,person.avatar_id,0);
		person.nusertype = atoi(NOTNULL(psql->GetField("usertype")));
	}
	else 
	{
		//zhuxj
		out << "{eid:0, active:0}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}

    // 2.获取用户服务类型信息
    sprintf(sql, "select servicetype_code from t00_user_activities where user_id = '%s'", uid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	person.servicetype_code = "";
	if(psql->NextRow())
	{
		 person.servicetype_code = NOTNULL(psql->GetField("servicetype_code"));
	}
    
    //3.获取用户公司信息
    sprintf(sql, "SELECT t1.company_key,t2.Name FROM t41_comp_contacts t1"\
          " LEFT OUTER JOIN t41_company t2"\
          " ON t1.COMPANY_KEY = t2.Company_Key"\
          " WHERE t1.USER_ID='%s' AND t1.flag='1'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    if(psql->NextRow())
	{
		person.compid = NOTNULL(psql->GetField("company_key"));
		person.compname = NOTNULL(psql->GetField("Name"));
    }

    // 格式化输出
    out<<"{user:\""<<uid<<"\""
       <<",nick:\""<<person.nickname<<"\""
       <<",first:\""<<person.firstname<<"\""
       <<",last:\""<<person.lastname<<"\""
       <<",birth:\""<<person.birthday<<"\""
       <<",gender:\""<<person.gender<<"\""
       <<",sal:\""<<person.salutation<<"\""
       <<",sign:\""<<person.sign<<"\""
       <<",country:\""<<person.country<<"\""
       <<",email:\""<<person.email<<"\""
       <<",tel:\""<<person.telno<<"\""
       <<",mobile:\""<<person.mobile<<"\""
       <<",zip:\""<<person.zip<<"\""
       <<",fax:\""<<person.fax<<"\""
       <<",addr:\""<<person.address<<"\""
       <<",city:\""<<person.city<<"\""
       <<",email_vd:"<<person.email_verified
       <<",mobile_vd:"<<person.mobile_verified
       <<",javatar:"<<person.avatar_id
       <<",st_code:\""<<person.servicetype_code<<"\""
       <<",compid:\""<<person.compid<<"\""
       <<",compname:\""<<person.compname<<"\""
       <<",usertp:"<<person.nusertype<<"}";
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

int MainSvc::updateUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MainSvc::updateUserInfo]bad format:", jsonString, 1);

	//获取请求数据
    Personality person;
	string uid			= root.getv("uid", "");
    person.nickname		= root.getv("nick", "");
    person.firstname	= root.getv("first", "");
    person.lastname		= root.getv("last", "");
    person.sign			= root.getv("sign", "");
    person.birthday		= root.getv("birth", "");
    person.address		= root.getv("addr", "");
    person.city			= root.getv("city", "");
    person.servicetype_code = root.getv("st_code", "");
    person.mobile		= root.getv("mobile", "");
    person.email		= root.getv("email", "");
   
	if(uid.empty())
		uid = pUid;
	//手机号码格式加判断
	string szMobile = person.mobile;
	string iso2;
	if(!szMobile.empty() && !g_MobileIllegal(szMobile, iso2))
		return 1;

    //判断手机和邮箱是否改变
	//若用户只是手机验证，邮箱未验证，则更改手机号之后必须验证，否则不保存更改的手机号 
	//若用户只是邮箱验证，手机未验证，则更改邮箱之后必须验证，否则不保存更改的邮箱 
	//若用户手机和邮件均已验证，则仅修改手机号或邮箱，不验证也可以保存 

    char sql[1024];
    sprintf(sql, "select mobile, email, mobile_verified, email_verified from t00_user where user_id = '%s'", uid.c_str());
    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
    {
        char mobile[64];
        char email[64];
		int mobile_verified = 0;
		int email_verified = 0;

        READMYSQL_STR(mobile, mobile);
        READMYSQL_STR(email, email);
		READMYSQL_INT(mobile_verified, mobile_verified, 0);
		READMYSQL_INT(email_verified, email_verified, 0);

        if(strcmp(mobile, person.mobile.c_str()))
        {
            sprintf(sql, "update t00_user set mobile_verified = 0 where user_id = '%s'",  uid.c_str());
            CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
        }
        if(strcmp(email, person.email.c_str()))
        {
            sprintf(sql, "update t00_user set email_verified = 0 where user_id = '%s'", uid.c_str());
            CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
        }
    }

    //获取GMT0的当前时间
    char gmt0now[20];
    GmtNow(gmt0now);

    sprintf(sql, "update t00_user set nickname = '%s',firstname = '%s',lastname = '%s',telno = '%s',mobile = '%s',email = '%s',country = '%s',gender = '%s',salutation = '%s',sign = '%s',birthday = '%s',zip = '%s',fax = '%s',address = '%s',curr_city = '%s', modificationdate = '%s' where user_id ='%s'",
             (person.nickname).c_str(),
             (person.firstname).c_str(),
             (person.lastname).c_str(),
             root.getv("tel", ""),
             root.getv("mobile", ""),
             root.getv("email", ""),
             root.getv("country", ""),
             root.getv("gender", ""),
             root.getv("sal", ""),
             (person.sign).c_str(),
             person.birthday.c_str(),
             root.getv("zip", ""),
             root.getv("fax", ""),
             (person.address).c_str(),
             (person.city).c_str(),
             gmt0now,
             uid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    sprintf(sql, "replace into t00_user_activities (user_id, servicetype_code) values ('%s','%s')", uid.c_str(), root.getv("st_code", ""));
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MainSvc::manageRemark(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MainSvc::manageRemark]bad format:", jsonString, 1);

    int  optype		= atoi(root.getv("optype", ""));
	string type		= root.getv("type", "");
	string objectid = root.getv("objectid", "");
	string seq		= root.getv("seq", "");
	string subject	= root.getv("subject", "");
	string body		= root.getv("body", "");

    subject = StrReplace(subject,"'","\\'");
    body = StrReplace(body,"'","\\'");

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;
    char sql[8192] ;
    if(optype==1)			//增
    {
        sprintf(sql, "insert into t41_remark(USER_ID, TYPE, OBJECT_ID, SEQ, SUBJECT, BODY, CREATE_DATE,UPDATE_DATE)" \
                 "values ('%s', %s, '%s', %s, '%s', '%s', UNIX_TIMESTAMP('%s') ,UNIX_TIMESTAMP('%s') )",
				 pUid,type.c_str(),objectid.c_str(),seq.c_str(),subject.c_str(),body.c_str(),gmt0now,gmt0now);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==2)		//删
    {
        if(seq.length()>0)
        {
            sprintf(sql,"delete from t41_remark  " \
                     " where USER_ID = '%s' and type = '%s' and OBJECT_ID = '%s' and seq = '%s' ",pUid,type.c_str(),objectid.c_str(),seq.c_str());
        }
        else
        {
            sprintf(sql,"delete from t41_remark  " \
                     " where USER_ID = '%s' and type = '%s' and OBJECT_ID = '%s' ",pUid,type.c_str(),objectid.c_str());
        }
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==3)		//改
    {
        sprintf(sql, "update t41_remark set SUBJECT = '%s',body='%s',update_date= UNIX_TIMESTAMP('%s') " \
                 " where USER_ID = '%s' and type = '%s' and OBJECT_ID = '%s' and seq = '%s' ",subject.c_str(),body.c_str(),gmt0now,pUid,type.c_str(),objectid.c_str(),seq.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    else if(optype==4)		//查
    {
        if(seq.length()>0)
        {
            sprintf(sql, "select SEQ, SUBJECT, BODY, CREATE_DATE, UPDATE_DATE from t41_remark   " \
                     " where USER_ID = '%s' and type = '%s' and OBJECT_ID = '%s' and seq = '%s' ",pUid,type.c_str(),objectid.c_str(),seq.c_str());
        }
        else
        {
            sprintf(sql, "select SEQ, SUBJECT, BODY, CREATE_DATE, UPDATE_DATE from t41_remark   " \
                     " where USER_ID = '%s' and type = '%s' and OBJECT_ID = '%s' ",pUid,type.c_str(),objectid.c_str());
        }
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		//返回 [ "SEQ|SUBJECT|BODY|CREATE_DATE|UPDATE_DATE","SEQ|SUBJECT|BODY|CREATE_DATE|UPDATE_DATE"]
		//[{seq:"seq",sub:"a",body:"d",cdate:"c",udate:"u"},{seq:"seq",sub:"a",body:"d",cdate:"c",udate:"u"}]
		int total = 0;
		out<<"[";
		while(psql->NextRow())
		{
			if(total != 0)
				out << ",";
			total++;

			string qbodystr = NOTNULL(psql->GetField("BODY"));
			string qsub = NOTNULL(psql->GetField("SUBJECT"));
			string seq = NOTNULL(psql->GetField("SEQ"));
			string creatdt = NOTNULL(psql->GetField("CREATE_DATE"));
			string updatedt = NOTNULL(psql->GetField("UPDATE_DATE"));
			qbodystr = StrReplace(qbodystr,"\"","\\\"");
			qsub = StrReplace(qsub,"\"","\\\"");

			out<<"{seq:\""<<seq<<"\",sub:\""<<qsub<<"\",body:\""<<qbodystr<<"\",cdate:\""<<creatdt<<"\",udate:\""<<updatedt<<"\"}";
		}
		out<<"]";
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MainSvc::getDaylayTip(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MainSvc::getDaylayTip]bad format:", jsonString, 1);
	string language = root.getv("lan", "");

	char sql[1024];
    sprintf(sql, "select t1.tip_id,t1.content,t2.publish_dt from t00_usagetips t1,t00_usagetips_user t2 where t2.user_id='%s' and t1.tip_id=t2.tip_id and t1.language='%s' order by t2.publish_dt asc",
		pUid, language.c_str());

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out<<"[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		out<<"{id:\""<<NOTNULL(psql->GetField("tip_id"))
			<<"\",dt:\""<<NOTNULL(psql->GetField("publish_dt"))
			<<"\",info:\""<<NOTNULL(psql->GetField("content"))
			<<"\"}";
	}
	out<<"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MainSvc::getIpInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MainSvc::getIpInfo]bad format:", jsonString, 1);
	string uid = root.getv("userid", "");
	string ip  = root.getv("ip", "");

	Tokens ips = StrSplit(ip, "|");
	if(ips.empty())
		return 1;

    out<<"[";
    for(int i = 0; i<(int)ips.size(); i++)
    {
		string record;
        unsigned long oneip = strtoul(ips[i].c_str(),NULL,10);
        string oneipstr		= Long2Ip(oneip);
		GeoIPRecord  *gir	= g_GeoipHandler::instance()->GetRecordByName(oneipstr.c_str());
		if(!gir)
		{
			record = "\"\"";
		}
		else
        {
            char **ret = g_GeoipHandler::instance()->GetRangeByName(oneipstr.c_str());
            char oneipres[128];
            snprintf(oneipres,sizeof(oneipres),"\"%ld|%s|%s|%s|%f|%f\"",(long)oneip,gir->country_code3,NOTNULL(GeoIP_region_name_by_code(gir->country_code, gir->region)),NOTNULL(gir->city),gir->longitude,gir->latitude);
            record = oneipres;
            GeoIP_range_by_ip_delete(ret);
            GeoIPRecord_delete(gir);
        }
        if(i == 0)
			out << record;
		else
			out << "," << record;
    }
    out<<"]";

    return 0;
}

int MainSvc::getUserIpinfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MainSvc::getUserIpinfo]bad format:", jsonString, 1);

    string usridstr="";
    for (int i=0; i<(int)root.size(); i++)
    {
        if(i!=0) usridstr+=",";
        usridstr+="'"+string(root.getv(i, ""))+"'";
    }
	if(usridstr.empty())
		return 1;

	char sql[4*1024];
    sprintf(sql, "select user_id,ip from t00_user_ip where user_id in (%s)", usridstr.c_str());

	StrStrMap UsrIpMap;
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	while(psql->NextRow())
	{
		char *userID=(char *)psql->GetField("user_id");
		char *oneipstr = (char *)psql->GetField("ip");
		if(!oneipstr)
			continue;
		UsrIpMap[string(userID)] = string(oneipstr);
	}

    int total=0;
    StrStrMapIter it=UsrIpMap.begin();
    out<<"[";
    for (; it!=UsrIpMap.end(); it++)
    {
        string userIdStr=it->first;
        string userIpStr=it->second;
		GeoIPRecord  *gir= g_GeoipHandler::instance()->GetRecordByName(userIpStr.c_str());
        if (gir != NULL)
        {
            const char *timezone_dd="8";
            const char *timezonestr="";
            char *ipRegion=gir->country_code;
            char *regioncode=gir->region;
            sprintf(sql, "select standtime,standtime_dd from t91_region_timezone where ipregion='%s' and region='%s'", ipRegion, regioncode);
            CHECK_MYSQL_STATUS(psql->Query(sql), 3);
            
			if (psql->NextRow())
			{
				timezonestr=NOTNULL(psql->GetField("standtime"));
				timezone_dd=NOTNULL(psql->GetField("standtime_dd"));
			}
			else
			{
				sprintf(sql, "select standtime,standtime_dd from t91_region_timezone where ipregion='%s' and region=''", ipRegion);
				CHECK_MYSQL_STATUS(psql->Query(sql), 3);
				if (psql->NextRow())
				{
					timezonestr=NOTNULL(psql->GetField("standtime"));
					timezone_dd=NOTNULL(psql->GetField("standtime_dd"));
				}
			}

            if (total != 0)
                out<<",";
			total++;

            char oneipres[256];
            snprintf(oneipres,sizeof(oneipres),"\"%s|%s|%s|%s|%s|%f|%f|%s|%s|%s\"",
				userIdStr.c_str(),userIpStr.c_str(),NOTNULL(gir->country_name),NOTNULL(GeoIP_region_name_by_code(gir->country_code, gir->region)),NOTNULL(gir->city),gir->longitude,gir->latitude,gir->country_code3,timezone_dd,timezonestr);
            out<<oneipres;

            GeoIPRecord_delete(gir);
        }
    }
    out<<"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MainSvc::getNotice2(const char* pUid, const char* jsonString, std::stringstream& out)
{
	//获取GMT0的当前时间
	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
    sprintf(sql, "select notice_id,notice_title,notice_sumary,publish_dt,language from t00_notice "\
                " where pubstatus='2' and (INVALIDATE_DT IS  NULL or INVALIDATE_DT>='%s')"\
                " order by publish_dt desc", gmt0now);
    
    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		const char* pTitle = psql->GetField("notice_title");
		const char* pSumary = psql->GetField("notice_sumary");
		if(!pTitle || !pSumary)
			continue;

		if(total != 0)
			out << ",";
		total++;

		out<<"{id:\""<<NOTNULL(psql->GetField("notice_id"))
			<<"\",tt:\""<< JsonReplace(pTitle).c_str()
			<<"\",sm:\""<< JsonReplace(pSumary).c_str()
			<<"\",pb:\""<<NOTNULL(psql->GetField("publish_dt"))
			<<"\",la:\""<<NOTNULL(psql->GetField("language"))
			<<"\"}";
	}
	out<<"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MainSvc::getUserAuthority(const char* pUid, const char* jsonString, std::stringstream& out)
{
	string uid = "";
	if (strlen(jsonString))
	{
		JSON_PARSE_RETURN("[MainSvc::getUserAuthority]bad format:", jsonString, 1);
		uid = root.getv("uid", "");
	}

	if (!uid.length())
		uid = pUid;

	char sql[1024];
	MySql* psql = CREATE_MYSQL;

	int usertype = 0;
	std::string strproduct = "";
	std::string strverify = "0";
	std::string strviewsta = "0";
	std::string strviewchart = "0";
	double satbal = 0;

	// 1.用户认证
	sprintf(sql, "SELECT t1.usertype, t1.user_verify, t2.product FROM t00_user t1 left join t00_user_ip t2 on t1.user_id = t2.user_id WHERE t1.user_id = '%s' AND t1.is_flag = '1'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
	{
		usertype = atoi(NOTNULL(psql->GetField("usertype")));
		strverify = NOTNULL(psql->GetField("user_verify"));
		if(strverify.empty())
			strverify = "0";
		strproduct = NOTNULL(psql->GetField("product"));

		// 2.权限判断
		char gmt0now[20];
		GmtNow(gmt0now);

		sprintf(sql, "SELECT product_id, balance - (UNIX_TIMESTAMP('%s') - UNIX_TIMESTAMP(Last_udp_dt))/3600/24 AS balance FROM t02_product_balance \
					 WHERE user_id = '%s' AND PRODUCT_ID in ('blmchartcn','blmchartus','blmchartall','BLMPENTERPRISEAIS','BLMPGLOBALAIS')", gmt0now, uid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		map<string,	double, NoCaseCmp> productBalanceMap;
		map<string, double, NoCaseCmp>::iterator iter, iter2, iter3;

		while(psql->NextRow())
		{
			const char* product = psql->GetField("product_id");
			const char* balance = NOTNULL(psql->GetField("balance"));
			productBalanceMap[product] = atof(balance);
		}

		// 2.1 卫星权限：包括购买卫星版的用户以及亲情版外贸、内部用户
		
		if((usertype == 8 || usertype == 22 || usertype == 1) && strproduct=="blm_family")	//外贸版和亲情大使在blm_family版本下有卫星权限
		{
			strviewsta = "1";
			iter = productBalanceMap.find("BLMPGLOBALAIS");
			satbal = iter->second;
		}
		else if(usertype == 9 && strproduct=="blm_family")		//内贸版没有卫星权限
		{
			strviewsta = "0";
		}
		else													//普通用户,查看卫星余额表
		{
			iter = productBalanceMap.find("BLMPGLOBALAIS");
			if(iter != productBalanceMap.end() && iter->second > 0)
			{
				strviewsta = "1";	
				satbal = iter->second;
			}
			else
			{
				iter = productBalanceMap.find("BLMPENTERPRISEAIS");
				if(iter != productBalanceMap.end() && iter->second > 0)
				{
					strviewsta = "2";	
					satbal = iter->second;
				}
			}
		}

		if (strviewsta == "0" && usertype != 9 && strproduct != "blm_family")
		{
			sprintf(sql, "SELECT product_id, balance - (UNIX_TIMESTAMP('%s') - UNIX_TIMESTAMP(Last_udp_dt))/3600/24 AS balance FROM t02_product_balance \
						 WHERE user_id IN (SELECT DEVICEUSERID FROM t00_user_deviceus WHERE USERID = '%s') AND PRODUCT_ID IN ('BLMPENTERPRISEAIS','BLMPGLOBALAIS')", gmt0now, uid.c_str());
			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
			while (psql->NextRow())
			{
				const char* product = psql->GetField("product_id");
				double balance = atof(NOTNULL(psql->GetField("balance")));
				if (!strcmp(product, "BLMPGLOBALAIS") && balance > 0.0 && satbal < balance)
				{
					strviewsta = "1";
					satbal = balance;
				}
				if (!strcmp(product, "BLMPENTERPRISEAIS") && balance > 0.0 && satbal < balance && strviewsta != "1")
				{
					strviewsta = "2";
					satbal = balance;
				}
			}
		}

		// 2.2 海图权限
		bool bAll, bCn, bUsa;
		bAll = bCn = bUsa = false;

		iter = productBalanceMap.find("blmchartall");
		iter2 = productBalanceMap.find("blmchartcn");
		iter3 = productBalanceMap.find("blmchartus");
		if(iter != productBalanceMap.end() && iter->second > 0)
		{
			bAll = true;
		}
		if(iter2 != productBalanceMap.end() && iter2->second > 0)
		{
			bCn = true;
		}
		if(iter3 != productBalanceMap.end() && iter3->second > 0)
		{
			bUsa = true;
		}

		if(bAll || (bCn&&bUsa))
			strviewchart = "3";
		else if(bCn)
			strviewchart = "1";
		else if(bUsa)
			strviewchart = "2";
	}
	else
	{
		
	}
	
	sprintf(sql, "{nverified:\"%s\",nviewsat:\"%s\",nviewchart:\"%s\",satbal:\"%d\",chartdate:\"0-0\"}", strverify.c_str(), strviewsta.c_str(), strviewchart.c_str(), (int)satbal);
	out << sql;

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{Id:\"BLM\",Ver:\"1.0.0.1\"}
int MainSvc::getUpdate(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MainSvc::getUpdate]bad format:", jsonString, 1);
    std::string id  =root.getv("Id", "");
    std::string ver =root.getv("Ver", "");
 
    char sql[1024];
    sprintf(sql, "SELECT item_text FROM T00_SYSTEM_PLUGIN WHERE PLUGIN_ID = '%s' AND VERSION = '%s'", id.c_str(), ver.c_str());
    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out << "[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* item_text = psql->GetField("ITEM_TEXT");
		out << "\"" << NOTNULL(item_text) << "\"";
	}
	out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

// {uid:"sdfad"}
// {eid:0,areacode:"86"}

int MainSvc::getUserCountry(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[MainSvc::getUserCountry]bad format:", jsonString, 1);
	std::string uid = root.getv("uid", "");
	if(uid.empty())
		return 1;

	char sql[1024];
	sprintf(sql, "SELECT t2.telcode FROM t00_user t1, t91_country t2 WHERE t1.user_id = '%s' AND t1.COUNTRY = t2.ISO3", uid.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
	{
		out << FormatString("{eid:0,areacode:\"%s\"}", NOTNULL(psql->GetField("telcode")));
	}
	else
	{
		out << "{eid:0,areacode:\"\"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

int MainSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	return 0;
}

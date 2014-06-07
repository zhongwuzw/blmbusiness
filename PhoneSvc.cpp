#include "blmcom_head.h"
#include "PhoneSvc.h"
#include "UserRegSvc.h"
#include "MessageService.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include "NotifyService.h"
#include <string.h>

using namespace std;

IMPLEMENT_SERVICE_MAP2(PhoneSvc)

PhoneSvc::PhoneSvc()
{

}

PhoneSvc::~PhoneSvc()
{
}

bool PhoneSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_PHONE, this))
        return false;

    SERVICE_MAP(SID_GETPHONESERVER,PhoneSvc,GetPhoneServer);
    SERVICE_MAP(SID_GETSIPNO,PhoneSvc,GetSipNo);
    SERVICE_MAP(SID_GETALLCONTACT,PhoneSvc,GetAllContact);
    SERVICE_MAP(SID_GETONECONTACTDETAIL,PhoneSvc,GetOneContactDetail);
    SERVICE_MAP(SID_UPDATECONTACT,PhoneSvc,UpdateContact);
    SERVICE_MAP(SID_DELETECONTACT,PhoneSvc,DeleteContact);
    SERVICE_MAP(SID_GETALLGROUP,PhoneSvc,GetAllGroup);
    SERVICE_MAP(SID_UPDATEGROUP,PhoneSvc,UpdateGroup);
    SERVICE_MAP(SID_DELETEGROUP,PhoneSvc,DeleteGroup);
    SERVICE_MAP(SID_GETBAL,PhoneSvc,GetBalance);
    SERVICE_MAP(SID_UPDATECONTACTTELNO,PhoneSvc,UpdateContactTel);
    SERVICE_MAP(SID_ISBALENOUGH,PhoneSvc,IsBalEnough);
	SERVICE_MAP(SID_SETSIPNO,PhoneSvc,SetSipNo);
	SERVICE_MAP(SID_VERIFYSIPNO,PhoneSvc,VerifySipNo);
	SERVICE_MAP(SID_BLMPHONE_RECHARGE,PhoneSvc,blmPhoneRecharge);
	SERVICE_MAP(SID_UPD_APP_UPDINFO,PhoneSvc,updAppUpdInfo);
	SERVICE_MAP(SID_GET_APP_UPDINFO,PhoneSvc,getAppUpdInfo);
	SERVICE_MAP(SID_IPHONE_BUYSERVICE,PhoneSvc,iPhoneBuyService);
	SERVICE_MAP(SID_GET_APP_PRICE,PhoneSvc,getAppPrice);

    DEBUG_LOG("[PhoneSvc::Start] OK......................................");
	//std::stringstream out;
	//GetSipNo("86202008","{userid:\"86202008\"}",out);
    return true;
}

// 连北京的数据库
int PhoneSvc::IsBalEnough(const char* pUid, const char* jsonString, std::stringstream& out)
{
	out << "{\"nsucc\":1,\"left\":10}";
	return 0;
}

// 连北京的数据库
int PhoneSvc::UpdateContactTel(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PhoneSvc::UpdateContactTel]bad format:", jsonString, 1);
	if(root.size() < 5)
		return 1;

	string userid, contactid, country_no, tel_no, tel_type;
	userid		= root.getv(0, "");
	contactid	= root.getv(1, "");
	country_no	= root.getv(2, "");
	tel_no		= root.getv(3, "");
	tel_type	= root.getv(4, "");
    
    char* country_col = "";
    char* tel_col ="";
    if (strcmp(tel_type.c_str(),"0") == 0)		//修改的是mb
    {
        country_col = "MOBILE_COUNTRY";
        tel_col = "MOBILE";
    }
    else if (strcmp(tel_type.c_str(),"1") == 0) //修改的是work1
    {
        country_col = "OFFICETEL1_COUNTRY";
        tel_col = "OFFICETEL1";
    }
    else										//修改的是work2
    {
        country_col = "OFFICETEL2_COUNTRY";
        tel_col = "OFFICETEL2";
    }

	MySqlBj *psql = CREATE_MYSQL_BJ;
    char sql[1024];
    sprintf(sql, "UPDATE T00_USER_TEL SET %s='%s',%s='%s' WHERE USER_ID='%s' AND CONTACT_ID='%s'",
             country_col,country_no.c_str(),tel_col,tel_no.c_str(),userid.c_str(),contactid.c_str());
    CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"nsucc\":1}";
    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连北京的数据库
int PhoneSvc::GetBalance(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PhoneSvc::GetBalance]bad format:", jsonString, 1);
	pUid = root.getv("userid", "");
	if(!pUid)
		return 1;

	string uid;
	if(pUid[0] == '+')
	{
		Tokens tokens = StrSplit(pUid, " ");
		if (tokens.size() != 2)
			return 1;
		uid = tokens[1];
	}
	else
	{
		uid = pUid;
	}	 

	char sip[64] = {'\0'};
	double balance = 0.0;
	const char* code = NULL;

	MySqlBj* psql = CREATE_MYSQL_BJ;
	char sql[256];

	//1、查询t02_user_tel_bal
	sprintf(sql, "SELECT curcode, bal FROM t02_user_tel_bal WHERE user_id = '%s'", uid.c_str());
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);

	double balance1 = 0.0;
	char   code1[10] = {'\0'};
	if(psql->NextRow())
	{
		const char* pCurCode = psql->GetField("curcode");
		strcat(code1, (pCurCode?pCurCode:"CNY"));
		balance1 = atof(psql->GetField("bal"));

		balance = balance1;
		code = code1;
	}

	sprintf(sql, "{\"bal\":\"%.2f\",\"curcode\":\"%s\",\"addr\":\"%s\",\"port\":%d}", balance,code?code:"EUR", g_MainConfig::instance()->GetPhoneHost(), g_MainConfig::instance()->GetPhonePort());
	out << sql;

    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连北京的数据库
int PhoneSvc::DeleteGroup(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PhoneSvc::DeleteGroup]bad format:", jsonString, 1);
	string groupid = root.getv("groupid", "");

	char sql[1024];
    MySqlBj *psql = CREATE_MYSQL_BJ;

    sprintf(sql, "DELETE FROM T00_USER_TELGROUP WHERE USER_ID='%s' AND GROUP_ID='%s'",pUid,groupid.c_str());
	CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

	sprintf(sql, "UPDATE T00_USER_GROUP_TEL SET GROUP_ID='0' WHERE USER_ID='%s' AND GROUP_ID='%s'",pUid,groupid.c_str());
    CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"nsucc\":0}";
    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连北京的数据库
int PhoneSvc::UpdateGroup(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PhoneSvc::UpdateGroup]bad format:", jsonString, 1);
	if(root.size() < 3)
		return 1;

	string userid = root.getv(0, "");
	string groupid = root.getv(1, "");
	string groupname = root.getv(2, "");

	char sql[1024];
    MySqlBj *psql = CREATE_MYSQL_BJ;

    char c[31] = { '\0' };
    if(groupid.empty())
    {
        //新增，需要获得一个contactid
        sprintf(sql, "SELECT MAX(GROUP_ID+0) maxid FROM T00_USER_TELGROUP WHERE USER_ID='%s'",userid.c_str());
		CHECK_MYSQL_BJ_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

		const char* maxgroupid = NOTNULL(psql->GetField("maxid"));
		int nmaxid = atoi(maxgroupid);
		sprintf(c,"%d",nmaxid+1);
		groupid = c;
    }

    sprintf(sql, "REPLACE INTO T00_USER_TELGROUP (USER_ID,GROUP_ID,GROUP_NAME) VALUES('%s',%s,'%s')",
		userid.c_str(),groupid.c_str(),groupname.c_str());
    CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

    out<<"{\"nsucc\":0,\"id\":\""<<groupid<<"\"}";
    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连北京的数据库
int PhoneSvc::GetAllGroup(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PhoneSvc::GetAllGroup]bad format:", jsonString, 1);

    MySqlBj *psql = CREATE_MYSQL_BJ;
    char sql[1024];

    sprintf(sql, "SELECT T.GROUP_ID, T.GROUP_NAME FROM T00_USER_TELGROUP T WHERE T.USER_ID='%s'",pUid);
    CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);

	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* groupid = NOTNULL(psql->GetField("GROUP_ID"));
		const char* groupname = NOTNULL(psql->GetField("GROUP_NAME"));
		out<<"[\""<<groupid<<"\","<<"\""<<groupname<<"\"]";
	}
	out<<']';

    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连北京的数据库
int PhoneSvc::DeleteContact(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PhoneSvc::DeleteContact]bad format:", jsonString, 1);  
	string contactid = root.getv("contactid", "");    

	MySqlBj *psql = CREATE_MYSQL_BJ;
    char sql[1024];

	//先从联系人表里删除
    sprintf(sql, "DELETE FROM T00_USER_TEL WHERE USER_ID='%s' AND CONTACT_ID='%s'",pUid,contactid.c_str());
    CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

	//再从联系人组里删除
	sprintf(sql, "DELETE FROM T00_USER_GROUP_TEL WHERE USER_ID='%s' AND CONTACT_ID='%s'",pUid, contactid.c_str());
	CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"nsucc\":0}";
    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连北京的数据库
int PhoneSvc::UpdateContact(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PhoneSvc::UpdateContact]bad format:", jsonString, 1);
    if(root.size() < 23)
		return 1;

    string userid = root.getv(0, "");
    string contactid = root.getv(1, "");
    string groupid = root.getv(2, "");
    string fullname = root.getv(3, "");
    string companyname = root.getv(4, "");
    string department = root.getv(5, "");
    string salutation = root.getv(6, "");
    string mobile_country = root.getv(7, "");
    string mobile = root.getv(8, "");
    string office1_country = root.getv(9, "");
    string office1 = root.getv(10, "");
    string office2_country = root.getv(11, "");
    string office2 = root.getv(12, "");
    string fax_country = root.getv(13, "");
    string fax = root.getv(14, "");
    string email = root.getv(15, "");
    string defaultcontact = root.getv(16, "");
    string addr1 = root.getv(17, "");
    string addr2 = root.getv(18, "");
    string addr3 = root.getv(19, "");
    string addr4 = root.getv(20, "");
    string addr5 = root.getv(21, "");
    string rmk = root.getv(22, "");

	char sql[8192];
	MySqlBj *psql = CREATE_MYSQL_BJ;
   
    char c[31] = { '\0' };
    if(contactid.empty())
    {
        //新增，需要获得一个contactid
        sprintf(sql, "SELECT MAX(CONTACT_ID) maxid FROM T00_USER_TEL WHERE USER_ID='%s'",userid.c_str());
		CHECK_MYSQL_BJ_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
        
		const char* maxcontactid = NOTNULL(psql->GetField("maxid"));
		int nmaxid = atoi(maxcontactid);
		sprintf(c,"%d",nmaxid+1);
		contactid = c;
    }

    sprintf(sql,"REPLACE INTO T00_USER_TEL"\
             " (USER_ID,CONTACT_ID,FULLNAME,COMPANYNAME,"\
             " DEPARTMENT,SALUTATION,MOBILE_COUNTRY,MOBILE,"\
             " OFFICETEL1_COUNTRY,OFFICETEL1,OFFICETEL2_COUNTRY,OFFICETEL2,"\
             " FAX_COUNTRY,FAX,EMAIL,DEFAULT_CONTACT,"\
             " STREET,CITY,PROVINCE,COUNTRY,"\
             " ZIP,REMARK) VALUES"\
             " ('%s',%s,'%s','%s','%s', '%s','%s','%s','%s','%s', '%s','%s','%s','%s','%s', '%s','%s','%s','%s','%s' ,'%s','%s')",
             userid.c_str(),contactid.c_str(),fullname.c_str(),companyname.c_str(),
             department.c_str(),salutation.c_str(),mobile_country.c_str(),mobile.c_str(),
             office1_country.c_str(),office1.c_str(),office2_country.c_str(),office2.c_str(),
             fax_country.c_str(),fax.c_str(),email.c_str(),defaultcontact.c_str(),
             addr1.c_str(),addr2.c_str(),addr3.c_str(),addr4.c_str(),addr5.c_str(),rmk.c_str());
    CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

	sprintf(sql, "DELETE FROM T00_USER_GROUP_TEL WHERE USER_ID='%s' AND CONTACT_ID='%s'",userid.c_str(),contactid.c_str());
	CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

	sprintf(sql, "INSERT INTO T00_USER_GROUP_TEL(USER_ID,GROUP_ID,CONTACT_ID) VALUES('%s','%s','%s')",userid.c_str(),groupid.c_str(),contactid.c_str());
	CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

	out<<"{\"nsucc\":0,\"id\":\""<<contactid<<"\"}";
	RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连北京的数据库
int PhoneSvc::GetOneContactDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PhoneSvc::GetOneContactDetail]bad format:", jsonString, 1);
	string contactid = root.getv("contactid", "");

    MySqlBj *psql = CREATE_MYSQL_BJ;
    char sql[1024];
    sprintf(sql, "SELECT T2.GROUP_ID,T.CONTACT_ID,T.FULLNAME,T.COMPANYNAME,T.DEPARTMENT,T.SALUTATION,"\
             " T.EMAIL,T.MOBILE_COUNTRY,T.MOBILE,T.OFFICETEL1_COUNTRY,T.OFFICETEL1,"\
             " T.OFFICETEL2_COUNTRY,T.OFFICETEL2,T.FAX_COUNTRY,T.FAX,T.DEFAULT_CONTACT,"\
             " T.STREET,T.CITY,T.PROVINCE,T.COUNTRY,T.ZIP,T.REMARK"\
             " FROM T00_USER_TEL T,T00_USER_GROUP_TEL T2 "\
             " WHERE T.USER_ID='%s' AND T.CONTACT_ID='%s' AND T.USER_ID=T2.USER_ID AND T.CONTACT_ID=T2.CONTACT_ID"
             , pUid,contactid.c_str());
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);

	out<<'[';
	if(psql->NextRow())
	{
		const char* groupid = NOTNULL(psql->GetField("GROUP_ID"));
		const char* contactid = NOTNULL(psql->GetField("CONTACT_ID"));
		const char* fullname = NOTNULL(psql->GetField("FULLNAME"));
		const char* companyname = NOTNULL(psql->GetField("COMPANYNAME"));
		const char* department = NOTNULL(psql->GetField("DEPARTMENT"));
		const char* salutation = NOTNULL(psql->GetField("SALUTATION"));
		const char* email = NOTNULL(psql->GetField("EMAIL"));
		const char* mobile_country = NOTNULL(psql->GetField("MOBILE_COUNTRY"));
		const char* mobile = NOTNULL(psql->GetField("MOBILE"));
		const char* office1_country = NOTNULL(psql->GetField("OFFICETEL1_COUNTRY"));
		const char* office1 = NOTNULL(psql->GetField("OFFICETEL1"));
		const char* office2_country = NOTNULL(psql->GetField("OFFICETEL2_COUNTRY"));
		const char* office2 = NOTNULL(psql->GetField("OFFICETEL2"));
		const char* fax_country = NOTNULL(psql->GetField("FAX_COUNTRY"));
		const char* fax = NOTNULL(psql->GetField("FAX"));
		const char* default_contact = NOTNULL(psql->GetField("DEFAULT_CONTACT"));
		const char* addr1 = NOTNULL(psql->GetField("STREET"));
		const char* addr2 = NOTNULL(psql->GetField("CITY"));
		const char* addr3 = NOTNULL(psql->GetField("PROVINCE"));
		const char* addr4 = NOTNULL(psql->GetField("COUNTRY"));
		const char* addr5 = NOTNULL(psql->GetField("ZIP"));
		const char* rmk = NOTNULL(psql->GetField("REMARK"));
		
		out<<"\""<<groupid<<"\","
			<<"\""<<contactid<<"\","
			<<"\""<<fullname<<"\","
			<<"\""<<companyname<<"\","
			<<"\""<<department<<"\","
			<<"\""<<salutation<<"\","
			<<"\""<<mobile_country<<"\","
			<<"\""<<mobile<<"\","
			<<"\""<<office1_country<<"\","
			<<"\""<<office1<<"\","
			<<"\""<<office2_country<<"\","
			<<"\""<<office2<<"\","
			<<"\""<<fax_country<<"\","
			<<"\""<<fax<<"\","
			<<"\""<<email<<"\","
			<<"\""<<default_contact<<"\","
			<<"\""<<addr1<<"\","
			<<"\""<<addr2<<"\","
			<<"\""<<addr3<<"\","
			<<"\""<<addr4<<"\","
			<<"\""<<addr5<<"\","
			<<"\""<<rmk<<"\"";
	}
	
	out<<']';

    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连接北京的数据库
int PhoneSvc::GetAllContact(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PhoneSvc::GetAllContact]bad format:", jsonString, 1);
	string groupid = root.getv("groupid", "");

    MySqlBj *psql = CREATE_MYSQL_BJ;
    char sql[1024];
    sprintf(sql, "SELECT T2.GROUP_ID,T1.USER_ID,T1.CONTACT_ID,"\
             " T1.FULLNAME,T1.EMAIL,T1.MOBILE_COUNTRY,T1.MOBILE,"\
             " T1.OFFICETEL1_COUNTRY,T1.OFFICETEL1,T1.OFFICETEL2_COUNTRY,T1.OFFICETEL2,T1.FAX_COUNTRY,T1.FAX,T1.DEFAULT_CONTACT"\
             " FROM T00_USER_TEL T1,T00_USER_GROUP_TEL T2"\
             " WHERE T1.USER_ID = '%s' AND T1.USER_ID=T2.USER_ID AND T1.CONTACT_ID=T2.CONTACT_ID"
             , pUid);
    if(!groupid.empty())
    {
        char sqlgroup[128];
        snprintf(sqlgroup, sizeof(sqlgroup), " AND T2.GROUP_ID='%s'",groupid.c_str());
        strcat(sql,sqlgroup);
    }
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);

	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* groupid = NOTNULL(psql->GetField("GROUP_ID"));
		const char* contactid = NOTNULL(psql->GetField("CONTACT_ID"));
		const char* fullname = NOTNULL(psql->GetField("FULLNAME"));
		const char* mobile_country = NOTNULL(psql->GetField("MOBILE_COUNTRY"));
		const char* mobile = NOTNULL(psql->GetField("MOBILE"));
		const char* office1_country = NOTNULL(psql->GetField("OFFICETEL1_COUNTRY"));
		const char* office1 = NOTNULL(psql->GetField("OFFICETEL1"));
		const char* office2_country = NOTNULL(psql->GetField("OFFICETEL2_COUNTRY"));
		const char* office2 = NOTNULL(psql->GetField("OFFICETEL2"));
		const char* fax_country = NOTNULL(psql->GetField("FAX_COUNTRY"));
		const char* fax = NOTNULL(psql->GetField("FAX"));
		const char* email = NOTNULL(psql->GetField("EMAIL"));
		const char* default_contact = NOTNULL(psql->GetField("DEFAULT_CONTACT"));

		out<<"[\""<<groupid<<"\","
			<<"\""<<contactid<<"\","
			<<"\""<<fullname<<"\","
			<<"\""<<mobile_country<<"\","
			<<"\""<<mobile<<"\","
			<<"\""<<office1_country<<"\","
			<<"\""<<office1<<"\","
			<<"\""<<office2_country<<"\","
			<<"\""<<office2<<"\","
			<<"\""<<fax_country<<"\","
			<<"\""<<fax<<"\","
			<<"\""<<email<<"\","
			<<"\""<<default_contact<<"\"]";
	}
	out<<']';

    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连北京的数据库
//{userid:"15652359695",money:1.03}
int  PhoneSvc::GetSipNo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PhoneSvc::GetSipNo]bad format:", jsonString, 1);
	string uid = root.getv("userid", "");
	if(uid.empty())
		uid = pUid;
	double money = root.getv("money", 0.0);

	//if (strlen(jsonString) > 2)
	//{
	//	JSON_PARSE_RETURN("[PhoneSvc::GetSipNo]bad format:", jsonString, 1);
	//	uid = root.getv("userid", "");
	//}

	char gmt0now[20];
	GmtNow(gmt0now);

    MySqlBj *psql = CREATE_MYSQL_BJ;
    char sql[1024];

	sprintf (sql, "SELECT usertype FROM t00_user WHERE user_id = '%s'", uid.c_str());
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	int usertype;
	READMYSQL_INT(usertype, usertype, -1);

	if (usertype != 20)
	{
		sprintf(sql, "SELECT T.USERID,T.SIPID,T.SIPTYPE,T2.SECRET FROM T00_USER_SIP T LEFT JOIN astercc.sipbuddies T2 ON T.SIPID=T2.NAME\
				 WHERE T.USERID = '%s' AND t.invalid_dt IS NULL", uid.c_str());
		CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);
		if(psql->NextRow())	//已经有sip帐户了
		{
			const char* sipid = NOTNULL(psql->GetField("SIPID"));
			const char* secret = NOTNULL(psql->GetField("SECRET"));
			const char* siptype = NOTNULL(psql->GetField("SIPTYPE"));
			if(!strlen(siptype))
				siptype = "3"; //旧的数据
			out.str("");
			out<<"{\"sip\":\""<<sipid<<"\",\"pwd\":\""<<secret<<"\",\"siptype\":"<<atoi(siptype)<<"}";

			
			if(money > 0)
			{
				sprintf(sql, "UPDATE boloomodb.t02_user_tel_bal SET bal = bal + %f, Last_upd_dt = %s WHERE user_id = '%s'", money, gmt0now, sipid);
				CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);
			}
		}
		else				//尚未SIP帐户，新增一个(sipid设置成userid)
		{
			char sipid[64];
			int i = 1;
			while(true)
			{
				//取最大的一个sipid+1作为新的sipid
				sprintf(sql, "select max(id)+%d as sipid from astercc.sipbuddies", i++);
				CHECK_MYSQL_BJ_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
				READMYSQL_STR(sipid, sipid);

				//检查这个sipid是否已经存在
				sprintf(sql, "select name from astercc.sipbuddies where name = '%s'", sipid);
				CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);
				if(!psql->NextRow())
					break;
			}
			

			//生成六位随机数字密码
			srand((unsigned int)time(0));
			static int nbegin = 100000;
			static int nend = 999999;
			int nsecr = rand()%(nend-nbegin+1)+nbegin;

			//插入t00_user_sip
			sprintf(sql, "INSERT INTO T00_USER_SIP(USERID,sipid, siptype, valid_dt) VALUES('%s', '%s', 3, '%s')",uid.c_str(), sipid,gmt0now);
			CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>0, 3);

			//插入asterisk.sipbuddies
			sprintf(sql, "INSERT INTO astercc.sipbuddies(name,host,secret,dtmfmode,allow,username,update_dt) VALUES('%s','dynamic','%d','rfc2833','ilbc;g729;gsm;amr','%s',unix_timestamp())"
							,sipid,nsecr,sipid);
			if(psql->Execute(sql)<=0)
			{
				sprintf(sql, "delete from t00_user_sip where userid = '%s' and sipid = '%s'", uid.c_str(), sipid);
				psql->Execute(sql);
				RELEASE_MYSQL_BJ_RETURN(psql, 3);
			}
			//sprintf(sql, "SELECT exchange FROM t41_exchange WHERE curcode = 'EUR' AND to_cur = 'CNY' ORDER BY start_dt DESC LIMIT 1");
			//CHECK_MYSQL_BJ_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
			//double eur2cny = atof(psql->GetField("exchange"));

			//double _money = 1.2;//第一次注册送话费
			//sprintf(sql, "INSERT IGNORE INTO boloomodb.t02_user_tel_bal (User_id, curcode, bal, valid_dt, Last_upd_dt, total_given) \
			//				VALUES('%s','CNY','%.2f',DATE_ADD(NOW(), INTERVAL 1 MONTH), NOW(), '%.2f')", pUid, _money/*money*eur2cny*/, _money/*money*eur2cny*/);
			//CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

			//生成返回结果
			out.str("");
			out<<"{\"sip\":\""<<sipid<<"\",\"pwd\":\""<<nsecr<<"\",\"siptype\":3}";
		}
	}
	else
	{
		sprintf(sql, "SELECT T.USERID,T.SIPID,T.SIPTYPE,T2.SECRET FROM T00_USER_SIP T LEFT JOIN boloomodb_s.ascc_sipbuddies T2 ON T.SIPID=T2.NAME\
						WHERE T.USERID = '%s' AND t.invalid_dt IS NULL", uid.c_str());
		CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);
		if(psql->NextRow())	//已经有sip帐户了
		{
			const char* sipid = NOTNULL(psql->GetField("SIPID"));
			const char* secret = NOTNULL(psql->GetField("SECRET"));
			const char* siptype = NOTNULL(psql->GetField("SIPTYPE"));
			if(!strlen(siptype))
				siptype = "3"; //旧的数据
			out.str("");
			out<<"{\"sip\":\""<<sipid<<"\",\"pwd\":\""<<secret<<"\",\"siptype\":"<<atoi(siptype)<<"}";

			if(money > 0)
			{
				sprintf(sql, "UPDATE boloomodb.t02_user_tel_bal SET bal = bal + %f, Last_upd_dt = %s WHERE user_id = '%s'", money, gmt0now, sipid);
				CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);
			}
		}
		else
			out << "{\"sip\":\"\",\"pwd\":\"\",\"siptype\":}";
	}
    
    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

int PhoneSvc::GetPhoneServer(const char* pUid, const char* jsonString, std::stringstream& out)
{
	bool isZHY = false; //是否是中海油电信用户

	Json root;
	if (jsonString && root.parse(jsonString))
		pUid = root.getv("userid", "");
	
	char sql[1024];
	sprintf(sql, "select usertype from t00_user where user_id = '%s'", pUid);
	MySql* psql = CREATE_MYSQL;
	if(psql->Query(sql) && psql->NextRow())
	{
		int usertype;
		READMYSQL_INT(usertype, usertype, 0);
		if(usertype == 20)
			isZHY = true;
	}

	out << "{\"ip\":\"" << (isZHY?"222.73.34.221":g_MainConfig::instance()->GetPhoneHost()) << 
		"\",\"port\":" << g_MainConfig::instance()->GetPhonePort() << 
		",\"other\":[\"" << g_MainConfig::instance()->GetPhoneOther() << "\"]}";
    
	RELEASE_MYSQL_RETURN(psql, 0);
}

// 连接北京的数据库
int PhoneSvc::SetSipNo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PhoneSvc::SetSipNo]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	int sipType  = root.getv("siptype", 0);
	const char* sipid = root.getv("sipid", "");
	if(sipType <= 0 || sipType > 3 || !sipid)
		return 1;

	// 主叫号码第一位不能是0
	if(*sipid == '0')
		return 1;

	char gmt0now[20] = "";
	GmtNow(gmt0now);

	char sql[1024];
	MySqlBj *psql = CREATE_MYSQL_BJ;

	// 0. 判断该用户是否存在
	sprintf(sql, "select * from t00_user where user_id = '%s'", uid.c_str());
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);
	if(!psql->NextRow())
		RELEASE_MYSQL_BJ_RETURN(psql, 1);

	// 1. 查找该sipid是否已经存在
	sprintf(sql, "select * from t00_user_sip where sipid = '%s' and invalid_dt is null", sipid);
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
		RELEASE_MYSQL_BJ_RETURN(psql, 1);

	// 2. 是否是该用户首次设置
	sprintf(sql, "SELECT T.USERID,T.SIPID FROM T00_USER_SIP T,astercc.sipbuddies T2"\
		" WHERE T.USERID = '%s' AND T.SIPID=T2.NAME and t.invalid_dt is null", uid.c_str());//pUid
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())	//更新
	{
		const char* oldSipid = psql->GetField("sipid");

		// t00_user_sip表旧记录失效
		sprintf(sql, "update t00_user_sip set invalid_dt = '%s' where userid = '%s' and invalid_dt is null", gmt0now, uid.c_str());
		CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

		sprintf(sql, "insert into t00_user_sip(userid,sipid,siptype,valid_dt)values('%s','%s',%d,'%s')",
					uid.c_str(),sipid,sipType,gmt0now);//pUid
		CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

		sprintf(sql, "update astercc.sipbuddies set name='%s',username='%s', update_dt=unix_timestamp() where name ='%s'",
					sipid,sipid,oldSipid);
		CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);
	}
	else				//新建
	{
		// t00_user_sip表旧记录失效
		sprintf(sql, "update t00_user_sip set invalid_dt = '%s' where userid = '%s' and invalid_dt is null", gmt0now, uid.c_str());
		CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

		sprintf(sql, "INSERT INTO T00_USER_SIP(USERID,SIPID,SIPTYPE,VALID_DT) VALUES('%s','%s',%d,'%s')",uid.c_str(),sipid,sipType,gmt0now);//pUid
		CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

		//生成六位随机数字密码
		srand((unsigned int)time(0));
		static int nbegin = 100000;
		static int nend = 999999;
		int nsecr = rand()%(nend-nbegin+1)+nbegin;

		//向asterisk.sipbuddies插入一条记录
		sprintf(sql, "INSERT INTO astercc.sipbuddies(name,host,secret,allow,dtmfmode,username,update_dt)VALUES('%s','dynamic','%d','ilbc;g729;gsm;amr','rfc2833','%s',unix_timestamp())"
			,sipid,nsecr,uid.c_str());//pUid
		CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);
	}

	RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

// 连接北京的数据库
int PhoneSvc::VerifySipNo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PhoneSvc::VerifySipNo]bad format:", jsonString, 1);
	const char* sipid = root.getv("sipid", "");
	
	char sql[1024];
	MySqlBj *psql = CREATE_MYSQL_BJ;

	sprintf(sql, "select * from t00_user_sip where sipid = '%s' and invalid_dt is null", sipid);
	//sprintf(sql, "select * from astercc.sipbuddies where id = '%s'", sipid);
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
		RELEASE_MYSQL_BJ_RETURN(psql, 2);

	RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

//zhuxj
//{uid:"gsf",amount:"50",cur:"CNY",payway:"0"}
int PhoneSvc::blmPhoneRecharge(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PhoneSvc::blmPhoneRecharge]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string amount = root.getv("amount", "");
	string cur = root.getv("cur", "");
	string payway = root.getv("payway", "");

	char gmt0now[20];
	GmtNow(gmt0now);
	int payWay = (payway[0] == '0')?13:11;		
	uint64 timeusec = GmtNowUsec();
	char sql[1024] = "";
	MySql *psql = CREATE_MYSQL;

	sprintf (sql, "INSERT INTO t02_order (order_id, user_id, product_id, order_time, currency, amout, pay_chnaele, pay_type, status_cd, last_upd_dt) \
					VALUES ('BLMVOIP%lld', '%s', 'BLMPIPPHONE', '%s', '%s', '%s', '%d', '1', '1', '%s')",
					timeusec, uid.c_str(), gmt0now, cur.c_str(), amount.c_str(), payWay, gmt0now);

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf (sql, "INSERT INTO blmdtmaintain.t02_buy_detail (order_id, user_id, product_id, order_time, currency, amout, pay_chnaele, pay_type, status_cd, last_upd_dt) \
				  VALUES ('BLMVOIP%lld', '%s', 'BLMPIPPHONE', '%s', '%s', '%s', '%d', '1', '1', '%s')",
				  timeusec, uid.c_str(), gmt0now, cur.c_str(), amount.c_str(), payWay, gmt0now);

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	
	out << "{\"eid\":0,\"ordered\":\"BLMVOIP" << timeusec << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{"appname":"blmphone","apkname":"blmphone.apk","verName":"1.0.1","verCode":1001,url:""}
int PhoneSvc::updAppUpdInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PhoneSvc::updAppUpdInfo]bad format:", jsonString, 1);
	string appname = root.getv("appname", "");
	string apkname = root.getv("apkname", "");
	string verName = root.getv("verName", "");
	int	verCode = root.getv("verCode", 0);
	string url = root.getv("url", "");

	char sql[1024] = "";
	MySql *psql = CREATE_MYSQL;

	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "UPDATE t00_app_version SET is_flag = '0' WHERE is_flag = '1' AND (appname = '%s' or apkname = '%s')", appname.c_str(), apkname.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf (sql, "INSERT INTO t00_app_version (appname, apkname, vername, vercode, is_flag, last_upd_dt, url)	VALUES ('%s', '%s', '%s', '%d', '1', '%s', '%s')",
				  CodeConverter::Gb2312ToUtf8(appname.c_str()).c_str(), CodeConverter::Gb2312ToUtf8(apkname.c_str()).c_str(), verName.c_str(), verCode, gmt0now, url.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{\"eid\":0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{"appname":"blmphone","apkname":"blmphone.apk"}
int PhoneSvc::getAppUpdInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PhoneSvc::updAppUpdInfo]bad format:", jsonString, 1);
	string appname = root.getv("appname", "");
	string apkname = root.getv("apkname", "");

	char sql[1024] = "";
	MySql *psql = CREATE_MYSQL;
	sprintf (sql, "SELECT url, vercode, vername FROM t00_app_version WHERE  (appname = '%s' or apkname = '%s') AND is_flag = '1'",
				  CodeConverter::Gb2312ToUtf8(appname.c_str()).c_str(), CodeConverter::Gb2312ToUtf8(apkname.c_str()).c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		const char* url = psql->GetField("url");
		const char* vercode = psql->GetField("vercode");
		const char* vername = psql->GetField("vername");
		out << "{\"eid\":0,\"flag\":1,\"url\":\"" << url << "\",\"vercode\":\"" << vercode << "\",\"vername\":\"" << vername << "\"}";
		RELEASE_MYSQL_RETURN(psql, 0);
	}
	out << "{\"eid\":0,\"flag\":0,\"url\":\"\",\"vercode\":\"\",\"vername\":\"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{userid:"boloomouid",product:"sate",ntp:0,payway:"",cur:"",android:1}
int PhoneSvc::iPhoneBuyService(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PhoneSvc::iPhoneBuyService]bad format:", jsonString, 1);
	string uid = root.getv("userid", "");
	string product = root.getv("product", "");
	string payway = root.getv("payway", "");
	string cur = root.getv("cur", "");

	int ntp = root.getv("ntp", 0);
	int android = root.getv("android", 0);

	char gmt0now[20];
	GmtNow(gmt0now);

	int payWay = (payway[0] == '0')?13:11;		
	uint64 timeusec = GmtNowUsec();

	int money = 0;
	string product_id = "";

	if(!android)
	{
		if (!ntp)
		{
			money = 16800;
			product_id = "BLMPGLOBALAIS"; //全球星
		}
		else 
		{
			money = 2000;
			product_id = "BLMPENTERPRISEAIS"; //企业星
		}
	}
	else
	{
		if (!ntp)
		{
			money = 36800;
			product_id = "BLMPGLOBALAIS"; //全球星
		}
		else 
		{
			money = 6800;
			product_id = "BLMPENTERPRISEAIS"; //企业星
		}
	}
	

	char sql[1024] = "";
	MySql *psql = CREATE_MYSQL;

	sprintf (sql, "INSERT INTO t02_order (order_id, user_id, product_id, order_time, currency, amout, pay_chnaele, pay_type, status_cd, last_upd_dt, quantity) \
					VALUES ('BLMAIS%lld', '%s', '%s', '%s', '%s', '%d', '%d', '1', '1', '%s', '1')",
					timeusec, uid.c_str(), product_id.c_str(), gmt0now, cur.c_str(), money, payWay, gmt0now);

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf (sql, "INSERT INTO blmdtmaintain.t02_buy_detail (order_id, user_id, product_id, order_time, currency, amout, pay_chnaele, pay_type, status_cd, last_upd_dt, quantity) \
				  VALUES ('BLMAIS%lld', '%s', '%s', '%s', '%s', '%d', '%d', '1', '1', '%s', '1')",
				  timeusec, uid.c_str(), product_id.c_str(), gmt0now, cur.c_str(), money, payWay, gmt0now);

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	
	out << "{\"eid\":0,\"ordered\":\"BLMAIS" << timeusec << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

// {sattp:0} 0全球星 1企业星
// {eid:0,nprice:6800}
int PhoneSvc::getAppPrice(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PhoneSvc::getAppPrice]bad format:", jsonString, 1);
	int ntp = root.getv("sattp", 0);

	out << "{\"eid\":0,\"nprice\":" << (ntp?6800:36800) << "}";
	return 0;
}

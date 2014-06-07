#include "MessengerSvc.h"
#include "blmcom_head.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "IBusinessService.h"
#include "kSQL.h"
#include "Util.h"
#include "ObjectPool.h"
#include "json.h"

MessageSvc::MessageSvc(void)
{
}

MessageSvc::~MessageSvc(void)
{
}

IMPLEMENT_SERVICE_MAP(MessageSvc)

bool MessageSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_MESSENGER, this))
        return false;

    SERVICE_MAP(0x01,MessageSvc,GetUserGroupList);
    SERVICE_MAP(0x02,MessageSvc,ContactGroupManager);
    SERVICE_MAP(0x03,MessageSvc,GetContactInfo);
    SERVICE_MAP(0x04,MessageSvc,GetBlmServiceUserList);
    SERVICE_MAP(0x05,MessageSvc,GeUsersNickList);
    SERVICE_MAP(0x06,MessageSvc,ReqMucRoomList);
    SERVICE_MAP(0x07,MessageSvc,ReqMucRoomMemberList);
    SERVICE_MAP(0x08,MessageSvc,QueryMucRoomCount);
    SERVICE_MAP(0x09,MessageSvc,QueryMucRoomList);
    SERVICE_MAP(0x0a,MessageSvc,ReqMucRoomDetail);
    SERVICE_MAP(0x0b,MessageSvc,ReqUserAuth);
    SERVICE_MAP(0x0c,MessageSvc,ReqUserTelsInfo);
    SERVICE_MAP(0x0d,MessageSvc,ReqSmsCount);
    SERVICE_MAP(0x0e,MessageSvc,ReqUserTelEmailInfo);
    SERVICE_MAP(0x0f,MessageSvc,ReqChangjiangUserLevel);
    SERVICE_MAP(0x10,MessageSvc,OnUserRegistCmpy);
    SERVICE_MAP(0x11,MessageSvc,OnUserUnRegistCmpy);
	SERVICE_MAP(0x12,MessageSvc,GetFamilyDaShiList);

    DEBUG_LOG("[MessageSvc::Start] OK......................................");

    return true;
}

int MessageSvc::GetUserGroupList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	char sql[1024];
    sprintf(sql, "SELECT GROUPNAME,GROUPDES FROM T00_GROUP WHERE USER_ID = '%s' ORDER BY RANK", pUid);
   
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out << '[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char *groupid=psql->GetField("GROUPNAME");
		const char *groupdes=psql->GetField("GROUPDES");

		out << '{';
		out << "\"gid\":\"" << NOTNULL(groupid) << "\"";
		out << ",\"gnm\":\"" << NOTNULL(groupdes) << "\"";
		out << '}';
	}
	out << ']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::ContactGroupManager(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::ContactGroupManager]bad format:", jsonString, 1);
	std::string groupId		= root.getv("gid", "");
	std::string groupName	= root.getv("gname", "");
	int type				= root.getv("type", 0);

    MySql *psql = CREATE_MYSQL;
    bool succ = false;

    switch (type)
    {
    case 0:
        succ = _AddContactGroup(pUid,groupId.c_str(),groupName.c_str(),psql);
        break;
    case 1:
        succ = _DelContactGroup(pUid,groupId.c_str(),psql);
        break;
    case 2:
        succ = _ModifyContactGroup(pUid,groupId.c_str(),groupName.c_str(),psql);
        break;
    }

	if(succ)
		out << MSG_SUCC;

	RELEASE_MYSQL_RETURN(psql, (succ?0:1));
}

int MessageSvc::GetContactInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::GetContactInfo]bad format:", jsonString, 1);
	std::string contactId = root.getv("uid", "");
   
    char sql[1024];
	sprintf(sql, "SELECT t1.usertype, t1.NICKNAME,t1.BIRTHDAY,t1.GENDER,t1.FIRSTNAME,t1.LASTNAME,t1.TELNO,t1.FAX,t1.MOBILE,t1.EMAIL,t1.ZIP,t1.ADDRESS,t1.COUNTRY,t2.sipid FROM T00_USER t1 left join t00_user_sip t2 on t1.user_id = t2.userid WHERE t1.USER_ID='%s' AND t1.IS_FLAG = '1' and t2.invalid_dt is null", contactId.c_str());
    
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	out<<'{'<<"\"uid\":\""<<contactId<<"\"";
	if (psql->NextRow())
	{
		const char *usertype=psql->GetField("usertype");
		const char *nkname=psql->GetField("NICKNAME");
		const char *firstname=psql->GetField("FIRSTNAME");
		const char *lastname=psql->GetField("LASTNAME");
		const char *birthday=psql->GetField("BIRTHDAY");
		const char *gender=psql->GetField("GENDER");
		const char *country=psql->GetField("COUNTRY");
		const char *email=psql->GetField("EMAIL");
		const char *telephone=psql->GetField("TELNO");
		const char *mobile=psql->GetField("MOBILE");
		const char *zip=psql->GetField("ZIP");
		const char *fax=psql->GetField("FAX");
		const char *address=psql->GetField("ADDRESS");
		const char* sipid=psql->GetField("SIPID");

		out<<",\"usertype\":\""<<NOTNULL(usertype)<<"\"";
		out<<",\"nkname\":\""<<NOTNULL(nkname)<<"\"";
		out<<",\"fname\":\""<<NOTNULL(firstname)<<"\"";
		out<<",\"lname\":\""<<NOTNULL(lastname)<<"\"";
		out<<",\"birth\":\""<<NOTNULL(birthday)<<"\"";
		out<<",\"gender\":\""<<NOTNULL(gender)<<"\"";
		out<<",\"ctry\":\""<<NOTNULL(country)<<"\"";
		out<<",\"email\":\""<<NOTNULL(email)<<"\"";
		out<<",\"telno\":\""<<NOTNULL(telephone)<<"\"";
		out<<",\"mobile\":\""<<NOTNULL(mobile)<<"\"";
		out<<",\"zip\":\""<<NOTNULL(zip)<<"\"";
		out<<",\"fax\":\""<<NOTNULL(fax)<<"\"";
		out<<",\"addr\":\""<<NOTNULL(address)<<"\"";
		out<<",\"blmvoip\":\"0 "<<NOTNULL(sipid)<<"\"";
	}

	sprintf(sql, "SELECT SERVICETYPE_CODE FROM T00_USER_ACTIVITIES WHERE USER_ID = '%s'", contactId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{
		out<<',';
		const char *servicetype=psql->GetField("SERVICETYPE_CODE");
		out<<"\"svrtype\":\""<<NOTNULL(servicetype)<<"\"";
	}
	else
	{
		out<<',';
		out<<"\"svrtype\":\""<<""<<"\"";
	}

	sprintf(sql, "SELECT T2.NAME, T2.COMPANY_KEY FROM T41_COMP_CONTACTS T1,T41_COMPANY T2 WHERE T1.COMPANY_KEY = T2.COMPANY_KEY AND USER_ID ='%s'", contactId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	if (psql->NextRow())
	{
		const char *cmpykey=psql->GetField("COMPANY_KEY");
		const char *cmpyname=psql->GetField("NAME");
		
		out<<",\"cmpyid\":\""<<NOTNULL(cmpykey)<<"\"";
		out<<",\"cmpy\":\""<<NOTNULL(cmpyname)<<"\"";
	}
	else
	{
		out<<",\"cmpyid\":\""<<""<<"\"";
		out<<",\"cmpy\":\""<<""<<"\"";
	}

	out<<'}';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::GetBlmServiceUserList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[MessageSvc::GetBlmServiceUserList]bad format:", jsonString, 1);
	std::string usrtype = root.getv("type", "");

    char sql[1024];
    if (usrtype == "cj")
    {
        strcpy(sql, "SELECT USER_ID,NICKNAME FROM T00_USER WHERE USERTYPE = '50' AND IS_FLAG = '1' AND STATUS='2'");
    }
    else
    {
        strcpy(sql, "SELECT USER_ID,NICKNAME FROM T00_USER WHERE USERTYPE = '3' AND IS_FLAG = '1'");
    }
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
    out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char *usrid=psql->GetField("USER_ID");
		const char *usrnick=psql->GetField("NICKNAME");
		out<<'{';
		out<<"\"svid\":\""<<NOTNULL(usrid)<<"\"";
		out<<',';
		out<<"\"svnm\":\""<<NOTNULL(usrnick)<<"\"";
		out<<'}';
	}
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::GeUsersNickList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::GeUsersNickList]bad format:", jsonString, 1);
    int size = root.size();

	std::stringstream usernameStr;
    for (int i=0; i<size; i++)
    {
        if (i>0)
        {
            usernameStr << ",";
        }
        usernameStr << "'" << root.getv(i, "") << "'";
    }
	string sql = "SELECT USER_ID,NICKNAME FROM T00_USER WHERE USER_ID IN ("+usernameStr.str()+") AND IS_FLAG = '1'";

	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);
    
	int total = 0;
    out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char *usrid=psql->GetField("USER_ID");
		const char *usrnick=psql->GetField("NICKNAME");
		out<<'{';
		out<<"\"uid\":\""<<NOTNULL(usrid)<<"\"";
		out<<',';
		out<<"\"nknm\":\""<<NOTNULL(usrnick)<<"\"";
		out<<'}';
	}
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::ReqMucRoomList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::ReqMucRoomList]bad format:", jsonString, 1);
	std::string jid = root.getv("jid", "");
   
	char sql[1024];
    sprintf(sql, "SELECT T2.NAME AS ROOMJID ,T1.NICKNAME AS UID,'30' AS ROOMROOL,NATURALNAME AS ROOMNAME,T2.iscmpyRoom,T2.SUBJECT FROM OFMUCMEMBER T1 ,OFMUCROOM T2 "\
                               " WHERE T1.ROOMID = T2.ROOMID AND T1.JID = '%s' AND NICKNAME <>'' UNION SELECT T2.NAME AS ROOMJID ,T1.JID AS UID, T1.AFFILIATION AS ROOMROOL,NATURALNAME AS ROOMNAME,T2.iscmpyRoom,T2.SUBJECT"\
                               " FROM OFMUCAFFILIATION T1 ,OFMUCROOM T2 WHERE T1.ROOMID = T2.ROOMID AND T1.JID = '%s'",jid.c_str(),jid.c_str());
    
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* roomjid=psql->GetField("ROOMJID");
		const char* roomname=psql->GetField("ROOMNAME");
		const char* subject=psql->GetField("SUBJECT");
		const char* usrId=psql->GetField("UID");
		const char* bCmpyFlag=psql->GetField("iscmpyRoom");
		const char* roomrole=psql->GetField("ROOMROOL");

		out<<'{';
		out<<"\"grid\":\""<<NOTNULL(roomjid)<<"\"";
		out<<",\"grname\":\""<<NOTNULL(roomname)<<"\"";
		out<<",\"grsubj\":\""<<NOTNULL(subject)<<"\"";
		out<<",\"usrnick\":\""<<NOTNULL(usrId)<<"\"";
		out<<",\"bCmpy\":\""<<NOTNULL(bCmpyFlag?bCmpyFlag:"0")<<"\"";
		out<<",\"role\":\""<<NOTNULL(roomrole)<<"\"";
		out<<'}';
	}
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::ReqMucRoomMemberList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::ReqMucRoomMemberList]bad format:", jsonString, 1);
	std::string roomJid = root.getv("grid", "");
   
	char sql[1024];
    sprintf(sql, "SELECT T1.JID AS USERJID,'30' AS ROOMROLE FROM OFMUCMEMBER T1,OFMUCROOM T2 WHERE T1.ROOMID = T2.ROOMID AND T2.NAME = '%s' AND NICKNAME <>'' UNION"\
                               " SELECT T1.JID AS USERJID, T1.AFFILIATION AS ROOMROLE FROM OFMUCAFFILIATION T1,OFMUCROOM T2  WHERE T1.ROOMID = T2.ROOMID AND T2.NAME = '%s'",roomJid.c_str(),roomJid.c_str());
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char *usrjid=psql->GetField("USERJID");
		const char *roomrole=psql->GetField("ROOMROLE");
		out<<'{';
		out<<"\"jid\":\""<<NOTNULL(usrjid)<<"\"";
		out<<",\"role\":\""<<NOTNULL(roomrole)<<"\"";
		out<<'}';
	}
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::ReqUserAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::ReqUserAuth]bad format:", jsonString, 1);

	char sql[1024];
	sprintf(sql, "SELECT COUNT(1) AS NUM FROM T41_COMP_CONTACTS  WHERE USER_ID ='%s'", pUid);

    MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    
    out<<'{';
    if(psql->NextRow())
    {
		int count = 0;
		READMYSQL_INT(NUM,count,0);
        out<<FormatString("auth:%d",count);
    }
    out<<'}';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::ReqUserTelsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::ReqUserTelsInfo]bad format:", jsonString, 1);
    int size = root.size();

    string usernameStr="";
    for (int i=0; i<size; i++)
    {
        if (i>0)
        {
            usernameStr+=",";
        }
        usernameStr=usernameStr+"'"+root.getv(i, "")+"'";
    }
    string sql="SELECT usertype,USER_ID,COUNTRY,TELNO,MOBILE FROM T00_USER WHERE USER_ID IN ("+usernameStr+") AND IS_FLAG = '1'";
    
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);
	
	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		const char* cry = NOTNULL(psql->GetField("COUNTRY"));

		if(total != 0)
			out << ",";
		total++;

		const char* usertype=psql->GetField("usertype");
		const char* usrid=psql->GetField("USER_ID");
		const char* tel=NOTNULL(psql->GetField("TELNO"));
		const char* mb1=NOTNULL(psql->GetField("MOBILE"));

		out<<'{';
		out<<"\"uid\":\""<<NOTNULL(usrid)<<"\"";
		out<<",\"usertype\":\""<<NOTNULL(usertype)<<"\"";
		out<<",\"tel1\":\""<<tel<<"\"";
		out<<",\"mb1\":\""<<mb1<<"\"";
		out<<",\"mb2\":\"\"";
		out<<'}';
	}
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::ReqSmsCount(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::ReqSmsCount]bad format:", jsonString, 1);

	char sql[1024];
    sprintf(sql, "SELECT T2.LOCAL_BAL, t2.inter_bal FROM T00_USER T"\
                 " LEFT OUTER JOIN t02_user_sms_bal T2 "\
                 " ON T.USER_ID=T2.USER_ID"
                 " WHERE T.USER_ID = '%s'", pUid);

	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
    {
		const char* smscount = psql->GetField("LOCAL_BAL");
		const char* inter_bal = psql->GetField("inter_bal");
		out<<FormatString("{nsms:%d}",(atoi(NOTNULL(smscount))+atoi(NOTNULL(inter_bal))));
    }
	else
	{
		out << "{nsms:0}";
	}

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::ReqUserTelEmailInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::ReqUserTelEmailInfo]bad format:", jsonString, 1);
    int size = root.size();

    string usernameStr="";
    for (int i=0; i<size; i++)
    {
        if (i>0)
        {
            usernameStr+=",";
        }
        usernameStr=usernameStr+"'"+root.getv(i, "")+"'";
    }
   
    string sql="SELECT USER_ID,COUNTRY,EMAIL,MOBILE FROM T00_USER WHERE USER_ID IN ("+usernameStr+") AND IS_FLAG = '1'";
    MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), 3);
	
	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* usrid=psql->GetField("USER_ID");
		const char* tel=psql->GetField("EMAIL");
		const char* mb1=psql->GetField("MOBILE");

		out<<'{';
		out<<"\"uid\":\""<<NOTNULL(usrid)<<"\"";
		out<<",\"email\":\""<<NOTNULL(tel)<<"\"";
		out<<",\"mb1\":\""<<(IsValidMobile(mb1)?mb1:"")<<"\"";
		out<<",\"mb2\":\"\"";
		out<<'}';
	}
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::QueryMucRoomCount(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::QueryMucRoomCount]bad format:", jsonString, 1);
	std::string roomJid		= root.getv("grid", "");
	std::string roomName	= root.getv("grname", "");
	std::string roomKey		= root.getv("grkey", "");
	std::string usrtype		= root.getv("usrtype", "");

	std::stringstream sql;
	sql << " SELECT COUNT(1) AS ROOMNUM FROM OFMUCROOM T1,OFMUCAFFILIATION T2 WHERE T1.ROOMID= T2.ROOMID AND T1.ISCMPYROOM=0";
    if (strcmp(usrtype.c_str(),"21")==0||strcmp(usrtype.c_str(),"1")==0||strcmp(usrtype.c_str(),"3")==0||strcmp(usrtype.c_str(),"")==0)
    {
        sql << " AND T1.SERVICEID !=8";
    }
    else if (strcmp(usrtype.c_str(),"50")==0)
    {
        sql << " AND T1.SERVICEID =8";
    }
    if (roomJid.length()>0)
    {
        sql << " AND T1.NAME = '" << roomJid <<"'";
    }
    if (roomName.length()>0)
    {
        sql << " AND T1.NATURALNAME LIKE '%" << roomName << "%'";
    }
    if (roomKey.length()>0)
    {
        sql << " AND T1.ROOMKEYWORDS LIKE '%" << roomKey << "%'";
    }

	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql.str().c_str()), 3);

    out<<'{';
    if(psql->NextRow())
    {
        const char *roomcount=psql->GetField("ROOMNUM");
        out<<FormatString("num:%s",(roomcount?roomcount:"0"));
    }
    out<<'}';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::QueryMucRoomList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::QueryMucRoomList]bad format:", jsonString, 1);
	std::string roomJid		= root.getv("grid", "");
	std::string roomName	= root.getv("grname", "");
	std::string roomKey		= root.getv("grkey", "");
	int curPage				= root.getv("curpage", 0);
	int pageCap				= root.getv("pagecap", 0);
	std::string usrtype		= root.getv("usrtype", "");

	std::stringstream sql;
	sql << "SELECT T1.NAME AS ROOMJID, T1.NATURALNAME AS ROOMNAME, T1.DESCRIPTION AS ROOMDES,T2.JID AS OWNERID, T1.MAXUSERS AS MAXNUM, T1.ROOMKEYWORDS, T1.ROOMBLMAUTH, T1.CREATIONDATE AS CREATETIME,IFNULL(TP1.CURR_NUM,0)+1 AS CURRENT_NUM";
    sql << " FROM OFMUCROOM T1 LEFT JOIN (SELECT COUNT(1) AS CURR_NUM, ROOMID FROM OFMUCMEMBER GROUP BY ROOMID) TP1 ON T1.ROOMID = TP1.ROOMID ,OFMUCAFFILIATION T2 WHERE T1.ROOMID= T2.ROOMID AND T1.ISCMPYROOM=0";
    if (strcmp(usrtype.c_str(),"21")==0||strcmp(usrtype.c_str(),"1")==0||strcmp(usrtype.c_str(),"3")==0||strcmp(usrtype.c_str(),"")==0)
    {
        sql << " AND T1.SERVICEID !=8";
    }
    else if (strcmp(usrtype.c_str(),"50")==0)
    {
        sql << " AND T1.SERVICEID =8";
    }
    if (roomJid.length()>0)
    {
        sql << " AND T1.NAME = '" << roomJid << "'";
    }
    if (roomName.length()>0)
    {
        sql << " AND T1.NATURALNAME LIKE '%" << roomName << "%'";
    }
    if (roomKey.length()>0)
    {
        sql << " AND T1.ROOMKEYWORDS LIKE '%" << roomKey << "%'";
    }

    int start=(curPage-1)*pageCap;
    sql << " LIMIT " << start << "," << pageCap;

	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql.str().c_str()), 3);

	int total = 0;
    out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* grid=psql->GetField("ROOMJID");
		const char* grname=psql->GetField("ROOMNAME");
		const char* grdes=psql->GetField("ROOMDES");
		const char* ownerid=psql->GetField("OWNERID");
		const char* maxnum=psql->GetField("MAXNUM");
		const char* currentnum=psql->GetField("CURRENT_NUM");
		const char* grkeys=psql->GetField("ROOMKEYWORDS");
		const char* blmauth=psql->GetField("ROOMBLMAUTH");
		const char* createdt=psql->GetField("CREATETIME");

		out<<'{';
		out<<"\"grid\":\""<<NOTNULL(grid)<<"\"";
		out<<",\"grname\":\""<<NOTNULL(grname)<<"\"";
		out<<",\"grdes\":\""<<NOTNULL(grdes)<<"\"";
		out<<",\"ownerid\":\""<<NOTNULL(ownerid)<<"\"";
		out<<",\"mnum\":\""<<NOTNULL(maxnum)<<"\"";
		out<<",\"cnum\":\""<<NOTNULL(currentnum)<<"\"";
		out<<",\"grkeys\":\""<<NOTNULL(grkeys)<<"\"";
		out<<",\"auth\":\""<<NOTNULL(blmauth)<<"\"";
		out<<",\"creatdt\":\""<<NOTNULL(createdt)<<"\"";
		out<<'}';
	}
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

int MessageSvc::ReqMucRoomDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::ReqMucRoomDetail]bad format:", jsonString, 1);
	std::string roomId = root.getv("grid", "");

	char sql[1024];
	sprintf(sql, "SELECT T1.NATURALNAME AS ROOMNAME, T1.DESCRIPTION AS ROOMDES,SUBJECT AS ROOMSUBJECT,T1.MAXUSERS AS MAXNUM, T1.ROOMKEYWORDS, T1.ROOMBLMAUTH FROM OFMUCROOM  T1 WHERE T1.NAME = '%s'", roomId.c_str());
    
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	out<<'{';
	out<<"\"grid\":\""<<(roomId)<<"\"";
	if (psql->NextRow())
	{
		const char* roomname=psql->GetField("ROOMNAME");
		const char* grdes=psql->GetField("ROOMDES");
		const char* subject=psql->GetField("ROOMSUBJECT");
		const char* maxnum=psql->GetField("MAXNUM");
		const char* roomkeys=psql->GetField("ROOMKEYWORDS");
		const char* blmauth=psql->GetField("ROOMBLMAUTH");

		out<<",\"grname\":\""<<NOTNULL(roomname)<<"\"";
		out<<",\"grdes\":\""<<NOTNULL(grdes)<<"\"";
		out<<",\"subject\":\""<<NOTNULL(subject)<<"\"";
		out<<",\"mnum\":\""<<NOTNULL(maxnum)<<"\"";
		out<<",\"keys\":\""<<NOTNULL(roomkeys)<<"\"";
		out<<",\"auth\":\""<<NOTNULL(blmauth)<<"\"";
	}
	out<<'}';

    RELEASE_MYSQL_RETURN(psql, 0);
}

bool MessageSvc::_AddContactGroup(const char *usrid,const char *grpId,const char *grpname,MySql *psql)
{
    char* sql = "INSERT INTO T00_GROUP (USER_ID, GROUPNAME, GROUPDES) VALUES('%s','%s','%s')";
    CHECK_MYSQL_STATUS(psql->Execute(FormatString(sql,usrid,grpId,grpname).c_str())>=0, false);
    return true;
}

bool MessageSvc::_DelContactGroup(const char *usrid,const char *grpId,MySql *psql)
{
    char* sql ="DELETE FROM T00_GROUP WHERE USER_ID ='%s'AND GROUPNAME='%s'";
	CHECK_MYSQL_STATUS(psql->Execute(FormatString(sql,usrid,grpId).c_str())>=0, false);
    return true;
}

bool MessageSvc::_ModifyContactGroup(const char *usrid,const char *grpId,const char *grpname,MySql *psql)
{
    char* sql = "UPDATE T00_GROUP SET GROUPDES = '%s' WHERE USER_ID ='%s' AND GROUPNAME='%s'";
	CHECK_MYSQL_STATUS(psql->Execute(FormatString(sql,grpname,usrid,grpId).c_str())>=0, false);
    return true;
}

int MessageSvc::ReqChangjiangUserLevel(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::ReqChangjiangUserLevel]bad format:", jsonString, 1);
   
    char *sql="SELECT userId,position,pilotlevel FROM t45_pilot_cj_man_cn";
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out<<'[';
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* userId=psql->GetField("userId");
		const char* position=psql->GetField("position");
		const char* level=psql->GetField("pilotlevel");

		out<<'{';
		out<<"\"uid\":\""<<NOTNULL(userId)<<"\"";
		out<<",\"pos\":\""<<NOTNULL(position)<<"\"";
		out<<",\"lev\":\""<<NOTNULL(level)<<"\"";
		out<<'}';
	}
	out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

// 连北京的数据库
int MessageSvc::OnUserRegistCmpy(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::OnUserRegistCmpy]bad format:", jsonString, 1);
	std::string UserId		= root.getv("uid", "");
	std::string CmpyId		= root.getv("cmypid", "");
	std::string CmpyName	= root.getv("cmypname", "");

    int len=CmpyId.length()>9?CmpyId.length()-9:1;
    CmpyId=CmpyId.replace(0,len,"2");
    bool roomExist=false;
    int roomId=-1;
    string userJid=string(UserId)+"@boloomo.com";

	char sql[1024];
    sprintf(sql, "select roomid from ofmucroom where name='%s'", CmpyId.c_str());

	MySqlBj *psql = CREATE_MYSQL_BJ;
	CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{
		roomExist=true;
		READMYSQL_INT(roomid,roomId,-1)
	}
   
    if (!roomExist) //不存在,创建群并设用户为管理员
    {
        sprintf(sql, "select max(roomid) as maxid from ofmucroom");
		CHECK_MYSQL_BJ_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
		READMYSQL_INT(maxid,roomId,1000);

        if (roomId<80000)
        {
            roomId=80000;
        }
        roomId++;

        sprintf(sql, "insert into ofmucroom(serviceId,roomId,creationDate,modificationDate,name,naturalName,description,lockedDate,emptyDate,canChangeSubject,maxUsers,publicRoom,moderated,membersOnly,canInvite,canDiscoverJID,logEnabled,rolesToBroadcast,useReservedNick,canChangeNick,canRegister,roomkeywords,roomblmauth,iscmpyRoom)"\
                   " values(7,%d,'000000000000000','000000000000000','%s','%s','...','000000000000000','000000000000000',0,50,1,0,1,0,1,0,7,0,1,1,'',1,1)",
				   roomId, CmpyId.c_str(), CmpyName.c_str());
		CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);

		sprintf(sql, "insert into ofmucaffiliation(roomID,jid,affiliation) values(%d,'%s',10)", roomId, userJid.c_str());
		CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);
    }
    else
    {
        sprintf(sql, "select * from ofmucaffiliation where roomId=%d and affiliation=10", roomId);
		CHECK_MYSQL_BJ_STATUS(psql->Query(sql), 3);
        
		int count=psql->GetRowCount();
		if (count>0) //存在管理员
		{
			sprintf(sql, "insert into ofmucmember(roomID,jid,nickname) values(%d,'%s','%s')", roomId, userJid.c_str(), userJid.c_str());
			CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);
		}
		else		//不存在管理员
		{
			sprintf(sql, "insert into ofmucaffiliation(roomID,jid,affiliation) values(%d,'%s',10)", roomId, userJid.c_str());
			CHECK_MYSQL_BJ_STATUS(psql->Execute(sql)>=0, 3);
		}
    }
    RELEASE_MYSQL_BJ_RETURN(psql, 0);
}

/* {uid:"jiangtao",cmypid:"C0904201000"}*/
int MessageSvc::OnUserUnRegistCmpy(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MessageSvc::OnUserUnRegistCmpy]bad format:", jsonString, 1);
	std::string UserId = root.getv("uid", "");
	std::string CmpyId = root.getv("cmypid", "");

	char sql[1024];
    sprintf(sql, "delete from t41_comp_contacts where user_Id='%s'", UserId.c_str());

	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    if (CmpyId.substr(0,1)=="C"||CmpyId.substr(0,1)=="c")
    {
        int len=CmpyId.length()>9?CmpyId.length()-9:1;
        CmpyId=CmpyId.replace(0,len,"2");
    }

    int roomId=-1;
    string userJid = UserId+"@boloomo.com";
    sprintf(sql, "select roomid from ofmucroom where name='%s'", CmpyId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
    READMYSQL_INT(roomid,roomId,-1);

    sprintf(sql, "select * from ofmucmember where roomId=%d and jid='%s'", roomId, userJid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int count = psql->GetRowCount();
	if (count>0)	//判断是否普通成员？是,直接删除
	{
		sprintf(sql, "delete from ofmucmember where roomId=%d and jid='%s'", roomId, userJid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else			//不是普通成员
	{
		sprintf(sql, "select jid from ofmucaffiliation where roomId=%d and affiliation=10", roomId);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		if(psql->NextRow())
		{
			char AdminJid[128]= {'\0'};
			READMYSQL_STR(jid,AdminJid);
			if(string(AdminJid)==userJid)	//是群的管理员
			{
				sprintf(sql, "select jid as NadJid from ofmucmember where roomId=%d", roomId);
				CHECK_MYSQL_STATUS(psql->Query(sql), 3);

				if(psql->NextRow())			//选择其中一位普通成员做管理员
				{
					char NJid[128]= {'\0'};
					READMYSQL_STR(NadJid,NJid);
					sprintf(sql, "update ofmucaffiliation set jid='%s' where roomId=%d", NJid, roomId);
					CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

					sprintf(sql, "delete from ofmucmember where jid='%s' where roomId=%d", NJid, roomId);
					CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
				}
				else						//没有可选的管理员
				{
					sprintf(sql, "delete from ofmucaffiliation where roomId=%d", roomId);
					CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

					sprintf(sql, "delete from ofmucroom where roomId=%d and serviceId=7", roomId);
					CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
				}
			}	
		}
	}

    RELEASE_MYSQL_RETURN(psql, 0);
}

/*{uid:"jiangtao",type:1}
type=1 请求亲情大使 
type=2 请求关注同一条船的亲情版用户
[{uid:"caiwj",nick:"bug maker"},{uid:"jiangtao",nick:"JT"}]
*/
int MessageSvc::GetFamilyDaShiList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[MessageSvc::GetFamilyDaShiList]bad format:", jsonString, 1);
	std::string UserId = root.getv("uid", "");
	int type = root.getv("type", 0);
	if(type < 1 || type > 2)
		return 1;

	char sql[1024];
	if(type == 1)
	{
		strcpy(sql, "select user_id, nickname from t00_user where usertype = '22'");
	}
	else
	{
		sprintf(sql, "SELECT t1.user_id, t1.nickname FROM t00_user t1, t00_user_conship t2, t00_user_conship t3\
			WHERE t1.USER_ID = t2.USER_ID AND t2.SHIPID = t3.SHIPID AND t3.USER_ID = '%s'", UserId.c_str());
	}

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out << "[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* uid = psql->GetField("user_id");
		const char* nick = psql->GetField("nickname");
		out << FormatString("{uid:\"%s\",nick:\"%s\"}", uid, nick);
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

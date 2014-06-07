#include "eMOPSurveySvc.h"
#include "blmcom_head.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"

IMPLEMENT_SERVICE_MAP(eMOPSurveySvc)

eMOPSurveySvc::eMOPSurveySvc()
{

}

eMOPSurveySvc::~eMOPSurveySvc()
{

}

bool eMOPSurveySvc::Start()
{
	if(!g_MessageService::instance()->RegisterCmd(MID_EMOP_SURVEY, this))
		return false;

	SERVICE_MAP(0x01,eMOPSurveySvc,GetAllShipsSurvey); 
	SERVICE_MAP(0x02,eMOPSurveySvc,GetAllShipsCertificate);
	SERVICE_MAP(0x03,eMOPSurveySvc,GetShipSurveyCertificate); 
	SERVICE_MAP(0x04,eMOPSurveySvc,AddShipSurvey);
	SERVICE_MAP(0x05,eMOPSurveySvc,GetShipSurvey);
	SERVICE_MAP(0x06,eMOPSurveySvc,UpdateShipSurvey);
	SERVICE_MAP(0x07,eMOPSurveySvc,DeleteShipSurvey);
	SERVICE_MAP(0x08,eMOPSurveySvc,FinishShipSurvey);
	SERVICE_MAP(0x09,eMOPSurveySvc,AddShipCertificate);
	SERVICE_MAP(0x0a,eMOPSurveySvc,GetShipCertificate);
	SERVICE_MAP(0x0b,eMOPSurveySvc,UpdateShipCertificate);
	SERVICE_MAP(0x0c,eMOPSurveySvc,DeleteShipCertificate);

	SERVICE_MAP(0x0d,eMOPSurveySvc,GetTemplateInfo);
	SERVICE_MAP(0x0e,eMOPSurveySvc,GetAlertTemplateInfo);
	SERVICE_MAP(0x0f,eMOPSurveySvc,AddAlertTemplateInfo);
	SERVICE_MAP(0x10,eMOPSurveySvc,DeleteAlertTemplate);
	SERVICE_MAP(0x11,eMOPSurveySvc,EditAlertTemplateInfo);
	SERVICE_MAP(0x12,eMOPSurveySvc,GetShipsInfo);
	SERVICE_MAP(0x13,eMOPSurveySvc,GetSurveyInfo);
	SERVICE_MAP(0x14,eMOPSurveySvc,AddRelateCERT);
	SERVICE_MAP(0x15,eMOPSurveySvc,DeleteRelateCERT);
	SERVICE_MAP(0x16,eMOPSurveySvc,GetEngineerList);
	SERVICE_MAP(0x17,eMOPSurveySvc,AddRelateEngineering);
	SERVICE_MAP(0x18,eMOPSurveySvc,DeleteRelateEngineering);
	SERVICE_MAP(0x19,eMOPSurveySvc,GetSurCERTRelate);
	SERVICE_MAP(0x1a,eMOPSurveySvc,AddRelateSur);
	SERVICE_MAP(0x1b,eMOPSurveySvc,DeleteRelateSur);
	SERVICE_MAP(0x1c,eMOPSurveySvc,GetSurveyMaTable);
	SERVICE_MAP(0x1d,eMOPSurveySvc,GetCERTMaTable);
	DEBUG_LOG("[eMOPSurveySvc::Start] OK......................................");
	return true;
} 

//查看证书
//{seq:"",sid:"",cid:"",ctype:1}
//{seq:"",pics:["","",""],data:[{id:"",name:""},{},{}]}
int eMOPSurveySvc::GetShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::DeleteShipCertificate]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string strUid= root.getv("uid", ""); 
	string strShipid= root.getv("sid", ""); 
	string strCid= root.getv("cid", "");   
	int ctype= root.getv("ctype", 0);   

	out<<"{seq:\""<<strSeq<<"\",pics:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	sprintf(sql,"SELECT PICK_ID FROM blm_emop_etl.t50_EMOP_PICTURE WHERE OBJ_TYPE=2 AND OBJ_ID='%s'",strCid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
	
	string picId="";
	int idx=0;
	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(PICK_ID,picId); 
		out<<"\""<<picId<<"\"";
	}
	out<<"],data:[";
	if(ctype==1){//只有检验证书有检验项
		sprintf(sql,"SELECT SURVEY_CODE,SURVEY_NAME FROM (\
					SELECT t3.SURVEY_CODE,t3.SURVEY_NAME FROM \
					blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE t1\
					LEFT JOIN blm_emop_etl.T50_EMOP_SC_SHIP_SURVERY_CERT_REL t2 ON t1.CERT_CODE=t2.CERT_CODE\
					LEFT JOIN blm_emop_etl.T50_EMOP_SC_SURVEY_CODE t3 ON t3.SURVEY_CODE=t2.SURVEY_CODE  \
					WHERE t1.SHIP_ID='%s' AND t1.ID='%s' ) t\
					WHERE SURVEY_CODE IS NOT NULL AND  SURVEY_CODE <>'';",strShipid.c_str(),strCid.c_str()); 
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
		string code="";
		string name="";
		idx=0;
		while(psql->NextRow()){
			if(idx>0)
				out<<",";
			idx++;
			READMYSQL_STRING(SURVEY_CODE,code); 
			READMYSQL_STRING(SURVEY_NAME,name); 
			out<<"{id:\""<<code<<"\",name:\""<<name<<"\"}";
		}
	}
	out<<"]}";
 
	RELEASE_MYSQL_RETURN(psql, 0);
}

//删除证书
//{seq:"",uid:"",sid:"",cid:""}
int eMOPSurveySvc::DeleteShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::DeleteShipCertificate]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string strUid= root.getv("uid", ""); 
	string strShipid= root.getv("sid", ""); 
	string strCid= root.getv("cid", "");    

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE WHERE id='%s';",strCid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//修改证书
//{seq:"",uid:"",sid:"",cid:"",idate:12,edate:12,num:"",org:""}
int eMOPSurveySvc::UpdateShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPSurveySvc::AddShipCertificate]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string strUid= root.getv("uid", ""); 
	string strShipid= root.getv("sid", ""); 
	string strCid= root.getv("cid", "");   
	string strNum= root.getv("num", ""); 
	string strOrg= root.getv("org", "");    
	int beginDate= root.getv("idate", 0);   
	int endDate= root.getv("edate", 0);  
	string strBeginDate = TimeToTimestampStr(beginDate); 
	string strEndDate = TimeToTimestampStr(endDate); 

	string todayStr = CurLocalTime();

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	sprintf(sql,"UPDATE  blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE \
				SET SHIP_ID='%s',CERT_NO='%s',CERT_SIGN_ORG='%s',CERT_SIGN_DT='%s',CERT_VALID_DT='%s',OPERTOR='%s',OP_DT='%s'\
				WHERE ID='%s';",strShipid.c_str(),strNum.c_str(),strOrg.c_str(),strBeginDate.c_str(),strEndDate.c_str(),strUid.c_str(),todayStr.c_str(),strCid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//新增船舶证书
//{seq:"",uid:"",sid:"",code:"",num:"",idate:1123,edate:1123,org:""}
int eMOPSurveySvc::AddShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::AddShipCertificate]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string strUid= root.getv("uid", ""); 
	string strShipid= root.getv("sid", ""); 
	string strCode= root.getv("code", "");   
	string strNum= root.getv("num", ""); 
	string strOrg= root.getv("org", "");    
	int beginDate= root.getv("idate", 0);   
	int endDate= root.getv("edate", 0);  
	string strBeginDate = TimeToTimestampStr(beginDate); 
	string strEndDate = TimeToTimestampStr(endDate); 

	string todayStr = CurLocalTime();

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	sprintf(sql,"SELECT MAX(ID)+0 AS MAX_NUM FROM blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE;"); 
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int maxNum=0;
	if(psql->NextRow()){  
		READMYSQL_INT(MAX_NUM,maxNum,0); 
	}
	char newid[30]="";
	sprintf(newid,"%020d",maxNum+1);
	sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE(ID,CERT_CODE,SHIP_ID,CERT_NO,CERT_SIGN_ORG,CERT_SIGN_DT,CERT_VALID_DT,OPERTOR,OP_DT) \
				VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s')",newid,strCode.c_str(),strShipid.c_str(),strNum.c_str(),strOrg.c_str(),
				strBeginDate.c_str(),strEndDate.c_str(),strUid.c_str(),todayStr.c_str()); 
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	out<<"{seq:\""<<strSeq<<"\",eid:0,id:\""<<newid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//请求模板信息（名称）
//{seq:"",uid:""}
//{seq:"",data:[{id:"",name:""},{},{},....]}
int eMOPSurveySvc::GetTemplateInfo(const char *pUid, const char *jsonString, std::stringstream &out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::GetTemplate]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strUid = root.getv("uid", "");  

	MySql* psql = CREATE_MYSQL;
	
	char sql[1024] = "";  
	sprintf(sql,"SELECT t1.SHIPID FROM blm_emop_etl.T50_EMOP_REGISTERED_USER_FLEET t1 \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP t2 \
				ON t1.SHIPID=t2.SHIPID \
				WHERE t1.USER_ID='%s'",strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	std::stringstream temp_sql;
	temp_sql<<"WHERE SHIP_ID IN (";
	int idx = 0;
	string shipid="";
	while(psql->NextRow()){
		if(idx>0)
			temp_sql<<",";
		idx++;
		READMYSQL_STRING(SHIPID,shipid);
		temp_sql<<"'"<<shipid<<"'";
	}
	temp_sql<<") ";

	sprintf(sql,"SELECT CERT_CODE,CERT_CWBT,CERT_NAME FROM blm_emop_etl.T50_EMOP_SC_CERTIFICATE_CODE \
				WHERE CERT_CODE IN (SELECT DISTINCT CERT_CODE FROM blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE \
				%s)",temp_sql.str().c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",data:[";
	idx=0;
	string cert_code="";
	string cert_cwbt="";
	string cert_name="";

	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(CERT_CODE,cert_code);
		READMYSQL_STRING(CERT_CWBT,cert_cwbt); 
		READMYSQL_STRING(CERT_NAME,cert_name);
		out<<"{code:\""<<cert_code<<"\",cwbt:\""<<cert_cwbt<<"\",name:\""<<cert_name<<"\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;

}

//{seq:"",uid:""}
//{seq:””,data:[{mid:””,mname:””,certs:[{cwbt:””,code:””,cname:””,notice:””},{},{},...],limit:[{flag:1,day:30,person:[{pid:””,pname:””},{},{},...],way:1},{},{}],ship:[{sid:””,sname:””},{},{},...]},
//{},{},...]}
//请求预警模板信息
int eMOPSurveySvc::GetAlertTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::GetSurvey]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strUid = root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;
	out<<"{seq:\""<<strSeq<<"\",data:[";
	char sql[1024] = "";  
	sprintf(sql,"SELECT TEMPLATE_ID,TEMPLATE_NAME FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE \
				WHERE USER_ID='%s'",strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	int idx=0;
	int template_id=0;
	string template_name="";
	while(psql->NextRow()){
		if(idx >0)
			out<<",";
		idx++;
		READMYSQL_INT(TEMPLATE_ID,template_id,0);
		READMYSQL_STRING(TEMPLATE_NAME,template_name);
		out<<"{mid:\""<<template_id<<"\",mname:\""<<template_name<<"\",";
		string certs="";
		string limits="";
		string ships="";
		getCerts(template_id,certs);
		getLimit(template_id,limits);
		DEBUG_LOG("endtestLIMIT");
		getShips(template_id,ships);
		out<<certs<<limits<<ships;
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//请求船舶id，船舶名
int eMOPSurveySvc::getShips(int id,std::string &outStr){
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	std::stringstream shipsSql; 
	sprintf(sql,"SELECT SHIPID,NAME_CN FROM blm_emop_etl.T41_EMOP_SHIP \
				WHERE SHIPID IN (SELECT SHIPID FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_SHIP \
				WHERE TEMPLATE_ID=%d)",id);
	std::stringstream out;
	out<<"ship:[";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int idx=0;
	string shipid="";
	string name_cn="";
	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(SHIPID,shipid);
		READMYSQL_STRING(NAME_CN,name_cn);
		out<<"{sid:\""<<shipid<<"\",sname:\""<<name_cn<<"\"}";
	}
	out<<"]}";
	outStr = out.str();
	RELEASE_MYSQL_RETURN(psql, 0);
}

//请求预警
int eMOPSurveySvc::getLimit(int id,std::string &outStr){
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	std::stringstream shipsSql; 
	DEBUG_LOG("2");
	sprintf(sql,"SELECT RED_DAYS,RED_NOTIFY FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE \
				WHERE TEMPLATE_ID=%d",id);
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
	DEBUG_LOG("1");
	int red_days = 0;
	int red_notify = 0;
	if(psql->NextRow())
	{
		READMYSQL_INT(RED_DAYS,red_days,0);
		READMYSQL_INT(RED_NOTIFY,red_notify,0);
	}
	DEBUG_LOG("start");
	shipsSql<<"limit:[{flag:1,day:"<<red_days<<",person:[";
	sprintf(sql,"SELECT USER_ID,NAME FROM blm_emop_etl.T50_EMOP_USERS \
				WHERE USER_ID IN (SELECT USER_ID FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE \
				WHERE ALERT_TYPE=1 AND TEMPLATE_ID=%d)",id);
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	DEBUG_LOG("end");
	int idx = 0;
	string user_id="";
	string name="";
	while(psql->NextRow())
	{
		if(idx > 0)
			shipsSql<<",";
		idx++;
		READMYSQL_STRING(USER_ID,user_id);
		READMYSQL_STRING(NAME,name);
		shipsSql<<"{pid:\""<<user_id<<"\",pname:\""<<name<<"\"}";
	}
	shipsSql<<"],way:"<<red_notify<<"},";
	
	
	sprintf(sql,"SELECT YELLOW_DAYS,YELLOW_NOTIFY FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE \
				WHERE TEMPLATE_ID=%d",id);
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int yellow_days = 0;
	int yellow_notify = 0;
	if(psql->NextRow())
	{
		READMYSQL_INT(YELLOW_DAYS,yellow_days,0);
		READMYSQL_INT(YELLOW_NOTIFY,yellow_notify,0);
	}
	shipsSql<<"{flag:2,day:"<<yellow_days<<",person:[";
	sprintf(sql,"SELECT USER_ID,NAME FROM blm_emop_etl.T50_EMOP_USERS \
				WHERE USER_ID IN (SELECT USER_ID FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE \
				WHERE ALERT_TYPE=2 AND TEMPLATE_ID=%d)",id);
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	idx = 0;
	user_id="";
	name="";
	while(psql->NextRow())
	{
		if(idx > 0)
			idx++;
		READMYSQL_STRING(USER_ID,user_id);
		READMYSQL_STRING(NAME,name);
		shipsSql<<"{pid:\""<<user_id<<"\",pname:\""<<name<<"\"}";
	}
	shipsSql<<"],way:"<<yellow_notify<<"},";


	sprintf(sql,"SELECT ORANGE_DAYS,ORANGE_NOTIFY FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE \
				WHERE TEMPLATE_ID=%d",id);
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int orange_days = 0;
	int orange_notify = 0;
	if(psql->NextRow())
	{
		READMYSQL_INT(ORANGE_DAYS,orange_days,0);
		READMYSQL_INT(ORANGE_NOTIFY,orange_notify,0);
	}
	shipsSql<<"{flag:3,day:"<<orange_days<<",person:[";
	sprintf(sql,"SELECT USER_ID,NAME FROM blm_emop_etl.T50_EMOP_USERS \
				WHERE USER_ID IN (SELECT USER_ID FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE \
				WHERE ALERT_TYPE=3 AND TEMPLATE_ID=%d)",id);
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	idx = 0;
	user_id="";
	name="";
	while(psql->NextRow())
	{
		if(idx > 0)
			idx++;
		READMYSQL_STRING(USER_ID,user_id);
		READMYSQL_STRING(NAME,name);
		shipsSql<<"{pid:\""<<user_id<<"\",pname:\""<<name<<"\"}";
	}
	shipsSql<<"],way:"<<orange_notify<<"}],";
	outStr = shipsSql.str();
	RELEASE_MYSQL_RETURN(psql, 0);
}

//请求证书
int eMOPSurveySvc::getCerts(int id,std::string &outStr){
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	std::stringstream shipsSql; 
	sprintf(sql,"SELECT t1.CERT_CWBT,t1.CERT_CODE,t1.CERT_NAME,t2.CONTENT \
				FROM blm_emop_etl.T50_EMOP_SC_CERTIFICATE_CODE t1,blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_CERT t2 \
				WHERE t1.CERT_CODE=t2.CERT_CODE AND t2.TEMPLATE_ID=%d",id);
	std::stringstream out;
	out<<"certs:[";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int idx=0;
	string cert_cwbt="";
	string cert_code="";
	string cert_name="";
	string content="";
	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(CERT_CWBT,cert_cwbt);
		READMYSQL_STRING(CERT_CODE,cert_code);
		READMYSQL_STRING(CERT_Name,cert_name);
		READMYSQL_STRING(CONTENT,content);
		out<<"{cwbt:\""<<cert_cwbt<<"\",code:\""<<cert_code<<"\",cname:\""<<cert_name<<"\",notice:\""<<content<<"\"}";
	}
	out<<"],";
	outStr = out.str();
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{seq:””,uid:"",name:"",certs:[{cwbt:””,code:””,cname:””,notice:””},{},{},...],limit:[{flag:1,day:30,person:[{pid:””,pname:””},{},{},...],way:1},{},{}],ship:[{sid:””,sname:””},{},{},...]},
//{},{},...]}
//{seq:"",modid:"",eid:0}
//新建预警模板信息
int eMOPSurveySvc::AddAlertTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::AddAlertTemplateInfo]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strUid = root.getv("uid", "");
	string name = root.getv("name", "");
	Json* certs = root.get("certs");
	Json* limit = root.get("limit");
	Json* ships = root.get("ship");
	int template_id = 0;
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	sprintf(sql,"SELECT MAX(TEMPLATE_ID)+0 AS TEMPLATE_ID  FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE");
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	if(psql->NextRow())
		READMYSQL_INT(TEMPLATE_ID,template_id,0);
	template_id++;

	sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE(TEMPLATE_ID,USER_ID,TEMPLATE_NAME) \
				VALUES(%d,'%s','%s')",template_id,strUid.c_str(),name.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);


	if(certs)
	{
		for(int i = 0;i < certs->size();i++)
		{
			Json* certsItem = certs->get(i);
			if(!certsItem)
				continue;
			string code = certsItem->getv("code","");
			string notice = certsItem->getv("notice","");
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_CERT(TEMPLATE_ID,CERT_CODE,CONTENT) \
						VALUES(%d,'%s','%s')",template_id,code.c_str(),notice.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		}
	}

	if(limit)
	{

		DEBUG_LOG("test");
		Json* limitItem = limit->get(0);
		DEBUG_LOG("2");
		Json* person = limitItem->get("person");
		DEBUG_LOG("3");
		int day = limitItem->getv("day",0);
		int way = limitItem->getv("way",0);

		DEBUG_LOG("4");
		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE SET \
					RED_DAYS=%d,RED_NOTIFY=%d \
					WHERE TEMPLATE_ID=%d",day,way,template_id);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		if(person)
		{
			for(int i = 0;i < person->size();i++)
			{
				Json* personItem = person->get(i);
				if(!personItem)
					continue;
				string pid=personItem->getv("pid","");
				string pname=personItem->getv("pname","");
				sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE(TEMPLATE_ID,ALERT_TYPE,USER_ID) \
							VALUES(%d,1,'%s')",template_id,pid.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			}
		}

		limitItem = limit->get(1);
		person = limitItem->get("person");
		day = limitItem->getv("day",0);
		way = limitItem->getv("way",0);

		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE SET \
					YELLOW_DAYS=%d,YELLOW_NOTIFY=%d \
					WHERE TEMPLATE_ID=%d",day,way,template_id);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		if(person)
		{
			for(int i = 0;i < person->size();i++)
			{
				Json* personItem = person->get(i);
				if(!personItem)
					continue;
				string pid=personItem->getv("pid","");
				string pname=personItem->getv("pname","");
				sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE(TEMPLATE_ID,ALERT_TYPE,USER_ID) \
							VALUES(%d,2,'%s')",template_id,pid.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			}
		}

		limitItem = limit->get(2);
		person = limitItem->get("person");
		day = limitItem->getv("day",0);
		way = limitItem->getv("way",0);

		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE SET \
					ORANGE_DAYS=%d,ORANGE_NOTIFY=%d \
					WHERE TEMPLATE_ID=%d",day,way,template_id);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		if(person)
		{
			for(int i = 0;i < person->size();i++)
			{
				Json* personItem = person->get(i);
				if(!personItem)
					continue;
				string pid=personItem->getv("pid","");
				string pname=personItem->getv("pname","");
				sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE(TEMPLATE_ID,ALERT_TYPE,USER_ID) \
							VALUES(%d,3,'%s')",template_id,pid.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			}
		}
	}

	if(ships)
	{
		for(int i = 0;i < ships->size();i++)
		{
			Json* shipsItem = ships->get(i);
			if(!shipsItem)
				continue;
			string sid = shipsItem->getv("sid","");
			string sname = shipsItem->getv("sname","");
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_SHIP(TEMPLATE_ID,SHIPID) \
						VALUES(%d,'%s')",template_id,sid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		}
	}

	out<<"{seq:\""<<strSeq<<"\",modid:\""<<template_id<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//删除预警模板
//{seq:"",id:}
//{seq:"",eid:0}
int eMOPSurveySvc::DeleteAlertTemplate(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::DeleteAlertTemplateDetail]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	int id = root.getv("id", 0);  

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";    
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE WHERE TEMPLATE_ID=%d",id); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE WHERE TEMPLATE_ID=%d",id); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_CERT WHERE TEMPLATE_ID=%d",id); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_SHIP WHERE TEMPLATE_ID=%d",id); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:””,modid:"",name:"",certs:[{code:””,notice:””},{},{},...],limit:[{flag:1,day:30,person:[{pid:””,pname:””},{},{},...],way:1},{},{}],ship:[{sid:””,sname:””},{},{},...]},
//{},{},...]}
//{seq:"",eid:0}
//修改预警模板信息
int eMOPSurveySvc::EditAlertTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::AddAlertTemplateInfo]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	int template_id = root.getv("modid", 0);
	string name = root.getv("name", "");
	Json* certs = root.get("certs");
	Json* limit = root.get("limit");
	Json* ships = root.get("ship");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  

	sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE \
				SET TEMPLATE_NAME='%s' WHERE TEMPLATE_ID=%d",name.c_str(),template_id);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_CERT WHERE TEMPLATE_ID=%d",template_id); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	if(certs)
	{
		for(int i = 0;i < certs->size();i++)
		{
			Json* certsItem = certs->get(i);
			if(!certsItem)
				continue;
			string code = certsItem->getv("code","");
			string notice = certsItem->getv("notice","");
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_CERT(TEMPLATE_ID,CERT_CODE,CONTENT) \
						VALUES(%d,'%s','%s')",template_id,code.c_str(),notice.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		}
	}


	if(limit)
	{
		Json* limitItem = limit->get(0);
		Json* person = limitItem->get("person");
		int day = limitItem->getv("day",0);
		int way = limitItem->getv("way",0);

		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE SET \
					RED_DAYS=%d,RED_NOTIFY=%d \
					WHERE TEMPLATE_ID=%d",day,way,template_id);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE WHERE TEMPLATE_ID=%d",template_id); 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

		if(person)
		{
			for(int i = 0;i < person->size();i++)
			{
				Json* personItem = person->get(i);
				if(!personItem)
					continue;
				string pid=personItem->getv("pid","");
				string pname=personItem->getv("pname","");
				sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE(TEMPLATE_ID,ALERT_TYPE,USER_ID) \
							VALUES(%d,1,'%s')",template_id,pid.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			}
		}

		limitItem = limit->get(1);
		person = limitItem->get("person");
		day = limitItem->getv("day",0);
		way = limitItem->getv("way",0);

		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE SET \
					YELLOW_DAYS=%d,YELLOW_NOTIFY=%d \
					WHERE TEMPLATE_ID=%d",day,way,template_id);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		if(person)
		{
			for(int i = 0;i < person->size();i++)
			{
				Json* personItem = person->get(i);
				if(!personItem)
					continue;
				string pid=personItem->getv("pid","");
				string pname=personItem->getv("pname","");
				sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE(TEMPLATE_ID,ALERT_TYPE,USER_ID) \
							VALUES(%d,2,'%s')",template_id,pid.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			}
		}

		limitItem = limit->get(2);
		person = limitItem->get("person");
		day = limitItem->getv("day",0);
		way = limitItem->getv("way",0);

		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE SET \
					ORANGE_DAYS=%d,ORANGE_NOTIFY=%d \
					WHERE TEMPLATE_ID=%d",day,way,template_id);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		if(person)
		{
			for(int i = 0;i < person->size();i++)
			{
				Json* personItem = person->get(i);
				if(!personItem)
					continue;
				string pid=personItem->getv("pid","");
				string pname=personItem->getv("pname","");
				sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_PEOPLE(TEMPLATE_ID,ALERT_TYPE,USER_ID) \
							VALUES(%d,3,'%s')",template_id,pid.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			}
		}
	}
	
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_SHIP WHERE TEMPLATE_ID=%d",template_id); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	if(ships)
	{
		for(int i = 0;i < ships->size();i++)
		{
			Json* shipsItem = ships->get(i);
			if(!shipsItem)
				continue;
			string sid = shipsItem->getv("sid","");
			string sname = shipsItem->getv("sname","");
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_SHIP(TEMPLATE_ID,SHIPID) \
						VALUES(%d,'%s')",template_id,sid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		}
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",sid:""}
//{seq:"",imo:"",reg:"",exorg:""}
//请求船舶信息
int eMOPSurveySvc::GetShipsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::GetShipsInfo]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string sid = root.getv("sid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	sprintf(sql,"SELECT IMO,REG_PORT,SURVEY_ORG FROM blm_emop_etl.T41_EMOP_SHIP \
				WHERE SHIPID='%s'",sid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	string imo = "";
	string regport = "";
	string exorg = "";

	if(psql->NextRow())
	{
		READMYSQL_STRING(IMO,imo);
		READMYSQL_STRING(REG_PORT,regport);
		READMYSQL_STRING(SURVEY_ORG,exorg);
	}

	out<<"{seq:\""<<strSeq<<"\",imo:\""<<imo<<"\",reg:\""<<regport<<"\",exorg:\""<<exorg<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:""}
//{seq:"",data:[{id:"",name:""},{},{},....]}
//请求检验证书
int eMOPSurveySvc::GetSurveyInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::GetSurvey]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	sprintf(sql,"SELECT CERT_CODE,CERT_NAME FROM blm_emop_etl.T50_EMOP_SC_CERTIFICATE_CODE \
				WHERE CERT_TYPE=1");
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",data:[";
	int idx=0;
	string cert_code="";
	string cert_name="";

	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(CERT_CODE,cert_code);
		READMYSQL_STRING(CERT_NAME,cert_name); 
		out<<"{id:\""<<cert_code<<"\",name:\""<<cert_name<<"\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",sid:"",exid:"",cid:"id|id|id"}
//{seq:"",eid:0}
//添加关联证书
int eMOPSurveySvc::AddRelateCERT(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::AddRelateCERT]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strSid = root.getv("sid", "");
	string strExid = root.getv("exid", "");
	string strCid = root.getv("cid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	Tokens relateVec = StrSplit(strCid,"|");
	for(Tokens::iterator it=relateVec.begin();it!=relateVec.end();it++)
	{
		sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_SHIP_SURVERY_CERT_REL(SHIPID,SURVEY_CODE,CERT_CODE) \
					VALUES('%s','%s','%s');",strSid.c_str(),strExid.c_str(),it->c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//删除关联证书
//{seq:"",sid:"",ecid:"",cid:""}
//{seq:"",eid:0}
int eMOPSurveySvc::DeleteRelateCERT(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::DeleteRelateCERTDetail]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strSid = root.getv("sid", "");    
	string strExid = root.getv("exid", "");
	string strCid = root.getv("cid", "");   

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";    
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVERY_CERT_REL WHERE SHIPID='%s' and SURVEY_CODE='%s' \
		and CERT_CODE='%s'",
		strSid.c_str(),strExid.c_str(),strCid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",sid:""}
//{seq:"",bid:["","",]}
//请求工程单列表
int eMOPSurveySvc::GetEngineerList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::GetEngineerList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strSid = root.getv("sid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	sprintf(sql,"SELECT REPAIR_ENGINEERING_ID FROM blm_emop_etl.T50_EMOP_SC_SHIP_SUR_ENG_REL \
				WHERE SHIPID='%s'",strSid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",bid:[";
	int idx=0;
	string repair_engineering_id="";

	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(REPAIR_ENGINEERING_ID,repair_engineering_id);
		out<<"\""<<repair_engineering_id<<"\"";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",sid:"",exid:"",bid:"id|id|id"}
//{seq:"",eid:0}
//添加关联工程单
int eMOPSurveySvc::AddRelateEngineering(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::AddRelateEngineering]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strSid = root.getv("sid", "");
	string strExid = root.getv("exid", "");
	string strBid = root.getv("bid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	Tokens relateVec = StrSplit(strBid,"|");
	for(Tokens::iterator it=relateVec.begin();it!=relateVec.end();it++)
	{
		sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_SHIP_SUR_ENG_REL(SHIPID,SURVEY_CODE,REPAIR_ENGINEERING_ID) \
					VALUES('%s','%s','%s');",strSid.c_str(),strExid.c_str(),it->c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//删除关联工程单
//{seq:"",sid:"",exid:"",bid:""}
//{seq:"",eid:0}
int eMOPSurveySvc::DeleteRelateEngineering(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::DeleteRelateCERTDetail]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strSid = root.getv("sid", "");    
	string strExid = root.getv("exid", "");
	string strBid = root.getv("bid", "");   

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";    
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_SHIP_SUR_ENG_REL WHERE SHIPID='%s' and SURVEY_CODE='%s' \
		and REPAIR_ENGINEERING_ID='%s'",
		strSid.c_str(),strExid.c_str(),strBid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",cid:""}
//{seq:"",data:[{id:"",name:""},{},{}...]}
//请求检验证书的相关检验
int eMOPSurveySvc::GetSurCERTRelate(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::GetSurCERTRelate]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strCid = root.getv("cid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	sprintf(sql,"SELECT SURVEY_CODE,SURVEY_NAME FROM blm_emop_etl.t50_emop_sc_survey_code \
				WHERE SURVEY_CODE IN(SELECT SURVEY_CODE FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVERY_CERT_REL WHERE CERT_CODE='%s')",strCid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",data:[";
	int idx=0;
	string survey_code="";
	string survey_name="";

	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(SURVEY_CODE,survey_code);
		READMYSQL_STRING(SURVEY_NAME,survey_name);
		out<<"{id:\""<<survey_code<<"\",name:\""<<survey_name<<"\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",sid:"",cid:"",exid:"id|id|id"}
//{seq:"",eid:0}
//添加相关检验
int eMOPSurveySvc::AddRelateSur(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::AddRelateSur]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strSid = root.getv("sid", "");
	string strCid = root.getv("cid", "");
	string strExid = root.getv("exid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	Tokens relateVec = StrSplit(strExid,"|");
	for(Tokens::iterator it=relateVec.begin();it!=relateVec.end();it++)
	{
		sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SC_SHIP_SURVERY_CERT_REL(SHIPID,SURVEY_CODE,CERT_CODE) \
					VALUES('%s','%s','%s');",strSid.c_str(),strCid.c_str(),it->c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//删除相关检验
//{seq:"",sid:"",cid:"",exid:""}
//{seq:"",eid:0}
int eMOPSurveySvc::DeleteRelateSur(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::DeleteRelateSur]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strSid = root.getv("sid", "");    
	string strCid = root.getv("cid", "");
	string strExid = root.getv("exid", "");   

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";    
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVERY_CERT_REL \
				WHERE SHIPID='%s' and SURVEY_CODE='%s' \
		and CERT_CODE='%s'",
		strSid.c_str(),strCid.c_str(),strExid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:""}
//{seq:"",data:[{id:"",name:""},{},{},....]}
//请求检验码表
int eMOPSurveySvc::GetSurveyMaTable(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::GetSurveyMaTable]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	sprintf(sql,"SELECT SURVEY_CODE,SURVEY_NAME FROM blm_emop_etl.T50_EMOP_SC_SURVEY_CODE");
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",data:[";
	int idx=0;
	string survey_code="";
	string survey_name="";

	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(SURVEY_CODE,survey_code);
		READMYSQL_STRING(SURVEY_NAME,survey_name); 
		out<<"{id:\""<<survey_code<<"\",name:\""<<survey_name<<"\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:""}
//{seq:"",data:[{code:"",cwbt:"",type:1,name:""},{},{},....]}
//请求证书码表
int eMOPSurveySvc::GetCERTMaTable(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPServeySvc::GetCERTMaTable]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	sprintf(sql,"SELECT CERT_CODE,CERT_CWBT,CERT_TYPE,CERT_NAME FROM blm_emop_etl.T50_EMOP_SC_CERTIFICATE_CODE");
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",data:[";
	int idx=0;
	string cert_code="";
	string cert_cwbt="";
	int cert_type=0;
	string cert_name="";

	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(CERT_CODE,cert_code);
		READMYSQL_STRING(CERT_CWBT,cert_cwbt); 
		READMYSQL_INT(CERT_TYPE,cert_type,0); 
		READMYSQL_STRING(CERT_NAME,cert_name);
		out<<"{code:\""<<cert_code<<"\",cwbt:\""<<cert_cwbt<<"\",type:"<<cert_type<<",name:\""<<cert_name<<"\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//完成检验
//{seq:"",uid:"",port:"",begin:12,end:12,together:"code|code”,ca:"code|code"}
//{seq:"",eid:0}
int eMOPSurveySvc::FinishShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::FinishShipSurvey]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string strUid= root.getv("uid", ""); 
	string strPortid= root.getv("port", "");   
	string strSurveyIds= root.getv("together", "");   
	string strCertIds= root.getv("ca", "");   
	int beginDate= root.getv("begin", 0);   
	int endDate= root.getv("end", 0);  
	string strBeginDate = TimeToTimestampStr(beginDate); 
	string strEndDate = TimeToTimestampStr(endDate); 

	string todayStr = CurLocalTime();

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	//检验处理
	Tokens surIds=StrSplit(strSurveyIds, "|");
	for(Tokens::iterator it=surIds.begin();it!=surIds.end();it++){
		sprintf(sql,"UPDATE  blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY \
					SET ACTUAL_START_DATE='%s',ACTUAL_END_DATE='%s',SURVEY_PORTID='%s',STATUS=3,OPERTOR='%s',OP_DT='%s'\
					WHERE ID='%s';",strBeginDate.c_str(),strEndDate.c_str(),strPortid.c_str(),strUid.c_str(),todayStr.c_str(),
					it->c_str()); 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  

		char sql[1024] = "";    
		sprintf(sql,"SELECT MAX(ID)+0 as MAX_NUM FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY");
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

		int maxNum=0;
		if(psql->NextRow()){  
			READMYSQL_INT(MAX_NUM,maxNum,0); 
		}

		char newid[30]="";
		sprintf(newid,"%020d",maxNum+1);

		sprintf(sql,"INSERT INTO \
					blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY(ID,SURVEY_CODE,SHIP_ID,SCHEDULED_START_DATE,SCHEDULED_END_DATE,ACTUAL_START_DATE,ACTUAL_END_DATE,SURVEY_PORTID,CCS,SURVEY_PEOPLE,OPERTOR,OP_DT)\
					(SELECT '%s',SURVEY_CODE,SHIP_ID,SCHEDULED_START_DATE,SCHEDULED_END_DATE,ACTUAL_START_DATE,ACTUAL_END_DATE,SURVEY_PORTID,CCS,SURVEY_PEOPLE,OPERTOR,OP_DT \
					FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY WHERE ID='%s');",newid,it->c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	}
	
	//证书处理
	time_t t=endDate;
	tm* aTm = localtime(&t); 	
    char curdate[20];
    snprintf(curdate, 20, "%04d-%02d-%02d", aTm->tm_year+1900+1, aTm->tm_mon+1, aTm->tm_mday);
	Tokens certIds=StrSplit(strCertIds, "|");
	for(Tokens::iterator it=certIds.begin();it!=certIds.end();it++){
		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE\
					SET CERT_SIGN_DT='%s',CERT_VALID_DT='%s',OPERTOR='%s',OP_DT='%s'\
					WHERE ID='';",strEndDate.c_str(),curdate,strUid.c_str(),todayStr.c_str(),it->c_str()); 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);   
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//删除船舶的检验
//{seq:"",uid:"",id:""}
//{seq:"",eid:0}
int eMOPSurveySvc::DeleteShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::AddShipSurvey]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	//string strUid= root.getv("uid", ""); 
	string strId= root.getv("id", "");  

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY WHERE ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}


//修改船舶的检验
//{seq:"",uid:"",id:"",pd_s:1123,pd_f:1234,ad_s:1123,ad_f:1234,port:""}
//{seq:"",eid:0}
int eMOPSurveySvc::UpdateShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::AddShipSurvey]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string strUid= root.getv("uid", ""); 
	//string strShipId= root.getv("sid", ""); 
	string strId= root.getv("id", "");  
	string strPort= root.getv("port", "");  

	int pdsDate = root.getv("pd_s", 0); 
	int pdfDate = root.getv("pd_f", 0); 
	int adsDate = root.getv("ad_s", 0); 
	int adfDate = root.getv("ad_f", 0);  
	
	string strPdsDate = TimeToTimestampStr(pdsDate); 
	string strPdfDate = TimeToTimestampStr(pdfDate); 
	string strAdsDate = TimeToTimestampStr(adsDate); 
	string strAdfDate = TimeToTimestampStr(adfDate);  

	string todayStr = CurLocalTime();

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	sprintf(sql,"UPDATE  blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY \
				SET SCHEDULED_START_DATE='%s',SCHEDULED_END_DATE='%s',ACTUAL_START_DATE='%s',ACTUAL_END_DATE='%s',SURVEY_PORTID='%s',OPERTOR='%s',OP_DT='%s'\
				WHERE ID='%s'",
				strPdsDate.c_str(),strPdfDate.c_str(),strAdsDate.c_str(),strAdfDate.c_str(),strPort.c_str(),strUid.c_str(),todayStr.c_str(),strId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//查看船舶的检验
//{seq:"",sid:"",exid:""}
//{seq:"",data:[{id:"",name:""},{},{}],bill:["","",""]}
int eMOPSurveySvc::GetShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::AddShipSurvey]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string strShipId= root.getv("sid", ""); 
	string strId= root.getv("exid", ""); 
	
	out<<"{seq:\""<<strSeq<<"\",data:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	sprintf(sql,"SELECT t3.ID,t2.CERT_NAME FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVERY_CERT_REL t1\
				LEFT JOIN blm_emop_etl.T50_EMOP_SC_CERTIFICATE_CODE t2 ON t2.CERT_CODE=t1.CERT_CODE\
				LEFT JOIN blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE t3 ON t3.CERT_CODE=t2.CERT_CODE\
				WHERE t1.SURVEY_CODE='%s' AND t1.SHIPID='%s'",strId.c_str(),strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	 string cerId="";
	 string cerName="";
	 int idx=0;
	 while(psql->NextRow()){
		 if(idx>0)
			 out<<",";
		 idx++;
		 READMYSQL_STRING(ID,cerId); 
		 READMYSQL_STRING(CERT_NAME,cerName); 
		 out<<"{id:\""<<cerId<<"\",name:\""<<cerName<<"\"}";
	 }

	 out<<"],bill:["; 
	
	 sprintf(sql,"SELECT REPAIR_ENGINEERING_ID FROM blm_emop_etl.T50_EMOP_SC_SHIP_SUR_ENG_REL WHERE SURVEY_CODE='%s' AND SHIPID='%s'",strId.c_str(),strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	 string enId=""; 
	 idx=0;
	 while(psql->NextRow()){
		 if(idx>0)
			 out<<",";
		 idx++;
		 READMYSQL_STRING(REPAIR_ENGINEERING_ID,enId);  
		 out<<"\""<<enId<<"\"";
	 }
	 out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}


//新增船舶的检验
//{seq:"",uid:"",sid:"",exname:"",port:"",society:"",person:"",pd_s:1123,pd_f:1234,ad_s:1123,ad_f:1234}
//{seq:"",eid:0,id:""}
int eMOPSurveySvc::AddShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::AddShipSurvey]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string strUid= root.getv("uid", ""); 
	string strShipId= root.getv("sid", ""); 
	string strExid= root.getv("exid", "");  
	string strPort= root.getv("port", ""); 
	string strSociety= root.getv("society", ""); 
	string strPerson= root.getv("person", "");  

	int pdsDate = root.getv("pd_s", 0); 
	int pdfDate = root.getv("pd_f", 0); 
	int adsDate = root.getv("ad_s", 0); 
	int adfDate = root.getv("ad_f", 0);  
	
	string strPdsDate = TimeToTimestampStr(pdsDate); 
	string strPdfDate = TimeToTimestampStr(pdfDate); 
	string strAdsDate = TimeToTimestampStr(adsDate); 
	string strAdfDate = TimeToTimestampStr(adfDate);  

	string todayStr = CurLocalTime();

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	sprintf(sql,"SELECT MAX(ID)+0 as MAX_NUM FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY");
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	int maxNum=0;
	if(psql->NextRow()){  
		READMYSQL_INT(MAX_NUM,maxNum,0); 
	}

	char newid[30]="";
	sprintf(newid,"%020d",maxNum+1);
	   
	sprintf(sql,"INSERT INTO \
				blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY(ID,SURVEY_CODE,SHIP_ID,SCHEDULED_START_DATE,SCHEDULED_END_DATE,ACTUAL_START_DATE,ACTUAL_END_DATE,SURVEY_PORTID,CCS,SURVEY_PEOPLE,OPERTOR,OP_DT)\
				VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",
				newid,strExid.c_str(),strShipId.c_str(),strPdsDate.c_str(),strPdfDate.c_str(),strAdsDate.c_str(),strAdfDate.c_str(),strPort.c_str(),strSociety.c_str(),strPerson.c_str(),strUid.c_str(),todayStr.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,id:\""<<newid<<"\"}";
	
	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}


//请求船舶的检验项的所有证书信息
//{seq:"",sid:"",surid:""}
//{seq:"",sid:"",sname:"",certs:[{id:"",days:1,level:1,ctype:1,cname:"",cwbt:"",num:"",idate:"",edate:"",org:""}]}
int eMOPSurveySvc::GetShipSurveyCertificate(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::GetAllShipsSurveyCertificate]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", ""); 
	string strShipId= root.getv("sid", ""); 
	string strSurid= root.getv("surid", "");  

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶

	sprintf(sql,"SELECT NAME_CN FROM  blm_emop_etl.T41_EMOP_SHIP t2 WHERE SHIPID='%s'",strShipId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	string shipName="";
	if(psql->NextRow()){  
		READMYSQL_STRING(NAME_CN,shipName); 
	}

	out<<"{seq:\""<<strSeq<<"\",sid:\""<<strShipId<<"\",sname:\""<<shipName<<"\",certs:[";

	sprintf(sql,"SELECT t3.ID,t3.CERT_NO,t3.CERT_SIGN_ORG,t2.CERT_CWBT,t2.CERT_TYPE,t2.CERT_NAME,\
				UNIX_TIMESTAMP(t3.CERT_SIGN_DT) AS CERT_SIGN_DT,UNIX_TIMESTAMP(t3.CERT_VALID_DT) AS CERT_VALID_DT,\
				t5.RED_DAYS,t5.ORANGE_DAYS,t5.YELLOW_DAYS\
				FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVERY_CERT_REL t1\
				LEFT JOIN blm_emop_etl.T50_EMOP_SC_CERTIFICATE_CODE t2 ON t1.CERT_CODE=t2.CERT_CODE\
				LEFT JOIN blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE t3 ON t3.CERT_CODE=t2.CERT_CODE AND t3.SHIP_ID=t1.SHIPID\
				LEFT JOIN blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_CERT t4 ON t4.CERT_CODE=t2.CERT_CODE\
				LEFT JOIN blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE t5 ON t5.TEMPLATE_ID=t4.TEMPLATE_ID\
				WHERE t3.ID IS NOT NULL AND t1.SHIPID='%s' AND t1.SURVEY_CODE='%s'",strShipId.c_str(),strSurid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	string cerId="";
	string cerNo="";
	string cerSignOrg="";
	string cerCwbt=""; //检验人员
	int cerType=0; 
	string cerName="";
	int signDate=0;
	int validDate=0;

	int redDays=0;
	int orangeDays=0;
	int yellowDays=0;

	int idx=0;
	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;

		READMYSQL_STRING(ID,cerId);
		READMYSQL_STRING(CERT_NO,cerNo);
		READMYSQL_STRING(CERT_SIGN_ORG,cerSignOrg);
		READMYSQL_STRING(CERT_CWBT,cerCwbt);
		READMYSQL_INT(CERT_TYPE,cerType,0);
		READMYSQL_STRING(CERT_NAME,cerName);
		READMYSQL_INT(CERT_SIGN_DT,signDate,0);
		READMYSQL_INT(CERT_VALID_DT,validDate,0);

		READMYSQL_INT(RED_DAYS,redDays,0);
		READMYSQL_INT(ORANGE_DAYS,orangeDays,0);
		READMYSQL_INT(YELLOW_DAYS,yellowDays,0); 

		int days=getRemainDay(signDate,validDate);
		int level=0;
		if(level<=redDays)
			level=1;
		else if(level<=orangeDays&&level>redDays)
			level=2;
		else if(level<=yellowDays&&level>orangeDays)
			level=3;
		out<<"{id:\""<<cerId<<"\",days:"<<days<<",level:"<<level<<",ctype:"<<cerType<<",cname:\""<<cerName;
		out<<"\",cwbt:\""<<cerCwbt<<"\",num:\""<<cerNo<<"\",idate:"<<signDate<<",edate:"<<validDate<<",org:\""<<cerSignOrg<<"\"}";
	}		
	out<<"]}"; 
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}


//请求用户管理的所有船舶的所有证书信息
//{seq:"",uid:"",sids:"",ctype:2}
//{seq:"",date:[{sid:"",sname:"",certs:[{id:"",days:1,level:1,ctype:2,cname:"",cwbt:"",num:"",idate:"",edate:"",org:""},{},{},...]},{},{},...]}
int eMOPSurveySvc::GetAllShipsCertificate(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::GetAllShipsSurveyCertificate]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", ""); 
	string strShipId= root.getv("sids", ""); 
	string strUid= root.getv("uid", ""); 
	int type= root.getv("ctype", 0); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶
	map<string,string> shipMap;
	if(strShipId.empty()){
		sprintf(sql,"SELECT t1.SHIPID,t2.NAME_CN FROM  blm_emop_etl.T50_EMOP_REGISTERED_USER_FLEET t1\
					LEFT JOIN blm_emop_etl.T41_EMOP_SHIP t2 ON t1.SHIPID=t2.SHIPID\
					WHERE t1.USER_ID='%s'",strUid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

		string shipid="";
		string shipName="";
		while(psql->NextRow()){ 
			READMYSQL_STRING(SHIPID,shipid);
			READMYSQL_STRING(NAME_CN,shipName);
			shipMap[shipid]=shipName;
		}
	}
	else
	{
		Tokens shipIdVec = StrSplit(strShipId,"|"); 
		stringstream shipsSql;
		shipsSql<<"(";
		int idx=0;
		for(Tokens::iterator it=shipIdVec.begin();it!=shipIdVec.end();it++)
		{
			if(idx>0)
				shipsSql<<",";
			idx++; 
			shipsSql<<"'"<<it->c_str()<<"'";
		}
		shipsSql<<")";

		sprintf(sql,"SELECT t1.SHIPID,t2.NAME_CN FROM  blm_emop_etl.T50_EMOP_REGISTERED_USER_FLEET t1\
					LEFT JOIN blm_emop_etl.T41_EMOP_SHIP t2 ON t1.SHIPID=t2.SHIPID\
					WHERE t1.SHIPID IN %s",shipsSql.str().c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

		idx=0;
		string shipid="";
		string shipName="";
		while(psql->NextRow()){
			if(idx>0)
				shipsSql<<",";
			idx++;
			READMYSQL_STRING(SHIPID,shipid);
			READMYSQL_STRING(NAME_CN,shipName);
			shipMap[shipid]=shipName;
		}
	}  

	out<<"{seq:\""<<strSeq<<"\",data:[";

	for(map<string,string>::iterator it=shipMap.begin();it!=shipMap.end();it++)
	{
		string shipid=it->first;
		string shipName=it->second;
		if(it!=shipMap.begin())
			out<<",";


		stringstream typeSql;
		typeSql<<"";

		if(type!=0)
			typeSql<<"AND t2.CERT_TYPE="<<type;

		sprintf(sql,"SELECT t1.ID,t1.CERT_NO,t1.CERT_SIGN_ORG,t2.CERT_CWBT,t2.CERT_TYPE,t2.CERT_NAME,\
					UNIX_TIMESTAMP(t1.CERT_SIGN_DT) AS CERT_SIGN_DT,UNIX_TIMESTAMP(t1.CERT_VALID_DT) AS CERT_VALID_DT,\
					t4.RED_DAYS,t4.ORANGE_DAYS,t4.YELLOW_DAYS\
					FROM blm_emop_etl.T50_EMOP_SC_SHIP_CERTIFICATE t1 \
					LEFT JOIN blm_emop_etl.T50_EMOP_SC_CERTIFICATE_CODE t2 ON t1.CERT_CODE=t2.CERT_CODE\
					LEFT JOIN blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE_CERT t3 ON t3.CERT_CODE=t2.CERT_CODE\
					LEFT JOIN blm_emop_etl.T50_EMOP_SC_ALERT_TEMPLATE t4 ON t4.TEMPLATE_ID=t3.TEMPLATE_ID\
					WHERE t1.SHIP_ID='%s' %s;",shipid.c_str(),typeSql.str().c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
 
		string cerId="";
		string cerNo="";
		string cerSignOrg="";
		string cerCwbt=""; //检验人员
		int cerType=0; 
		string cerName="";
		int signDate=0;
		int validDate=0;
		
		int redDays=0;
		int orangeDays=0;
		int yellowDays=0;

		out<<"{sid:\""<<shipid<<"\",sname:\""<<shipName<<"\",certs:[";
		int idx=0;
		while(psql->NextRow()){
			if(idx>0)
				out<<",";
			idx++;

			READMYSQL_STRING(ID,cerId);
			READMYSQL_STRING(CERT_NO,cerNo);
			READMYSQL_STRING(CERT_SIGN_ORG,cerSignOrg);
			READMYSQL_STRING(CERT_CWBT,cerCwbt);
			READMYSQL_INT(CERT_TYPE,cerType,0);
			READMYSQL_STRING(CERT_NAME,cerName);
			READMYSQL_INT(CERT_SIGN_DT,signDate,0);
			READMYSQL_INT(CERT_VALID_DT,validDate,0);

			READMYSQL_INT(RED_DAYS,redDays,0);
			READMYSQL_INT(ORANGE_DAYS,orangeDays,0);
			READMYSQL_INT(YELLOW_DAYS,yellowDays,0); 
 
			int days=getRemainDay(signDate,validDate);
			int level=0;
			if(level<=redDays)
				level=1;
			else if(level<=orangeDays&&level>redDays)
				level=2;
			else if(level<=yellowDays&&level>orangeDays)
				level=3;
			out<<"{id:\""<<cerId<<"\",days:"<<days<<",level:"<<level<<",ctype:"<<cerType<<",cname:\""<<cerName;
			out<<"\",cwbt:\""<<cerCwbt<<"\",num:\""<<cerNo<<"\",idate:"<<signDate<<",edate:"<<validDate<<",org:\""<<cerSignOrg<<"\"}";
		}		
		out<<"]}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}


//请求用户管理的所有船舶的所有证书信息
//{seq:"",uid:"",sids:""}
//{seq:"",data:[{sid:"",sname:"",surs:[{id:"",status:1,exid:"",exname:"",pd_s:12,pd_f:12,ad_s:12,ad_f:12,port:"",society:"",person:""},{},{},...]},{},{},...]}
int eMOPSurveySvc::GetAllShipsSurvey(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPSurveySvc::GetAllShipsSurvey]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", ""); 
	string strShipId= root.getv("sids", ""); 
	string strUid= root.getv("uid", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶
	map<string,string> shipMap;
	if(strShipId.empty()){
		sprintf(sql,"SELECT t1.SHIPID,t2.NAME_CN FROM  blm_emop_etl.T50_EMOP_REGISTERED_USER_FLEET t1\
					LEFT JOIN blm_emop_etl.T41_EMOP_SHIP t2 ON t1.SHIPID=t2.SHIPID\
					WHERE t1.USER_ID='%s'",strUid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

		string shipid="";
		string shipName="";
		while(psql->NextRow()){ 
			READMYSQL_STRING(SHIPID,shipid);
			READMYSQL_STRING(NAME_CN,shipName);
			shipMap[shipid]=shipName;
		}
	}
	else
	{
		Tokens shipIdVec = StrSplit(strShipId,"|"); 
		stringstream shipsSql;
		shipsSql<<"(";
		int idx=0;
		for(Tokens::iterator it=shipIdVec.begin();it!=shipIdVec.end();it++)
		{
			if(idx>0)
				shipsSql<<",";
			idx++; 
			shipsSql<<"'"<<it->c_str()<<"'";
		}
		shipsSql<<")";

		sprintf(sql,"SELECT t1.SHIPID,t2.NAME_CN FROM  blm_emop_etl.T50_EMOP_REGISTERED_USER_FLEET t1\
					LEFT JOIN blm_emop_etl.T41_EMOP_SHIP t2 ON t1.SHIPID=t2.SHIPID\
					WHERE t1.SHIPID IN %s",shipsSql.str().c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

		idx=0;
		string shipid="";
		string shipName="";
		while(psql->NextRow()){
			if(idx>0)
				shipsSql<<",";
			idx++;
			READMYSQL_STRING(SHIPID,shipid);
			READMYSQL_STRING(NAME_CN,shipName);
			shipMap[shipid]=shipName;
		}
	}  

	out<<"{seq:\""<<strSeq<<"\",data:[";

	for(map<string,string>::iterator it=shipMap.begin();it!=shipMap.end();it++)
	{
		string shipid=it->first;
		string shipName=it->second;
		if(it!=shipMap.begin())
			out<<",";

		sprintf(sql,"SELECT ID,STATUS,t1.SURVEY_CODE,t2.SURVEY_NAME,t1.SURVEY_PORTID,t1.CCS,t1.SURVEY_PEOPLE,\
					UNIX_TIMESTAMP(SCHEDULED_START_DATE) AS SCHEDULED_START_DATE,UNIX_TIMESTAMP(SCHEDULED_END_DATE) AS SCHEDULED_END_DATE,UNIX_TIMESTAMP(ACTUAL_START_DATE) AS ACTUAL_START_DATE,UNIX_TIMESTAMP(ACTUAL_END_DATE) AS ACTUAL_END_DATE\
					FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY t1 \
					LEFT JOIN blm_emop_etl.T50_EMOP_SC_SURVEY_CODE t2 ON t1.SURVEY_CODE=t2.SURVEY_CODE\
					WHERE t1.SHIP_ID='%s'",shipid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

		string id="";
		int status=0;
		string surveyCode="";
		string surveyName="";
		string surveyPort="";
		string surveyPeople=""; //检验人员
		string ccs=""; 
		int pStartDate=0;
		int pEndDate=0;
		int aStartDate=0;
		int aEndtDate=0;

		out<<"{sid:\""<<shipid<<"\",sname:\""<<shipName<<"\",surs:[";
		int idx=0;
		while(psql->NextRow()){
			if(idx>0)
				out<<",";
			idx++;

			READMYSQL_STRING(ID,id);
			READMYSQL_INT(STATUS,status,0);
			READMYSQL_STRING(SURVEY_CODE,surveyCode);
			READMYSQL_STRING(SURVEY_NAME,surveyName);

			READMYSQL_INT(SCHEDULED_START_DATE,pStartDate,0);
			READMYSQL_INT(SCHEDULED_END_DATE,pEndDate,0);
			READMYSQL_INT(ACTUAL_START_DATE,aStartDate,0);
			READMYSQL_INT(ACTUAL_END_DATE,aEndtDate,0);

			READMYSQL_STRING(SURVEY_PORTID,surveyPort);
			READMYSQL_STRING(CCS,ccs);
			READMYSQL_STRING(SURVEY_PEOPLE,surveyPeople);

			int nowTime = GmtNowUsec()/1000;
			int days=getRemainDay(aStartDate,nowTime);
			out<<"{id:\""<<id<<"\",days:"<<days<<",status:"<<status<<",exid:\""<<surveyCode<<"\",exname:\""<<surveyName;
			out<<"\",pd_s:"<<pStartDate<<",pd_f:"<<pEndDate<<",ad_s:"<<aStartDate<<",ad_f:"<<aEndtDate;
			out<<",port:\""<<surveyPort<<"\",society:\""<<ccs<<"\",person:\""<<surveyPeople<<"\"}";
		}		
		out<<"]}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

int eMOPSurveySvc::getRemainDay(int startTime,int endTime)
{ 
	int days = int((endTime-startTime)/60/60/24);
	return days;
}




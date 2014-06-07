#include "blmcom_head.h"
#include "eMOPSMSSvc.h"
#include "MessageService.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory.h>

using namespace std;

IMPLEMENT_SERVICE_MAP(eMOPSMSSvc)

eMOPSMSSvc::eMOPSMSSvc()
{
}

eMOPSMSSvc::~eMOPSMSSvc()
{
}

bool eMOPSMSSvc::Start()
{
	if(!g_MessageService::instance()->RegisterCmd(MID_EMOP_SMS,this))
		return false;

	SERVICE_MAP(0x01,eMOPSMSSvc,GetSMSList);
	SERVICE_MAP(0x02,eMOPSMSSvc,GetSMSDetail);
	SERVICE_MAP(0x03,eMOPSMSSvc,NewSMSStudy);
	SERVICE_MAP(0x04,eMOPSMSSvc,GetAddableFileDeparShip);
	SERVICE_MAP(0x05,eMOPSMSSvc,AddDelFileDeparShip);
	SERVICE_MAP(0x06,eMOPSMSSvc,UpdateState);
	SERVICE_MAP(0x07,eMOPSMSSvc,GetAuditList);
	SERVICE_MAP(0x08,eMOPSMSSvc,GetAuditDetail);
	SERVICE_MAP(0x09,eMOPSMSSvc,NewAudit);
	SERVICE_MAP(0x10,eMOPSMSSvc,AddFlaw);
	SERVICE_MAP(0x21,eMOPSMSSvc,GetFile);
	SERVICE_MAP(0x24,eMOPSMSSvc,DeleteFile);
	SERVICE_MAP(0x25,eMOPSMSSvc,GetSuggest);
	SERVICE_MAP(0x28,eMOPSMSSvc,DeleteSuggest);
	SERVICE_MAP(0x29,eMOPSMSSvc,ChangeVersions);
	


    DEBUG_LOG("[eMOPSMSSvc::Start] OK......................................");
	return true;
}


//请求学习列表
//{seq：“”，info：[{id:””,st:1,tp:1,theme:””,dn:””,pub:””,bdate:1,fdate:1}，{}]}
int eMOPSMSSvc::GetSMSList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetSMSList]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	out<< "{seq:\""<<strSeq.c_str()<<"\",info:[";

	string studyId="";
	int state=0;
	int type=0;
	string theme="";
	string content="";
	string publisher="";
	int publishDate=0;
	int dueDate=0;
	int countN=0;

	MySql* psql = CREATE_MYSQL;

	char sql[500] ="SELECT STUDY_ID,STATUS,STUDY_TYPE,STUDY_TITLE,PUBLISHER,UNIX_TIMESTAMP(PUB_DATE) AS PUB_DATE_TEMP, UNIX_TIMESTAMP(DEMAND_FINISH_DATE) AS DEMAND_FINISH_DATE_TEMP FROM blm_emop_etl.T50_EMOP_SMS_STUDY ORDER BY PUB_DATE";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	while(psql->NextRow())
	{
		READMYSQL_STRING(STUDY_ID,studyId);
		READMYSQL_INT(STATUS,state,0);
		READMYSQL_INT(STUDY_TYPE,type,0);
		READMYSQL_STRING(STUDY_TITLE,theme);
		READMYSQL_STRING(PUBLISHER,publisher);
		READMYSQL_INT(PUB_DATE_TEMP,publishDate,0);
		READMYSQL_INT(DEMAND_FINISH_DATE_TEMP,dueDate,0);

		MySql* psql2 = CREATE_MYSQL;
		char sql2[200]="";
		sprintf(sql2,"SELECT NOTIFY_OBJECT_ID FROM blm_emop_etl.T50_EMOP_SMS_STUDY_NOTIFY WHERE STUDY_ID = '%s'",studyId.c_str());
		CHECK_MYSQL_STATUS(psql2->Query(sql2)>=0, 3);  

		char contentId[32]="";
		int contentType=0;
		stringstream contentC;
		int count=0;
		int countShip=0;
		while(psql2->NextRow())
		{
		    READMYSQL_STR1(NOTIFY_OBJECT_ID,contentId,psql2);

			if(type==1)
			{
				if(count>0)
					contentC<<"|";
				count++;
				contentC<<contentId;
			}
			else if(type==2)
				countShip++;
		}
		if(state==0)
			state=2;
		else if(state!=0)
			state=state-1;


		if(type==1)//公司
		{
            if (countN++)
			     out<<",";
			out<<"{id:\""<<studyId.c_str()<<"\",st:"<<state<<",tp:"<<type-1<<",theme:\""<<theme.c_str()<<"\",dn:\""<<contentC.str().c_str()<<"\",pub:\""<<publisher.c_str()<<"\",bdate:"<<publishDate<<",fdate:"<<dueDate<<"}";
		}
		else if(type==2)//船舶
		{
			if (countN++)
			     out<<",";
			out<<"{id:\""<<studyId.c_str()<<"\",st:"<<state<<",tp:"<<type-1<<",theme:\""<<theme.c_str()<<"\",dn:\""<<countShip<<"\",pub:\""<<publisher.c_str()<<"\",bdate:"<<publishDate<<",fdate:"<<dueDate<<"}";
		}
		RELEASE_MYSQL_NORETURN(psql2);
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{seq:””,id:””}
//{seq:””,files:[{id:””,na:””},{}],depar:[{id:””,na:””},{id:””,na:””}],ships:[{id:””,na:””},{}]}
int eMOPSMSSvc::GetSMSDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetSMSDetail]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string studyId=root.getv("id","");

	out<< "{seq:\""<<strSeq.c_str()<<"\",files:[";

	MySql* psql = CREATE_MYSQL;

	string fileId="";
	char fileNa[300]="";
	int idx=0;

	char sql[100]="";
	sprintf(sql,"SELECT FILE_ID FROM blm_emop_etl.T50_EMOP_SMS_STUDY_FILES WHERE STUDY_ID = '%s'",studyId.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	while(psql->NextRow())
	{
		READMYSQL_STRING(FILE_ID,fileId);
		MySql* psql2=CREATE_MYSQL;
		char sql2[100]="";
		sprintf(sql2,"SELECT FILE_NAME FROM blm_emop_etl.T50_EMOP_SMS_FILE WHERE FILE_ID='%s'",fileId.c_str());
		CHECK_MYSQL_STATUS(psql2->Query(sql2)>=0, 3);  
		psql2->NextRow();
		READMYSQL_STR1(FILE_NAME,fileNa,psql2);

		if(idx>0)
		    out<<",";
      	idx++;
		out<<"{id:\""<<fileId.c_str()<<"\",na:\""<<fileNa<<"\"}";
		RELEASE_MYSQL_NORETURN(psql2);
	}

	sprintf(sql,"SELECT STUDY_TYPE FORM blm_emop_etl.T50_EMOP_SMS_STUDY WHERE STUDY_ID = '%s'",studyId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int studyType=0;
	while(psql->NextRow())
		READMYSQL_INT(STUDY_TYPE,studyType,0);

	memset(sql,0,100);
	sprintf(sql,"SELECT NOTIFY_OBJECT_ID FROM blm_emop_etl.T50_EMOP_SMS_STUDY_NOTIFY WHERE STUDY_ID = '%s'",studyId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	string contentId="";
	int countDepart=0;
	int countShip=0;
	stringstream departStream;
	stringstream shipStream;
	char sql2[100]="";
	char contentNa[512]="";

	while(psql->NextRow())
	{
		READMYSQL_STRING(NOTIFY_OBJECT_ID,contentId);
		MySql* psql2=CREATE_MYSQL;
		if(studyType==1)
		{
			sprintf(sql2,"SELECT NAME_CN FROM blm_emop_etl.T50_EMOP_DEPARTMENT_CODE WHERE DEPARTMENT_CODE = '%s'",contentId.c_str());
			CHECK_MYSQL_STATUS(psql2->Query(sql2)>=0, 3);  
			while(psql2->NextRow())
			{
				READMYSQL_STR1(NAME_CN,contentNa,psql2);
				if(countDepart>0)
					departStream<<",";
				countDepart++;
				departStream<<"{id:\""<<contentId.c_str()<<"\",na:\""<<contentNa<<"\"}";
			}
		}
		else if(studyType==2)
		{
			sprintf(sql2,"SELECT NAME_CN FROM blm_emop_etl.T41_EMOP_SHIP WHERE SHIPID = '%s'",contentId.c_str());
			CHECK_MYSQL_STATUS(psql2->Query(sql2)>=0, 3);  

			while(psql2->NextRow())
			{
				READMYSQL_STR1(NAME_CN,contentNa,psql2);
				if(countShip>0)
					shipStream<<",";
				countShip++;
				shipStream<<"{id:\""<<contentId.c_str()<<"\",na:\""<<contentNa<<"\"}";
			}
		}
		RELEASE_MYSQL_NORETURN(psql2);
	}
	out<<"],depar:["<<departStream.str().c_str()<<"],ships:["<<shipStream.str().c_str()<<"]}";

RELEASE_MYSQL_RETURN(psql,0);
}


//{seq:””, uid:””,tp:1,theme:””,fdate:1}
//{seq:””,eid:0,id:””}

int eMOPSMSSvc::NewSMSStudy(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::NewSMSStudy]bad format:", jsonString, 1);
	string strSeq= root.getv("seq","");
	string uid=root.getv("uid","");
	int tp=root.getv("tp",0);
	string theme=root.getv("theme","");
	int fdate=root.getv("fdate",0);
	string studyId=_GetNewObj("S","");
	string userNa="";

	MySql* psql = CREATE_MYSQL;
	char sql[256]="";
	sprintf(sql,"SELECT NAME FROM blm_emop_etl.T50_EMOP_USERS WHERE USER_ID='%s'",uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	while(psql->NextRow())
		READMYSQL_STRING(NAME,userNa);

	memset(sql,0,256);
	sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SMS_STUDY(STUDY_ID,STUDY_TITLE,STUDY_TYPE,PUBLISHER,DEMAND_FINISH_DATE,STATUS)\
				VALUES('%s','%s','%d','%s','%s','%d')",studyId.c_str(),theme.c_str(),tp,userNa.c_str(),TimeToTimestampStr(fdate).c_str(),0);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,"<<"id:\""<<studyId.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
//{seq:””,id:””,tp:”” }
//{seq:””,info:[{id:””,na:””}]}
int eMOPSMSSvc::GetAddableFileDeparShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetAddableFileDeparShip]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", ""); 
	string uid=root.getv("uid","");
	int type=root.getv("tp",0);

	out<< "{seq:\""<<strSeq.c_str()<<"\",info:[";

	MySql* psql = CREATE_MYSQL;
	char sql[256]="SELECT FILE_ID,FILE_NAME FROM blm_emop_etl.T50_EMOP_SMS_FILE";
	string companyId="";
	string deparId="";
	string deparNa="";
	int count=0;

	if(type==0)
	{
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		while(psql->NextRow())
		{
			if(count++)
				out<<",";
			READMYSQL_STRING(FILE_ID,deparId);
			READMYSQL_STRING(FILE_NAME,deparNa);
			out<<"{id:\""<<deparId.c_str()<<"\",na:\""<<deparNa.c_str()<<"\"}";
		}
	}
	else if(type==1)//公司
	{
		
		sprintf(sql,"SELECT COMPANY_KEY FROM blm_emop_etl.T50_EMOP_USERS WHERE USER_ID = '%s'",uid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		while(psql->NextRow())
			READMYSQL_STRING(COMPANY_KEY,companyId);
		memset(sql,0,256);
		sprintf(sql,"SELECT NAME_CN,DEPARTMENT_CODE FROM blm_emop_etl.T50_EMOP_DEPARTMENT_CODE WHERE REGISTED_OBJECT_ID = '%s'",companyId.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		while(psql->NextRow())
		{
			if(count>0)
				out<<",";
			count++;

			READMYSQL_STRING(DEPARTMENT_CODE,deparId);
			READMYSQL_STRING(NAME_CN,deparNa);
			out<<"{id:\""<<deparId.c_str()<<"\",na:\""<<deparNa.c_str()<<"\"}";
		}
	}
	else if(type==2)//船舶
	{
		sprintf(sql,"SELECT SHIPID FROM blm_emop_etl.T50_EMOP_REGISTERED_USER_FLEET WHERE USER_ID = '%s'",uid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		while(psql->NextRow())
		{
			READMYSQL_STRING(COMPANY_KEY,deparId);

			if(count>0)
				out<<",";
			count++;

			MySql* psql2=CREATE_MYSQL;
			char sql2[100]="";
			sprintf(sql2,"SELECT NAME_CN FROM blm_emop_etl.T41_EMOP_SHIP WHERE SHIPID = '%s'",deparId.c_str());
			CHECK_MYSQL_STATUS(psql2->Query(sql2)>=0, 3);  
			while(psql2->NextRow())
				READMYSQL_STRING(NAME_CN,deparNa);
			out<<"{id:\""<<deparId.c_str()<<"\",na:\""<<deparNa.c_str()<<"\"}";
			RELEASE_MYSQL_NORETURN(psql2);
		}
	}

	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);


}
//{seq:””,id:””,dis:1,tp:1,del:”id|id|id”}
//{seq:””,tp：1，eid:0}
int eMOPSMSSvc::AddDelFileDeparShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::AddDelFileDeparShip]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	string studyId=root.getv("id","");
	int dis=root.getv("dis",0);
	int type=root.getv("tp",0);
	string detail=root.getv("del","");


	MySql* psql = CREATE_MYSQL;

	char sql[256]="";

	if(type==0)//删除
	{
		if(dis==0)//文件
		{
			if(!detail.empty())
			{
				Tokens conVec = StrSplit(detail,"|");
				for(Tokens::iterator it=conVec.begin();it!=conVec.end();it++)
				{
					memset(sql,0,256);
					sprintf(sql,"DELETE  FROM blm_emop_etl.T50_EMOP_SMS_STUDY_FILES WHERE STUDY_ID='%s' AND FILE_ID='%s'",studyId.c_str(),it->c_str());
					CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
				}
			}
		}
		else if(dis==1)//部门或者船舶
		{
			if(!detail.empty())
			{
				Tokens conVec = StrSplit(detail,"|");
				for(Tokens::iterator it=conVec.begin();it!=conVec.end();it++)
				{
					memset(sql,0,256);
					sprintf(sql,"DELETE  FROM blm_emop_etl.T50_EMOP_SMS_STUDY_NOTIFY WHERE STUDY_ID='%s' AND NOTIFY_OBJECT_ID='%s'",studyId.c_str(),it->c_str());
					CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
				}
			}
		}
	}
	else if(type==1)//增加
	{
		if(dis==0)
		{
			if(!detail.empty())
			{
				Tokens conVec = StrSplit(detail,"|");
				for(Tokens::iterator it=conVec.begin();it!=conVec.end();it++)
				{
					memset(sql,0,256);
					sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SMS_STUDY_FILES(STUDY_ID,FILE_ID) VALUES('%s','%s')",studyId.c_str(),it->c_str());
					CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
				}
			}
		}
		else if(dis==1)
		{
			if(!detail.empty())
			{
				Tokens conVec = StrSplit(detail,"|");
				for(Tokens::iterator it=conVec.begin();it!=conVec.end();it++)
				{
					memset(sql,0,256);
					sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SMS_STUDY_NOTIFY(STUDY_ID,NOTIFY_OBJECT_ID) VALUES('%s','%s')",studyId.c_str(),it->c_str());
					CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
				}
			}
		}
	}

	out<<"{seq:\""<<strSeq.c_str()<<"\",tp:"<<type<<",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);

}

//{seq:””,id:””}
//{seq:””,eid:0}
int eMOPSMSSvc::UpdateState(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::UpdateState]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	string studyId=root.getv("id","");
	string strFinishDay=CurLocalTime();

	MySql* psql = CREATE_MYSQL;
	char sql[256]="";
	int status=0;

	sprintf(sql,"SELECT STATUS FROM blm_emop_etl.T50_EMOP_SMS_STUDY WHERE STUDY_ID ='%s'",studyId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	while(psql->NextRow())
		READMYSQL_INT(STATUS,status,0);

	memset(sql,0,256);
	sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SMS_STUDY SET STATUS='%d' WHERE STUDY_ID ='%s'",status+1,studyId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	if(status==1)
	{
		memset(sql,0,256);
		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SMS_STUDY SET FINISH_DATE='%s' WHERE STUDY_ID ='%s'",strFinishDay.c_str(),studyId.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		memset(sql,0,256);
		sprintf(sql,"SELECT NOTIFY_OBJECT_ID FROM blm_emop_etl.T50_EMOP_SMS_STUDY_NOTIFY WHERE STUDY_ID ='%s'",studyId.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		string noticeId="";
		while(psql->NextRow())
		{
			READMYSQL_STRING(NOTIFY_OBJECT_ID,noticeId);
			MySql* psql2 = CREATE_MYSQL;
			memset(sql,0,256);
			sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SMS_STUDY SET STATUS='%d',FINISH_DATE='%s' WHERE STUDY_ID ='%s' AND NOTIFY_OBJECT_ID='%s'",1,strFinishDay.c_str(),studyId.c_str(),noticeId.c_str());
			CHECK_MYSQL_STATUS(psql2->Execute(sql)>=0, 3);
			RELEASE_MYSQL_NORETURN(psql2);
		}
	}

	out<< "{seq:\""<<strSeq.c_str()<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);


}
//{seq:””,info:[{id:””,st:1,tp:1,shipid:””,shipn:””,cdate:,cport:””,cman:””,edate:1,re:1,bdate:,fdate:,bport:””,fport:””,sdate:,enddate},{}]}
int eMOPSMSSvc::GetAuditList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetAuditList]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	out<< "{seq:\""<<strSeq.c_str()<<"\",info:[";

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="SELECT CHECK_ID,SHIP_ID,TYPE,STATUS,UNIX_TIMESTAMP(CHECK_DATE) AS CHECK_DATE_TEMP,CHECK_PORT,\
				  CHECKER,UNIX_TIMESTAMP(RECTIFY_DATE) AS RECTIFY_DATE_TEMP,MEMO,UNIX_TIMESTAMP(SAIL_DATE) AS SAIL_DATE_TEMP,\
				  SAIL_LOC,UNIX_TIMESTAMP(ARRIVAL_DATE) AS  ARRIVAL_DATE_TEMP,ARRIVAL_LOC,UNIX_TIMESTAMP(RETENTION_START_DATE) AS RETENTION_START_DATE_TEMP,\
				  UNIX_TIMESTAMP(RETENTION_END_DATE) AS RETENTION_END_DATE_TEMP FROM blm_emop_etl.T50_EMOP_SMS_CHECK";
	string id="";
	int  state=0;
	int type=0;
	string shipId="";
	int checkDate=0;
	string checkPort="";
	string checker="";
	int reformDate=0;
	int memo=0;
	int beforeDate=0;
	string beforePort="";
	int afterDate=0;
	string afterPort="";
	int delayStartDate=0;
	int delayEndDate=0;
	int count=0;
	char shipNa[512]="";

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
	while(psql->NextRow())
	{
		READMYSQL_STRING(CHECK_ID,id);
		READMYSQL_INT(STATUS,state,0);
		READMYSQL_INT(TYPE,type,0);
		READMYSQL_STRING(SHIP_ID,shipId);
		READMYSQL_INT(CHECK_DATE_TEMP,checkDate,0);
		READMYSQL_STRING(CHECK_PORT,checkPort);
		READMYSQL_STRING(CHECKER,checker);
		READMYSQL_INT(RECTIFY_DATE_TEMP,reformDate,0);
		READMYSQL_INT(MEMO,memo,0);
		READMYSQL_INT(SAIL_DATE_TEMP,beforeDate,0);
		READMYSQL_STRING(SAIL_LOC,beforePort);
		READMYSQL_INT(ARRIVAL_DATE_TEMP,afterDate,0);
		READMYSQL_STRING(ARRIVAL_LOC,afterPort);
		READMYSQL_INT(RETENTION_START_DATE_TEMP,delayStartDate,0);
		READMYSQL_INT(RETENTION_END_DATE_TEMP,delayEndDate,0);

		MySql* psql2 = CREATE_MYSQL;
		memset(sql,0,256);
		sprintf(sql,"SELECT NAME_CN FROM blm_emop_etl.T41_EMOP_SHIP WHERE SHIPID='%s'",shipId.c_str());
		CHECK_MYSQL_STATUS(psql2->Query(sql)>=0, 3); 
		while(psql2->NextRow())
			READMYSQL_STR1(NAME_CN,shipNa,psql2);
		
		if(count>0)
			out<<",";
		count++;
		out<<"{id:\""<<id.c_str()<<"\",st:"<<state<<",tp:"<<type<<",shipid:\""<<shipId.c_str()<<"\",shipn:\""<<shipNa<<"\",cdate:"<<checkDate<<",cport:\""<<checkPort.c_str()<<"\",cman:\""<<checker.c_str()<<"\",edate:"<<reformDate
			<<",re:"<<memo<<",bdate:"<<beforeDate<<",fdate:"<<afterDate<<",bport:\""<<beforePort.c_str()<<"\",fport:\""<<afterPort.c_str()<<"\",sdate:"<<delayStartDate<<",enddate:"<<delayEndDate<<"}";

		RELEASE_MYSQL_NORETURN(psql2);
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);


}
//{seq：““，info：[{id:””,code:"",cat:””,con:””,man:””,duty:””,pf:rf:},{}]}

int eMOPSMSSvc::GetAuditDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetAuditDetail]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	string id=root.getv("id","");
	out<< "{seq:\""<<strSeq.c_str()<<"\",info:[";

	MySql* psql = CREATE_MYSQL;
	char sql[500]="";
	sprintf(sql,"SELECT DEFECT_ID, DEFECT_CODE,DEFECT_CAT,DEFECT_CONTENT,PRINCIPAL,DUTY,UNIX_TIMESTAMP(SCHEDULED_DATE) AS SCHEDULED_DATE_TEMP,\
				UNIX_TIMESTAMP(ACTUAL_DATE) AS ACTUAL_DATE_TEMP FROM blm_emop_etl.T50_EMOP_SMS_CHECK_DEFECT WHERE CHECK_ID='%s'",id.c_str());
	string flawId="";
    string code="";
	string cat="";
	string con="";
	string man="";
	string duty="";
	int pf=0;
	int rf=0;
	int count=0;

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
	while(psql->NextRow())
	{
		READMYSQL_STRING(DEFECT_ID,flawId);
		READMYSQL_STRING(DEFECT_CODE,code);
		READMYSQL_STRING(DEFECT_CAT,cat);
		READMYSQL_STRING(DEFECT_CONTENT,con);
		READMYSQL_STRING(PRINCIPAL,man);
		READMYSQL_STRING(DUTY,duty);
		READMYSQL_INT(SCHEDULED_DATE_TEMP,pf,0);
		READMYSQL_INT(ACTUAL_DATE_TEMP,rf,0);
		//{seq：““，info：[{id:””,code:"",cat:””,con:””,man:””,duty:””,pf:rf:},{}]}
		if(count>0)
			out<<",";
		count++;
		out<<"{id:\""<<flawId.c_str()<<"\",code:\""<<code.c_str()<<"\",cat:\""<<cat.c_str()<<"\",con:\""<<con.c_str()<<"\",man:\""<<man.c_str()<<"\",duty:\""<<duty.c_str()<<"\",pf:"<<pf<<",rf:"<<rf<<"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);


}
//{seq:””,st:1,tp:1,shipid:"",shipn:””,cdate:,cport:””,cman:””,edate:1,re:1,bdate:,fdate:,bport:””,fport:””,sdate:,enddate :}
//{seq:””,eid:0,id:””}
int eMOPSMSSvc::NewAudit(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::NewAudit]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	int st=root.getv("st",0);
	int tp=root.getv("tp",0);
	string shipid=root.getv("shipid","");
	int cdate=root.getv("cdate",0);
	string cport=root.getv("cport","");
	string cman=root.getv("cman","");
	int edate=root.getv("edate",0);
	int re=root.getv("re",0);
	int bdate=root.getv("bdate",0);
	int fdate=root.getv("fdate",0);
	string bport=root.getv("bport","");
	string fport=root.getv("fport","");
	int sdate=root.getv("sdate",0);
	int enddate=root.getv("enddate",0);
	float retentionTime=(enddate-sdate)/1000/60/60/24;
	string checkId=_GetNewObj("C","");

	MySql* psql = CREATE_MYSQL;
	char sql[500]="";
	sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SMS_CHECK(CHECK_ID,SHIP_ID,TYPE,STATUS,CHECK_DATE,CHECK_PORT,\
				CHECKER,RECTIFY_DATE,MEMO,SAIL_DATE,SAIL_LOC,ARRIVAL_DATE,ARRIVAL_LOC,RETENTION_START_DATE,RETENTION_END_DATE,RETENTION_DAYS)\
				VALUES('%s','%s','%d','%d','%s','%s','%s','%s','%d','%s','%s','%s','%s','%s','%s','%f')",checkId.c_str(),shipid.c_str(),tp,st,TimeToTimestampStr(cdate).c_str(),
				cport.c_str(),cman.c_str(),TimeToTimestampStr(edate).c_str(),re,TimeToTimestampStr(bdate).c_str(),bport.c_str(),TimeToTimestampStr(fdate).c_str(),fport.c_str(),
				TimeToTimestampStr(sdate).c_str(),TimeToTimestampStr(enddate).c_str(),retentionTime);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,id:\""<<checkId.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);


}
//{seq:””,id:””,code:"", cat:””,con:””,man:””,duty:””,pf:}
//{seq：“”，eid：0}
int eMOPSMSSvc::AddFlaw(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::AddFlaw]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	string checkId=root.getv("id","");
	string flawCode=root.getv("code","");
	string cat=root.getv("cat","");
	string con=root.getv("con","");
	string man=root.getv("man","");
	string duty=root.getv("duty","");
	int pf=root.getv("pf",0);
	string flawId=_GetNewObj("F","");

	MySql* psql = CREATE_MYSQL;
	char sql[512]="";
	sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SMS_CHECK_DEFECT(DEFECT_ID,CHECK_ID,DEFECT_CODE,DEFECT_CAT,DEFECT_CONTENT,PRINCIPAL,DUTY,SCHEDULED_DATE,COLUMN_10)\
				VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%d')",
		flawId.c_str(),checkId.c_str(),flawCode.c_str(),cat.c_str(),con.c_str(),man.c_str(),duty.c_str(),TimeToTimestampStr(pf).c_str(),0);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,"<<"fid:\""<<flawId.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}

//SMS文件*********************************************************************
//获取文件列表  {seq:””,files:[{state:0,type:1,did:”D0953”,dna:””,num:”09244”,fid:””,fna:””,furl:””,eid:””,ena:””,eurl:””,vsn:”2.0”,comm:””,opp:””,opd:578434, sdate :455445, edate:5675765},{},…]}
int eMOPSMSSvc::GetFile(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[SARSvc::GetAuditDetail]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	out<< "{seq:\""<<strSeq.c_str()<<"\",files:[";

	MySql* psql = CREATE_MYSQL;
	char sql[500]="SELECT FILE_ID,FILE_NAME,OBJECT_TYPE,OBJECT_ID,FILE_NO,FILE_TYPE,FILE_TABLE_NAME,FILE_VERSION,REMARK,STATUS,OPERATOR,UNIX_TIMESTAMP(OP_DATE) AS OP_DATE_TEMP,UNIX_TIMESTAMP(VALID_START_DT) AS VALID_START_DT_TEMP,UNIX_TIMESTAMP(VALID_END_DT) AS VALID_END_DT_TEMP FROM blm_emop_etl.T50_EMOP_SMS_FILE";
	int state=-1;
	int type=-1;
	int objType=0;
	string did="";
	char dna[128]="";
	string num="";
	string fid="";
	string fna="";
	string ena="";
	string vsn="";
	string comm="";
	string opp="";
	int opd=0;
	int sdate=0;
	int edate=0;
    CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
	int count=0;
	while(psql->NextRow())
	{
		READMYSQL_INT(STATUS,state,-1);
		if(state<=2&&state!=-1)
			state++;
		else if(state>2)
			state=state-3;
        READMYSQL_INT(FILE_TYPE,type,-1);
		if(type)
		READMYSQL_INT(OBJECT_TYPE,objType,0);
		if(objType==3)
		{
              READMYSQL_STRING(OBJECT_ID,did);
			  MySql* psql2 = CREATE_MYSQL;
			  char sql2[200]="";
			  sprintf(sql2,"SELECT NAME_CN FROM blm_emop_etl.T50_EMOP_DEPARTMENT_CODE WHERE DEPARTMENT_CODE = '%s'",did.c_str());
			  CHECK_MYSQL_STATUS(psql2->Query(sql2)>=0, 3);  
			  while(psql2->NextRow())
				  READMYSQL_STR1(NAME_CN,dna,psql2);
			  RELEASE_MYSQL_NORETURN(psql2);
		}
		READMYSQL_STRING(FILE_NO,num);
		READMYSQL_STRING(FILE_ID,fid);
		READMYSQL_STRING(FILE_NAME,fna);
		READMYSQL_STRING(FILE_TABLE_NAME,ena);
		READMYSQL_STRING(FILE_VERSION,vsn);
		READMYSQL_STRING(REMARK,comm);
		READMYSQL_STRING(OPERATOR,opp);
		READMYSQL_INT(OP_DATE_TEMP,opd,0);
		READMYSQL_INT(VALID_START_DT_TEMP,sdate,0);
		READMYSQL_INT(VALID_END_DT_TEMP,edate,0);
		if(count++>0)
			out<<",";
		out<<"{state:"<<state<<",type:"<<--type<<",did:\""<<did.c_str()<<"\",dna:\""<<dna<<"\",num:\""<<num.c_str()<<"\",fid:\""<<fid.c_str()<<"\",fna:\""<<fna.c_str()
			<<"\",ena:\""<<ena.c_str()<<"\",vsn:\""<<vsn.c_str()<<"\",comm:\""<<comm.c_str()<<"\",opp:\""<<opp.c_str()<<"\",opd:"<<opd<<",sdate:"<<sdate<<",edate:"<<edate<<"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//删除文件
int eMOPSMSSvc::DeleteFile(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[SARSvc::GetAuditDetail]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	string fid=root.getv("fid","");
	MySql* psql = CREATE_MYSQL;
	char sql[512]="";
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SMS_FILE WHERE FILE_ID ='%s'",fid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	memset(sql,0,512);
	sprintf(sql,"SELECT * FROM blm_emop_etl.T50_EMOP_SMS_STUDY_FILES WHERE FILE_ID='s'",fid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
	while(psql->NextRow())
	{
		memset(sql,0,512);
		sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_SMS_STUDY_FILES WHERE FILE_ID ='%s'",fid.c_str());
		 MySql* psql2 = CREATE_MYSQL;
		 CHECK_MYSQL_STATUS(psql2->Execute(sql)>=0, 3);
		 RELEASE_MYSQL_NORETURN(psql2);
	}
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:"<<0<<"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//获取文件所有建议
//{seq:””,sugg:[{id:””,state:2,did:””,dna:””,sp:””,time:45545,vsn:”2.0”,reason:””, content:””},{},…] }
int eMOPSMSSvc::GetSuggest(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetAuditDetail]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	string fid=root.getv("fid","");
	out<< "{seq:\""<<strSeq.c_str()<<"\",sugg:[";

	MySql* psql = CREATE_MYSQL;
	char sql[512]="";
	sprintf(sql,"SELECT SUG_ID,FILE_ID,SUG_PERSON,MODIFY_RESON,MODIFY_CONTENT,UNIX_TIMESTAMP(SUG_DATE) AS SUG_DATE_TEMP FROM blm_emop_etl.T50_EMOP_FILE_SUGGEST WHERE FILE_ID = '%s'",fid.c_str());
//{id:””,state:2,did:””,dna:””,sp:””,time:45545,vsn:”2.0”,reason:””, content:””}
	string id="";
	string fileId="";
	int state=-1;
	char did[20]="";
	char dna[128]="";
	string sp="";
	int time=0;
	char vsn[16]="";
	string reason="";
	string content="";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
	int count=0;
	while(psql->NextRow())
	{
		READMYSQL_STRING(SUG_ID,id);
		READMYSQL_STRING(FILE_ID,fileId);
		READMYSQL_INT(STATUS,state,0);
		READMYSQL_STRING	(SUG_PERSON,sp);
		READMYSQL_INT(SUG_DATE_TEMP,time,0);
		READMYSQL_STRING(MODIFY_RESON,reason);
		READMYSQL_STRING(MODIFY_CONTENT,content);
		MySql* psql2 = CREATE_MYSQL;
		char sql2[256]="";
		sprintf(sql2,"SELECT DEPARTMENT_CODE FROM blm_emop_etl.T50_EMOP_USERS WHERE USER_ID='%s'",sp.c_str());
		 CHECK_MYSQL_STATUS(psql2->Query(sql2)>=0, 3);  
		 while(psql2->NextRow())
			 READMYSQL_STR1(DEPARTMENT_CODE,did,psql2);
		 memset(sql2,0,256);
		 sprintf(sql2,"SELECT NAME_CN FROM blm_emop_etl.T50_EMOP_DEPARTMENT_CODE WHERE DEPARTMENT_CODE='%s'",did);
		 CHECK_MYSQL_STATUS(psql2->Query(sql2)>=0, 3);  
		 while(psql2->NextRow())
			 READMYSQL_STR1(NAME_CN,dna,psql2);
		 memset(sql2,0,256);
		 sprintf(sql2,"SELECT FILE_VERSION FROM blm_emop_etl.T50_EMOP_SMS_FILE WHERE FILE_ID = '%s'",fileId.c_str());
		 CHECK_MYSQL_STATUS(psql2->Query(sql2)>=0, 3);  
		 while(psql2->NextRow())
			 READMYSQL_STR1(FILE_VERSION,vsn,psql2);
		 RELEASE_MYSQL_NORETURN(psql2);
		 //{id:””,state:2,did:””,dna:””,sp:””,time:45545,vsn:”2.0”,reason:””, content:””}
		 if(count++>0)
			 out<<",";
		 out<<"{id:\""<<id.c_str()<<"\",state:"<<state<<",did:\""<<did<<"\",dna:\""<<dna<<"\",sp:\""<<sp.c_str()<<"\",time:"<<time<<",vsn:\""<<vsn<<"\",reason:\""<<reason.c_str()<<"\",content:\""<<content.c_str()<<"\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//删除建议
int eMOPSMSSvc::DeleteSuggest(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[SARSvc::GetAuditDetail]bad format:", jsonString, 1);
	string strSeq=root.getv("seq","");
	string id=root.getv("id","");
	char sql[256]="";
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_FILE_SUGGEST WHERE SUG_ID='%s'",id.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:"<<0<<"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//文件或表格版本改变
int eMOPSMSSvc::ChangeVersions(const char* pUid, const char* jsonString, std::stringstream& out)
{
return 0;
}


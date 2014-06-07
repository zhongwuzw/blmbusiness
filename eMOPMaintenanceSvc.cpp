#include "blmcom_head.h"
#include "eMOPMaintenanceSvc.h"
#include "UserRegSvc.h"
#include "MessageService.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include "NotifyService.h"
#include "SequenceManager.h"
#include "eMopCommDataSvr.h"
#include "eMOPMaintenanceUtil.h"
#include "eMOPEquipSvc.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h> 


using namespace std;


IMPLEMENT_SERVICE_MAP(eMOPMaintenanceSvc)

eMOPMaintenanceSvc::eMOPMaintenanceSvc()
{

}

eMOPMaintenanceSvc::~eMOPMaintenanceSvc()
{
	CleanMTSteps();
}

bool eMOPMaintenanceSvc::Start()
{
	if(!g_MessageService::instance()->RegisterCmd(MID_EMOP_MAINTENANCES, this))
		return false;

	int interval = 60;
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));

	if(timerId <= 0)
		return false;

	SERVICE_MAP(0x01,eMOPMaintenanceSvc,GetEquipCardList);
	SERVICE_MAP(0x02,eMOPMaintenanceSvc,GetEquipCardDetail);
	SERVICE_MAP(0x03,eMOPMaintenanceSvc,NewEquipCardItem);
	SERVICE_MAP(0x04,eMOPMaintenanceSvc,ModifyEquipCardItem);
	SERVICE_MAP(0x05,eMOPMaintenanceSvc,DeleteEquipCardItem);


	SERVICE_MAP(0x06,eMOPMaintenanceSvc,GetWorkCardList);
	SERVICE_MAP(0x07,eMOPMaintenanceSvc,GetWorkCardDetail);
	SERVICE_MAP(0x08,eMOPMaintenanceSvc,NewWorkCardItem);
	SERVICE_MAP(0x09,eMOPMaintenanceSvc,ModifyWorkCardItem);
	SERVICE_MAP(0x0a,eMOPMaintenanceSvc,DeleteWorkCardItem);

	SERVICE_MAP(0x10,eMOPMaintenanceSvc,GetWorkPlanList);
	SERVICE_MAP(0x11,eMOPMaintenanceSvc,FirstTimeAssignPlan);
	SERVICE_MAP(0x12,eMOPMaintenanceSvc,ChangePlanTime);
	SERVICE_MAP(0x13,eMOPMaintenanceSvc,GetWorkPlanDetail);

	SERVICE_MAP(0x14,eMOPMaintenanceSvc,GetMaintenanceWorkFlow);
	SERVICE_MAP(0x15,eMOPMaintenanceSvc,ShipFillPlanReport);
	SERVICE_MAP(0x16,eMOPMaintenanceSvc,ConformPlanReport);
	SERVICE_MAP(0x17,eMOPMaintenanceSvc,GetPlanWorkFlow);


	SERVICE_MAP(0x20,eMOPMaintenanceSvc,RoutineMaintenanceList);
	SERVICE_MAP(0x21,eMOPMaintenanceSvc,SetRoutineMaintenanceVal);
	SERVICE_MAP(0x22,eMOPMaintenanceSvc,GetRoutineMaintenanceTemplate);
	SERVICE_MAP(0x23,eMOPMaintenanceSvc,NewRoutineMaintenanceItem);
	SERVICE_MAP(0x24,eMOPMaintenanceSvc,DelRoutineMaintenanceItem);
	SERVICE_MAP(0x25,eMOPMaintenanceSvc,SignRoutineMaintenanceItem);


	SERVICE_MAP(0x26,eMOPMaintenanceSvc,GetEquipRunRecord_Shore);
	SERVICE_MAP(0x27,eMOPMaintenanceSvc,GetMainEquipTimeRecord);
	SERVICE_MAP(0x28,eMOPMaintenanceSvc,GetSubEquipMaintenancePlan);
	SERVICE_MAP(0x29,eMOPMaintenanceSvc,GetSubEquipRunTime);
	SERVICE_MAP(0x2a,eMOPMaintenanceSvc,GetEquipRunRecord_Ship);
	SERVICE_MAP(0x2b,eMOPMaintenanceSvc,SetEquipRunRecord_Ship);
	SERVICE_MAP(0x2c,eMOPMaintenanceSvc,GetWorkPlanChangeHistory);
	SERVICE_MAP(0x2d,eMOPMaintenanceSvc,UpdateRoutineMaintenanceWorkCard);
	//
	SERVICE_MAP(0x30,eMOPMaintenanceSvc,GetMainSysCode);
	SERVICE_MAP(0x31,eMOPMaintenanceSvc,GetCWBT);
	SERVICE_MAP(0x32,eMOPMaintenanceSvc,GetWCardplan);
	SERVICE_MAP(0x33,eMOPMaintenanceSvc,GetEquipChangeHis);

	//	LoadDevicecard();
	LoadMTSteps();

	DEBUG_LOG("[eMOPMaintenanceSvc::Start] OK......................................");
	return true;
}
int eMOPMaintenanceSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{

	return 0;
}


//修改常规保养工作卡 0x2d
int eMOPMaintenanceSvc::UpdateRoutineMaintenanceWorkCard(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::UpdateRoutineMaintenanceWorkCard]bad format:", jsonString, 1);
 
	string strSeq = root.getv("seq", "");
	string strid=root.getv("id", ""); 
	string strShipId=root.getv("shipid", ""); 
	string strdepart=root.getv("depart", "");
	string strgrade=root.getv("grade", "");

	char timestr[20]="";
	time_t lt= time(NULL);
	struct tm *ptr = localtime(&lt);
	strftime(timestr,30,"%Y-%m-%d %H:%M:%S",ptr);

	MySql* psql = CREATE_MYSQL;
 
	char sql[1024]="";
	sprintf(sql,"UPDATE blm_emop_etl.T50_ABC_CBWT_SHIP_EQUIP_CARD SET SHIPID='%s',DEPARTMENT_CODE='%s',LEVEL='%s' WHERE WORK_CARD_ID='JC0001001A000001'",
		strShipId.c_str(),strdepart.c_str(),strgrade.c_str(),strid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql), 3); 

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}


// 根据船舶和部门请求设备卡列表信息 0x01
int eMOPMaintenanceSvc::GetEquipCardList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetEquipCardList]bad format:", jsonString, 1);


	string strSeq = root.getv("seq", "");
	string strShipid=root.getv("shipid", "");
	string strDepartid=root.getv("departid", "");

	if (strShipid.empty())
	{
		return 0;
	}
	char szTmp[256]="";memset(szTmp,0,sizeof(szTmp));
	if(!strDepartid.empty())
		sprintf(szTmp," and t1.DEPARTMENT_CODE='%s'",strDepartid.c_str());

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf (sql, "select t1.DEPARTMENT_CODE as dptid,t3.NAME_CN as dptname,t1.EQUIP_ID,t1.MAIN_CWBT,t1.cwbt,t1.pms_check,t1.name_cn,t1.status,t1.main_flag,t1.sc_code,UNIX_TIMESTAMP(t2.start_date) AS start_date,t2.running_hrs \
				  from blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD t1 LEFT JOIN \
				  blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL t2 ON t1.EQUIP_ID=t2.EQUIP_ID left join \
				  blm_emop_etl.t50_emop_department_code T3 on t1.DEPARTMENT_CODE=t3.DEPARTMENT_CODE \
				  where t2.VALID_FLAG=1 and t1.SHIPID='%s' %s",strShipid.c_str(),szTmp);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	int cnt = 0;

	char id[80] = "";
	char name_cn[256]="";
	char cwbt[100] = "";
	char maincwbt[100]="";//主计时设备的CWBT码
	char sur[100]="";//船检编码

	int startdate=0;//启用日期
	int runtime=0;//运行时长
	int status=1; //状态1正常 2更换 3停用
	char dptid[32];
	char dptname[32];

	while (psql->NextRow())
	{
		READMYSQL_STR(EQUIP_ID, id)
			READMYSQL_STR(name_cn, name_cn)
			READMYSQL_STR(cwbt, cwbt)
			READMYSQL_STR(MAIN_CWBT, maincwbt)
			READMYSQL_INT(status, status,1)
			READMYSQL_INT(start_date, startdate,0)
			READMYSQL_INT(running_hrs,runtime,0)
			READMYSQL_STR(sc_code,sur)
			READMYSQL_STR(dptid,dptid)
			READMYSQL_STR(dptname,dptname)

			if (cnt++)
				out << ",";
		out << "{id:\"" << id << "\",cwbt:\"" << cwbt << "\",sur:\"" << sur <<"\",cn:\"" << name_cn << "\",sdate:" << startdate <<",run:\"" << runtime;
		out	<<"\",gcwbt:\"" << maincwbt<<"\",st:" << status<< ",dtId:\"" << dptid << "\",dtname:\"" << dptname<< "\"}" ;//:””,:””
	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
// 请求设备卡详细信息 0x02
int eMOPMaintenanceSvc::GetEquipCardDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetEquipCardDetail]bad format:", jsonString, 1);


	string strSeq = root.getv("seq", "");
	string id=root.getv("id", "");

	if (id.empty())
	{
		out<<"{seq:\""<<strSeq<<"\"}";
		return 0;
	}

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";


	sprintf (sql, "SELECT t4.PART_CAT_ID,t1.EQUIP_ID,t1.MAIN_CWBT,t1.cwbt,t1.pms_check,t1.name_cn,t1.name_en,t1.status,t1.main_flag,t1.sc_code,t1.Timing_Flag,UNIX_TIMESTAMP(t2.start_date) AS start_date,t2.running_hrs,t3.MODE AS equipmode,t3.COMPANY_NAME,t2.EQUIP_CODE  \
				  FROM blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD t1 \
				  LEFT JOIN blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL t2 ON t1.EQUIP_ID=t2.EQUIP_ID \
				  LEFT JOIN blm_emop_etl.T50_EMOP_EQUIP_MODEL t3 ON t3.EQUIP_MODEL_ID=t2.EQUIP_MODEL_ID LEFT JOIN blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t4 ON t3.EQUIP_MODEL_ID=t4.EQUIP_MODEL_ID\
				  WHERE  t2.VALID_FLAG=1 and t1.EQUIP_ID='%s'",id.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	char name_cn[256]="";
	char name_en[256]="";
	char cwbt[100] = "";
	char maincwbt[100]="";//主计时设备的CWBT码
	char sur[100]="";//船检编码
	char startdate[20]="";//启用日期
	int runtime=0;//运行时长
	int status=1; //状态1正常 2更换 3停用

	char eno[128]=""; //设备编号
	char etype[256]="";//设备型号（设备ID）
	char emade[512]="";//制造商
	string techdes="";//技术参数描述
	char fno[128]="";//出厂编号
	char paper[128]="";//图纸编号
	char eid[128]="";//图纸编号
	int host=0;
	//char host[10]="";//定时主设备标记


	if (psql->NextRow())
	{

		READMYSQL_STR(name_cn, name_cn)
			READMYSQL_STR(name_en, name_en)
			READMYSQL_STR(cwbt, cwbt)

			READMYSQL_STR(MAIN_CWBT, maincwbt)
			READMYSQL_INT(status, status,1)
			READMYSQL_STR(start_date, startdate)
			READMYSQL_INT(running_hrs,runtime,0)
			READMYSQL_STR(sc_code,sur)
			READMYSQL_STR(EQUIP_CODE,eno)
			READMYSQL_STR(equipmode,etype)
			READMYSQL_STR(COMPANY_NAME,emade)
			READMYSQL_INT(main_flag,host,0)
			READMYSQL_STR(PART_CAT_ID,eid)
	}


	out <<"{seq:\""<<strSeq<<"\",epid:\""<<eid<<"\",cwbt:\""<<cwbt<< "\",sur:\"" << sur<< "\",cn:\"" << name_cn<< "\",en:\"" <<name_en;
	out <<"\",sdate:"<<startdate<< ",run:\"" << runtime<< "\",gcwbt:\"" << maincwbt<< "\",st:" << status;
	out <<",md:\""<<emade<< "\",etype:\"" <<etype<< "\",eno:\"" << eno<< "\",td:\"" << techdes<< "\",fno:\"" << fno;
	out <<"\",paper:\""<<paper<< "\",host:" <<host<<",image:[]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
// 新增设备卡 0x03
int eMOPMaintenanceSvc::NewEquipCardItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::NewEquipCardItem]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	string strDepartId=root.getv("departid", "");
	string strCwbt=root.getv("cwbt", "");

	string strScCode=root.getv("sur", "");//船检编码
	string strName_cn=root.getv("cn", "");
	string strName_en=root.getv("en", "");
	string strSdate=root.getv("sdate", "");//启用日期
	string strRun=root.getv("run", "");//运行时间 小时？天

	string strMainCwbt=root.getv("gcwbt", "");//主计时设备的CWBT码
	int status=root.getv("st", 1);//1正常 2更换 3停用

	string strEtype=root.getv("etype", "");//设备型号名称
	string strEno=root.getv("eno", "");//设备编号
	string strMd=root.getv("md", ""); //制造商

	string strTechDesc=root.getv("td", ""); //技术参数描述
	string strFno=root.getv("fno", ""); //出厂编号
	string strPaper=root.getv("paper", ""); //图纸编号
	int  bHost=root.getv("host", 0); //定时主设备标记

	string equipModelId="";
	g_eMOPEquipSvc::instance()->GetEquipModelIdByCode(strEtype,equipModelId);
	if(equipModelId.empty())
		g_eMOPEquipSvc::instance()->AddEquipModel(strEtype,equipModelId);

	MySql* psql = CREATE_MYSQL;

	string sequence="";
	GET_EMOP_SEQUENCE_STRING(EB,sequence);

	string equipid="EBC"+strShipId+sequence;

	char sql[1024]="";
	sprintf (sql, "INSERT INTO blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD(EQUIP_ID,CWBT,SHIPID,DEPARTMENT_CODE,NAME_CN,NAME_EN,STATUS,SC_CODE,MAIN_CWBT)\
				  VALUES('%s','%s','%s','%s','%s','%s','%d','%s','%s')",equipid.c_str(),strCwbt.c_str(),strShipId.c_str(),strDepartId.c_str(),
				  strName_cn.c_str(),strName_en.c_str(),status,strScCode.c_str(),strMainCwbt.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


	char sql1[1024]="";


	sprintf (sql1, "INSERT INTO blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL(EQUIP_ID,SEQ,EQUIP_MODEL_ID,START_DATE,RUNNING_HRS,EQUIP_CODE,VALID_FLAG)\
				   VALUES('%s',1,'%s','%s','%s','%s','1')",equipid.c_str(),equipModelId.c_str(),strSdate.c_str(),strRun.c_str(),strEno.c_str());



	CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);


	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\",id:\""<<equipid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
// 修改设备卡 0x04
int eMOPMaintenanceSvc::ModifyEquipCardItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::ModifyEquipCardItem]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string equipid=root.getv("id", "");
	string strShipId=root.getv("shipid", "");
	string strDepartId=root.getv("departid", "");
	string strCwbt=root.getv("cwbt", "");

	string strScCode=root.getv("sur", "");//船检编码
	string strName_cn=root.getv("cn", "");
	string strName_en=root.getv("en", "");
	string strSdate=root.getv("sdate", "");//启用日期
	string strRun=root.getv("run", "");//运行时间 小时？天

	string strMainCwbt=root.getv("gcwbt", "");//主计时设备的CWBT码
	int status=root.getv("st", 1);//1正常 2更换 3停用

	string strEtype=root.getv("etype", "");//设备型号ID
	string strEno=root.getv("eno", "");//设备编号
	string strMd=root.getv("md", ""); //制造商

	string strTechDesc=root.getv("td", ""); //技术参数描述
	string strFno=root.getv("fno", ""); //出厂编号
	string strPaper=root.getv("paper", ""); //图纸编号
	int  bHost=root.getv("host", 0); //定时主设备标记

	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";
	string equipModelId="";
	g_eMOPEquipSvc::instance()->GetEquipModelIdByCode(strEtype,equipModelId);
	if(equipModelId.empty())
		g_eMOPEquipSvc::instance()->AddEquipModel(strEtype,equipModelId);

	sprintf (sql, "update blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD set CWBT='%s',SHIPID='%s',DEPARTMENT_CODE='%s',\
				  NAME_CN='%s',NAME_EN='%s',STATUS='%d',SC_CODE='%s',MAIN_CWBT='%s' where EQUIP_ID='%s'",
				  strCwbt.c_str(),strShipId.c_str(),strDepartId.c_str(),strName_cn.c_str(),strName_en.c_str(),status,strScCode.c_str(),strMainCwbt.c_str(),equipid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL \
				  SET START_DATE='%s',RUNNING_HRS=%s,EQUIP_MODEL_ID='%s',OPERATOR='%s',OP_DT='%s', VALID_FLAG=1  \
				  WHERE EQUIP_ID='%s';",
				  strSdate.c_str(),strRun.c_str(),equipModelId.c_str(),pUid,CurLocalDate().c_str(),equipid.c_str());


	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
// 删除设备卡 0x05
int eMOPMaintenanceSvc::DeleteEquipCardItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::DeleteEquipCardItem]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string id=root.getv("id", "");

	if (id.empty())
	{
		out<<"{seq:\""<<strSeq<<"\"}";
		return 0;
	}

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";
	sprintf (sql, "DELETE FROM blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD WHERE equip_id = '%s'", id.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


	//关联删除相应的工作卡和计划



	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";


	RELEASE_MYSQL_RETURN(psql, 0);
}

int eMOPMaintenanceSvc::GetEquipChangeHis(const char* pUid, const char* jsonString, std::stringstream& out){

	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetWordCardList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipid=root.getv("sid", "");
	string strEquipcardid=root.getv("epid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT t0.SHIPID,t0.EQUIP_ID,UNIX_TIMESTAMP(t1.END_DATE) AS END_DATE,t2.MODE,t2.COMPANY_NAME \
				  FROM blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD t0 \
				  JOIN blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL  t1 ON t0.EQUIP_ID = t1.EQUIP_ID \
				  LEFT JOIN blm_emop_etl.T50_EMOP_EQUIP_MODEL t2 ON t2.EQUIP_MODEL_ID = t1.EQUIP_MODEL_ID \
				  WHERE VALID_FLAG=0 AND t0.SHIPID='%s' AND t1.EQUIP_ID='%s';",strShipid.c_str(),strEquipcardid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3); 

	int endDate=0;
	string companyName=""; 
	string mode="";
	out<<"{seq:\"\",his:["; 
	int idx=0;
	while (psql->NextRow())
	{
		if(idx>0)
			out<<",";
		READMYSQL_STRING(MODE, mode) ;
		READMYSQL_INT(END_DATE, endDate,0) ;
		READMYSQL_STRING(COMPANY_NAME,companyName) ;

		out<<"{md:\""<<mode<<"\",time:"<<endDate<<",comp:\""<<companyName<<"\"}";
		++idx;
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//根据设备卡ID 获取工作卡列表信息 0x06
int eMOPMaintenanceSvc::GetWorkCardList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetWordCardList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipid=root.getv("shipid", "");
	string strDepartid=root.getv("departid", "");
	string strEquipcardid=root.getv("equipcardid", "");

	if (strEquipcardid.empty())
	{
		out<<"{seq:\""<<strSeq<<"\"}";
		return 0;
	}

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//SQL需要更换
	sprintf (sql, "select WORK_CARD_ID,LEVEL,STATUS,EXCUTE_WAY, TIMING_FLAG,PERIOD,TOLERANCE,OWNER,CONTENTS from blm_emop_etl.T50_EMOP_WORK_CARD WHERE EQUIP_ID='%s'",strEquipcardid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	char id[80] = "";
	char nature[100] = "";//工作性质
	char grade[10]="";//级别
	int mode=-1;//模式
	int cycle=0;//周期
	int differ=0;//允差
	char dutyMember[256]="";//责任人
	int runtime=0;//运行时长
	string workdesc="";//工作描述
	int status=0;//(暂时未用) 1，正常	2，停用 3，未首排 

	int cnt=0;

	while (psql->NextRow())
	{
		READMYSQL_STR(WORK_CARD_ID, id)
			READMYSQL_STR(LEVEL, grade)
			READMYSQL_INT(STATUS, status,1)
			READMYSQL_STR(EXCUTE_WAY, nature)
			READMYSQL_INT(TIMING_FLAG,mode,-1)
			READMYSQL_INT(PERIOD,cycle,0)
			READMYSQL_INT(TOLERANCE,differ,0)
			READMYSQL_STR(OWNER,dutyMember)
			READMYSQL_STRING_JSON(CONTENTS,workdesc)

			if (cnt++)
				out << ",";
		out << "{id:\"" << id << "\",nature:\"" << nature << "\",grade:\"" << grade <<"\",mode:" << mode << ",cycle:" << cycle <<",differ:" << differ;
		out	<<",pduty:\"" << dutyMember<<"\",run:\"" << runtime<<"\",work:\"" << workdesc<<"\"}" ;
	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}

// 请求工作卡详细信息 0x07
int eMOPMaintenanceSvc::GetWorkCardDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetWordCardDetail]bad format:", jsonString, 1);


	string strSeq = root.getv("seq", "");
	string strid=root.getv("id", "");


	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf (sql, "select WORK_CARD_ID,LEVEL,STATUS,EXCUTE_WAY, TIMING_FLAG,PERIOD,TOLERANCE,OWNER,CONTENTS,DIRECTIONS,SAFE_DIRECTIONS,\
				  ATTACHMENT_REQUIRED,UNIX_TIMESTAMP(EXPIRE_DT) AS EXPIRE_DT from blm_emop_etl.T50_EMOP_WORK_CARD WHERE WORK_CARD_ID='%s'",strid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char id[80] = "";
	char nature[100] = "";//工作性质
	char grade[10]="";//级别
	int mode=-1;//模式
	int cycle=0;//周期
	int differ=0;//允差
	char dutyMember[256]="";//责任人
	int runtime=0;//运行时长
	string workdesc="";//工作描述

	string maindesc="";//保养说明
	string safedesc="";//安全须知

	int attach=0;//携带附件 0 否 1 是
	int benable=1;//工作卡启用标记 0停用，1启用
	int validdt=0;//有效日期


	if (psql->NextRow())
	{
		READMYSQL_STR(WORK_CARD_ID, id)
			READMYSQL_STR(LEVEL, grade)
			READMYSQL_INT(STATUS, benable,1)
			READMYSQL_STR(EXCUTE_WAY, nature)
			READMYSQL_INT(TIMING_FLAG,mode,-1)
			READMYSQL_INT(PERIOD,cycle,0)
			READMYSQL_INT(TOLERANCE,differ,0)
			READMYSQL_STR(OWNER,dutyMember)
			READMYSQL_STRING(CONTENTS,workdesc)
			READMYSQL_STRING(DIRECTIONS,maindesc)
			READMYSQL_STRING(SAFE_DIRECTIONS,safedesc)
			READMYSQL_INT(ATTACHMENT_REQUIRED,attach,0)
			READMYSQL_INT(EXPIRE_DT,validdt,0)


			out <<"{seq:\""<<strSeq<<"\",id:\"";	
		out << id << "\",nature:\"" << nature << "\",grade:\"" << grade <<"\",mode:" << mode << ",cycle:" << cycle <<",differ:" << differ;
		out	<<",pduty:\""<< dutyMember<<"\",run:\""<< runtime<<"\",work:\""<< workdesc<<"\",main:\""<< maindesc<<"\",safe:\""<< safedesc;
		out	<<"\",attach:" << attach<<",wsign:"<<benable<<",date:"<<validdt<<",spare:[";

		char sql1[1024] = "";

		sprintf (sql1, "select t1.REQUIRED_NUM,T1.PART_MODEL_ID,T1.PART_FLAG,T2.NAME_CN,T2.NAME_EN from blm_emop_etl.T50_EMOP_WORK_CARD_PARTS t1 \
					   LEFT JOIN blm_emop_etl.t50_emop_parts_models t2 ON t1.PART_MODEL_ID=t2.PART_MODEL_ID \
					   WHERE T1.WORK_CARD_ID='%s'",strid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

		char no[64]="";
		char partnm[128]="";
		int flag=0;
		int number=0;

		int idx=0;
		while (psql->NextRow())
		{

			READMYSQL_STR(PART_MODEL_ID, no);
			READMYSQL_INT(REQUIRED_NUM,number,0);
			READMYSQL_INT(PART_FLAG,flag,0);
			READMYSQL_STR(NAME_CN,partnm);

			if (strlen(partnm)==0)
			{
				READMYSQL_STR(NAME_EN,partnm)
			}

			if (idx>0)
			{
				out<<",";
			}

			out<<"{no:\"" << no << "\",na:\"" << partnm << "\",count:" << number <<",hsign:" << flag<<"}";
			idx++;
		}

		out<<"],report:[";//关联报表暂时没加

		out<<"]}" ;

	}


	RELEASE_MYSQL_RETURN(psql, 0);

}

// 新增工作卡 0x08
int eMOPMaintenanceSvc::NewWorkCardItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::NewWordCardItem]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string equipId=root.getv("equipcardid", "");
	string strShipId=root.getv("shipid", "");
	string strDepartId=root.getv("departid", "");
	string strNature=root.getv("nature", "");//工作性质
	string strGrade=root.getv("grade", "");//级别
	int mode=root.getv("mode", 0);//模式 0 定期/1定时/2优先/3临时定期/4临时定时
	int cycle=root.getv("cycle", 0);//周期
	int differ=root.getv("differ", 0);//允差

	int runtime=root.getv("run", 0);//累计运行时长
	string pduty=root.getv("pduty","");//责任人
	string workdes=root.getv("work","");//工作描述
	string mandes=root.getv("main","");//保养说明
	string safedes=root.getv("safe","");//安全须知

	int attach=root.getv("attach",0);//附件
	int benable=root.getv("wsign",0);//工作卡启用标记 0停用，1启用
	string validdate=root.getv("date","");


	string sequence="";
	GET_EMOP_SEQUENCE_STRING(JB,sequence)

		MySql* psql = CREATE_MYSQL;

	string workcardId="JBC"+strShipId+sequence;//暂时先以seq作为流水


	char sql[1024]="";
	sprintf (sql, "	INSERT INTO blm_emop_etl.T50_EMOP_WORK_CARD(WORK_CARD_ID,EQUIP_ID,LEVEL,STATUS,EXCUTE_WAY,TIMING_FLAG,PERIOD,TOLERANCE,\
				  OWNER,CONTENTS,DIRECTIONS,SAFE_DIRECTIONS,ATTACHMENT_REQUIRED,EXPIRE_DT)\
				  VALUES('%s','%s','%s','%d','%s',%d,%d,%d,'%s','%s','%s','%s',%d,'%d')",
				  workcardId.c_str(),equipId.c_str(),strGrade.c_str(),benable,strNature.c_str(),mode,cycle,differ, 
				  pduty.c_str(),workdes.c_str(),mandes.c_str(),safedes.c_str(),attach,validdate.c_str());

	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	//添加备件

	Json* parts = root.get("spare");

	if (parts)
	{
		for (int i=0;i<parts->size();i++)
		{
			Json* partItem=parts->get(i);
			if (!partItem)
			{
				continue;
			}

			string partid=partItem->getv("sid",""); 
			int consump=partItem->getv("count",0);
			int hsign=partItem->getv("hsign",0);

			char sql1[512]="";
			sprintf (sql1, " INSERT INTO blm_emop_etl.T50_EMOP_WORK_CARD_PARTS(WORK_CARD_ID,PART_MODEL_ID,REQUIRED_NUM,PART_FLAG)\
						   VALUES('%s','%s',%d,%s)", workcardId.c_str(),partid.c_str(),consump,hsign);
			DEBUG_LOG(sql);
			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);

		}
	}

	//附件







	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\",id:\""<<workcardId<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
// 修改工作卡 0x09
int eMOPMaintenanceSvc::ModifyWorkCardItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::ModifyEquipCardItem]bad format:", jsonString, 1);


	string strSeq=root.getv("seq", "");
	string Id=root.getv("id", "");
	string strShipId=root.getv("shipid", "");
	string strDepartId=root.getv("departid", "");
	string strNature=root.getv("nature", "");//工作性质
	string strGrade=root.getv("grade", "");//级别
	int mode=root.getv("mode", 0);//模式 0 定期/1定时/2优先/3临时定期/4临时定时
	int cycle=root.getv("cycle", 0);//周期
	int differ=root.getv("differ", 0);//允差

	int runtime=root.getv("run", 0);//累计运行时长
	string pduty=root.getv("pduty","");//责任人
	string workdes=root.getv("work","");//工作描述
	string mandes=root.getv("main","");//保养说明
	string safedes=root.getv("safe","");//安全须知

	int attach=root.getv("attach",0);//附件
	int benable=root.getv("wsign",0);//工作卡启用标记 0停用，1启用
	string validdate=root.getv("date","");


	MySql* psql = CREATE_MYSQL;


	char sql[1024]="";

	sprintf (sql, "update blm_emop_etl.T50_EMOP_WORK_CARD set LEVEL='%s',STATUS='%d',EXCUTE_WAY='%s',TIMING_FLAG=%d,PERIOD=%d,TOLERANCE=%d,OWNER='%s',\
				  CONTENTS='%s',DIRECTIONS='%s',SAFE_DIRECTIONS='%s',ATTACHMENT_REQUIRED=%d,EXPIRE_DT='%s' where WORK_CARD_ID='%s'",
				  strGrade.c_str(),benable,strNature.c_str(),mode,cycle,differ, 
				  pduty.c_str(),workdes.c_str(),mandes.c_str(),safedes.c_str(),attach,validdate.c_str(),Id.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	Json* parts = root.get("spare");

	if (parts)
	{
		for (int i=0;i<parts->size();i++)
		{
			Json* partItem=parts->get(i);
			if (!partItem)
			{
				continue;
			}

			string partid=partItem->getv("sid",""); 
			int consump=partItem->getv("count",0);
			int hsign=partItem->getv("hsign",0);

			char sql1[512]="";
			sprintf (sql1, " UPDATE blm_emop_etl.T50_EMOP_WORK_CARD_PARTS \
						   SET REQUIRED_NUM=%d,PART_FLAG=%d \
						   WHERE WORK_CARD_ID='%s' AND PART_MODEL_ID='%s'", consump,hsign,Id.c_str(),partid.c_str());
			DEBUG_LOG(sql);
			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);

		}
	}

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
// 删除工作卡 0x0a
int eMOPMaintenanceSvc::DeleteWorkCardItem(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::DeleteWorkCardItem]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string id=root.getv("id", "");

	if (id.empty())
	{
		out<<"{seq:\""<<strSeq<<"\"}";
		return 0;
	}

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";
	sprintf (sql, "DELETE FROM blm_emop_etl.T50_EMOP_WORK_CARD WHERE WORK_CARD_ID = '%s'", id.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);



	//删除关联的计划



	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}

//根据船舶和部门请求工作卡计划列表 0x10
int eMOPMaintenanceSvc::GetWorkPlanList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetWorkPlanList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipid=root.getv("shipid", "");
	string strDepartid=root.getv("departid", "");

	if (strShipid.empty())
	{
		return 0;
	}

	MySql* psql = CREATE_MYSQL;


	vector<WorkCardPlanInfo> vecPlanList;

	char sql[1024] = "";

	sprintf (sql, "SELECT T3.MANTENANCE_ID,T3.STATUS,T3.SCHEDULED_YEAR,T3.SCHEDULED_MON,T3.FINISHED_DATE,T3.TOTAL_TIMING,T2.OWNER,T2.LEVEL,T2.PERIOD,T2.ATTACHMENT_REQUIRED,T1.CWBT,T1.NAME_CN \
				  FROM blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD T1,blm_emop_etl.T50_EMOP_WORK_CARD T2, blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE T3 \
				  WHERE T1.SHIPID='%s' and t1.DEPARTMENT_CODE='%s' AND T1.EQUIP_ID=T2.EQUIP_ID AND T2.WORK_CARD_ID=T3.WORK_CARD_ID \
				  AND (T3.SCHEDULED_YEAR>=2013 OR T3.SCHEDULED_YEAR=0) ",strShipid.c_str(),strDepartid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	


	char plandt[20]=""; //开始时间

	char taskid[64]="";
	char stepid[16]="";
	int planstatus=0;

	while (psql->NextRow())
	{
		WorkCardPlanInfo workPlanItem;

		READMYSQL_STRING(MANTENANCE_ID, workPlanItem.planid)
			READMYSQL_INT(STATUS,workPlanItem.status,0)
			READMYSQL_INT(SCHEDULED_YEAR,workPlanItem.year,0)
			READMYSQL_INT(SCHEDULED_MON,workPlanItem.month,0)

			if (workPlanItem.year>0&&workPlanItem.month>0)
			{
				sprintf(plandt,"%04d-%02d",workPlanItem.year,workPlanItem.month);
				workPlanItem.plandt=plandt;
			}


			READMYSQL_STRING(FINISHED_DATE, workPlanItem.finishdt)
				READMYSQL_INT(TOTAL_TIMING,workPlanItem.runtime,0)


				READMYSQL_STRING(OWNER, workPlanItem.pduty)
				READMYSQL_STRING(LEVEL, workPlanItem.grade)
				READMYSQL_INT(PERIOD, workPlanItem.period,0)
				READMYSQL_INT(ATTACHMENT_REQUIRED,workPlanItem.battach,0)

				READMYSQL_STRING(CWBT,workPlanItem.cwbt)
				READMYSQL_STRING(NAME_CN,workPlanItem.equipname)

				vecPlanList.push_back(workPlanItem);

	}

	for (int i=0;i<vecPlanList.size();i++)
	{
		WorkCardPlanInfo workPlanItem=vecPlanList[i];

		if (i>0)
		{
			out << ",";
		}

		out << "{id:\"" << workPlanItem.planid << "\",status:" << workPlanItem.status << ",pd:\"" << workPlanItem.plandt <<"\",cd:\"" << workPlanItem.finishdt << "\",cwbt:\"" << workPlanItem.cwbt <<"\",dname:\"" << workPlanItem.equipname;
		out	<<"\",grade:\"" << workPlanItem.grade<<"\",pduty:\"" << workPlanItem.pduty<<"\",cycle:" <<workPlanItem.period<<",run:" << workPlanItem.runtime<<",flag:" << workPlanItem.battach<<",flow:";


		int plantype=0;

		char sql1[512]="";

		sprintf (sql1, "select OPERATOR,OP_DATE,STATUS,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID \
					   from blm_emop_etl.T50_EMOP_MAINTENANCE_STEPS_STATUS WHERE MANTENANCE_ID='%s' ORDER BY OP_DATE DESC",workPlanItem.planid.c_str());

		psql->Query(sql1);

		if (psql->NextRow())
		{
			READMYSQL_STR(WORKFLOW_TASK_ID, taskid)
				READMYSQL_STR(WORKFLOW_STEP_ID, stepid)
				READMYSQL_INT(STATUS, planstatus,0)

				out<<"{taskid:\""<<taskid<<"\",stepid:\""<<stepid<<"\",status:"<<planstatus<<"}";
		}
		else
		{
			out<<"{taskid:\"\",stepid:\"\",status:0}";
		}


		if (workPlanItem.year==0||workPlanItem.month==0)//未收排
		{
			plantype=2;

		}
		else if (string(taskid)=="52"&&workPlanItem.status==1)//待批复计划
		{
			plantype=1;
		}

		out<<",type:"<<plantype<<"}";

	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}


int eMOPMaintenanceSvc::FirstTimeAssignPlan(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::FirstTimeAssignPlan]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strPlanId=root.getv("pid", "");
	int year=root.getv("year", 0);
	int month=root.getv("month", 0);


	MySql* psql = CREATE_MYSQL;

	char sql[512]="";

	sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE SET SCHEDULED_YEAR=%d,SCHEDULED_MON=%d WHERE MANTENANCE_ID='%s'",
		year,month,strPlanId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}

int eMOPMaintenanceSvc::ChangePlanTime(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::ChangePlanTime]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strPlanId=root.getv("pid", "");
	int year=root.getv("year", 0);
	int month=root.getv("month", 0);
	string reason=root.getv("reason","");

	int oldyear=0,oldmonth=0;
	string workcardid="";

	MySql* psql = CREATE_MYSQL;
	string timestr=GetCurrentTmStr();
	char sql[512]="";
	sprintf (sql, "SELECT WORK_CARD_ID,SCHEDULED_YEAR,SCHEDULED_MON FROM blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE \
				  WHERE MANTENANCE_ID='%s'",strPlanId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (psql->NextRow())
	{

		READMYSQL_STRING(WORK_CARD_ID, workcardid)
			READMYSQL_INT(SCHEDULED_YEAR, oldyear,0)
			READMYSQL_INT(SCHEDULED_MON, oldmonth,0)

			string sequence="";
		GET_EMOP_SEQUENCE_STRING(MDY,sequence)

			string modifyid="MDY"+sequence;
		sprintf (sql, "INSERT INTO blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE_HIS(MODIFY_ID,MANTENANCE_ID,WORK_CARD_ID,\
					  SCHEDULED_YEAR,SCHEDULED_MON,MODIFIED_YEAR,MODIFIED_MON,MODIFIED_REASON,OP_DT) \
					  VALUES('%s','%s','%s',%d,%d,%d,%d,'%s','%s')",modifyid.c_str(),strPlanId.c_str(),workcardid.c_str(),
					  oldyear,oldmonth,year,month,reason.c_str(),timestr.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


		sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE SET SCHEDULED_YEAR=%d,SCHEDULED_MON=%d WHERE MANTENANCE_ID='%s'",
			year,month,strPlanId.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}


//请求某计划报告基本信息0x13
int eMOPMaintenanceSvc::GetWorkPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetWorkPlanDetail]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string planid=root.getv("pid", "");

	if (planid.empty())
	{
		out<<"{seq:\""<<strSeq<<"\"}";
		return 0;
	}

	MySql* psql = CREATE_MYSQL;

	//step1 请求基本信息
	char sql[1024] = "";
	sprintf (sql, "SELECT T2.CONTENTS,T2.DIRECTIONS,T2.SAFE_DIRECTIONS,T2.TIMING_FLAG,T2.TOLERANCE \
				  FROM blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE T1,blm_emop_etl.T50_EMOP_WORK_CARD T2 \
				  WHERE T1.MANTENANCE_ID='%s' and T1.WORK_CARD_ID =T2.WORK_CARD_ID",planid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	string content="";
	string safedes="";
	string mandes="";
	char mode[3]="";
	int differ=0;

	if (psql->NextRow())
	{
		string content="";
		string safedes="";
		string mandes="";

		READMYSQL_STRING(CONTENTS,content)
			READMYSQL_STRING(DIRECTIONS,safedes)
			READMYSQL_STRING(SAFE_DIRECTIONS,mandes)
			READMYSQL_STR(TIMING_FLAG,mode)
			READMYSQL_INT(TOLERANCE,differ,0)

			out <<"{seq:\""<<strSeq<<"\",mode:"<<mode<<",differ:"<<differ<<",work:\""<<content<< "\",main:\"" << mandes<< "\",safe:\"" << safedes<< "\",mattach:[],report:[";


		//step2 加载关联报表
		char sql1[512] = "";
		sprintf (sql1, "select REPORT_ID,NAME,FILE_ID from blm_emop_etl.T50_EMOP_MANTENANCE_RPTS WHERE MANTENANCE_ID='%s'",planid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

		char reportId[32]="";
		char reportnm[128]="";

		int idx=0;
		while(psql->NextRow())
		{
			READMYSQL_STR(REPORT_ID,reportId)
				READMYSQL_STR(NAME,reportnm)

				if (idx)
				{
					out <<",";
				}
				out <<"\""<<reportId<<"|"<<reportnm<<"\"";
				idx++;
		}

		out <<"],wattach:[";


		//step3 加载附件
		char sql2[512] = "";
		sprintf (sql2, "select ATTACH_ID,NAME,FILE_ID from blm_emop_etl.T50_EMOP_MANTENANCE_ATTACHMENTS WHERE MANTENANCE_ID='%s'",planid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql2), 3);

		idx=0;
		while(psql->NextRow())
		{
			READMYSQL_STR(ATTACH_ID,reportId)
				READMYSQL_STR(NAME,reportnm)

				if (idx)
				{
					out <<",";
				}
				out <<"\""<<reportId<<"|"<<reportnm<<"\"";
				idx++;
		}

		out <<"],spare:[";

		//step4 加载备件列表

		char sql3[1024] = "";
		sprintf (sql3, "SELECT T1.PART_MODEL_ID,T1.REQUIRED_NUM,T1.CONSUMPTION,T1.PART_FLAG,T2.NAME_CN,T2.NAME_EN,\
					   T2.MODEL,T2.CHART_NO,T2.STOCKS,T2.UNIT FROM blm_emop_etl.T50_EMOP_MAINTENANCE_PARTS T1,blm_emop_etl.T50_EMOP_PARTS_MODELS T2 \
					   WHERE T1.MANTENANCE_ID='%s' AND T2.PART_MODEL_ID=T1.PART_MODEL_ID",planid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql3), 3);

		char partid[64]="";
		char name[128]="";
		char model[128]="";
		char chartno[128]="";
		char unit[20]="";

		int reqnum=0;
		int stocknum=0;
		int consumpnum=0;

		idx=0;
		while(psql->NextRow())
		{
			READMYSQL_STR(PART_MODEL_ID,partid)
				READMYSQL_STR(NAME_CN,name)

				if (strlen(name)==0)
			 {
				 READMYSQL_STR(NAME_EN,name)
			 }

				READMYSQL_STR(MODEL,model)
					READMYSQL_STR(CHART_NO,chartno)
					READMYSQL_STR(UNIT,unit)
					READMYSQL_INT(REQUIRED_NUM,reqnum,0)
					READMYSQL_INT(CONSUMPTION,consumpnum,0)
					READMYSQL_INT(STOCKS,stocknum,0)


					if (idx)
					{
						out <<",";
					}

					out <<"{sid:\""<<partid<<"\",na:\""<<name<< "\",sno:\"" << model<< "\",pno:\"" << chartno<<"\",unit:\"" << unit;
					out <<"\",ycount:"<<reqnum<<",kcount:"<<stocknum<<",xcount:"<<consumpnum<<"}";

					idx++;
		}


		out <<"]}";

	}
	else
	{
		out<<"{seq:\""<<strSeq<<"\"}";
	}



	RELEASE_MYSQL_RETURN(psql, 0);
}


//请求计划保养总工作流 0x14
int eMOPMaintenanceSvc::GetMaintenanceWorkFlow(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetMaintenanceWorkFlow]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf (sql, "select t1.workflow_task_id,t1.name,t2.workflow_step_id,t2.name as stepname \
				  from blm_emop_etl.t50_emop_workflow_task t1,blm_emop_etl.t50_emop_workflow_steps t2 \
				  where t1.workflow_id='%d' and t1.workflow_task_id=t2.workflow_task_id \
				  order by t1.sequence_number",5);


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",workflow:[";	

	char pretaskid[20]="";
	char taskid[20] = "";
	char taskname[80]=""; 
	char stepid[20]=""; 
	char stepname[80]=""; 

	int cnt = 0;

	while (psql->NextRow())
	{
		READMYSQL_STR(workflow_task_id, taskid)
			READMYSQL_STR(name, taskname)
			READMYSQL_STR(workflow_step_id, stepid)
			READMYSQL_STR(stepname, stepname)

			if (strcmp(pretaskid,taskid)!=0)
			{
				if (cnt++)
				{
					out << "]},";
				}

				out << "{taskid:\"" << taskid << "\",tasknm:\"" << taskname<<"\",steps:[{stepid:\""<<stepid<<"\",stepnm:\"" << stepname<<"\"}";

			}
			else
			{
				out << ",{stepid:\"" << stepid << "\",stepnm:\"" << stepname<<"\"}";
			}

			strcpy(pretaskid,taskid);

	}

	if (cnt)
	{
		out << "]}";
	}

	out << "]}";


	RELEASE_MYSQL_RETURN(psql, 0);
}
//船端填写实绩报告 0x15
int eMOPMaintenanceSvc::ShipFillPlanReport(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::ShipFillPlanReport]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strPlanid = root.getv("pid", "");
	string strUid=root.getv("uid", "");
	string strTaskid=root.getv("taskid", "");
	string strStepid=root.getv("stepid", "");
	string strDate=root.getv("date", "");
	string strContent=root.getv("content", "");
	int status=root.getv("ack", 1);



	MySql* psql = CREATE_MYSQL;
	char sql[512]="";
	sprintf (sql, "INSERT INTO blm_emop_etl.T50_EMOP_MAINTENANCE_STEPS_STATUS(MANTENANCE_ID,OPERATOR,OP_DATE,STATUS,REMARK,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID) \
				  VALUES('%s','%s','%s','%d','%s','%s','%s')",
				  strPlanid.c_str(),strUid.c_str(),strDate.c_str(),status,strContent.c_str(),strTaskid.c_str(),strStepid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


	Json* parts = root.get("spare");

	if (parts)
	{
		for (int i=0;i<parts->size();i++)
		{
			Json* partItem=parts->get(i);
			if (!partItem)
			{
				continue;
			}


			string partmodelid=partItem->getv("sid","");
			int consump=partItem->getv("xcount",0);
			string unit=partItem->getv("unit","");

			char sql1[512]="";
			sprintf (sql1, " update blm_emop_etl.T50_EMOP_MAINTENANCE_PARTS set CONSUMPTION=%d \
						   where MANTENANCE_ID='%s' and PART_MODEL_ID='%s'",
						   consump,strPlanid.c_str(),partmodelid.c_str());

			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);

		}
	}

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	return 0;
}
//实绩报告确认、反馈、批复 0x16
int eMOPMaintenanceSvc::ConformPlanReport(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::ConformPlanReport]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strPlanId=root.getv("pid", "");
	string strUid=root.getv("uid", "");
	string strTaskid=root.getv("taskid", "");
	string strStepid=root.getv("stepid", "");
	string strDate=root.getv("date", "");
	string strContent=root.getv("content", "");
	int status=root.getv("ack", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[512]="";
	sprintf (sql, "INSERT INTO blm_emop_etl.T50_EMOP_MAINTENANCE_STEPS_STATUS(MANTENANCE_ID,OPERATOR,OP_DATE,STATUS,REMARK,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID) \
				  VALUES('%s','%s','%s','%d','%s','%s','%s')",
				  strPlanId.c_str(),strUid.c_str(),strDate.c_str(),status,strContent.c_str(),strTaskid.c_str(),strStepid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//请求某计划已执行工作流0x17
int eMOPMaintenanceSvc::GetPlanWorkFlow(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetPlanWorkFlow]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strPlanId=root.getv("pid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";

	sprintf (sql, "select MANTENANCE_ID,OPERATOR,OP_DATE,STATUS,REMARK,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID \
				  from blm_emop_etl.T50_EMOP_MAINTENANCE_STEPS_STATUS where MANTENANCE_ID='%s'",strPlanId.c_str());


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",pid:\""<<strPlanId<<"\",workflow:[";	


	char taskid[20] = "";
	char stepid[20]=""; 
	char date[20]=""; 
	char operator1[64]="";
	string jobs="";
	string content="";
	int status=-1;

	int cnt = 0;

	while (psql->NextRow())
	{
		READMYSQL_STR(WORKFLOW_TASK_ID, taskid)
			READMYSQL_STR(OP_DATE, date)
			READMYSQL_STR(WORKFLOW_STEP_ID, stepid)
			READMYSQL_STR(OPERATOR, operator1)
			READMYSQL_STRING(REMARK,content)
			READMYSQL_INT(STATUS,status,-1)

			if (cnt++)
			{
				out << ",";
			}

			out << "{taskid:\"" << taskid << "\",stepid:\"" << stepid<<"\",content:\""<<content<<"\",date:\"" << date<<"\",operator:\"" << operator1<<"\",job:\"" << jobs<<"\",status:" << status<<"}";

	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}


//请求常规保养计划实时运行信息 0x20
int eMOPMaintenanceSvc::RoutineMaintenanceList(const char* pUid, const char* jsonString, std::stringstream& out)
{


	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::RoutineMaintenanceList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	string strDepartId=root.getv("departid", "");
	string strGrade=root.getv("grade", "");
	string strDate=root.getv("month", "");

	string orderby2="";
	string tablename2="blm_emop_etl.T50_ABC_MAINTENANCE_"+strGrade;
    int year=0,month=0; 
	char sql[1024]="";
	char departWhere[100]="";
	if(!strDepartId.empty())
		sprintf(sql,"t1.DEPARTMENT_CODE='%s'",strDepartId.c_str());

	if (strGrade=="A"||strGrade=="a")
	{
		orderby2="day"; 
		sscanf(strDate.c_str(),"%d-%d",&year,&month); 
		sprintf(sql,"select t1.WORK_CARD_ID,t1.DEPARTMENT_CODE,t1.LEVEL,t1.CWBT,t1.NAME_CN,t2.status \
					from blm_emop_etl.t50_abc_cbwt_ship_equip_card t1,%s t2 \
					WHERE t1.SHIPID='%s' %s and t1.Level='%s' \
					and t2.year=%d and t2.month=%d and t1.WORK_CARD_ID=t2.WORK_CARD_ID \
					order by t1.WORK_CARD_ID,t2.%s",
					tablename2.c_str(),strShipId.c_str(),departWhere,strGrade.c_str(),year,month,orderby2.c_str());
	}
	else if (strGrade=="B"||strGrade=="b")
	{ 
		orderby2="week"; 
		sscanf(strDate.c_str(),"%d-%d",&year,&month);
		sprintf(sql,"select t1.WORK_CARD_ID,t1.DEPARTMENT_CODE,t1.LEVEL,t1.CWBT,t1.NAME_CN,t2.status \
					from blm_emop_etl.t50_abc_cbwt_ship_equip_card t1,%s t2 \
					WHERE t1.SHIPID='%s' %s and t1.Level='%s' \
					and t2.year=%d and t2.month=%d and t1.WORK_CARD_ID=t2.WORK_CARD_ID \
					order by t1.WORK_CARD_ID,t2.%s",
					tablename2.c_str(),strShipId.c_str(),departWhere,strGrade.c_str(),year,month,orderby2.c_str());
	}
	else if (strGrade=="C"||strGrade=="c")
	{
		orderby2="month"; 
		sscanf(strDate.c_str(),"%d",&year);
		sprintf(sql,"select t1.WORK_CARD_ID,t1.DEPARTMENT_CODE,t1.LEVEL,t1.CWBT,t1.NAME_CN,t2.status \
					from blm_emop_etl.t50_abc_cbwt_ship_equip_card t1,%s t2 \
					WHERE t1.SHIPID='%s' %s and t1.Level='%s' \
					and t2.year=%d  and t1.WORK_CARD_ID=t2.WORK_CARD_ID \
					order by t1.WORK_CARD_ID,t2.%s",
					tablename2.c_str(),strShipId.c_str(),departWhere,strGrade.c_str(),year,orderby2.c_str());
	}
	else
	{
		return 0;
	}

	


	MySql* psql = CREATE_MYSQL; 

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	map<string,RoutineMaintenanceItem> mapRoutine;

	string workid="";
	int status=0;

	while (psql->NextRow())
	{

		READMYSQL_STRING(WORK_CARD_ID, workid)

			if (mapRoutine.find(workid)==mapRoutine.end())
			{
				RoutineMaintenanceItem item;
				item.id=workid;
				READMYSQL_STRING(NAME_CN,item.name)
					READMYSQL_STRING(CWBT,item.cwbt)
					READMYSQL_INT(status,status,0)
					item.vecStatus.push_back(status);
				mapRoutine.insert(make_pair(item.id,item)); 
			}
			else
			{
				RoutineMaintenanceItem *pItem=&(mapRoutine[workid]);
				READMYSQL_INT(status,status,0)
					pItem->vecStatus.push_back(status);
			}

	}


	out <<"{seq:\""<<strSeq<<"\",data:[";

	for (map<string,RoutineMaintenanceItem>::iterator it=mapRoutine.begin();it!=mapRoutine.end();it++)
	{
		if (it!=mapRoutine.begin())
		{
			out<<",";
		}
		RoutineMaintenanceItem item=it->second;
		string stateStr="";

		for (int i=0;i<item.vecStatus.size();i++)
		{
			if (i>0)
			{
				stateStr+=",";
			}
			stateStr+=IntToStr(item.vecStatus[i]);
		} 

		out<<"{id:\"" << item.id << "\",cwbt:\"" << item.cwbt<<"\",name:\""<<item.name<<"\",state:\"" << stateStr<<"\"}";

	}

	out<<"],signature:[";


	char sql1[1024]="";
	sprintf(sql1,"select CEO_OP,CEO_ID,CEO_DT,CEO_NAME,EM_OP,EM_ID,EM_DT,EM_NAME,CO_OP,CO_ID,CO_DT,CO_NAME,\
				 MM_OP,MM_ID,MM_DT,MM_NAME, CEE_OP,CEE_ID,CEE_DT,CEE_NAME,EEM_OP,EEM_ID,EEM_DT,EEM_NAME \
				 from blm_emop_etl.T50_ABC_MAINTENANCE_SIGN where shipid='%s' and year=%d and month=%d",strShipId.c_str(),year,month);

	CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

#define JSON_OUT_TYPE(type) {if(strlen(uid)>0&&strlen(signdate)>0){if (idx>0)out<<",";\
	out<<"{type:"<<type<<",name:\"" <<uid<<"\",date:\""<<signdate<<"\"}"; idx++;}\
	}	



	char uid[64];
	char uname[128];
	char signdate[32];

	int idx=0;

	if (psql->NextRow())
	{
		READMYSQL_STR(CEO_ID,uid)  //轮机长
			READMYSQL_STR(CEO_NAME,uname)
			READMYSQL_STR(CEO_DT,signdate)

			JSON_OUT_TYPE(0)


			READMYSQL_STR(CO_ID,uid)  //大副
			READMYSQL_STR(CO_NAME,uname)
			READMYSQL_STR(CO_DT,signdate)

			JSON_OUT_TYPE(1)

			READMYSQL_STR(CEE_ID,uid)  //电机员
			READMYSQL_STR(CEE_NAME,uname)
			READMYSQL_STR(CEE_DT,signdate)

			JSON_OUT_TYPE(2)


			READMYSQL_STR(EM_ID,uid)  //机务主管
			READMYSQL_STR(EM_NAME,uname)
			READMYSQL_STR(EM_DT,signdate)

			JSON_OUT_TYPE(3)

			READMYSQL_STR(MM_ID,uid)  //海务主管
			READMYSQL_STR(MM_NAME,uname)
			READMYSQL_STR(MM_DT,signdate)

			JSON_OUT_TYPE(4)

			READMYSQL_STR(EEM_ID,uid)  //电机主管
			READMYSQL_STR(EEM_NAME,uname)
			READMYSQL_STR(EEM_DT,signdate)

			JSON_OUT_TYPE(5)

	}


	out<<"]}";



	RELEASE_MYSQL_RETURN(psql, 0);
}
//设置常规保养值状态 0x21
int eMOPMaintenanceSvc::SetRoutineMaintenanceVal(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::SetRoutineMaintenanceVal]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strId=root.getv("cardid", "");

	int year=root.getv("year", 0);
	int month=root.getv("month", 0);
	string level=root.getv("grade", "");
	int idx=root.getv("index", 0);
	int status=root.getv("status", 0);

	MySql* psql = CREATE_MYSQL;

	char sql[512]="";

	if (level=="A"||level=="a")
	{
		sprintf(sql,"REPLACE INTO blm_emop_etl.T50_ABC_MAINTENANCE_A(WORK_CARD_ID,YEAR,MONTH,DAY,STATUS) VALUES('%s',%d,%d,%d,%d)",
			strId.c_str(),year,month,idx,status);

	}
	else if (level=="B"||level=="b")
	{
		sprintf(sql,"REPLACE INTO blm_emop_etl.T50_ABC_MAINTENANCE_B(WORK_CARD_ID,YEAR,MONTH,WEEK,STATUS) VALUES('%s',%d,%d,%d,%d)",
			strId.c_str(),year,month,idx,status);

	}
	else if (level=="C"||level=="c")
	{
		sprintf(sql,"REPLACE INTO blm_emop_etl.T50_ABC_MAINTENANCE_C(WORK_CARD_ID,YEAR,MONTH,STATUS) VALUES('%s',%d,%d,%d)",
			strId.c_str(),year,month,status);

	}


	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";


	RELEASE_MYSQL_RETURN(psql, 0);

}
//请求常规保养计划维护模板信息 0x22
int eMOPMaintenanceSvc::GetRoutineMaintenanceTemplate(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetRoutineMaintenanceTemplate]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	string strDepartId=root.getv("departid", "");
	string strGrade=root.getv("grade", "");

	MySql* psql = CREATE_MYSQL;

	if (strShipId.empty())
	{
		out <<"{seq:\""<<strSeq<<"\",data:[]}";
		return 0;
	}

	stringstream ss;

	ss<<" SELECT t1.WORK_CARD_ID,t1.DEPARTMENT_CODE,t1.LEVEL,t1.CWBT,t1.NAME_CN,t2.NAME_CN AS DEPART_NAME FROM blm_emop_etl.t50_abc_cbwt_ship_equip_card t1";
	ss<<" LEFT JOIN blm_emop_etl.T50_EMOP_DEPARTMENT_CODE t2 ON t1.DEPARTMENT_CODE=t2.DEPARTMENT_CODE ";
	ss<<" WHERE t1.SHIPID='"<<strShipId<<"'";

	if (!strDepartId.empty())
	{
		ss<<" AND t1.DEPARTMENT_CODE='"<<strDepartId<<"'";
	}

	if (!strGrade.empty())
	{
		ss<<" AND t1.LEVEL='"<<strGrade<<"'";
	}

	out <<"{seq:\""<<strSeq<<"\",data:[";

	CHECK_MYSQL_STATUS(psql->Query(ss.str().c_str()), 3);


	char id[64]="";
	char departcode[64]="";
	char departname[200]="";
	char level[6]="";
	char cwbt[64]="";
	char name[128]="";

	int cnt=0;

	while (psql->NextRow())
	{
		READMYSQL_STR(WORK_CARD_ID, id)
			READMYSQL_STR(DEPARTMENT_CODE, departcode)
			READMYSQL_STR(DEPART_NAME,departname)
			READMYSQL_STR(LEVEL, level)
			READMYSQL_STR(CWBT, cwbt)
			READMYSQL_STR(NAME_CN,name)

			if (cnt++)
			{
				out << ",";
			}

			out << "{id:\"" << id << "\",cwbt:\"" << cwbt<<"\",ename:\""<<name<<"\",departid:\"" << departcode<<"\",departName:\""<<departname<<"\",grade:\"" << level<<"\"}";

	}
	out <<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//新增常规保养项 0x23
int eMOPMaintenanceSvc::NewRoutineMaintenanceItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::NewRoutineMaintenanceItem]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	string strDepartId=root.getv("departid", "");
	string strGrade=root.getv("grade", "");
	string strCWBT=root.getv("cwbt", "");
	string strName=root.getv("ename", "");

	string sequence="";
	GET_EMOP_SEQUENCE_STRING(WK,sequence)

		string workcardid="WKS"+strShipId+sequence;


	MySql* psql = CREATE_MYSQL;

	char sql[512]="";
	sprintf (sql, "INSERT INTO blm_emop_etl.t50_abc_cbwt_ship_equip_card(SHIPID,WORK_CARD_ID,DEPARTMENT_CODE,LEVEL,CWBT,NAME_CN) \
				  VALUES('%s','%s','%s','%s','%s','%s')",strShipId.c_str(),
				  workcardid.c_str(),strDepartId.c_str(),strGrade.c_str(),strCWBT.c_str(),strName.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//删除常规保养项 0x24
int eMOPMaintenanceSvc::DelRoutineMaintenanceItem(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::DelRoutineMaintenanceItem]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;



	char sql[512]="";
	sprintf (sql, "DELETE FROM blm_emop_etl.t50_abc_cbwt_ship_equip_card WHERE WORK_CARD_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";


	RELEASE_MYSQL_RETURN(psql, 0);
}
//常规保养签字 0x25
int eMOPMaintenanceSvc::SignRoutineMaintenanceItem(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::SignRoutineMaintenanceItem]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	int year=root.getv("year", 0);
	int month=root.getv("month", 0);
	int role=atoi(root.getv("role", "-1"));
	string content=root.getv("content", "");

	char timestr[20]="";
	time_t lt= time(NULL);
	struct tm *ptr = localtime(&lt);
	strftime(timestr,30,"%Y-%m-%d %H:%M:%S",ptr);

	MySql* psql = CREATE_MYSQL;

	//先判断是否存在
	int COUNT=0;
	char selsql[512]="";
	sprintf(selsql,"select count(1) AS COUNT from blm_emop_etl.T50_ABC_MAINTENANCE_SIGN where shipid='%s' and year=%d and month=%d",strShipId.c_str(),year,month);

	CHECK_MYSQL_STATUS(psql->Query(selsql)&&psql->NextRow(), 3);
	READMYSQL_INT(COUNT,COUNT,0);

	if(COUNT == 0)
	{
		char insertsql[512]="";
		sprintf(insertsql,"insert into blm_emop_etl.T50_ABC_MAINTENANCE_SIGN(shipid,year,month)values('%s',%d,%d)",strShipId.c_str(),year,month);

		CHECK_MYSQL_STATUS(psql->Execute(insertsql), 3);
	}


	string updsql="";
	char sql[512]="";

	switch (role)
	{
	case 0://轮机长签名
		updsql="UPDATE blm_emop_etl.T50_ABC_MAINTENANCE_SIGN SET CEO_ID='%s',CEO_DT='%s',CEO_OP='%s' WHERE SHIPID='%s' AND YEAR=%d AND MONTH=%d";
		break;
	case 1://大  副签名
		updsql="UPDATE blm_emop_etl.T50_ABC_MAINTENANCE_SIGN SET CO_ID='%s',CO_DT='%s',CO_OP='%s' WHERE SHIPID='%s' AND YEAR=%d AND MONTH=%d";
		break;
	case 2://电机员签名
		updsql="UPDATE blm_emop_etl.T50_ABC_MAINTENANCE_SIGN SET CEE_ID='%s',CEE_DT='%s',CEE_OP='%s' WHERE SHIPID='%s' AND YEAR=%d AND MONTH=%d";
		break;
	case 3://机务主管签名
		updsql="UPDATE blm_emop_etl.T50_ABC_MAINTENANCE_SIGN SET EM_ID='%s',EM_DT='%s',EM_OP='%s' WHERE SHIPID='%s' AND YEAR=%d AND MONTH=%d";
		break;
	case 4://海务主管签名
		updsql="UPDATE blm_emop_etl.T50_ABC_MAINTENANCE_SIGN SET MM_ID='%s',MM_DT='%s',MM_OP='%s' WHERE SHIPID='%s' AND YEAR=%d AND MONTH=%d";
		break;
	case 5: //电气主管签名
		updsql="UPDATE blm_emop_etl.T50_ABC_MAINTENANCE_SIGN SET EEM_ID='%s',EEM_DT='%s',EEM_OP='%s' WHERE SHIPID='%s' AND YEAR=%d AND MONTH=%d";
		break;
	}

	sprintf(sql,updsql.c_str(),pUid,timestr,content.c_str(),strShipId.c_str(),year,month);

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}


//请求定时设备(子设备)运行记录(岸端) 0x26
int eMOPMaintenanceSvc::GetEquipRunRecord_Shore(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetEquipRunRecord_Shore]bad format:", jsonString, 1);

	//只取定时保养的设备

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	string strEquipId=root.getv("eid", "");
	int year=root.getv("year", 0);
	int month=root.getv("month", 0);

	MySql* psql = CREATE_MYSQL;

	if (strShipId.empty())
	{
		out <<"{seq:\""<<strSeq<<"\",data:[]}";
		return 0;
	}

	map<string,EquipRunInfo> mapEquipInfo;

	char sql[1024]="";
	if (strEquipId.empty()) //请求主设备 定时设备
	{
		sprintf(sql,"SELECT T1.EQUIP_ID,T1.CWBT,T1.NAME_CN,T1.NAME_EN,T3.UPDATE_DT,T3.BEFORE_HRS,T3.AFTER_HUR,T4.START_DATE \
					FROM blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD T1 \
					LEFT JOIN blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL_MONTH_DETAIL T3 ON T1.EQUIP_ID=T3.EQUIP_ID AND T3.YEAR=%d AND T3.MONTH=%d \
					LEFT JOIN blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL t4 ON T4.EQUIP_ID=T1.EQUIP_ID AND T4.VALID_FLAG=1 \
					WHERE t4.VALID_FLAG=1 and  T1.SHIPID='%s' AND T1.MAIN_FLAG=1",year,month,strShipId.c_str());
	}
	else //请求子设备
	{
		sprintf(sql,"SELECT T1.EQUIP_ID,T1.CWBT,T1.NAME_CN,T1.NAME_EN,T3.UPDATE_DT,T3.BEFORE_HRS,T3.AFTER_HUR,T4.START_DATE \
					FROM blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD T1 \
					LEFT JOIN blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL_MONTH_DETAIL T3 ON T3.EQUIP_ID=T1.EQUIP_ID AND T3.YEAR=%d AND T3.MONTH=%d \
					LEFT JOIN blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL t4 ON T4.EQUIP_ID=T1.EQUIP_ID AND T4.VALID_FLAG=1 \
					WHERE t4.VALID_FLAG=1 and  T1.SHIPID='%s' AND T1.MAIN_CWBT='%s' ",year,month,strShipId.c_str(),strEquipId.c_str());
	}
	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int before_hrs=0,after_hur=0;

	stringstream euqipStr;
	int idx=0;
	while(psql->NextRow())
	{
		EquipRunInfo EquipItem;
		READMYSQL_STRING(EQUIP_ID, EquipItem.equipId)
			READMYSQL_STRING(CWBT, EquipItem.cwbt)
			READMYSQL_STRING(NAME_CN, EquipItem.equipName)

			if (EquipItem.equipName.empty())
			{
				READMYSQL_STRING(NAME_EN, EquipItem.equipName)
			}

			READMYSQL_STRING(UPDATE_DT,EquipItem.recorddt)
				READMYSQL_STRING(START_DATE,EquipItem.startdt)
				READMYSQL_INT(BEFORE_HRS,before_hrs,0)
				READMYSQL_INT(AFTER_HUR,after_hur,0) 

				EquipItem.runtm_curmonth=before_hrs+after_hur;
			mapEquipInfo[EquipItem.equipId]=EquipItem;

			if (idx>0)
			{
				euqipStr<<",";
			}
			++idx;
			euqipStr<<"'"+EquipItem.equipId+"'";

	}

	if (euqipStr.str().length()>0)
	{
		char sql1[10240]="";
		sprintf(sql1,"select EQUIP_ID,SUM(BEFORE_HRS) AS TOTAL_BEFORE,SUM(AFTER_HUR) AS TOTAL_AFTER from blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL_MONTH_DETAIL \
					 where EQUIP_ID IN(%s) GROUP BY EQUIP_ID",euqipStr.str().c_str());

		DEBUG_LOG(sql1);
		CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

		while(psql->NextRow())
		{
			string eqid="";
			long total_before=0,totalafter=0;
			READMYSQL_STRING(EQUIP_ID, eqid)
				READMYSQL_INT(TOTAL_BEFORE,total_before,0)
				READMYSQL_INT(TOTAL_AFTER,totalafter,0)

				if (mapEquipInfo.find(eqid)!=mapEquipInfo.end())
				{
					long total=total_before+totalafter;
					mapEquipInfo[eqid].runtm_total+=total;
				}
		}
	}

	out <<"{seq:\""<<strSeq<<"\",data:[";	


	for (map<string,EquipRunInfo>::iterator it=mapEquipInfo.begin();it!=mapEquipInfo.end();it++)
	{
		if (it!=mapEquipInfo.begin())
		{
			out <<",";
		}

		EquipRunInfo equipItem=it->second;

		out << "{id:\"" << equipItem.equipId << "\",cwbt:\"" << equipItem.cwbt<<"\",cn:\""<<equipItem.equipName<<"\",nt:"; 
		out <<equipItem.runtm_curmonth<<",tt:" << equipItem.runtm_total<<",sd:\"" << equipItem.startdt<<"\",rd:\"" << equipItem.recorddt<<"\"}";

	}

	out <<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//请求主设备运行时间记录 0x27
int eMOPMaintenanceSvc::GetMainEquipTimeRecord(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetMainEquipTimeRecord]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	if (strId.empty())
	{
		out <<"{seq:\""<<strSeq<<"\",data:[]}";
		return 0;
	}

	char sql[512] = "";
	sprintf (sql, "select YEAR,MONTH,BEFORE_HRS,AFTER_HUR,UPDATE_DT from blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL_MONTH_DETAIL WHERE EQUIP_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	int year=0,month=0;
	int beforehrs=0;
	int afterhur=0;
	char updt[20]="";
	char date[20]="";

	int cnt = 0;

	while (psql->NextRow())
	{
		READMYSQL_INT(YEAR,year,0)
			READMYSQL_INT(MONTH,month,0)
			sprintf(date,"%4d-%02d",year,month);

		READMYSQL_INT(BEFORE_HRS,beforehrs,0)
			READMYSQL_INT(AFTER_HUR,afterhur,0)
			READMYSQL_STR(UPDATE_DT, updt)

			int hours=beforehrs+afterhur;

		if (cnt++)
		{
			out << ",";
		}
		out << "{rd:\"" << date << "\",rt:" << hours<<"}";
	}

	out << "]}";


	RELEASE_MYSQL_RETURN(psql, 0);

}
//请求子设备各级保养计划 0x28
int eMOPMaintenanceSvc::GetSubEquipMaintenancePlan(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetSubEquipMaintenancePlan]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	if (strId.empty())
	{
		out <<"{seq:\""<<strSeq<<"\",data:[]}";
		return 0;
	}

	char sql[512] = "";
	sprintf (sql, "select T1.LEVEL,T1.OWNER,T2.MANTENANCE_ID,T2.SCHEDULED_YEAR,T2.SCHEDULED_MON,T2.STATUS,T2.FINISHED_DATE,T2.TIMING \
				  from blm_emop_etl.T50_EMOP_WORK_CARD T1, blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE T2 \
				  where T1.EQUIP_ID='%s' AND T1.WORK_CARD_ID=T2.WORK_CARD_ID",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	int year=0,month=0;
	char id[64]="";
	int status=-1;
	char finishdate[20]="";
	char plandate[20]="";
	long timing=0;
	char level[4]="";
	char dutyman[128]="";

	int cnt = 0;

	while (psql->NextRow())
	{
		READMYSQL_INT(SCHEDULED_YEAR,year,0)
			READMYSQL_INT(SCHEDULED_MON,month,0)
			sprintf(plandate,"%4d-%02d",year,month);

		READMYSQL_STR(MANTENANCE_ID, id)
			READMYSQL_INT(STATUS,status,-1)
			READMYSQL_STR(FINISHED_DATE, finishdate)
			READMYSQL_INT(TIMING,timing,0)

			READMYSQL_STR(LEVEL, level)
			READMYSQL_STR(OWNER, dutyman)

			if (cnt++)
			{
				out << ",";
			}
			out << "{pid:\"" << id << "\",pd:\"" << plandate<<"\",wd:\"" << finishdate;
			out	<<"\",grade:\"" << level<<"\",pduty:\"" << dutyman<<"\",td:" << timing<<",st:" << status<<"}";
	}

	out << "]}";


	RELEASE_MYSQL_RETURN(psql, 0);

}
//子设备累计运行时间 0x29
int eMOPMaintenanceSvc::GetSubEquipRunTime(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetSubEquipRunTime]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string equipId=root.getv("id", "");
	MySql* psql = CREATE_MYSQL;


	char sql[1024]="";

	//只取正在执行的计划
	sprintf(sql,"SELECT T1.LEVEL,T1.PERIOD,T1.TOLERANCE,T1.OWNER,T2.MANTENANCE_ID,T2.TIMING,T2.SCHEDULED_YEAR,T2.SCHEDULED_MON,T2.FINISHED_DATE \
				FROM blm_emop_etl.T50_EMOP_WORK_CARD T1,blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE T2 \
				WHERE T1.EQUIP_ID='%s' AND T1.WORK_CARD_ID=T2.WORK_CARD_ID AND T2.STATUS=2",equipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	long period=0,differ=0;
	char id[64]="";
	char plandate[20]="";
	long timing=0;
	char level[4]="";
	char dutyman[128]="";
	char pms[128]="";

	int cnt = 0;

	while (psql->NextRow())
	{

		READMYSQL_STR(MANTENANCE_ID, id)
			READMYSQL_STR(LEVEL, level)
			READMYSQL_INT(PERIOD,period,0)
			READMYSQL_INT(TOLERANCE,differ,0)
			READMYSQL_STR(OWNER, dutyman)
			READMYSQL_INT(TIMING,timing,0)

			if (cnt++)
			{
				out << ",";
			}
			out << "{pms:\"" << pms <<"\",planid:\"" << id <<"\",differ:" << differ<<",grade:\"" << level<<"\",pduty:\"" << dutyman<<"\",circle:" <<period<<",rt:" << timing<<"}";

	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//请求定时设备(子设备)当前记录(船端) 0x2a
int eMOPMaintenanceSvc::GetEquipRunRecord_Ship(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetEquipRunRecord_Ship]bad format:", jsonString, 1);

	//只取定时保养的设备
	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	string strEquipId=root.getv("eid", "");
	int year=root.getv("year", 0);
	int month=root.getv("month", 0);

	MySql* psql = CREATE_MYSQL;

	if (strShipId.empty())
	{
		out <<"{seq:\""<<strSeq<<"\",data:[]}";
		return 0;
	}

	map<string,EquipRunInfo> mapEquipInfo;

	char sql[1024]="";
	if (strEquipId.empty()) //请求主设备 定时设备
	{
		sprintf(sql,"SELECT T1.EQUIP_ID,T1.CWBT,T1.NAME_CN,T1.NAME_EN,T3.UPDATE_DT,T3.BEFORE_HRS,T3.AFTER_HUR \
					FROM blm_emop_etl.T50_EMOP_WORK_CARD T2,blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD T1 \
					LEFT JOIN blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL_MONTH_DETAIL t3 ON T3.EQUIP_ID=T1.EQUIP_ID AND T3.YEAR=%d AND T3.MONTH=%d \
					WHERE T1.SHIPID='%s' AND T1.MAIN_FLAG=1 AND T1.EQUIP_ID=T2.EQUIP_ID AND T2.TIMING_FLAG=2",year,month,strShipId.c_str());
	}
	else //请求子设备
	{
		sprintf(sql,"SELECT T1.EQUIP_ID,T1.CWBT,T1.NAME_CN,T1.NAME_EN,T3.UPDATE_DT,T3.BEFORE_HRS,T3.AFTER_HUR \
					FROM blm_emop_etl.T50_EMOP_WORK_CARD T2,blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD T1 \
					LEFT JOIN blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL_MONTH_DETAIL t3 ON T3.EQUIP_ID=T1.EQUIP_ID AND T3.YEAR=%d AND T3.MONTH=%d \
					WHERE T1.SHIPID='%s' AND T1.MAIN_CWBT='%s' AND T1.EQUIP_ID=T2.EQUIP_ID AND T2.TIMING_FLAG=2",year,month,strShipId.c_str(),strEquipId.c_str());
	}


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	stringstream euqipStr;

	int idx=0;
	while(psql->NextRow())
	{
		EquipRunInfo EquipItem;
		READMYSQL_STRING(EQUIP_ID, EquipItem.equipId)
			READMYSQL_STRING(CWBT, EquipItem.cwbt)
			READMYSQL_STRING(NAME_CN, EquipItem.equipName)

			if (EquipItem.equipName.empty())
			{
				READMYSQL_STRING(NAME_EN, EquipItem.equipName)
			}

			READMYSQL_STRING(UPDATE_DT,EquipItem.recorddt)
				READMYSQL_INT(BEFORE_HRS,EquipItem.runtm_before,0)
				READMYSQL_INT(AFTER_HUR,EquipItem.runtm_after,0) 

				mapEquipInfo[EquipItem.equipId]=EquipItem;

			if (idx>0)
			{
				euqipStr<<",";
			}
			euqipStr<<"'"+EquipItem.equipId+"'";

	}

	if (euqipStr.str().length()>0)
	{
		char sql1[4096]="";
		sprintf(sql1,"select EQUIP_ID,SUM(BEFORE_HRS) AS TOTAL_BEFORE,SUM(AFTER_HUR) AS TOTAL_AFTER from blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL_MONTH_DETAIL \
					 where EQUIP_ID IN(%s) GROUP BY EQUIP_ID",euqipStr.str().c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

		while(psql->NextRow())
		{
			string eqid="";
			long total_before=0,totalafter=0;
			READMYSQL_STRING(EQUIP_ID, eqid)
				READMYSQL_INT(TOTAL_BEFORE,total_before,0)
				READMYSQL_INT(TOTAL_AFTER,totalafter,0)

				if (mapEquipInfo.find(eqid)!=mapEquipInfo.end())
				{
					long total=total_before+totalafter;
					mapEquipInfo[eqid].runtm_total+=total;
				}
		}
	}



	out <<"{seq:\""<<strSeq<<"\",data:[";	


	for (map<string,EquipRunInfo>::iterator it=mapEquipInfo.begin();it!=mapEquipInfo.end();it++)
	{
		if (it!=mapEquipInfo.begin())
		{
			out <<",";
		}

		EquipRunInfo equipItem=it->second;

		out << "{id:\"" << equipItem.equipId << "\",cwbt:\"" << equipItem.cwbt<<"\",cn:\""<<equipItem.equipName<<"\",bt:"; 
		out <<equipItem.runtm_before<<",at:" << equipItem.runtm_after<<",tt:"<< equipItem.runtm_total<<",rd:\""<< equipItem.recorddt<<"\"}";

	}

	out <<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);


}
//设置定时设备(子设备)运行时间(船端) 0x2b
int eMOPMaintenanceSvc::SetEquipRunRecord_Ship(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::SetEquipRunRecord_Ship]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	string strEquipId=root.getv("eid", "");
	int year=root.getv("year", 0);
	int month=root.getv("month", 0);
	string rdate=root.getv("rdate", "");
	int before_tm=root.getv("bt",0);
	int after_tm=root.getv("at",0);


	MySql* psql = CREATE_MYSQL;


	char sql[512]="";

	sprintf(sql,"REPLACE INTO blm_emop_etl.T50_EMOP_SHIP_EQUIP_MODEL_MONTH_DETAIL(EQUIP_ID,YEAR,MONTH,BEFORE_HRS,AFTER_HUR,UPDATE_DT) \
				VALUES('%s',%d,%d,%d,%d,'%s')",strEquipId.c_str(),year,month,before_tm,after_tm,rdate.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//计划调整历史 0x2c
int eMOPMaintenanceSvc::GetWorkPlanChangeHistory(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetWorkPlanChangeHistory]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	MySql* psql = CREATE_MYSQL;


	char sql[1024]="";

	//只取正在执行的计划
	sprintf(sql,"select T1.MODIFY_ID,T1.MANTENANCE_ID,T1.WORK_CARD_ID,T1.SCHEDULED_YEAR,T1.SCHEDULED_MON,T1.MODIFIED_YEAR,\
				T1.MODIFIED_MON,T1.MODIFIED_REASON,T1.OP_DT,T3.EQUIP_ID,T3.CWBT,T3.NAME_CN,T3.NAME_EN,T2.LEVEL,T2.TIMING_FLAG \
				from blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE_HIS T1,blm_emop_etl.T50_EMOP_WORK_CARD T2 ,blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD T3 \
				where t3.SHIPID='%s' AND T3.EQUIP_ID=T2.EQUIP_ID AND T2.WORK_CARD_ID=T1.WORK_CARD_ID",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	

	char equipid[64]="";
	char wcardid[64]="";//工作卡ID
	char cwbt[64]="";
	char enm[128]="";
	char planid[64]="";
	int oldyear=0,oldmonth=0;
	int newyear=0,newmonth=0;
	char olddt[20]="";
	char newdt[20]="";
	string reason;
	char level[3]="";
	char mode[3]="";
	char upddt[20]="";
	char operatperson[128]="";

	int cnt = 0;

	while (psql->NextRow())
	{

		READMYSQL_STR(WORK_CARD_ID,wcardid)
			READMYSQL_STR(MANTENANCE_ID,planid)
			READMYSQL_INT(SCHEDULED_YEAR,oldyear,0)
			READMYSQL_INT(SCHEDULED_MON,oldmonth,0)
			READMYSQL_INT(MODIFIED_YEAR,newyear,0)
			READMYSQL_INT(MODIFIED_MON,newmonth,0)

			sprintf(olddt,"%d-%02d",oldyear,oldmonth);
		sprintf(newdt,"%d-%02d",newyear,newmonth);

		READMYSQL_STRING(MODIFIED_REASON, reason)
			READMYSQL_STR(EQUIP_ID, equipid)

			READMYSQL_STR(TIMING_FLAG,mode)
			READMYSQL_STR(OP_DT,upddt)
			READMYSQL_STR(LEVEL,level)
			READMYSQL_STR(CWBT,cwbt)

			READMYSQL_STR(NAME_CN,enm)

			if (strlen(enm)==0)
			{
				READMYSQL_STR(NAME_EN,enm)
			}


			if (cnt++)
			{
				out << ",";
			}
			out << "{equipid:\"" << equipid << "\",cwbt:\"" << cwbt<<"\",enm:\"" << enm<<"\",wcardid:\"" << wcardid;
			out <<"\",planid:\"" <<planid<< "\",oldm:\"" << olddt<<"\",newm:\"" << newdt<<"\",operator:\"" << operatperson;
			out <<"\",reason:\"" <<reason<< "\",grade:\"" << level<<"\",mode:\"" << mode<<"\",date:\"" << upddt<<"\"}";
	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//请求主系统代码
//0x30
int eMOPMaintenanceSvc::GetMainSysCode(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetMainSysCode]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT MAIN_SYS_CODE FROM blm_emop_etl.T50_EMOP_CWBT_MAIN_SYS");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";	
	char syscode[32]="";
	int cnt=0;
	while (psql->NextRow())
	{		
		READMYSQL_STR(MAIN_SYS_CODE, syscode)
			if(cnt++)
				out << ",";
		out<<"\""<<syscode<<"\"";
	}
	out <<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//请求某船某主系统下的CWBT
//0x31 {seq:”” ,shipid:””,msys:””}
//{seq:””,data:[{cwbt:””,cn:””,en:””,flag:0}，{}] }
int eMOPMaintenanceSvc::GetCWBT(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetCWBT]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipid = root.getv("shipid", "");
	string strMSys = root.getv("msys", "");
	//

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT CWBT,NAME_CN,NAME_EN,MAIN_FLAG FROM blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD where SHIPID='%s'",strShipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";
	char cwbt[32]="";
	char cn[32]="";
	char en[32]="";
	int flag=0;
	string strTmp="";
	int cnt=0;
	while (psql->NextRow())
	{		
		READMYSQL_STR(CWBT, cwbt)
			READMYSQL_STR(NAME_CN, cn)
			READMYSQL_STR(NAME_EN, en)
			READMYSQL_INT(MAIN_FLAG, flag,-1)
			strTmp=cwbt;
		if(strTmp.find(strMSys)==std::string::npos)
			continue;
		if(cnt++)
			out << ",";
		out<<"{cwbt:\""<<cwbt<<"\",cn:\""<<cn<<"\",en:\""<<en<<"\",flag:"<<flag<<"}";
	}
	out <<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//请求某船舶某部门的所有设备的工作卡计划
// 0x32
//{seq:””,shipid:””,departid:”” ,pdate:””, ptype:””, cwbt:””} 
//{seq:””,data:[{id:””,pd:”2013-09”,cd:”2012-12-26 03:00:00”,cwbt:””,dname:””,grade:””,pduty:””,cycle:0,run:0,flag:0,type:0,flow:{taskid:”51”,stepid:“1”,status:0}},{},{}……]}
int eMOPMaintenanceSvc::GetWCardplan(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaintenanceSvc::GetWCardplan]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipid=root.getv("shipid", "");
	string strDepartid=root.getv("departid", "");
	string strPdate=root.getv("pdate", "");// 附加条件1
	string strPtype=root.getv("ptype", "");// 附加条件2
	string strCwbt=root.getv("cwbt", "");// 附加条件3

	if(strShipid.empty())
		return 0;

	string strCon_ship,strCon_dpt,strCon_pdate,strCon_ptype,strCon_cwbt;
	strCon_ptype="";
	//船舶
	eMOPMaintenanceUtil::getShipSQLs(strShipid,strCon_ship);
	//部门
	eMOPMaintenanceUtil::getDepartSQLs(strDepartid,strCon_dpt);
	//cwbt
	eMOPMaintenanceUtil::getCWBT_SQLs(strCwbt,strCon_cwbt);
	eMOPMaintenanceUtil::getPlanDateSQLs(strPdate,strCon_pdate);
	eMOPMaintenanceUtil::getPlanTypeSQLs(strPtype,strCon_ptype);


	MySql* psql = CREATE_MYSQL;
	vector<WorkCardPlanInfo> vecPlanList;
	char sql[1024] = "";
	//	sprintf (sql, "SELECT T3.MANTENANCE_ID,T3.STATUS,T3.SCHEDULED_YEAR,T3.SCHEDULED_MON,T3.FINISHED_DATE,T3.TOTAL_TIMING,\
	T2.OWNER,T2.LEVEL,T2.PERIOD,T2.ATTACHMENT_REQUIRED,T1.CWBT,T1.NAME_CN FROM \
	blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD T1,\
	blm_emop_etl.T50_EMOP_WORK_CARD T2, \
	blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE T3 \
	WHERE T1.SHIPID='%s' and t1.DEPARTMENT_CODE='%s' AND T1.EQUIP_ID=T2.EQUIP_ID AND T2.WORK_CARD_ID=T3.WORK_CARD_ID \
	AND (T3.SCHEDULED_YEAR>=2013 OR T3.SCHEDULED_YEAR=0) ",strShipid.c_str(),strDepartid.c_str());
	sprintf (sql, "SELECT t4.DEPARTMENT_CODE as dptid,t4.NAME_CN as dptname,t3.TIMING,T3.MANTENANCE_ID,T3.STATUS,T3.SCHEDULED_YEAR,T3.SCHEDULED_MON,T3.FINISHED_DATE,T3.TOTAL_TIMING,t3.STATUS AS SCHEDULE_STATUS,\
				  T2.OWNER,T2.LEVEL,T2.PERIOD,T2.ATTACHMENT_REQUIRED,T1.CWBT,T1.NAME_CN FROM \
				  blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD T1 inner join\
				  blm_emop_etl.T50_EMOP_WORK_CARD T2 on T1.EQUIP_ID=T2.EQUIP_ID inner join \
				  blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE T3 on T2.WORK_CARD_ID=T3.WORK_CARD_ID inner join \
				  blm_emop_etl.t50_emop_department_code T4 on t1.DEPARTMENT_CODE=t4.DEPARTMENT_CODE \
				  WHERE %s %s %s %s %s",\
				  strCon_ship.c_str(),strCon_dpt.c_str(),strCon_cwbt.c_str(),strCon_pdate.c_str(),strCon_ptype.c_str());
	FILE *pf;
	pf=fopen("/tmp/6232.log","wb");
	fwrite(sql,sizeof(char),strlen(sql),pf);
	fclose(pf);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out <<"{seq:\""<<strSeq<<"\",data:[";

	char plandt[20]=""; //开始时间

	char taskid[64]="";
	char stepid[16]="";
	int planstatus=0;

	while (psql->NextRow())
	{
		WorkCardPlanInfo workPlanItem;

		READMYSQL_STRING(MANTENANCE_ID, workPlanItem.planid)
			READMYSQL_INT(STATUS,workPlanItem.status,0)
			READMYSQL_INT(SCHEDULED_YEAR,workPlanItem.year,0)
			READMYSQL_INT(SCHEDULED_MON,workPlanItem.month,0)

			if (workPlanItem.year>0&&workPlanItem.month>0)
			{
				sprintf(plandt,"%04d-%02d",workPlanItem.year,workPlanItem.month);
				workPlanItem.plandt=plandt;
			}


			READMYSQL_STRING(FINISHED_DATE, workPlanItem.finishdt)
				READMYSQL_INT(TOTAL_TIMING,workPlanItem.runtime,0)


				READMYSQL_STRING(OWNER, workPlanItem.pduty)
				READMYSQL_STRING(LEVEL, workPlanItem.grade)
				READMYSQL_INT(PERIOD, workPlanItem.period,0)
				READMYSQL_INT(ATTACHMENT_REQUIRED,workPlanItem.battach,0)

				READMYSQL_STRING(CWBT,workPlanItem.cwbt)
				READMYSQL_STRING(NAME_CN,workPlanItem.equipname)
				READMYSQL_STRING(dptname, workPlanItem.m_strDptname)
				READMYSQL_INT(TIMING,workPlanItem.timing,0)
				READMYSQL_STRING(dptid, workPlanItem.m_strDptid)
				READMYSQL_INT(SCHEDULE_STATUS,workPlanItem.status,0)

				vecPlanList.push_back(workPlanItem);

	}
	int cnt=0;
	for (int i=0;i<vecPlanList.size();i++)
	{
		WorkCardPlanInfo workPlanItem=vecPlanList[i];
		if(strPtype=="1")//待批复
		{
			int plantype=0;
			if (workPlanItem.year<=0||workPlanItem.month<=0)//未收排
			{
				continue;
			}

			CMTSteps stData;
			if(GetMTSteps(stData,workPlanItem.planid))
			{
				if (string(taskid)=="52"&&workPlanItem.status==1)//待批复计划
				{
					plantype=1;
					out<<"{taskid:\""<<taskid<<"\",stepid:\""<<stepid<<"\",status:"<<planstatus<<"}";
				}
				else
					continue;
			}	
			else 
				continue;

			if (cnt++)
				out << ",";

			out << "{id:\"" << workPlanItem.planid << "\",status:" << workPlanItem.status << ",pd:\"" << workPlanItem.plandt <<"\",cd:\"" << workPlanItem.finishdt << "\",cwbt:\"" << workPlanItem.cwbt <<"\",dname:\"" << workPlanItem.equipname;
			out	<<"\",grade:\"" << workPlanItem.grade<<"\",pduty:\"" << workPlanItem.pduty<<"\",cycle:" <<workPlanItem.period<<",run:" << workPlanItem.runtime<<",flag:" << workPlanItem.battach<<",flow:";

			//			out<<",dtname:\""<<workPlanItem.m_strDptname <<"\",crun:"<< workPlanItem.timing<<"}";
			out<<",dtname:\""<<workPlanItem.m_strDptname <<"\",crun:"<< workPlanItem.timing<<",dtId:\""<<workPlanItem.m_strDptid <<"\"}";
		}
		else
		{
			if (i>0)
			{
				out << ",";
			}

			out << "{id:\"" << workPlanItem.planid << "\",status:" << workPlanItem.status << ",pd:\"" << workPlanItem.plandt <<"\",cd:\"" << workPlanItem.finishdt << "\",cwbt:\"" << workPlanItem.cwbt <<"\",dname:\"" << workPlanItem.equipname;
			out	<<"\",grade:\"" << workPlanItem.grade<<"\",pduty:\"" << workPlanItem.pduty<<"\",cycle:" <<workPlanItem.period<<",run:" << workPlanItem.runtime;
			out <<",annex:[{axid:\"id-test-1\",axna:\"annexname-1\"},{axid:\"id-test-2\",axna:\"annexname-2\"}],flow:";

			int plantype=0;

			char sql1[512]="";

			sprintf (sql1, "select OPERATOR,OP_DATE,STATUS,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID \
						   from blm_emop_etl.T50_EMOP_MAINTENANCE_STEPS_STATUS WHERE MANTENANCE_ID='%s' ORDER BY OP_DATE DESC",workPlanItem.planid.c_str());

			psql->Query(sql1);

			if (psql->NextRow())
			{
				READMYSQL_STR(WORKFLOW_TASK_ID, taskid)
					READMYSQL_STR(WORKFLOW_STEP_ID, stepid)
					READMYSQL_INT(STATUS, planstatus,0)

					out<<"{taskid:\""<<taskid<<"\",stepid:\""<<stepid<<"\",status:"<<planstatus<<"}";
			}
			else
			{
				out<<"{taskid:\"\",stepid:\"\",status:0}";
			}


			if (workPlanItem.year==0||workPlanItem.month==0)//未收排
			{
				plantype=2;

			}
			else if (string(taskid)=="52"&&workPlanItem.status==1)//待批复计划
			{
				plantype=1;
			}
			out<<",dtname:\""<<workPlanItem.m_strDptname <<"\",crun:"<< workPlanItem.timing<<",dtId:\""<<workPlanItem.m_strDptid <<"\"}";
		}		
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
int eMOPMaintenanceSvc::LoadDevicecard()
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT t1.*,t2.NAME_CN as dptname_cn,t2.NAME_EN as dptname_en FROM blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD t1 left join t50_emop_department_code t2 on t1.DEPARTMENT_CODE=t2.DEPARTMENT_CODE");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	CDeviceCard* pData=NULL;
	while (psql->NextRow())
	{
		pData=new CDeviceCard;
		READMYSQL_STR(EQUIP_ID, pData->m_szId)
			READMYSQL_STR(CWBT, pData->m_szCwbt)
			READMYSQL_STR(SHIPID, pData->m_szShipid)
			READMYSQL_STR(DEPARTMENT_CODE, pData->m_szDptid)
			READMYSQL_STR(NAME_CN, pData->m_szEqname_cn)
			READMYSQL_STR(NAME_EN, pData->m_szEqname_en)
			READMYSQL_STR(dptname_cn, pData->m_szDptname)

			string strTmp=pData->m_szId;
		LoadWorkcard(pData->m_mapWorkcard,strTmp);
		m_mapDevicecard[strTmp]=pData;
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
int eMOPMaintenanceSvc::LoadWorkcard(map<string,WorkCard*> &o_mapWorkcard,string &i_strDeviceid)
{
	MySql* psql1 = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT *FROM blm_emop_etl.T50_EMOP_WORK_CARD where EQUIP_ID='%s'",i_strDeviceid.c_str());
	//	CHECK_MYSQL_STATUS(psql1->Query(sql), 3);
	if(!psql1->Query(sql)) {RELEASE_MYSQL_RETURN(psql1, 3)};
	WorkCard* pData=NULL;
	while (psql1->NextRow())
	{
		pData=new WorkCard;
		READMYSQL_STR1(WORK_CARD_ID, pData->m_szId,psql1)
			READMYSQL_STR1(LEVEL, pData->m_szLlevel,psql1)
			READMYSQL_STR1(OWNER, pData->m_szOwner,psql1)
			READMYSQL_STR1(EXCUTE_WAY, pData->m_szExcuteway,psql1)

			READMYSQL_INT1(STATUS, pData->m_iStatus,0,psql1)
			READMYSQL_INT1(TIMING_FLAG, pData->m_iTFlag,0,psql1)
			READMYSQL_INT1(PERIOD, pData->m_iPeriod,0,psql1)
			READMYSQL_INT1(TOLERANCE, pData->m_iTolerence,0,psql1)
			READMYSQL_INT1(ATTACHMENT_REQUIRED, pData->m_iAddFlag,0,psql1)
			READMYSQL_INT1(EXPIRE_DT, pData->m_iExpiredt,0,psql1)

			string strTmp=pData->m_szId;
		LoadPlan(pData->m_mapMTPlan,strTmp);
		o_mapWorkcard[strTmp]=pData;
	}
	RELEASE_MYSQL_RETURN(psql1, 0);
}
int eMOPMaintenanceSvc::LoadPlan(map<string,CMaintainPlan*> &o_mapMPlan,string &i_strWorkcardid)
{
	MySql* psql2 = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT *,UNIX_TIMESTAMP(FINISHED_DATE) as finishtm,YEAR(FINISHED_DATE) as finishyear,MONTH(FINISHED_DATE) as finishmon FROM blm_emop_etl.T50_EMOP_MAINTENANCE_SCHEDULE where WORK_CARD_ID='%s'",i_strWorkcardid.c_str());
	//	CHECK_MYSQL_STATUS(psql2->Query(sql), 3);
	if(!psql2->Query(sql)) {RELEASE_MYSQL_RETURN(psql2, 3)};
	CMaintainPlan* pData=NULL;
	while (psql2->NextRow())
	{
		pData=new CMaintainPlan;
		READMYSQL_STR1(MANTENANCE_ID, pData->m_szId,psql2)

			READMYSQL_INT1(SCHEDULED_YEAR, pData->m_iScYear,0,psql2)
			READMYSQL_INT1(SCHEDULED_MON, pData->m_iScMon,0,psql2)
			READMYSQL_INT1(STATUS, pData->m_iStatus,0,psql2)
			READMYSQL_INT1(finishtm, pData->m_iFinishTime,0,psql2)
			READMYSQL_INT1(finishyear, pData->m_iFinishYear,0,psql2)
			READMYSQL_INT1(finishmon, pData->m_iFinishMon,0,psql2)
			READMYSQL_INT1(TIMING, pData->m_iTiming,0,psql2)
			READMYSQL_INT1(TOTAL_TIMING, pData->m_iTotalTiming,0,psql2)

			o_mapMPlan[string(pData->m_szId)]=pData;
	}
	RELEASE_MYSQL_RETURN(psql2, 0);
}
int eMOPMaintenanceSvc::LoadMTSteps()
{
	MySql* psql2 = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT MANTENANCE_ID,OPERATOR,OP_DATE,STATUS,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID FROM blm_emop_etl.T50_EMOP_MAINTENANCE_STEPS_STATUS  ORDER BY OP_DATE DESC");
	//	CHECK_MYSQL_STATUS(psql2->Query(sql), 3);
	if(!psql2->Query(sql)) {RELEASE_MYSQL_RETURN(psql2, 3)};
	CMTSteps* pData=NULL;
	while (psql2->NextRow())
	{
		pData=new CMTSteps;
		READMYSQL_STR1(MANTENANCE_ID, pData->m_szMtid,psql2)
			READMYSQL_STR1(WORKFLOW_TASK_ID, pData->m_szWktaskid,psql2)
			READMYSQL_STR1(WORKFLOW_STEP_ID, pData->m_szWkstepid,psql2)
			READMYSQL_INT1(STATUS, pData->m_iStatus,0,psql2)

			m_vecSteps.push_back(pData);
	}
	RELEASE_MYSQL_RETURN(psql2, 0);
}

void eMOPMaintenanceSvc::CleanMTSteps()
{
	int iSize=m_vecSteps.size();
	for(int i=0;i<iSize;i++)
	{
		delete m_vecSteps.at(i);
	}
	m_vecSteps.clear();
}
bool eMOPMaintenanceSvc::GetMTSteps(CMTSteps &o_stData,string &i_strIndex)
{
	bool bRlt=false;
	int iSize=m_vecSteps.size();
	for(int i=0;i<iSize;i++)
	{
		if(m_vecSteps.at(i)->m_szMtid==i_strIndex)
		{
			bRlt=true;
			o_stData=*m_vecSteps.at(i);
			break;
		}
	}
	return bRlt;
}
// 待办工作
int eMOPMaintenanceSvc::GetUnfinishedTasks(const char* pUid, const char* jsonString, std::stringstream& out)
{
	return 1;
}
// 日历模式(待办工作)





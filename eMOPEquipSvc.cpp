#include "blmcom_head.h"
#include "eMOPEquipSvc.h"
#include "UserRegSvc.h"
#include "MessageService.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include "NotifyService.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "eMopCommDataSvr.h"
#include "SequenceManager.h"

using namespace std;


IMPLEMENT_SERVICE_MAP(eMOPEquipSvc)

eMOPEquipSvc::eMOPEquipSvc()
{
	//	std::stringstream out;
	//	SearchAppOrder("1"," {seq:\"apply_search\",ship:\"0001022\",part:\"\",state:0,taskId:\"11\",stepId:\"1|\"}",out);
}

eMOPEquipSvc::~eMOPEquipSvc()
{

}

bool eMOPEquipSvc::Start()
{
	if(!g_MessageService::instance()->RegisterCmd(MID_EMOP_EQUIP, this))
		return false;

	int interval = 60;
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
	if(timerId <= 0)
		return false;

	SERVICE_MAP(0x01,eMOPEquipSvc,getAllEquipment);
	SERVICE_MAP(0x02,eMOPEquipSvc,GetEquipPara);
	SERVICE_MAP(0x03,eMOPEquipSvc,NewEquipment);
	SERVICE_MAP(0x04,eMOPEquipSvc,DelEquip);
	SERVICE_MAP(0x05,eMOPEquipSvc,GetEquipParts);
	SERVICE_MAP(0x06,eMOPEquipSvc,GetPartsPara);
	SERVICE_MAP(0x07,eMOPEquipSvc,NewParts);
	SERVICE_MAP(0x08,eMOPEquipSvc,DelParts);
	SERVICE_MAP(0x09,eMOPEquipSvc,GetPartsOrderHis);

	SERVICE_MAP(0x10,eMOPEquipSvc,GetPartsStoreHis);
	SERVICE_MAP(0x1a,eMOPEquipSvc,GetPartsInstock);
	SERVICE_MAP(0x1b,eMOPEquipSvc,SetStorePos);//
	SERVICE_MAP(0x1c,eMOPEquipSvc,SearchPartsOnShip);
	SERVICE_MAP(0x1d,eMOPEquipSvc,GetWarndata);
	SERVICE_MAP(0x1f,eMOPEquipSvc,GetPaths);
	SERVICE_MAP(0x20,eMOPEquipSvc,NewApplyOrder);
	SERVICE_MAP(0x21,eMOPEquipSvc,SearchAppOrder);
	SERVICE_MAP(0x22,eMOPEquipSvc,GetAppOrderDetails);
	SERVICE_MAP(0x23,eMOPEquipSvc,ModifyAppOrder);
	SERVICE_MAP(0x24,eMOPEquipSvc,AppOrderOpt);
	SERVICE_MAP(0x2b,eMOPEquipSvc,GetApporderNum);
	SERVICE_MAP(0x2c,eMOPEquipSvc,GetWorkfllow);
	SERVICE_MAP(0x29,eMOPEquipSvc,MkInquirySheete);
	SERVICE_MAP(0x2a,eMOPEquipSvc,GetAllPartsCorps);

	SERVICE_MAP(0x41,eMOPEquipSvc,GetInquirySheeteList);//
	SERVICE_MAP(0x42,eMOPEquipSvc,GetInquirySheetDetail);
	SERVICE_MAP(0x43,eMOPEquipSvc,GetAppOrderDetail_Iqsid);
	SERVICE_MAP(0x44,eMOPEquipSvc,GetIqsHis_Partsid);
	SERVICE_MAP(0x45,eMOPEquipSvc,GetIqsList_Appo);
	SERVICE_MAP(0x46,eMOPEquipSvc,GetPartsList_Iqs);
	SERVICE_MAP(0x47,eMOPEquipSvc,GetPartsList_Appo);
	SERVICE_MAP(0x48,eMOPEquipSvc,GetIqsList_Partsid);
	SERVICE_MAP(0x49,eMOPEquipSvc,GetAppoList_Partsid);
	SERVICE_MAP(0x4a,eMOPEquipSvc,DeleteIqs);
	SERVICE_MAP(0x4b,eMOPEquipSvc,CompleteIqs);
	SERVICE_MAP(0x4c,eMOPEquipSvc,MkPurchaseSheete);
	SERVICE_MAP(0x25,eMOPEquipSvc,SearchPurchase);
	SERVICE_MAP(0x26,eMOPEquipSvc,GetPurchaseDetail);
	SERVICE_MAP(0x27,eMOPEquipSvc,ModifyPurchase);
	SERVICE_MAP(0x28,eMOPEquipSvc,GetPchStatusCount);
	SERVICE_MAP(0x2d,eMOPEquipSvc,PurchaseOpt);
	SERVICE_MAP(0x2e,eMOPEquipSvc,GetPartsHis);


	SERVICE_MAP(0x50,eMOPEquipSvc,GetInstockOrderList);
	SERVICE_MAP(0x51,eMOPEquipSvc,GetInstockOrderPartList);
	SERVICE_MAP(0x52,eMOPEquipSvc,DeleteInstockOrderPartItem);
	/*SERVICE_MAP(0x53,eMOPEquipSvc,ConformPartInstockOrder);*/
	SERVICE_MAP(0x53,eMOPEquipSvc,GetEquipListInfo);
	SERVICE_MAP(0x54,eMOPEquipSvc,GetPurchaseOrderList);
	/*SERVICE_MAP(0x55,eMOPEquipSvc,GetPurchaseOrderPartList);*/
	SERVICE_MAP(0x55,eMOPEquipSvc,GetApplyInqueryInfo);
	SERVICE_MAP(0x56,eMOPEquipSvc,GetApplyOrdersInfo);
	/*SERVICE_MAP(0x56,eMOPEquipSvc,SetPartInstockOrder);*/
	SERVICE_MAP(0x57,eMOPEquipSvc,GetDetailApplyInfo);
	/*SERVICE_MAP(0x58,eMOPEquipSvc,GetOutstockOrderPartList);*/
	SERVICE_MAP(0x58,eMOPEquipSvc,GetIqsInfo);
	SERVICE_MAP(0x59,eMOPEquipSvc,GetIqsEquipList);
	/*SERVICE_MAP(0x59,eMOPEquipSvc,SetPartOutstockOrder);*/
	SERVICE_MAP(0x5a,eMOPEquipSvc,GetPurchaseEquipList);
	//SERVICE_MAP(0x5a,eMOPEquipSvc,DeleteOutstockOrderPartItem);
	SERVICE_MAP(0x5b,eMOPEquipSvc,ConformPartOutstockOrder);
	SERVICE_MAP(0x5c,eMOPEquipSvc,EquipPurchaseCheck);

	SERVICE_MAP(0x60,eMOPEquipSvc,GetPartsInventoryCounting);
	SERVICE_MAP(0x61,eMOPEquipSvc,SetPartsInventoryCount);
	SERVICE_MAP(0x62,eMOPEquipSvc,GetPartsMonthInOutStatistic);

	SERVICE_MAP(0x71,eMOPEquipSvc,ApplySearch);
	SERVICE_MAP(0x72,eMOPEquipSvc,EditApllyInfo);
	SERVICE_MAP(0x73,eMOPEquipSvc,EditCheckNum);
	SERVICE_MAP(0x74,eMOPEquipSvc,GetCheckList);
	SERVICE_MAP(0x75,eMOPEquipSvc,DelApplyInfo);
	SERVICE_MAP(0x76,eMOPEquipSvc,DelEquipFile);
	SERVICE_MAP(0x77,eMOPEquipSvc,GetWorkFlowInfo);
	SERVICE_MAP(0x78,eMOPEquipSvc,CreateCheckList);
	SERVICE_MAP(0x79,eMOPEquipSvc,GetCheckInfo);

	DEBUG_LOG("[eMOPEquipSvc::Start] OK......................................");
	//std::stringstream out;
	//delEqPict("libh", "{eqid:\"EQ00031\",picd:\"00001261.png\"}",out);
	return true;
}
int eMOPEquipSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	
	return 0;
}

int eMOPEquipSvc::GetEquipModelIdByCode(string &name,string &outModelId){

	MySql* psql = CREATE_MYSQL; 
	char sql[512]="";
	sprintf (sql, "SELECT EQUIP_MODEL_ID FROM blm_emop_etl.T50_EMOP_EQUIP_MODEL WHERE MODE='%s';",name.c_str());  
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow()){
		READMYSQL_STRING(EQUIP_MODEL_ID,outModelId);
	}
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

int eMOPEquipSvc::AddEquipModel(string &name,string &newId)
{
	MySql* psql = CREATE_MYSQL; 
	char sql[512]="";
	time_t t = time(NULL);
	tm* aTm = localtime(&t); 
	char idchart[50]="";
	
	sprintf (sql, "SELECT MAX(EQUIP_MODEL_NUM)+1 AS NEW_MODEL_NUM FROM (SELECT SUBSTRING(EQUIP_MODEL_ID,10) AS EQUIP_MODEL_NUM  \
				  FROM blm_emop_etl.T50_EMOP_EQUIP_MODEL WHERE EQUIP_MODEL_ID LIKE 'EM%04d%02d%02%%') t;",aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday);  

	int maxNum=0;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow()){
		READMYSQL_INT(NEW_MODEL_NUM,maxNum,0);
	} 
	sprintf(idchart,"EM%04d%02d%02%06d", aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday,maxNum); 
	sprintf (sql, "INSERT INTO blm_emop_etl.T50_EMOP_EQUIP_MODEL(EQUIP_MODEL_ID,MODE) VALUES('%s','%s');",idchart,name.c_str());   
	CHECK_MYSQL_STATUS(psql->Execute(sql), 3);
	newId = idchart;
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//获取备件入库单列表
int eMOPEquipSvc::ApplySearch(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::ApplySearch]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strShips=root.getv("ships", "");
	string strDepart=root.getv("depart", ""); 
	string strTaskId=root.getv("taskId", ""); 
	string strStepId=root.getv("stepId", ""); 
	string strStatus=root.getv("status", ""); 
	MySql* psql = CREATE_MYSQL;


	char append[1024]="";
	string appendTemp="";

	//搜索满足条件的申请单状态
	if(!strTaskId.empty())
		sprintf(append, "WHERE WORKFLOW_TASK_ID='%s' ",strTaskId.c_str());

	if(!strTaskId.empty() && !strStepId.empty())
	{
		appendTemp = append;
		sprintf(append, "%s AND WORKFLOW_STEP_ID='%s'",appendTemp.c_str(),strStepId.c_str());
	}

	if(!strTaskId.empty() && !strStepId.empty() && !strStatus.empty())
	{
		appendTemp = append;
		sprintf(append, "%s AND STATUS=%s",appendTemp.c_str(),strStatus.c_str());
	}

	char sql[2048] = ""; 
	sprintf(sql, "SELECT PARTS_APPLY_ID,MAX(UNIX_TIMESTAMP(OP_DATE)) AS  OP_DATE \
				 FROM blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS  %s GROUP BY PARTS_APPLY_ID",append);
	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	map<string,int> applyTimeSet;
	string applyId;
	int time=0;
	while (psql->NextRow())
	{ 
		READMYSQL_STRING(PARTS_APPLY_ID, applyId);
		READMYSQL_INT(OP_DATE, time, 0);

		applyTimeSet[applyId]=time;
	}

	//搜索满足条件的申请单 
	string shipIds = "'"+StrReplace(strShips,"|","','")+"'";
	char append2[1024]="";
	DEBUG_LOG(shipIds.c_str()); 
	if(!strDepart.empty())
		sprintf(append2,"AND t_apply.DEPARTMENT_CODE='%s'",strDepart.c_str());
	DEBUG_LOG(append2); 
	sprintf(sql, "SELECT t_apply.PARTS_APPLY_ID,UNIX_TIMESTAMP(t_apply.DELIVERY_DATE) AS DELIVERY_TIME,t_apply.DELIVERY_LOCATION,t_apply.DELIVERY_WAY,t_apply.REASON, \
				 t_ship.SHIPID,t_ship.NAME_CN AS SHIP_NAME, \
				 t_depart.DEPARTMENT_CODE,t_depart.NAME_CN AS DEPART_NAME, \
				 t_status.WORKFLOW_TASK_ID,t_status.WORKFLOW_STEP_ID,t_status.STATUS,t_status.OPERATOR,t_status.REMARK,UNIX_TIMESTAMP(t_status.OP_DATE) as OP_DATE\
				 FROM blm_emop_etl.T50_EMOP_PARTS_APPLIES t_apply \
				 JOIN blm_emop_etl.T41_EMOP_SHIP t_ship ON t_ship.SHIPID=t_apply.SHIPID \
				 JOIN blm_emop_etl.T50_EMOP_DEPARTMENT_CODE t_depart ON t_depart.DEPARTMENT_CODE=t_apply.DEPARTMENT_CODE \
				 LEFT JOIN blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS t_status ON t_apply.PARTS_APPLY_ID=t_status.PARTS_APPLY_ID \
				 WHERE t_apply.SHIPID IN (%s) %s",shipIds.c_str(),append2);
	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
 
	int deliveryTime=0;
	string deliveryLoc="";
	string deliveryWay="";
	string shipId="";
	string shipName="";
	string departId="";
	string departName="";
	string tasdId="";
	string stepId="";
	string operator_p="";
	string remark="";
	string reason="";
	int optime=0;
	int status=0;
	int op_time=0;
	//{seq:””,datas:[{id:””,sid:””,sn:””,did:””,dn:””,tid:””,pid:””,st:””,lt:1212,lp:””,lc:””,gd:””,gw:””,ga:””,rs:””},{},....]}
	int idx=0;
	out<<"{seq:\""<<strSeq<<"\",datas:[";

	while(psql->NextRow()){   

		READMYSQL_STRING(PARTS_APPLY_ID,applyId);
		READMYSQL_INT(OP_DATE,op_time,0);

		//所有状态，直接跳过
		if(!strTaskId.empty()){
			map<string,int>::iterator it=applyTimeSet.find(applyId);
			if(it !=applyTimeSet.end()){
				if(it->second!=op_time)
					continue; //状态不是最后一次更改
			}
		}

		if(idx>0)
			out<<","; 
		++idx; 

		READMYSQL_INT(DELIVERY_TIME,deliveryTime,0);
		READMYSQL_STRING(deliveryLoc,deliveryLoc);
		READMYSQL_STRING(DELIVERY_WAY,deliveryWay);
		READMYSQL_STRING(SHIPID,shipId);
		READMYSQL_STRING(SHIP_NAME,shipName);
		READMYSQL_STRING(DEPARTMENT_CODE,departId);
		READMYSQL_STRING(DEPART_NAME,departName);
		READMYSQL_STRING(WORKFLOW_TASK_ID,tasdId);
		READMYSQL_STRING(WORKFLOW_STEP_ID,stepId);
		READMYSQL_STRING(OPERATOR,operator_p);
		READMYSQL_STRING_JSON(REMARK,remark);
		READMYSQL_INT(STATUS,status,0); 
		READMYSQL_STRING(REASON,reason);

		out<<"{id:\""<<applyId<<"\",sid:\""<<shipId<<"\",sn:\""<<shipName<<"\",did:\""<<departId<<"\",dn:\""<<departName<<"\",";
		out<<"tid:\""<<tasdId<<"\",pid:\""<<stepId<<"\",st:\""<<status<<"\",lt:"<<op_time<<",lp:\""<<operator_p<<"\",lc:\""<<remark<<"\",";
		out<<"gd:\""<<deliveryTime<<"\",gw:\""<<deliveryWay<<"\",ga:\""<<deliveryLoc<<"\",rs:\""<<reason<<"\"}";
	} 
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//申请单详情
//0x6157
//{seq:"",id:""}
//{seq:””,id:””,shipId:””,shipName:””,departId:””,departName:””,uname””,atime:””,taskId:””,stepId:””,status:””,dtime:””,way:””,address:””,isUrgency:””,isCheck:””,reason:””}
int eMOPEquipSvc::GetDetailApplyInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetDetailApplyInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql,"SELECT MAX(UNIX_TIMESTAMP(OP_DATE)) AS OP_DATE \
				FROM blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS WHERE PARTS_APPLY_ID='%s' \
				",strId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int op_date=0;
	if(psql->NextRow())
	{
		READMYSQL_INT(OP_DATE,op_date,0);
	}

	//t1 = 备件申请表，t2 = 船舶, t3 = 部门 ,t4 = APPLY_STATUS表
	sprintf(sql, "SELECT t1.PARTS_APPLY_ID,t1.SHIPID,t2.NAME_CN AS SHIP_NAME,t1.DEPARTMENT_CODE,t3.NAME_CN AS DEPART_NAME, \
				 t1.USER_ID,t1.APPLY_DATE,t4.WORKFLOW_TASK_ID,t4.WORKFLOW_STEP_ID, \
				 t4.STATUS,t1.DELIVERY_DATE,t1.DELIVERY_WAY,t1.DELIVERY_LOCATION, \
				 t1.EMERGENCY_FLAG,t1.CS_CERTIFIED_FLAG,t1.REASON,UNIX_TIMESTAMP(t4.OP_DATE) AS OP_DATE \
				 FROM blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 \
				 JOIN blm_emop_etl.T41_EMOP_SHIP t2 \
				 ON t1.SHIPID=t2.SHIPID \
				 JOIN blm_emop_etl.T50_EMOP_DEPARTMENT_CODE t3 \
				 ON t1.DEPARTMENT_CODE=t3.DEPARTMENT_CODE \
				 LEFT JOIN blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS t4 \
				 ON t1.PARTS_APPLY_ID=t4.PARTS_APPLY_ID WHERE t1.PARTS_APPLY_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int opdate_final = 0;
	string parts_apply_id="";
	string shipid="";
	string ship_name="";
	string department_code="";
	string depart_name="";
	string user_id="";
	string apply_date="";
	string workflow_task_id="";
	string workflow_step_id="";
	string status="";
	string delivery_date="";
	string delivery_way="";
	string delivery_location="";
	string emergency_flag="";
	string cs_certified_flag="";
	string reason="";
	while(psql->NextRow())
	{
		READMYSQL_INT(OP_DATE,opdate_final,0);
		if(opdate_final != op_date)
		{
			continue;
		}
		READMYSQL_STRING(PARTS_APPLY_ID,parts_apply_id);
		READMYSQL_STRING(SHIPID,shipid);
		READMYSQL_STRING(SHIP_NAME,ship_name);
		READMYSQL_STRING(DEPARTMENT_CODE,department_code);
		READMYSQL_STRING(DEPART_NAME,depart_name);
		READMYSQL_STRING(USER_ID,user_id);
		READMYSQL_STRING(APPLY_DATE,apply_date);
		READMYSQL_STRING(WORKFLOW_TASK_ID,workflow_task_id);
		READMYSQL_STRING(WORKFLOW_STEP_ID,workflow_step_id);
		READMYSQL_STRING(STATUS,status);
		READMYSQL_STRING(DELIVERY_DATE,delivery_date);
		READMYSQL_STRING(DELIVERY_WAY,delivery_way);
		READMYSQL_STRING(DELIVERY_LOCATION,delivery_location);
		READMYSQL_STRING(EMERGENCY_FLAG,emergency_flag);
		READMYSQL_STRING(CS_CERTIFIED_FLAG,cs_certified_flag);
		READMYSQL_STRING(REASON,reason);
		break;

	}
	out <<"{seq:\""<<strSeq<<"\",id:\""<<strId<<"\",shipId:\""<<shipid<<"\",shipName:\""<<ship_name<<"\",departId:\""<<department_code<<"\",departName:\""<<depart_name<<"\",uname:\""<<user_id<<"\",atime:\""<<apply_date<<"\",taskId:\""<<workflow_task_id<<"\",stepId:\""<<workflow_step_id<<"\",status:\""<<status<<"\",dtime:\""<<delivery_date<<"\",way:\""<<delivery_way<<"\",address:\""<<delivery_location<<"\",isUrgency:\""<<emergency_flag<<"\",isCheck:\""<<cs_certified_flag<<"\",reason:\""<<reason<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//根据申请单id获取备件列表信息
//0x6153
//{seq:"",id:""}
//{seq:””,data:[id:””,eid:””,qid:””,departId:””,departName:””,uname””,atime:””,taskId:””,stepId:””,status:””,dtime:””,way:””,address:””,isUrgency:””,isCheck:””,reason:””}
int eMOPEquipSvc::GetEquipListInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetEquipListInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql,"SELECT t1.PART_MODEL_ID,t1.EQUIP_MODEL_ID,t1.QUATITY, \
				t1.PART_CAT_ID,t3.LOCATION_ID,t3.INSTOCK, \
				t4.NAME_CN,t4.NAME_EN,t4.CHART_NO,t4.UNIT, \
				t6.COMPANY_NAME,t6.MODE \
				FROM blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS t1 \
				JOIN blm_emop_etl.T50_EMOP_PARTS_APPLIES t2 \
				ON t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID \
				JOIN blm_emop_etl.T50_EMOP_SHIP_PARTS t3 \
				ON t3.PART_MODEL_ID=t1.PART_MODEL_ID AND t3.SHIPID=t2.SHIPID  AND t3.PART_CAT_ID=t1.PART_CAT_ID \
				JOIN blm_emop_etl.T50_EMOP_PARTS_MODELS t4 \
				ON t4.PART_MODEL_ID=t3.PART_MODEL_ID \
				JOIN blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t5 \
				ON t5.PART_CAT_ID=t3.PART_CAT_ID AND t5.SHIPID=t3.SHIPID \
				JOIN blm_emop_etl.T50_EMOP_EQUIP_MODEL t6\
				ON t6.EQUIP_MODEL_ID=t5.EQUIP_MODEL_ID \
				WHERE t1.PARTS_APPLY_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	string part_model_id="";
	string equip_model_id="";
	string quatity="";
	string part_cat_id="";
	string location_id="";
	string instock="";
	string name_cn="";
	string name_en="";
	string chart_no="";
	string unit="";
	string company_name="";
	string mode="";

	int idx=0;

	out <<"{seq:\""<<strSeq<<"\",data:[";
	while(psql->NextRow())
	{
		if (idx++)
		{
			out<<",";
		}
		READMYSQL_STRING(PART_MODEL_ID,part_model_id);
		READMYSQL_STRING(EQUIP_MODEL_ID,equip_model_id);
		READMYSQL_STRING(QUATITY,quatity);
		READMYSQL_STRING(PART_CAT_ID,part_cat_id);
		READMYSQL_STRING(LOCATION_ID,location_id);
		READMYSQL_STRING(INSTOCK,instock);
		READMYSQL_STRING(NAME_CN,name_cn);
		READMYSQL_STRING(NAME_EN,name_en);
		READMYSQL_STRING(CHART_NO,chart_no);
		READMYSQL_STRING(UNIT,unit);
		READMYSQL_STRING(COMPANY_NAME,company_name);
		READMYSQL_STRING(MODE,mode);

		out <<"{id:\""<<part_model_id<<"\",eid:\""<<equip_model_id<<"\",cn:\""<<name_cn<<"\",en:\""<<name_en<<"\",tp:\""<<mode<<"\",num:\""<<quatity<<"\",mf:\""<<company_name<<"\",img:\""<<chart_no<<"\",rep:\""<<instock<<"\",unit:\""<<unit<<"\",loc:\""<<location_id<<"\",bg:\""<<part_cat_id<<"\"}";

	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


//备件申请单修改
//0x6172
//{seq:””,id:””,shipId:””,departId:””,time:””,way:””,site:””,isUrgency:,isCheck:,reason:””,spare:””}
//{seq:””,eid:””}
int eMOPEquipSvc::EditApllyInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::EditApplyInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strId=root.getv("id", "");
	string strShipid=root.getv("shipId", "");
	string strDepartid=root.getv("departId", "");
	int strTime= root.getv("time", 0); 
	string strWay=root.getv("way", "");
	string strSite=root.getv("site", "");
	int strIsurgency=root.getv("isUrgency", 0);
	int strIscheck=root.getv("isCheck", 0);
	string strReason=root.getv("reason", "");
	string strSpare=root.getv("spare", "");
	string strAttachment=root.getv("attachment","");

	string strTime1 = TimeToTimestampStr(strTime); 

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_PARTS_APPLIES \
				SET SHIPID='%s',DEPARTMENT_CODE='%s',\
				DELIVERY_DATE='%s',DELIVERY_WAY='%s',DELIVERY_LOCATION='%s', \
				EMERGENCY_FLAG=%d,CS_CERTIFIED_FLAG=%d,REASON='%s' \
				WHERE PARTS_APPLY_ID='%s'",strShipid.c_str(),strDepartid.c_str(),strTime1.c_str(),strWay.c_str(),strSite.c_str(),strIsurgency,strIscheck,strReason.c_str(),strId.c_str());
	
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_PARTS_APPLIES_ATTACHMENTS \
				WHERE PARTS_APPLY_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	Tokens fileVec = StrSplit(strAttachment,"@@");
	for(Tokens::iterator it = fileVec.begin();it!=fileVec.end();it++)
	{
		Tokens filefinalVec = StrSplit(*it,"|");
		//int fid=atoi(filefinalVec[0].c_str());
		int maxNum=0;
		sprintf(sql,"SELECT MAX(ATTACH_ID)+0 AS MAX_NUM FROM \
					blm_emop_etl.T50_EMOP_PARTS_APPLIES_ATTACHMENTS");
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
		if(psql->NextRow())
		{
			READMYSQL_INT(MAX_NUM,maxNum,0);
		}
		maxNum = maxNum + 1;
		//string timestr = TimeToTimestampStr(time);
		//我并没有把真正的附件id存进去，因为可能会出现数据的duplicate
		sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_PARTS_APPLIES_ATTACHMENTS(ATTACH_ID,PARTS_APPLY_ID,NAME) \
					VALUES(%d,'%s','%s')",maxNum,strId.c_str(),filefinalVec[1].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
	}

	//CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	Tokens spareVec = StrSplit(strSpare,"@@");
	for(Tokens::iterator it = spareVec.begin();it!=spareVec.end();it++)
	{
		Tokens sparefinalVec = StrSplit(*it,"|");
		sprintf(sql,"SELECT * FROM blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS \
					WHERE PARTS_APPLY_ID='%s' AND PART_MODEL_ID='%s'",strId.c_str(),sparefinalVec[0].c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
		int quatity=atoi(sparefinalVec[1].c_str());
		
		if(psql->NextRow())
		{
			sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS SET \
						QUATITY=%d WHERE PARTS_APPLY_ID='%s' AND PART_MODEL_ID='%s'",quatity,strId.c_str(),sparefinalVec[0].c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		}
		else
		{
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS(PARTS_APPLY_ID,PART_MODEL_ID,QUATITY) \
						VALUES('%s','%s',%d)",strId.c_str(),sparefinalVec[0].c_str(),quatity);
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		}

	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//修改审核数量
//0x6173
//　　{seq:””,type:””,id:””,taskId:””,stepId:””,spare:”id|num@@id|num”}
//{seq:””,eid:””}
int eMOPEquipSvc::EditCheckNum(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::EditCheckNum]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strtype=root.getv("type", "");
	string strId=root.getv("id", "");
	string strSpare=root.getv("spare", "");

	MySql* psql = CREATE_MYSQL;
	string tempStr="";
	string tempStr1="";

	int strType=atoi(strtype.c_str());

	if(strType == 0)
	{
		tempStr="T50_EMOP_PARTS_APPLY_DETAILS";
		tempStr1="PARTS_APPLY_ID";
	}
	else if(strType == 1)
	{
		tempStr="T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS";
		tempStr1="PARTS_INQUERIED_QUOTED_ID";
	}
	else if(strType == 2)
	{
		tempStr="T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS";
		tempStr1="PARTS_PURCHASE_ORDER_ID";
	}

	char sql[1024] = "";

	Tokens spareVec = StrSplit(strSpare,"@@");
	for(Tokens::iterator it = spareVec.begin();it!=spareVec.end();it++)
	{
		Tokens sparefinalVec = StrSplit(*it,"|");
		int quatity=atoi(sparefinalVec[1].c_str());
		
		sprintf(sql,"UPDATE blm_emop_etl.%s SET \
					QUATITY=%d WHERE %s='%s' AND PART_MODEL_ID='%s'",tempStr.c_str(),quatity,tempStr1.c_str(),strId.c_str(),sparefinalVec[0].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//根据订购单获取验收单列表
//0x6174
//{seq:””,id:””}
//{seq:””,id:””,data:[{cid:””,name:””,status:},{},....]}
int eMOPEquipSvc::GetCheckList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetCheckList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf(sql,"SELECT PART_INSTOCK_ID,CHECKER,REVIEW_STATUS \
				FROM blm_emop_etl.T50_EMOP_PARTS_INSTOCK \
				WHERE PARTS_PURCHASE_ORDER_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	string part_instock_id="";
	string checker="";
	int review_status=0;
	int idx=0;
	out<<"{seq:\""<<strSeq<<"\",id:\""<<strId<<"\",data:[";

	while(psql->NextRow())
	{
		if(idx)
			out<<",";
		idx++;

		READMYSQL_STRING(PART_INSTOCK_ID,part_instock_id);
		READMYSQL_STRING(CHECKER,checker);
		READMYSQL_INT(REVIEW_STATUS,review_status,0);
		
		out<<"{cid:\""<<part_instock_id<<"\",name:\""<<checker<<"\",status:"<<review_status<<"}";
	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//获取验收单详情
//0x6179
//{seq:””,cid:””}
//{seq:””,id:””,data:[{cid:””,name:””,status:},{},....]}
int eMOPEquipSvc::GetCheckInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetCheckInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strCid=root.getv("cid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql,"SELECT t1.PARTS_PURCHASE_ORDER_ID,t1.DELIVERYER,t1.COMPANY_KEY, \
				UNIX_TIMESTAMP(t1.OP_DATE) AS OP_DATE,t1.CHECKER,t1.INSTOCK_LOCTION,t1.REVIEW_STATUS, \
				UNIX_TIMESTAMP(t1.CHECK_DT) AS CHECK_DT,t2.NAME_CN AS SHIPNAME,t3.NAME_CN AS DEPNAME \
				FROM blm_emop_etl.T50_EMOP_PARTS_INSTOCK t1 \
				JOIN blm_emop_etl.T41_EMOP_SHIP t2 \
				ON t1.SHIPID=t2.SHIPID \
				JOIN blm_emop_etl.T50_EMOP_DEPARTMENT_CODE t3\
				ON t1.DEPARTMENT_ID=t3.DEPARTMENT_CODE \
				WHERE t1.PART_INSTOCK_ID='%s'",strCid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	string parts_purchase_order_id="";
	string deliveryer="";
	string company_key="";
	string op_date="";
	string checker="";
	string instock_loction="";
	int review_status=0;
	string check_dt="";
	string shipname="";
	string depname="";
	out<<"{seq:\""<<strSeq<<"\",";

	if(psql->NextRow())
	{
		READMYSQL_STRING(PARTS_PURCHASE_ORDER_ID,parts_purchase_order_id);
		READMYSQL_STRING(DELIVERYER,deliveryer);
		READMYSQL_STRING(COMPANY_KEY,company_key);
		READMYSQL_STRING(OP_DATE,op_date);
		READMYSQL_STRING(CHECKER,checker);
		READMYSQL_STRING(INSTOCK_LOCTION,instock_loction);
		READMYSQL_STRING(CHECK_DT,check_dt);
		READMYSQL_INT(REVIEW_STATUS,review_status,0);
		READMYSQL_STRING(SHIPNAME,shipname);
		READMYSQL_STRING(DEPNAME,depname);
		

		out<<"oid:\""<<parts_purchase_order_id<<"\",sname:\""<<deliveryer<<"\",provider:"<<company_key<<"\",stime:"<<op_date<<"\",cname:"<<checker<<"\",company:"<<instock_loction<<"\",ctime:"<<check_dt<<"\",shipName:"<<shipname<<"\",departName:"<<depname<<"\",status:\""<<review_status<<"\"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}
// 生成验收单
//0x6178
//{seq: ””,id: ”” ,oid:”” sname: ””,provider: ”” ,stime:”” ,uid: ””,checkId: ” ,ctime:”” ,shipId: ””,departId: ”” ,spare:”” }
//{seq: ””,eid: ”” ,cid:”” ,name: ””}
int eMOPEquipSvc::CreateCheckList(const char* pUid, const char * jsonString, std::stringstream& out)
{
      JSON_PARSE_RETURN( "[eMOPEquipSvc::CreateCheckList]bad format:" , jsonString, 1);

      string strSeq=root.getv( "seq" , "" );
      string strId=root.getv( "id" , "" );
      string strOid=root.getv( "oid" , "" );
      string strSname=root.getv( "sname" , "" );
      string strProvider=root.getv( "provider" , "" );
      string strStime=root.getv( "stime" , "" );
      string strUid=root.getv( "uid" , "" );
      string strCheckid=root.getv( "checkId" , "" );
      string strCtime=root.getv( "ctime" , "" );
      string strShipid=root.getv( "shipId" , "" );
      string strDepartid=root.getv( "departId" , "" );
      string strSpare=root.getv( "spare" , "" );

	  string strTemp = "BRS";
	  long ltime=0;
	  ltime=time(0);
	  char szTmp[32];
	  sprintf(szTmp,"%d",ltime);
	  strTemp+=szTmp;

      MySql* psql = CREATE_MYSQL;

      char sql[1024] = "" ;
	  sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_PARTS_INSTOCK(PART_INSTOCK_ID,PARTS_PURCHASE_ORDER_ID,DELIVERYER,COMPANY_KEY,OP_DATE,\
				  CHECKER,INSTOCK_LOCTION,CHECK_DT,SHIPID,DEPARTMENT_ID) \
				  VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",strTemp.c_str(),strOid.c_str(),strSname.c_str(),strProvider.c_str(),strStime.c_str(),strUid.c_str(),strCheckid.c_str(),strCtime.c_str(),strShipid.c_str(),strDepartid.c_str());

	  CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	  Tokens spareVec=StrSplit(strSpare,"@@");
	  for(Tokens::iterator it=spareVec.begin();it!=spareVec.end();it++)
	  {
		  Tokens spareFinalVec=StrSplit(*it,"|");
		  sprintf(sql,"SELECT INSTOCK \
					  FROM blm_emop_etl.T50_EMOP_SHIP_PARTS \
					  WHERE PART_CAT_ID='%s' AND SHIPID='%s' AND PART_MODEL_ID='%s'",spareFinalVec[1].c_str(),spareFinalVec[0].c_str(),spareFinalVec[2].c_str());
		  CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		  int instock=0;
		  int Num=atoi(spareFinalVec[3].c_str());

		  if(psql->NextRow())
		  {
			  READMYSQL_INT(INSTOCK,instock,0);
		  }

		  sprintf(sql,"SELECT MIN(t1.REST) AS REST FROM \
					  blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS t1 \
					  JOIN blm_emop_etl.T50_EMOP_PARTS_INSTOCK t2 \
					  ON t1.PART_INSTOCK_ID=t2.PART_INSTOCK_ID \
					  WHERE t2.PARTS_PURCHASE_ORDER_ID='%s' AND t1.PART_MODEL_ID='%s'",strOid.c_str(),spareFinalVec[2].c_str());

		  CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		  int rest=0;
		  int temp_rest=0;
		  if(psql->NextRow())
		  {
			  READMYSQL_INT(REST,rest,0);
			  
			  temp_rest = rest-Num;
			  sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS(PART_INSTOCK_ID,PART_MODEL_ID,QUATITY,INSTOCK_REST,REST) \
						  VALUES('%s','%s',%d,%d,%d)",strTemp.c_str(),spareFinalVec[2].c_str(),Num,instock,temp_rest);
			  CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	
		  }
		  else
		  {
			  sprintf(sql,"SELECT QUATITY FROM blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS \
						  WHERE PARTS_PURCHASE_ORDER_ID='%s' AND PART_MODEL_ID='%s'",strOid.c_str(),spareFinalVec[2].c_str());
			  CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
			  int quatity=0;
			  if(psql->NextRow())
			  {
				  READMYSQL_INT(QUATITY,quatity,0);
			  }

			  sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS(PART_INSTOCK_ID,PART_MODEL_ID,QUATITY,INSTOCK_REST,REST) \
						  VALUES('%s','%s',%d,%d,%d)",strTemp.c_str(),spareFinalVec[2].c_str(),Num,instock,quatity);
			  CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
		  }
		  int final_instock=0;
		  final_instock = Num+instock;
		  sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_SHIP_PARTS \
					  SET LOCATION_ID='%s',INSTOCK=%d",spareFinalVec[4].c_str(),final_instock);
		  CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	  }

	  out<<"{seq:\""<<strSeq<<"\",eid:0,cid:\""<<strTemp<<"\",name:\""<<strUid<<"\"}";

      RELEASE_MYSQL_RETURN(psql, 0);
}


//备件采购审核
//0x615c
//{seq:””,id:””,type:,uid:””,taskId:””,stepId:””,status:””,idea:””}
//{seq:””,eid:””}
int eMOPEquipSvc::EquipPurchaseCheck(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::EquipPurchaseCheck]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	int strType=root.getv("type",0);
	string strId=root.getv("id", "");
	string strUid=root.getv("uid", "");
	string strTaskid=root.getv("taskId", "");
	string strStepid=root.getv("stepId", "");
	string strStatus=root.getv("status", "");
	string strIdea=root.getv("idea", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	string strTemp="";
	string strTemp1="";

	if(strType==0)
	{
		strTemp="T50_EMOP_PARTS_APPLY_STATUS";
		strTemp1="PARTS_APPLY_ID";
	}
	else if(strType==2)
	{
		strTemp="T50_EMOP_PURCHASE_ORDER_STATUS";
		strTemp1="PARTS_PURCHASE_ORDER_ID";
	}
	sprintf(sql,"SELECT NAME FROM blm_emop_etl.T50_EMOP_USERS \
				WHERE USER_ID='%s'",strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	
	string name="";
	if(psql->NextRow())
	{
		READMYSQL_STRING(NAME,name);
	}

	sprintf(sql,"UPDATE blm_emop_etl.%s SET STATUS=1,REMARK='%s',OPERATOR='%s' \
				WHERE %s='%s' AND WORKFLOW_TASK_ID='%s' AND WORKFLOW_STEP_ID='%s'",strTemp.c_str(),strIdea.c_str(),name.c_str(),strTemp1.c_str(),strId.c_str(),strTaskid.c_str(),strStepid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//根据订购单获取流程操作日志
//0x6177
//{seq:””,type:””,id:””}
//{seq:””,datas:[{optime:””,operator:””,action:””,taskId:””,stepId:””,idea:””},{},...]}
int eMOPEquipSvc::GetWorkFlowInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetWorkFlowInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strtype=root.getv("type","");
	string strId=root.getv("id", "");

	int strType=atoi(strtype.c_str());

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	string strTemp="";
	string strTemp1="";

	if(strType==0)
	{
		strTemp="T50_EMOP_PARTS_APPLY_STATUS";
		strTemp1="PARTS_APPLY_ID";
	}
	else if(strType==2)
	{
		strTemp="T50_EMOP_PURCHASE_ORDER_STATUS";
		strTemp1="PARTS_PURCHASE_ORDER_ID";
	}
	sprintf(sql,"SELECT STATUS,UNIX_TIMESTAMP(OP_DATE) AS OP_DATE,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,REMARK \
				FROM blm_emop_etl.%s \
				WHERE %s='%s'",strTemp.c_str(),strTemp1.c_str(),strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	string op_date="";
	string operators="";
	int status=0;
	string workflow_task_id="";
	string workflow_step_id="";
	string remark="";

	out<<"{seq:\""<<strSeq<<"\",datas:[";
	int idx=0;

	while(psql->NextRow())
	{
		if(idx)
			out<<",";
		idx++;
		READMYSQL_INT(STATUS,status,0);
		READMYSQL_STRING(OP_DATE,op_date);
		READMYSQL_STRING(OPERATOR,operators);
		READMYSQL_STRING(WORKFLOW_TASK_ID,workflow_task_id);
		READMYSQL_STRING(WORKFLOW_STEP_ID,workflow_step_id);
		READMYSQL_STRING(REMARK,remark);
		
		out<<"{optime:\""<<op_date<<"\",operator:\""<<operators<<"\",action:\""<<status<<"\",taskId:\""<<workflow_task_id<<"\",stepId:\""<<workflow_step_id<<"\",idea:\""<<remark<<"\"}";
	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//申请单删除
//0x6175
//{seq:””,id:””}
//{seq:””,etype:0}
int eMOPEquipSvc::DelApplyInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::DelApplyInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql,"SELECT STATUS,WORKFLOW_TASK_ID \
				FROM blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS \
				WHERE PARTS_APPLY_ID='%s' AND WORKFLOW_TASK_ID='11' AND \
				WORKFLOW_STEP_ID='1'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	
	int status=0;
	int etype=1;
	if(psql->NextRow())
	{
		READMYSQL_INT(STATUS,status,0);
		if(status == 0)
		{
			sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS WHERE PARTS_APPLY_ID='%s'",strId.c_str()); 
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_PARTS_APPLIES WHERE PARTS_APPLY_ID='%s'",strId.c_str()); 
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS WHERE PARTS_APPLY_ID='%s'",strId.c_str()); 
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			etype = 0;
		}
	}
	else
	{
		sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_PARTS_APPLIES WHERE PARTS_APPLY_ID='%s'",strId.c_str()); 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS WHERE PARTS_APPLY_ID='%s'",strId.c_str()); 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		etype=0;
	}


	out<<"{seq:\""<<strSeq<<"\",etype:\""<<etype<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//附件删除
//0x6176
//{seq:””,id:””,fid:""}
//{seq:””,eid:0}
int eMOPEquipSvc::DelEquipFile(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::DelEquipFile]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strId=root.getv("id", "");
	string strFid=root.getv("fid","");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	int strFId=atoi(strFid.c_str());

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_PARTS_APPLIES_ATTACHMENTS \
				WHERE PARTS_APPLY_ID='%s' AND ATTACH_ID=%d",strId.c_str(),strFId);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//根据申请单id获取询价单
//0x6155
//{seq:"",id:""}
//{seq:””,data:[id:””,eid:””,qid:””,departId:””,departName:””,uname””,atime:””,taskId:””,stepId:””,status:””,dtime:””,way:””,address:””,isUrgency:””,isCheck:””,reason:””}
int eMOPEquipSvc::GetApplyInqueryInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetApplyInqueryInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql,"SELECT t3.PARTS_INQUERIED_QUOTED_ID,t3.STATUS,t4.NAME \
				FROM blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 \
				JOIN blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 \
				ON t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID \
				JOIN blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t3 \
				ON t3.PARTS_APPLY_INQUERY_ID=t2.PARTS_APPLY_INQUERY_ID \
				JOIN blm_emop_etl.T41_COMPANY t4 \
				ON t4.COMPANY_KEY=t3.COMPANY_KEY \
				WHERE t1.PARTS_APPLY_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	string parts_inqueried_quoted_id="";
	string name="";
	int status=0;

	int idx=0;

	out <<"{seq:\""<<strSeq<<",id:\""<<strId<<"\",data:[";
	while(psql->NextRow())
	{
		if (idx++)
		{
			out<<",";
		}
		READMYSQL_STRING(PARTS_INQUERIED_QUOTED_ID,parts_inqueried_quoted_id);
		READMYSQL_STRING(NAME,name);
		READMYSQL_INT(STATUS,status,0);

		out <<"{eid:\""<<parts_inqueried_quoted_id<<"\",name:\""<<name<<"\",status:"<<status<<"}";

	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//根据申请单id获取订购单
//0x6156
//{seq:"",id:""}
//{seq:””,data:[id:””,eid:””,qid:””,departId:””,departName:””,uname””,atime:””,taskId:””,stepId:””,status:””,dtime:””,way:””,address:””,isUrgency:””,isCheck:””,reason:””}
int eMOPEquipSvc::GetApplyOrdersInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetApplyOrdersInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql,"SELECT t1.PARTS_APPLY_ID,t2.PARTS_APPLY_INQUERY_ID, \
				t4.PARTS_PURCHASE_ORDER_ID,t5.NAME FROM \
				blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 \
				JOIN blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 \
				ON t2.PARTS_APPLY_ID=t1.PARTS_APPLY_ID \
				JOIN blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t3 \
				ON t3.PARTS_APPLY_INQUERY_ID=t2.PARTS_APPLY_INQUERY_ID \
				JOIN blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t4 \
				ON t4.PARTS_APPLY_INQUERIED_QUOTED_ID=t3.PARTS_INQUERIED_QUOTED_ID \
				JOIN blm_emop_etl.T41_COMPANY t5 \
				ON t5.COMPANY_KEY=t4.COMPANY_KEY \
				WHERE t1.PARTS_APPLY_ID='%s'",strId.c_str());


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	string parts_apply_id="";
	string parts_apply_inquery_id="";
	string parts_purchase_order_id="";
	string name="";

	int idx=0;

	out <<"{seq:\""<<strSeq<<"\",data:[";
	while(psql->NextRow())
	{
		if (idx++)
		{
			out<<",";
		}
		READMYSQL_STRING(PARTS_APPLY_ID,parts_apply_id);
		READMYSQL_STRING(PARTS_APPLY_INQUERY_ID,parts_apply_inquery_id);
		READMYSQL_STRING(PARTS_PURCHASE_ORDER_ID,parts_purchase_order_id);
		READMYSQL_STRING(NAME,name);

		out <<"{id:\""<<parts_apply_id<<"\",eid:\""<<parts_apply_inquery_id<<"\",oid:\""<<parts_purchase_order_id<<"\",name:\""<<name<<"\"}";

	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//获取备件入库单列表
int eMOPEquipSvc::GetInstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsInventoryCounting]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strShipid=root.getv("shipid", "");
	string strUid=root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql, "select PART_INSTOCK_ID,OP_DATE,DEPARTMENT_ID,INSTOCK_LOCTION,TYPE,REVIEW_STATUS,STOCK_STATUS,OPERATOR,PARTS_PURCHASE_ORDER_ID,REMARK \
				 from blm_emop_etl.T50_EMOP_PARTS_INSTOCK WHERE SHIPID='%s'",strShipid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char orderid[64]="";//入库单ID
	char orderdt[20]="";//入库单日期
	char departid[32]="";//部门ID
	char instocklocation[256]="";//入库位置
	int type=0;//1、验收入库 2、盘点入库
	int reviewstatus=0;//0: 未审核 1;已审核（部门长已确认）
	int stockstatus=0;//0：部分入库 1：全部入库
	char operatorid[64]="";
	string remark="";
	char purchaseid[64]="";

	int idx=0;

	out <<"{seq:\""<<strSeq<<"\",who:\"\",info:[";	

	while (psql->NextRow())
	{

		READMYSQL_STR(PART_INSTOCK_ID, orderid)
			READMYSQL_STR(OP_DATE, orderdt)
			READMYSQL_STR(DEPARTMENT_ID, departid)
			READMYSQL_STR(INSTOCK_LOCTION, instocklocation)
			READMYSQL_INT(TYPE,type,0)
			READMYSQL_INT(REVIEW_STATUS,reviewstatus,0)
			READMYSQL_INT(STOCK_STATUS,stockstatus,0)
			READMYSQL_STR(OPERATOR,operatorid)
			READMYSQL_STRING(REMARK,remark)
			READMYSQL_STR(PARTS_PURCHASE_ORDER_ID,purchaseid)

			if (idx++)
			{
				out<<",";
			}

			out << "{id:\"" << orderid << "\",date:\"" << orderdt <<"\",detpar:\"" << departid <<"\",stateC:\"" << reviewstatus << "\",stateB:\"";
			out	<< stockstatus <<"\",codeB:\"" << purchaseid<<"\",typeE:\"" << type<<"\",loca:\""<<instocklocation<<"\",remark:\"" << remark<<"\"}";

	}

	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//获取某入库单下所有的备件列表
int eMOPEquipSvc::GetInstockOrderPartList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetInstockOrderPartList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strOrderId=root.getv("id", "");
	string strShipId=root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";

	sprintf(sql,"select T1.PART_INSTOCK_ID,T1.PART_MODEL_ID,T1.QUATITY,T1.REMARK,T1.INSTOCK_REST,T2.INSTOCK,T2.IS_IMPORTANT,T2.LOCATION_ID \
				from blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS T1,blm_emop_etl.T50_EMOP_SHIP_PARTS T2 \
				WHERE T1.PART_INSTOCK_ID='%s' AND T2.SHIPID='%s' AND T1.PART_MODEL_ID=T2.PART_MODEL_ID",strOrderId.c_str(),strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char partid[64]="";
	int quantity=0;//入库数量
	int instocknum=0;//库存量
	string remark="";
	int isimport=0;
	char locationid[64]="";

	string price="";

	out <<"{eid:\"0\",seq:\""<<strSeq<<"\",id:\""<<strOrderId<<"\",info:[";	

	int idx=0;

	while(psql->NextRow())
	{

		READMYSQL_STR(PART_MODEL_ID, partid)
			READMYSQL_INT(QUATITY,quantity,0)
			READMYSQL_INT(INSTOCK,instocknum,0)
			READMYSQL_STRING(REMARK, remark)
			READMYSQL_INT(IS_IMPORTANT,isimport,0)
			READMYSQL_STR(LOCATION_ID,locationid)


			SparePartsInfo *pPartInfo=g_eMOPCommDataSvr::instance()->GetPartItemById(string(partid));

		string unitStr="";
		string partName="";
		string partModel="";

		if (pPartInfo)
		{
			unitStr=pPartInfo->m_unit;
			partName=pPartInfo->m_name_cn.empty()?pPartInfo->m_name_en:pPartInfo->m_name_cn;
			partModel=pPartInfo->m_model;
		}

		if (idx++)
		{
			out<<",";
		}

		out << "{did:\"" << partid << "\",md:\"" << partModel <<"\",name:\"" << partName <<"\",entryN:\"" << quantity << "\",existN:\"";
		out	<< instocknum <<"\",unit:\"" << unitStr<<"\",price:\""<<price<<"\",loc:\"" << locationid<<"\",rem:\"" <<remark;
		out <<"\",maj:\"" << isimport<<"\"}";

	}

	out <<"]}";


	RELEASE_MYSQL_RETURN(psql, 0);
}

//获取备件订购单列表（备件待入库） 0x54
int eMOPEquipSvc::GetPurchaseOrderList(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPurchaseOrderList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strShipid=root.getv("shipid", "");
	string strUid=root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql, "select T1.PARTS_PURCHASE_ORDER_ID,T1.ORDER_DATE,T1.REMARK,T1.DEPARTMENT_CODE,T2.STOCK_STATUS \
				 from blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS T1 LEFT JOIN blm_emop_etl.T50_EMOP_PARTS_INSTOCK T2 \
				 ON T1.SHIPID=T2.SHIPID AND T1.PARTS_PURCHASE_ORDER_ID=T2.PARTS_PURCHASE_ORDER_ID WHERE T1.SHIPID='%s'",strShipid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	map<string,PuchaseOrderItem> mapOrders;


	while (psql->NextRow())
	{
		string stockstatus="";

		PuchaseOrderItem orderItem;
		READMYSQL_STRING(PARTS_PURCHASE_ORDER_ID, orderItem.orderid)
			READMYSQL_STRING(ORDER_DATE, orderItem.orderdt)
			READMYSQL_STRING(DEPARTMENT_CODE, orderItem.departid)
			READMYSQL_STRING(STOCK_STATUS, stockstatus)

			if (stockstatus.empty())
			{
				orderItem.status=-1;
			}
			else
			{
				orderItem.status=atoi(stockstatus.c_str());
			}


			map<string,PuchaseOrderItem>::iterator it=mapOrders.find(orderItem.orderid);

			if (it==mapOrders.end())
			{
				mapOrders.insert(make_pair(orderItem.orderid, orderItem));

			}
			else
			{
				if (orderItem.status>it->second.status)
				{
					it->second.status=orderItem.status;
				}
			}

	}

	out <<"{seq:\""<<strSeq<<"\",info:[";	


	for (map<string,PuchaseOrderItem>::iterator it=mapOrders.begin();it!=mapOrders.end();it++)
	{

		PuchaseOrderItem orderItem=it->second;

		if (it!=mapOrders.begin())
		{
			out<<",";
		}

		out << "{id:\"" << orderItem.orderid << "\",date:\"" << orderItem.orderdt <<"\",depar:\"" << orderItem.departid <<"\",deparId:\"" << orderItem.departid;
		out	<<"\",type:\"" << orderItem.status<<"\"}";

	}


	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);

}

//获取某订购单下的备件信息列表 0x55
//int eMOPEquipSvc::GetPurchaseOrderPartList(const char* pUid, const char* jsonString, std::stringstream& out)
//{
//	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPurchaseOrderPartList]bad format:", jsonString, 1);
//
//	string strSeq=root.getv("seq", "");
//	string strOrderId=root.getv("id", "");
//	string strShipId=root.getv("shipId", "");
//
//	MySql* psql = CREATE_MYSQL;
//
//	char sql[512] = "";
//
//	sprintf(sql,"select T1.PARTS_PURCHASE_ORDER_ID,T1.PART_MODEL_ID,T1.QUATITY,T1.UNIT_PRICE,T1.REMARK,T2.INSTOCK,T2.IS_IMPORTANT,T2.LOCATION_ID \
//				from blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS T1 LEFT JOIN blm_emop_etl.T50_EMOP_SHIP_PARTS T2 ON T1.PART_MODEL_ID=T2.PART_MODEL_ID \
//				WHERE T1.PARTS_PURCHASE_ORDER_ID='%s'",strOrderId.c_str());//,strShipId.c_str()
//
//	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
//
//	char partid[64]="";
//	int quantity=0;//入库数量
//	int instocknum=0;//
//	string remark="";
//	int isimport=0;
//	char locationid[64]="";
//
//	string price="";
//
//	out <<"{eid:\"0\",seq:\""<<strSeq<<"\",id:\""<<strOrderId<<"\",info:[";	
//
//	int idx=0;
//
//	while(psql->NextRow())
//	{
//
//		READMYSQL_STR(PART_MODEL_ID, partid)
//			READMYSQL_INT(QUATITY,quantity,0)
//			//READMYSQL_INT(INSTOCK,instocknum,0)
//			READMYSQL_STRING(REMARK, remark)
//			READMYSQL_INT(IS_IMPORTANT,isimport,0)
//			READMYSQL_STR(LOCATION_ID,locationid)
//			READMYSQL_STRING(UNIT_PRICE,price)
//
//
//			SparePartsInfo *pPartInfo=g_eMOPCommDataSvr::instance()->GetPartItemById(string(partid));
//
//		string unitStr="";
//		string partName="";
//		string partModel="";
//
//		if (pPartInfo)
//		{
//			unitStr=pPartInfo->m_unit;
//			partName=pPartInfo->m_name_cn.empty()?pPartInfo->m_name_en:pPartInfo->m_name_cn;
//			partModel=pPartInfo->m_model;
//		}
//
//		if (idx++)
//		{
//			out<<",";
//		}
//
//		out << "{did:\"" << partid << "\",md:\"" << partModel <<"\",name:\"" << partName <<"\",buyN:\"" << quantity << "\",entryN:\"";
//		out	<< instocknum <<"\",unit:\"" << unitStr<<"\",price:\""<<price<<"\",loc:\"" << locationid<<"\",rem:\"" <<remark;
//		out <<"\",maj:\"" << isimport<<"\"}";
//
//	}
//
//	out <<"]}";
//
//
//	RELEASE_MYSQL_RETURN(psql, 0);
//}

//提交备件入库单 0x56
//int eMOPEquipSvc::SetPartInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out)
//{
//	JSON_PARSE_RETURN("[eMOPEquipSvc::SetPartInstockOrder]bad format:", jsonString, 1);
//
//	string strSeq=root.getv("seq", "");
//	string strShipid=root.getv("shipId", "");
//	string strDepartid=root.getv("deparId", "");
//	string strLocation=root.getv("loca", "");//入库地点
//	//int type=root.getv("loca", 1);//类型（1、验收入库 2、盘点入库）
//	int type=1;
//	int state=root.getv("state", 0);//入库状态（0：部分入库 1：全部入库）
//	string purchaseid=root.getv("id", "");//订购单ID
//
//	string partIds=root.getv("did", "");
//	string checks=root.getv("dNum", "");
//
//	Tokens partIdTokens=StrSplit(partIds,"|");
//	Tokens checkTokens=StrSplit(checks,"|");
//
//	if (partIdTokens.size()!=checkTokens.size())
//	{
//		out <<"{eid:1,seq:\""<<strSeq<<"\"}";	
//		return 0;
//	}
//
//
//	string sequence="";
//	GET_EMOP_SEQUENCE_STRING(BR,sequence)
//		string instockid="BRS"+strShipid+sequence;
//
//	string date=CurLocalDateSeq();
//
//	MySql* psql = CREATE_MYSQL;
//
//	char sql[512]="";
//	sprintf (sql, "INSERT INTO blm_emop_etl.T50_EMOP_PARTS_INSTOCK(PART_INSTOCK_ID,OP_DATE,SHIPID,DEPARTMENT_ID,\
//				  INSTOCK_LOCTION,TYPE,PARTS_PURCHASE_ORDER_ID,REVIEW_STATUS,STOCK_STATUS,OPERATOR,REMARK) \
//				  VALUES('%s','%s','%s','%s','%s',%d,'%s',%d,%d,'%s','%s')",instockid.c_str(),date.c_str(),strShipid.c_str(),
//				  strDepartid.c_str(),strLocation.c_str(),type,purchaseid.c_str(),0,state,pUid,"");
//	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
//
//
//	for (int i=0;i<partIdTokens.size();i++)
//	{
//
//		char sql1[512]="";
//		sprintf (sql1, "INSERT INTO blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS(PART_INSTOCK_ID,PART_MODEL_ID,QUATITY,REMARK,INSTOCK_REST)\
//					   VALUES('%s','%s',%s,'%s',%d)",instockid.c_str(),partIdTokens[i].c_str(),checkTokens[i].c_str(),"",0);
//		CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);
//
//	}
//
//	out <<"{eid:0,seq:\""<<strSeq<<"\"}";	
//
//
//	RELEASE_MYSQL_RETURN(psql, 0);
//}
//备件入库单部门长确认
//int eMOPEquipSvc::ConformPartInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out)
//{
//	JSON_PARSE_RETURN("[eMOPEquipSvc::ConformPartInstockOrder]bad format:", jsonString, 1);
//
//	string strSeq=root.getv("seq", "");
//	string Id=root.getv("id", "");
//
//	MySql* psql = CREATE_MYSQL;
//
//	char sql[512]="";
//
//	sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_PARTS_INSTOCK SET REVIEW_STATUS='1' WHERE PART_INSTOCK_ID='%s'",Id.c_str());
//
//	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
//
//	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";
//
//	RELEASE_MYSQL_RETURN(psql, 0);
//}
//删除备件入库单中某备件
int eMOPEquipSvc::DeleteInstockOrderPartItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::DeleteInstockOrderPartItem]bad format:", jsonString, 1);


	string strSeq=root.getv("seq", "");
	string strOrderId=root.getv("id", "");
	string strPartId=root.getv("did", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512]="";

	sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS where PART_INSTOCK_ID='%s' and PART_MODEL_ID='%s'",strOrderId.c_str(),strPartId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

////获取备件出库单列表
//int eMOPEquipSvc::GetOutstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out)
//{
//	JSON_PARSE_RETURN("[eMOPEquipSvc::GetOutstockOrderList]bad format:", jsonString, 1);
//
//	string strSeq=root.getv("seq", "");
//	string strShipid=root.getv("shipid", "");
//	string strUid=root.getv("uid", "");
//
//	MySql* psql = CREATE_MYSQL;
//
//	char sql[1024] = "";
//
//	sprintf(sql, "select PART_OUTSTOCK_ID,OP_DATE,DEPARTMENT_ID,STATUS,REVIEW_STATUS,STOCK_STATUS,OPERATOR,REMARK \
//				 from blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK WHERE SHIPID='%s'",strShipid.c_str());
//
//	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
//
//	char orderid[64]="";//出库单ID
//	char orderdt[20]="";//出库单日期
//	char departid[32]="";//部门ID
//	int status=0;//出库类型 0：领用，1：保养，2：报废，3：丢失，4盘点
//	int reviewstatus=0;//0: 未审核 1;已审核（部门长已确认）
//	int stockstatus=0;//0：部分出库 1：全部出库
//	char operatorid[64]="";
//	string remark="";
//
//	int idx=0;
//
//	out <<"{seq:\""<<strSeq<<"\",who:\"\",info:[";	
//
//	while (psql->NextRow())
//	{
//
//		READMYSQL_STR(PART_OUTSTOCK_ID, orderid)
//			READMYSQL_STR(OP_DATE, orderdt)
//			READMYSQL_STR(DEPARTMENT_ID, departid)
//			READMYSQL_INT(STATUS,status,0)
//			READMYSQL_INT(REVIEW_STATUS,reviewstatus,0)
//			READMYSQL_INT(STOCK_STATUS,stockstatus,0)
//			READMYSQL_STR(OPERATOR,operatorid)
//			READMYSQL_STRING(REMARK,remark)
//
//			if (idx++)
//			{
//				out<<",";
//			}
//
//			out << "{id:\"" << orderid << "\",date:\"" << orderdt <<"\",detpar:\"" << departid <<"\",stateC:\"" << reviewstatus << "\",type:\"";
//			out	<< status<<"\",remark:\"" << remark<<"\"}";
//
//	}
//	out<<"]}";
//
//	RELEASE_MYSQL_RETURN(psql, 0);
//
//}
//获取某出库单下所有的备件列表
//int eMOPEquipSvc::GetOutstockOrderPartList(const char* pUid, const char* jsonString, std::stringstream& out)
//{
//	JSON_PARSE_RETURN("[eMOPEquipSvc::GetOutstockOrderPartList]bad format:", jsonString, 1);
//
//	string strSeq=root.getv("seq", "");
//	string strOrderId=root.getv("id", "");
//	string strShipId=root.getv("shipid", "");
//
//	MySql* psql = CREATE_MYSQL;
//
//	char sql[512] = "";
//
//	sprintf(sql,"select T1.PART_MODEL_ID,T1.PART_INSTOCK_ID,T1.QUATITY,T1.REMARK,T2.INSTOCK,T2.IS_IMPORTANT,T2.LOCATION_ID \
//				from blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK_DETAILS T1,blm_emop_etl.T50_EMOP_SHIP_PARTS T2 \
//				WHERE T1.PART_OUTSTOCK_ID='%s' AND T2.SHIPID='%s' AND T1.PART_MODEL_ID=T2.PART_MODEL_ID",strOrderId.c_str(),strShipId.c_str());
//
//	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
//
//	char partid[64]="";
//	int quantity=0;//出库数量
//	int instocknum=0;//库存量
//	string remark="";
//	int isimport=0;
//	char locationid[64]="";
//
//	string price="";
//
//	out <<"{eid:\"0\",seq:\""<<strSeq<<"\",id:\""<<strOrderId<<"\",info:[";	
//
//	int idx=0;
//
//	while(psql->NextRow())
//	{
//
//		READMYSQL_STR(PART_MODEL_ID, partid)
//			READMYSQL_INT(QUATITY,quantity,0)
//			READMYSQL_INT(INSTOCK,instocknum,0)
//			READMYSQL_STRING(REMARK, remark)
//			READMYSQL_INT(IS_IMPORTANT,isimport,0)
//			READMYSQL_STR(LOCATION_ID,locationid)
//
//
//			SparePartsInfo *pPartInfo=g_eMOPCommDataSvr::instance()->GetPartItemById(string(partid));
//
//		string unitStr="";
//		string partName="";
//		string partModel="";
//
//		if (pPartInfo)
//		{
//			unitStr=pPartInfo->m_unit;
//			partName=pPartInfo->m_name_cn.empty()?pPartInfo->m_name_en:pPartInfo->m_name_cn;
//			partModel=pPartInfo->m_model;
//		}
//
//		if (idx++)
//		{
//			out<<",";
//		}
//
//		out << "{did:\"" << partid << "\",md:\"" << partModel <<"\",name:\"" << partName <<"\",outN:\"" << quantity << "\",existN:\"";
//		out	<< instocknum <<"\",unit:\"" << unitStr<<"\",price:\""<<price<<"\",loc:\"" << locationid<<"\",rem:\"" <<remark;
//		out <<"\",maj:\"" << isimport<<"\"}";
//
//	}
//
//	out <<"]}";
//
//
//	RELEASE_MYSQL_RETURN(psql, 0);
//
//}
//提交备件出库单
//int eMOPEquipSvc::SetPartOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out)
//{
//
//	JSON_PARSE_RETURN("[eMOPEquipSvc::SetPartOutstockOrder]bad format:", jsonString, 1);
//
//
//	string strSeq=root.getv("seq", "");
//	string strShipid=root.getv("shipId", "");
//	string strDepartid=root.getv("deparId", "");
//	int type=root.getv("type", 0);//出库类型 0：领用，1：保养，2：报废，3：丢失，4盘点）
//
//	int stockstatus=0;//0：部分出库 1：全部出库 
//
//
//	string sequence="";
//	GET_EMOP_SEQUENCE_STRING(BC,sequence)
//		string outstockid="BCS"+strShipid+sequence;
//
//	string date=CurLocalDateSeq();
//
//	MySql* psql = CREATE_MYSQL;
//
//
//	char sql[512]="";
//
//	sprintf (sql, "INSERT INTO blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK(PART_OUTSTOCK_ID,OP_DATE,SHIPID,DEPARTMENT_ID,\
//				  STATUS,REVIEW_STATUS,STOCK_STATUS,OPERATOR,REMARK) \
//				  VALUES('%s','%s','%s','%s',%d,%d,%d,'%s','%s')",outstockid.c_str(),date.c_str(),strShipid.c_str(),
//				  strDepartid.c_str(),type,0,stockstatus,pUid,"");
//	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
//
//	Json* parts = root.get("info");
//
//	if (parts)
//	{
//		for (int i=0;i<parts->size();i++)
//		{
//			Json* partItem=parts->get(i);
//			if (!partItem)
//			{
//				continue;
//			}
//
//			string partid=partItem->getv("id","");
//			int number=atoi(partItem->getv("num",""));
//			string instockorder=partItem->getv("entryId","");
//
//			char sql1[512]="";
//			sprintf (sql1, "INSERT INTO blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK_DETAILS(PART_OUTSTOCK_ID,PART_MODEL_ID,PART_INSTOCK_ID,QUATITY,REMARK) \
//						   VALUES('%s','%s','%s',%d,'%s')",outstockid.c_str(),partid.c_str(),instockorder.c_str(),number,"");
//			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);
//		}
//	}
//
//
//	out <<"{seq:\""<<strSeq<<"\",eid:0}";
//
//	RELEASE_MYSQL_RETURN(psql, 0);
//}
//备件出库单部门长确认
int eMOPEquipSvc::ConformPartOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPEquipSvc::ConformPartOutstockOrder]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string Id=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512]="";

	sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK SET REVIEW_STATUS='1' WHERE PART_OUTSTOCK_ID='%s'",Id.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";


	RELEASE_MYSQL_RETURN(psql, 0);

}
//删除备件出库单中某备件
//int eMOPEquipSvc::DeleteOutstockOrderPartItem(const char* pUid, const char* jsonString, std::stringstream& out)
//{
//	JSON_PARSE_RETURN("[eMOPEquipSvc::DeleteOutstockOrderPartItem]bad format:", jsonString, 1);
//
//	MySql* psql = CREATE_MYSQL;
//
//	string strSeq=root.getv("seq", "");
//	string strOrderId=root.getv("id", "");
//	string strPartId=root.getv("did", "");
//
//	char sql[512]="";
//
//	sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK_DETAILS where PART_OUTSTOCK_ID='%s' and PART_MODEL_ID='%s'",strOrderId.c_str(),strPartId.c_str());
//
//	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
//
//	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";
//
//
//	RELEASE_MYSQL_RETURN(psql, 0);
//}



//请求备件盘点信息 0x60
int eMOPEquipSvc::GetPartsInventoryCounting(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsInventoryCounting]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipId", "");
	string strEquipId=root.getv("equipId", "");
	string strPropertyId=root.getv("propertyId", "");


	char sql[1024] = "";
	sprintf (sql, "select T1.LOCATION_ID,T1.LAST_STOCK,UNIX_TIMESTAMP(T1.LAST_STOCK_DT) AS LAST_DT,T1.PART_MODEL_ID,T2.MODEL,T2.NAME_CN,T2.NAME_EN,T2.REMARKS,T2.UNIT \
				  from blm_emop_etl.T50_EMOP_SHIP_PARTS T1 LEFT JOIN blm_emop_etl.T50_EMOP_PARTS_MODELS T2 ON T1.PART_MODEL_ID=T2.PART_MODEL_ID \
				  where T1.SHIPID='%s' AND T1.PART_CAT_ID='%s'",strShipId.c_str(),strEquipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	long laststockdt=0;
	char locationid[64]="";
	int laststock=0;
	long lastdt=0;
	char equip_partid[64]="";
	char model[256]="";//型号
	char name[256]="";
	char unit[10]="";
	char spec[64]="";//g规格
	string remarks="";
	int nowcheck=0;

	out <<"{seq:\""<<strSeq<<"\",info:[";	


	int idx=0;
	while(psql->NextRow())
	{
		READMYSQL_STR(LOCATION_ID, locationid)
			READMYSQL_INT(LAST_STOCK,laststock,0)
			READMYSQL_INT(LAST_DT,lastdt,0)
			READMYSQL_STR(EQUIP_MODEL_ID, equip_partid)
			READMYSQL_STR(MODEL, model)
			READMYSQL_STR(NAME_CN, name)
			if (strlen(name)==0)
			{
				READMYSQL_STR(NAME_EN, name)
			}
			READMYSQL_STRING(REMARKS, remarks)
				READMYSQL_STR(UNIT,unit)


				if (lastdt>laststockdt)
				{
					laststockdt=lastdt;
				}

				if (idx++)
				{ 
					out<<",";
				}


				out << "{id:\"" << equip_partid << "\",name:\"" << name<<"\",lastS:\"" << laststock;
				out	<<"\",checkS:\"" << nowcheck<<"\",speci:\"" << spec<<"\",unit:\"" << unit<<"\",remark:\"" << remarks<<"\"}";

	}

	out<<"],date:"<<laststockdt<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//设置备件盘点 0x61
int eMOPEquipSvc::SetPartsInventoryCount(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::SetPartsInventoryCount]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipId", "");
	string strEquipId=root.getv("equipId", "");
	string strPropertyId=root.getv("propertyId", "");
	int type=root.getv("type", -1);
	string partIds=root.getv("id", "");
	string checks=root.getv("checkS", "");


	Tokens partIdTokens=StrSplit(partIds,"|");
	Tokens checkTokens=StrSplit(checks,"|");

	if (partIdTokens.size()!=checkTokens.size())
	{
		out <<"{seq:\""<<strSeq<<"\",type:\""<<type<<"\",check:\"0\"}";	
		return 0;
	}

	MySql* psql = CREATE_MYSQL;

	string date=CurLocalDateSeq();


	for (int i=0;i<partIdTokens.size();i++)
	{
		char sql[512]="";

		sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_SHIP_PARTS SET LAST_STOCK='%s',LAST_STOCK_DT='%s' \
					  where SHIPID='%s' AND PART_CAT_ID='%s' AND EQUIP_MODEL_ID='%s'",
					  checkTokens[i].c_str(),date.c_str(),strShipId.c_str(),strEquipId.c_str(),partIdTokens[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}


	//需要补充盘点出库入库流水

	out <<"{seq:\""<<strSeq<<"\",type:\""<<type<<"\",check:\"1\"}";	

	RELEASE_MYSQL_RETURN(psql, 0);
}
//备件月度增减统计 0x62
int eMOPEquipSvc::GetPartsMonthInOutStatistic(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsMonthInOutStatistic]bad format:", jsonString, 1);


	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipId", "");
	string strDate=root.getv("date", "");

	string startdt=strDate+"-01 00:00:00";
	string enddt=strDate+"-31 23:59:59";

	map<string,PartMonthStatistic> map_PartStatis;


	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";

	sprintf (sql, "select T2.PART_MODEL_ID,T2.QUATITY,T3.LOCATION_ID,T3.INSTOCK,T4.EQUIP_MODEL_ID,T4.NAME \
				  from blm_emop_etl.T50_EMOP_PARTS_APPLIES T1,blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS T2,blm_emop_etl.T50_EMOP_SHIP_PARTS T3,blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES T4 \
				  WHERE T1.SHIPID='%s' AND APPLY_DATE>='%s' AND APPLY_DATE<='%s' \
				  AND T1.PARTS_APPLY_ID=T2.PARTS_APPLY_ID AND T3.SHIPID=T1.SHIPID AND T3.PART_MODEL_ID=T2.PART_MODEL_ID  \
				  AND T4.SHIPID=T1.SHIPID AND T3.PART_CAT_ID=T4.PART_CAT_ID",strShipId.c_str(),startdt.c_str(),enddt.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	char part_id[64]="";
	int appnum=0;
	char equipmodelid[64]="";
	char location[128]="";
	int instock=0;
	char equipname[128]="";

	while(psql->NextRow())
	{

		READMYSQL_STR(PART_MODEL_ID, part_id)
			READMYSQL_INT(QUATITY,appnum,0)
			READMYSQL_INT(INSTOCK,instock,0)
			READMYSQL_STR(LOCATION_ID, location)
			READMYSQL_STR(EQUIP_MODEL_ID, equipmodelid)
			READMYSQL_STR(NAME, equipname)


			string partid=string(part_id);

		if (map_PartStatis.find(partid)==map_PartStatis.end())
		{
			PartMonthStatistic staticItem;
			staticItem.part_id=part_id;
			staticItem.appnum=appnum;
			staticItem.stocknum=instock;
			staticItem.locateid=location;
			staticItem.equip_id=equipmodelid;
			staticItem.equip_name=equipname;
			map_PartStatis[staticItem.part_id]=staticItem;
		}
		else
		{
			map_PartStatis[partid].appnum+=appnum;
		}


	}



	char sql1[1024]="";

	sprintf (sql1, "select T2.PART_MODEL_ID,T2.QUATITY,T3.LOCATION_ID,T3.INSTOCK,T4.EQUIP_MODEL_ID,T4.NAME \
				   from blm_emop_etl.T50_EMOP_PARTS_INSTOCK T1,blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS T2,blm_emop_etl.T50_EMOP_SHIP_PARTS T3,blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES T4 \
				   WHERE T1.SHIPID='%s' AND T1.OP_DATE>='%s' AND T1.OP_DATE<='%s' \
				   AND T1.PART_INSTOCK_ID=T2.PART_INSTOCK_ID AND T3.SHIPID=T1.SHIPID AND T3.PART_MODEL_ID=T2.PART_MODEL_ID  \
				   AND T4.SHIPID=T1.SHIPID AND T3.PART_CAT_ID=T4.PART_CAT_ID",strShipId.c_str(),startdt.c_str(),enddt.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql1), 3);


	while(psql->NextRow())
	{

		READMYSQL_STR(PART_MODEL_ID, part_id)
			READMYSQL_INT(QUATITY,appnum,0)
			READMYSQL_INT(INSTOCK,instock,0)
			READMYSQL_STR(LOCATION_ID, location)
			READMYSQL_STR(EQUIP_MODEL_ID, equipmodelid)
			READMYSQL_STR(NAME, equipname)

			string partid=string(part_id);

		if (map_PartStatis.find(partid)==map_PartStatis.end())
		{

			PartMonthStatistic staticItem;
			staticItem.part_id=partid;
			staticItem.instonum=appnum;
			staticItem.stocknum=instock;
			staticItem.locateid=location;
			staticItem.equip_id=equipmodelid;
			staticItem.equip_name=equipname;
			map_PartStatis[staticItem.part_id]=staticItem;
		}
		else
		{
			map_PartStatis[partid].instonum+=appnum;
		}	

	}


	char sql2[1024]="";

	sprintf (sql2, "select T2.PART_MODEL_ID,T2.QUATITY,T3.LOCATION_ID,T3.INSTOCK,T4.EQUIP_MODEL_ID,T4.NAME \
				   from blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK T1,blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK_DETAILS T2,blm_emop_etl.T50_EMOP_SHIP_PARTS T3,blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES T4 \
				   WHERE T1.SHIPID='%s' AND T1.OP_DATE>='%s' AND T1.OP_DATE<='%s' \
				   AND T1.PART_OUTSTOCK_ID=T2.PART_OUTSTOCK_ID AND T3.SHIPID=T1.SHIPID AND T3.PART_MODEL_ID=T2.PART_MODEL_ID \
				   AND T4.SHIPID=T1.SHIPID AND T3.PART_CAT_ID=T4.PART_CAT_ID",strShipId.c_str(),startdt.c_str(),enddt.c_str());



	CHECK_MYSQL_STATUS(psql->Query(sql2), 3);


	while(psql->NextRow())
	{

		READMYSQL_STR(PART_MODEL_ID, part_id)
			READMYSQL_INT(QUATITY,appnum,0)
			READMYSQL_INT(INSTOCK,instock,0)
			READMYSQL_STR(LOCATION_ID, location)
			READMYSQL_STR(EQUIP_MODEL_ID, equipmodelid)
			READMYSQL_STR(NAME, equipname)

			string partid=string(part_id);

		if (map_PartStatis.find(partid)==map_PartStatis.end())
		{

			PartMonthStatistic staticItem;
			staticItem.part_id=partid;
			staticItem.outstonum=appnum;
			staticItem.stocknum=instock;
			staticItem.locateid=location;
			staticItem.equip_id=equipmodelid;
			staticItem.equip_name=equipname;
			map_PartStatis[staticItem.part_id]=staticItem;
		}
		else
		{
			map_PartStatis[partid].outstonum+=appnum;
		}

	}


	out<<"{seq:\""<<strSeq.c_str()<<"\",shipId:\""<<strShipId<<"\",info:[";

	map<string,PartMonthStatistic>::iterator it=map_PartStatis.begin();

	for (;it!=map_PartStatis.end();it++)
	{
		if (it!=map_PartStatis.begin())
		{
			out<<",";
		}

		PartMonthStatistic item=it->second;

		SparePartsInfo *pPartItem=g_eMOPCommDataSvr::instance()->GetPartItemById(item.part_id);
		if (pPartItem)
		{
			if (!pPartItem->m_name_cn.empty())
			{
				item.part_name=pPartItem->m_name_cn;
			}
			else
			{
				item.part_name=pPartItem->m_name_en;
			}
		}


		EquipBaseInfo *pEquipItem=g_eMOPCommDataSvr::instance()->GetEquipItemById(item.equip_id);
		if (pEquipItem)
		{
			item.equip_model=pEquipItem->m_model;
		}


		out << "{sN:\"" << item.part_name << "\",sId:\"" << item.part_id<<"\",eN:\"" << item.equip_name;
		out	<<"\",eId:\"" << item.equip_model<<"\",stN:\"" << item.stocknum<<"\",stId:\"" << item.locateid;
		out	<<"\",applyN:\"" << item.appnum<<"\",entryN:\"" << item.instonum<<"\",outN:\"" << item.outstonum<<"\"}";

	}


	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID: 0x61    SID: 0x0001
//{ seq: "ssssssss001",sid:" ",cid:"0"}
//{seq:"ssssssss001",eid:0,data:[{fid:"",id:"",pid:””,na:"",pics:["","",....],haschild: ,hasparts:},{},.....]}
int eMOPEquipSvc::getAllEquipment(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::getAllEquipment]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("sid", "");
	string strCId=root.getv("cid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",data:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	if(strCId=="0")// 暂时没有图片表和数据
		strCId="";
	//		sprintf (sql, "select PART_CAT_ID,NAME,PARENT_ID,PATH,EQUIP_MODEL_ID from blm_emop_etl.t50_emop_part_ship_categories where SHIPID='%s' and PARENT_ID=''",strShipId.c_str());
	//	else
	//		sprintf (sql, "select PART_CAT_ID,NAME,PARENT_ID,PATH,EQUIP_MODEL_ID from blm_emop_etl.t50_emop_part_ship_categories where PARENT_ID='%s' AND SHIPID='%s'",strCId.c_str(),strShipId.c_str());

	sprintf (sql, "SELECT COUNT(t2.part_cat_id) as CCOUNT, t1.* FROM blm_emop_etl.t50_emop_part_ship_categories t1 LEFT JOIN blm_emop_etl.t50_emop_part_ship_categories t2 ON (t1.shipid=t2.shipid AND t1.part_cat_id=t2.parent_id)WHERE t1.shipid='%s' AND t1.parent_id='%s' GROUP BY t1.part_cat_id",strShipId.c_str(),strCId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char id[100] = "";
	char name[128]="";
	char pid[100] = "";
	char fid[100] = "";
	char path[512]="";
	char cattype[100]="";
	int iHaschild=0;
	string strTmp,strCatid;
	int cnt = 0;
	out << "[";	
	while (psql->NextRow())
	{
		READMYSQL_STR(PART_CAT_ID, id);
		strCatid=id;
		READMYSQL_STR(NAME, name);
		READMYSQL_STR(PARENT_ID, fid);
		READMYSQL_STR(PATH, path);
		READMYSQL_STR(EQUIP_MODEL_ID, pid);
		READMYSQL_INT(CCOUNT,iHaschild,-1);
		READMYSQL_STR(CAT_TYPE, cattype);
		GetPics(strTmp,strShipId,strCatid);
		if(iHaschild>0)
			iHaschild=1;
		else
			iHaschild=0;
		if (cnt++)
			out << ",";
		out << "{fid:\"" << fid << "\",id:\"" << id << "\",na:\"" << name <<"\",pid:\"" << pid << "\",pics:[" << strTmp.c_str() <<"],haschild:" << iHaschild<< ",category:\"" << cattype<< "\"}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
int eMOPEquipSvc::GetPics(string &o_strPics,string &i_strShipid,string &i_strCatid)
{
	MySql* psql1 = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT PIC_INDEX FROM blm_emop_etl.t50_emop_ship_parts_pic WHERE SHIPID='%s' AND PART_CAT_ID='%s'",i_strShipid.c_str(),i_strCatid.c_str());
	if(!(psql1->Query(sql)))
	{RELEASE_MYSQL_RETURN(psql1, 3);}
	char index[100] = "";
	int cnt = 0;
	o_strPics="";	
	while (psql1->NextRow())
	{
		READMYSQL_STR1(PIC_INDEX, index,psql1);

		if (cnt++)
			o_strPics += ",";
		o_strPics+="\"";
		o_strPics +=index;
		o_strPics+="\"";
	}
	RELEASE_MYSQL_RETURN(psql1, 0);
}
//
//Mid:0x61  SID: 0x0002
//{seq: "ssssssss001",pid:" "}
//{seq:"ssssssss001",eid:0,data:{tp:" ",es:””,mf:" ",tepa:[{key:" ",value:" "},{},....]}}
int eMOPEquipSvc::GetEquipPara(const char* pUid, const char* jsonString, std::stringstream& out)
{
	//tp:设备型号t50_emop_equip_model
	//mf:厂商t50_emop_equip_model

	//na:设备名称t50_emop_part_ship_categories	
	//es:设备系列号t50_emop_part_ship_categories

	//tepa:技术参数数据t50_emop_equip_para
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetEquipPara]bad format:", jsonString, 1);
	string strPid = root.getv("pid", "");
	string shipid= root.getv("shipId", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char name[128]="";
	char name_en[200]="";
	char corp[100] = "";  //厂商
	char mode[100] = ""; //设备型号
	char menucode[100]=""; //系列号
	int cattype=0; //类型
	char pname[128]=""; //父名
	int num=0; //台数
	int outdate=0;  //出厂日期
	char outcode[100] = "";  //出厂编号 无此字段  
	char modelId[100] = ""; //设备属性id

	sprintf(sql,"SELECT CAT_TYPE,NAME,NAME_EN FROM blm_emop_etl.t50_emop_part_ship_categories  \
				WHERE PART_CAT_ID='%s' AND SHIPID='%s'",strPid.c_str(),shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
	{
		READMYSQL_STR(NAME, name);
		READMYSQL_STR(NAME_EN, name_en); 
		READMYSQL_INT(CAT_TYPE, cattype,0);
	}else{
		out<<"{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",data:{es:\"\",na:\"\",name:\"\",num:0,outDate:0,outCode:\"\",tp:\"\",mf:\"\",un:\"\",category:0,pics:[],tepa:[]}}";
		return 0;
	}

	//设备
	if(cattype==0){
		sprintf(sql,\
			"SELECT t1.MODE,t1.COMPANY_NAME,t2.MANU_CODE,t2.NAME_CN AS NAME_CN,t2.NAME_EN AS NAME_EN,t2.QUANTITY,t2.MANU_DATE,t2.MANU_CODE,\
			t3.CAT_TYPE,\
			t3.EQUIP_MODEL_ID,\
			t4.NAME AS PNAME\
			FROM blm_emop_etl.t50_emop_equip_model t1 LEFT JOIN \
			blm_emop_etl.T50_EMOP_SHIP_EQUIP t2 ON t1.EQUIP_MODEL_ID=t2.EQUIP_MODEL_ID LEFT JOIN \
			blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t3 ON t1.EQUIP_MODEL_ID=t3.EQUIP_MODEL_ID LEFT JOIN\
			blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t4 ON t4.PART_CAT_ID=t3.PARENT_ID \
			WHERE t3.PART_CAT_ID='%s' AND t3.SHIPID='%s'",strPid.c_str(),shipid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		if(psql->NextRow())
		{
			READMYSQL_STR(MODE, mode);
			READMYSQL_STR(NAME_CN, name);
			READMYSQL_STR(NAME_EN, name_en);
			READMYSQL_STR(COMPANY_NAME, corp);
			READMYSQL_STR(MANU_CODE, menucode);
			READMYSQL_INT(CAT_TYPE, cattype,0);
			READMYSQL_STR(PNAME, pname);
			READMYSQL_INT(QUANTITY, num,0);
			READMYSQL_INT(MANU_DATE, outdate,0);
			READMYSQL_STR(EQUIP_MODEL_ID, modelId);
			if(pname=="")
				strcpy(pname,"0");
		}

		out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",data:{es:\""<<menucode<<"\",na:\""<<name<<"\",name:\""<<name_en<<"\",num:"<<num<<",outDate:"<< outdate <<",outCode:\""<<outcode<<"\",tp:\""<<mode<<"\",mf:\""<<corp<< "\",un:\"" << pname<< "\",category:" << cattype<<",pics:[],tepa:[";
		sprintf(sql,\
			"select NAME_CN,NAME_EN,VALUE from blm_emop_etl.t50_emop_equip_para where EQUIP_MODEL_ID='%s'",modelId);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		char key[100] = "";
		char value[100] = "";
		int cnt = 0;
		while (psql->NextRow())
		{
			READMYSQL_STR(NAME_CN, key);
			READMYSQL_STR(VALUE, value);
			if (cnt++)
				out << ",";
			out << "{key:\"" << key << "\",value:\"" << value<< "\"}";
		}
		out << "]}}";
	}
	else if(cattype==1){
		//目录
		out<<"{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",data:{es:\"\",na:\""<<name<<"\",name:\""<<name_en<<"\",num:0,outDate:0,outCode:\"\",tp:\"\",mf:\"\",un:\"\",category:"<< cattype<<",pics:[],tepa:[]}}";
	}else if(cattype==2){
		sprintf(sql,"SELECT PIC_INDEX FROM blm_emop_etl.t50_emop_ship_parts_pic WHERE PART_CAT_ID='%s' ",strPid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3); 

		out<<"{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",data:{es:\"\",na:\""<<name<<"\",name:\""<<name_en<<"\",num:0,outDate:0,outCode:\"\",tp:\"\",mf:\"\",un:\"\",category:"<< cattype<<",pics:[";

		int index=0;
		char file[100];
		while(psql->NextRow())
		{
			READMYSQL_STR(PIC_INDEX, file);
			if(index>0)
				out<<",";
			out<<"\""<<file<<"\"";
			index++;
		}
		out<<"],tepa:[]}}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID: 0x61    SID: 0x03
//{seq:"ssssssss001",type:0,data:{shipId:””,pid:””,id:"",na:" ",tp:" ",mf:" ",pics:[" "," ",...],tepa:[{key:" ",value:" "},{},....]}}
//{seq: "ssssssss001",eid:0,id:" ",pid:””}
int eMOPEquipSvc::NewEquipment(const char* pUid, const char* jsonString, std::stringstream& out)
{// 接口问题：公司名称  图片id可以用一个字符串
	// 新建设备，新建关系表 ？还是实实在在 添加个设备 ？
	JSON_PARSE_RETURN("[eMOPEquipSvc::NewEquipment]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", "");
	int iType=root.getv("type",-1);

	Json *child0=root.get("data");

	if (!child0)
	{
		return 0;
	}

	string strShipid= child0->getv("shipid", "");
	string strPid= child0->getv("pid", "");//属性id
	string strId= child0->getv("id", ""); //添加父设备id或者被修改设备id
	string strNa= child0->getv("na", "");// 中文名称
	string strNa_en= child0->getv("name", "");// 中文名称
	string strTp= child0->getv("tp", "");//设备型号
	string strMf= child0->getv("mf", "");//厂商 
	int num= child0->getv("num", 0);//数量
	int category= child0->getv("category", 0);//数量

	string strOutdate= child0->getv("outDate", "");//出厂日期
	string strCode= child0->getv("outCode", "");//出厂编号 数据库无此字段

	Json *child1=child0->get("tepa");
	Json *child2=child0->get("pics"); 

	string updatedt=GetCurrentTmStr();
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf(sql,\
		"SELECT PATH FROM blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES WHERE SHIPID='%s' AND PART_CAT_ID='%s'"
		,strShipid.c_str(),strId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3); 

	char parentPath[1024]="";
	if(psql->NextRow())
		READMYSQL_STR(PATH, parentPath);//父路径

	string outId = "";
	if(iType==0)
	{ 
		//新增设备
		string sequence="";
		GET_EMOP_SEQUENCE_STRING(EM,sequence)
			string datestr=CurLocalDateSeq();
		strPid="EM"+datestr+sequence;

		string strNewEquipid="GI"+datestr+sequence;

		string strPath="";
		if(sizeof(parentPath)>0)
			strPath = string(parentPath)+"#"+strNewEquipid;
		else
			strPath = strNewEquipid; //没有父路径，则为跟路径

		outId = strNewEquipid;
		//设备型号
		sprintf (sql, "insert into blm_emop_etl.t50_emop_equip_model(EQUIP_MODEL_ID,MODE, COMPANY_NAME) values ('%s','%s','%s')",\
			strPid.c_str(),strTp.c_str(), strMf.c_str());
		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		//设备分类
		sprintf (sql, "insert into blm_emop_etl.t50_emop_part_ship_categories(PART_CAT_ID,SHIPID, EQUIP_MODEL_ID,NAME,NAME_EN,PARENT_ID,PATH,CAT_TYPE) values ('%s','%s','%s','%s','%s','%s','%s',%d)",\
			strNewEquipid.c_str(),strShipid.c_str(), strPid.c_str(), strNa.c_str(),strNa_en.c_str(),strId.c_str(),strPath.c_str(),category);	
		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


		//设备属性 数量 出厂日期型号
		sprintf (sql, "insert into blm_emop_etl.T50_EMOP_SHIP_EQUIP(SHIPID, EQUIP_MODEL_ID,QUANTITY,NAME_CN,NAME_EN,MANU_DATE) values ('%s','%s',%d,'%s','%s','%s')",\
			strShipid.c_str(), strPid.c_str(), num,strNa.c_str(),strNa_en.c_str(),strOutdate.c_str());	
		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		int iSize=child1->size();
		string strKey;
		string strValue;
		for(int i=0;i<iSize;i++)
		{
			Json* para = child1->get(i);
			strKey=para->getv("key","");
			strValue=para->getv("value","");
			//技术参数
			sprintf (sql, "insert into blm_emop_etl.t50_emop_equip_para(EQUIP_MODEL_ID,NAME_CN, VALUE) values ('%s','%s','%s')",\
				strPid.c_str(),strKey.c_str(), strValue.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}		

		iSize=child2->size();
		string fileid; 
		for(int i=0;i<iSize;i++)
		{ 
			Json *jsonDetail=child2->get(i); 
			fileid=(string)jsonDetail->getv(""); 
			sprintf (sql, "INSERT blm_emop_etl.t50_emop_ship_parts_pic(PART_CAT_ID,PIC_INDEX) VALUE('%s','%s')",strNewEquipid.c_str(),fileid.c_str());
			DEBUG_LOG(sql);
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}	
	}
	else if (iType==1)
	{
		outId =strId;
		sprintf (sql, "UPDATE blm_emop_etl.t50_emop_equip_model SET MODE = '%s', COMPANY_NAME = '%s' WHERE EQUIP_MODEL_ID = '%s'",\
			strTp.c_str(), strMf.c_str(),strPid.c_str()); 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

		sprintf (sql, "UPDATE blm_emop_etl.t50_emop_part_ship_categories SET NAME='%s',NAME_EN='%s',PARENT_ID='%s',CAT_TYPE=%d WHERE PART_CAT_ID = '%s' and SHIPID = '%s' ",\
			strNa.c_str(),strNa_en.c_str(),strId.c_str(),category,strId.c_str(), strShipid.c_str());	 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		//设备属性 数量 出厂日期型号
		sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_SHIP_EQUIP set QUANTITY=%d,set NAME='%s',set NAME_EN='%s',set MANU_DATE='%s' WHERE EQUIP_MODEL_ID = '%s' and SHIPID = '%s' ",\
			num,strNa.c_str(),strNa_en.c_str(),strOutdate.c_str(),strPid.c_str(),strShipid.c_str());	

		int iSize=child1->size();
		string strKey;
		string strValue;		

		sprintf (sql, "delete from blm_emop_etl.t50_emop_equip_para WHERE EQUIP_MODEL_ID = '%s'",strPid.c_str());				
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		for(int i=0;i<iSize;i++)
		{
			Json* para = child1->get(i);
			strKey=para->getv("key","");
			strValue=para->getv("value","");
			sprintf (sql, "insert into blm_emop_etl.t50_emop_equip_para(EQUIP_MODEL_ID,NAME_CN, VALUE) values ('%s','%s','%s')",\
				strPid.c_str(),strKey.c_str(), strValue.c_str());		
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}

	//加入到缓存
	EquipBaseInfo equipInfo;
	equipInfo.m_equipid=strPid;
	equipInfo.m_model=strTp;
	equipInfo.m_cmpyname=strMf;

	g_eMOPCommDataSvr::instance()->AddNewEquipItem(equipInfo);

	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",id:\""<<outId.c_str()<<"\",pid:\""<<strPid.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
// MID: 0x61    SID: 0x04
//{seq: "ssssssss001",shipId:"",id:" "}
//{seq: "ssssssss001",eid:0}
int eMOPEquipSvc::DelEquip(const char* pUid, const char* jsonString, std::stringstream& out)
{// 船和模板是否分开
	JSON_PARSE_RETURN("[eMOPEquipSvc::DelEquip]bad format:", jsonString, 1);
	string strShipid = root.getv("shipId", "");
	string strEquipid= root.getv("id", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_part_ship_categories WHERE SHIPID = '%s' and PART_CAT_ID='%s'", strShipid.c_str(),strEquipid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	g_eMOPCommDataSvr::instance()->DeleteEquipItem(strEquipid);


	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID: 0x61    SID: 0x05
//{ seq: "ssssssss001",type:0 ,shipId:””,equipId:" ",pid:””}
//{seq:"ssssssss001",eid:0,data:[{eqid:"",id:"",snc:"",sne:" ",st:" ",pnu:" ",sto: ,su:" ",slc:" ",ls: ,hs: ,si:,en:" ",et:" ",es:" ",em:" "},{}......]}
int eMOPEquipSvc::GetEquipParts(const char* pUid, const char* jsonString, std::stringstream& out)
{// 低储预警 高储预警  
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetEquipParts]bad format:", jsonString, 1);
	string strShipid = root.getv("shipId", "");
	string strEquipid = root.getv("equipId", "");
	string strSeq = root.getv("seq", "");
	int iType = root.getv("type", -1);

	out<<"{eid:0,seq:\""<<strSeq.c_str()<<"\",data:[";

	char sql[4096] = "";

	MySql* psql = CREATE_MYSQL;

	char partid[64]="";
	char partcatid[64]="";
	char equipmodelid[64]="";
	char isimport[3]="";
	char equipname[128]="";
	int instock=0; //库存量
	char locationid[32]="";

	if(iType==0)
	{//获取某设备的备件

		sprintf (sql, "SELECT T1.PART_MODEL_ID,T1.PART_CAT_ID,T1.IS_IMPORTANT,T1.INSTOCK,T1.LOCATION_ID,T2.EQUIP_MODEL_ID,T2.NAME \
					  FROM blm_emop_etl.T50_EMOP_SHIP_PARTS T1,blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES T2 \
					  WHERE T1.SHIPID='%s' and T1.PART_CAT_ID='%s' AND T1.SHIPID=T2.SHIPID AND T1.PART_CAT_ID=T2.PART_CAT_ID",
					  strShipid.c_str(),strEquipid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		int idx=0;
		while (psql->NextRow())
		{

			READMYSQL_STR(PART_MODEL_ID, partid);//备件ID
			READMYSQL_STR(PART_CAT_ID, partcatid);//
			READMYSQL_STR(EQUIP_MODEL_ID, equipmodelid);
			READMYSQL_STR(MOIS_IMPORTANTDEL, isimport);
			READMYSQL_STR(NAME, equipname);
			READMYSQL_INT(INSTOCK, instock,0);
			READMYSQL_STR(LOCATION_ID, locationid);

			if (strlen(isimport)==0)
			{
				strcpy(isimport,"0");
			}


			SparePartsInfo *partInfo=g_eMOPCommDataSvr::instance()->GetPartItemById(string(partid));

			if (!partInfo)
			{				
				continue;
			}

			if (idx++)
			{
				out<<",";
			}
			string equipmodel="";
			string equiptype="";
			string equipmaker="";

			EquipBaseInfo *pEquipInfo=g_eMOPCommDataSvr::instance()->GetEquipItemById(string(equipmodelid));
			if (pEquipInfo)
			{
				equipmodel=pEquipInfo->m_model;
				equiptype=pEquipInfo->m_typecode;
				equipmaker=pEquipInfo->m_cmpyname;
			}

			string equipnameStr = equipname; 
			equipnameStr = StrReplace(equipnameStr,"\"","\\\"");

			out << "{eqid:\"" << partcatid <<"\",id:\"" << partid << "\",snc:\"";
			out << partInfo->m_name_cn <<"\",sne:\"" << partInfo->m_name_en<< "\",st:\""<< partInfo->m_model<< "\",pnu:\"";
			out << partInfo->m_charno<<"\",sto:" << instock<< ",su:\""<< partInfo->m_unit<< "\",slc:\"";
			out <<locationid<<"\",ls:" << partInfo->m_lowreq<< ",hs:"<< partInfo->m_highreq<< ",si:"<<isimport<<",en:\"";
			out <<equipnameStr<< "\",et:\"" << equipmodel << "\",es:\"" << equiptype <<"\",em:\"" << equipmaker<<"\"}";
		}

	}
	else //获取某设备包含子设备的所有备件 
	{


		//step1.检索出所有子设备

		char sql1[1024]="";
		sprintf(sql1,"select PART_CAT_ID,EQUIP_MODEL_ID,NAME from blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES \
					 where shipid='%s' and INSTR(PATH,'%s')",strShipid.c_str(),strEquipid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

		stringstream ss;
		map<string,PartMonthStatistic> mapEquipInfo;//设备信息，暂时借用PartMonthStatistic这个结构
		int idx=0;
		while (psql->NextRow())
		{

			PartMonthStatistic equipItem;

			READMYSQL_STRING(PART_CAT_ID, equipItem.equip_id);
			READMYSQL_STRING(NAME, equipItem.equip_name);
			READMYSQL_STRING(EQUIP_MODEL_ID, equipItem.equip_model);//设备型号ID
			if (idx++)
			{
				ss<<",";
			}
			ss<<"'"<<equipItem.equip_id<<"'";

			mapEquipInfo[equipItem.equip_id]=equipItem;
		}


		//step2.查找所有设备子设备及下的备件

		if (!mapEquipInfo.empty())
		{
			sprintf(sql, "SELECT T1.PART_MODEL_ID,T1.PART_CAT_ID,T1.IS_IMPORTANT,T1.INSTOCK,T1.LOCATION_ID \
						 FROM blm_emop_etl.T50_EMOP_SHIP_PARTS T1 WHERE T1.SHIPID='%s' AND T1.PART_CAT_ID IN (%s)",strShipid.c_str(),ss.str().c_str());
			DEBUG_LOG(sql);
			CHECK_MYSQL_STATUS(psql->Query(sql), 3);

			idx=0;
			while (psql->NextRow())
			{

				READMYSQL_STR(PART_MODEL_ID, partid);//备件ID
				READMYSQL_STR(PART_CAT_ID, partcatid);//
				READMYSQL_STR(MOIS_IMPORTANTDEL, isimport);
				READMYSQL_INT(INSTOCK, instock,0);
				READMYSQL_STR(LOCATION_ID, locationid);

				if (strlen(isimport)==0)
				{
					strcpy(isimport,"0");
				}

				SparePartsInfo *partInfo=g_eMOPCommDataSvr::instance()->GetPartItemById(string(partid)); 
				if (!partInfo)
				{ 
					continue;
				} 
				string equipname="";
				string equipmodelid="";

				string equipmodel="";//设备型号
				string equiptype="";//设备系列号
				string equipmaker="";//制造商

				if (mapEquipInfo.find(string(partcatid))!=mapEquipInfo.end())
				{
					PartMonthStatistic equipItem1=mapEquipInfo[string(partcatid)];

					equipname=equipItem1.equip_name;
					equipmodelid=equipItem1.equip_model;

					EquipBaseInfo *pEquipInfo=g_eMOPCommDataSvr::instance()->GetEquipItemById(string(equipmodelid));
					if (pEquipInfo)
					{
						equipmodel=pEquipInfo->m_model;
						equiptype=pEquipInfo->m_typecode;
						equipmaker=pEquipInfo->m_cmpyname;
					}

				}

				if (idx++)
				{
					out<<",";
				}


				equipname = StrReplace(equipname,"\"","\\\""); 
				out << "{eqid:\"" << partcatid << "\",id:\"" << partid << "\",snc:\"";
				out << partInfo->m_name_cn <<"\",sne:\"" << partInfo->m_name_en<< "\",st:\""<< partInfo->m_model<< "\",pnu:\"";
				out << partInfo->m_charno<<"\",sto:" << instock<< ",su:\""<< partInfo->m_unit<< "\",slc:\"";
				out <<locationid<<"\",ls:" << partInfo->m_lowreq<< ",hs:"<< partInfo->m_highreq<< ",si:"<<isimport<<",en:\"";
				out <<equipname<< "\",et:\"" << equipmodel << "\",es:\"" << equiptype <<"\",em:\"" << equipmaker<<"\"}";

			}


		}


	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID: 0x61    SID: 0x06
//{seq: "ssssssss001",shipId:””,equipId:””,id:" ",pid:""}
//{seq:"ssssssss001",eid:0,data:{id:" ",snc:"",sne:" ",st:" ",pnu:" ",sto: ,su:" ",sln:" ",slc:" ",ls: ,hs: ,en:" ",et:" ",es:" ";em:" ",pics:[" "," ",....],rem:[{per:" ",sub:" ",cont:" ",dt:""},{},......]}}
int eMOPEquipSvc::GetPartsPara(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsPara]bad format:", jsonString, 1);
	string strShipId=root.getv("shipId", "");
	string strEquipId = root.getv("equipId", ""); // 设备id
	string strPartsId = root.getv("id", "");// 备件id
	string strSeq= root.getv("seq", "");
	/*
	id：备件ID
	snc:中文名称
	sne:英文名称
	st:备件编号/型号
	pnu:图号
	sto:库存量
	su:单位
	sln:库位名称
	slc:库位代码
	ls:低储值
	hs:高储值
	en:所属设备名称
	et:设备型号
	es:设备系列号
	em:设备制造商
	pics:备件图片
	rem:备注数据
	per:填写人
	sub:主题
	cont:内容
	dt:时间
	*///少了是否重大备件
	out<<"{eid:0,seq:\""<<strSeq.c_str()<<"\",data:";
	char sql[1024] = "";
	MySql* psql = CREATE_MYSQL;
	char eqid[100] = "";
	char id[100] = "";
	char snc[100] = "";
	char sne[128]="";
	char st[128]="";
	char pnu[128]="";
	int iSto=0;
	char su[128]="";
	char slc_1[100]="";
	/*char slc_2[16]="";
	char slc_3[16]="";
	char slc_4[16]="";
	char slc_5[16]="";
	char sln_1[16]="";
	char sln_2[16]="";
	char sln_3[16]="";
	char sln_4[16]="";*/
	char sln_5[100]="";
	char ls[128]="";
	char si[128]="";
	char en[128]="";
	char et[128]="";
	char es[128]="";
	char em[128]="";
	int iHighl=0;//高储预警
	int iLowl=0;//低储预警
	int iStock=0;//库存量
	int iflag=-1;
	//	string strSlc;
	//	string strSln;

	//t4.MANU_CODE
	sprintf (sql, \
		"SELECT t1.PART_CAT_ID,t1.INSTOCK,t1.LOCATION_ID,t2.PART_MODEL_ID,t2.NAME_CN,t2.NAME_EN,t2.MODEL,t2.CHART_NO,t2.UNIT,\
		t2.HIGH_REQUIRED,t2.LOW_REQUIRED,t3.NAME AS LCNAME,t4.NAME AS EQNAME,t5.MODE as EQMODE,t5.COMPANY_NAME FROM \
		blm_emop_etl.t50_emop_ship_parts t1 LEFT JOIN \
		blm_emop_etl.t50_emop_parts_models t2 ON t1.PART_MODEL_ID=t2.PART_MODEL_ID LEFT JOIN \
		blm_emop_etl.t50_emop_ship_inventory_location t3 ON t1.LOCATION_ID=t3.LOCATION_ID LEFT JOIN \
		blm_emop_etl.t50_emop_part_ship_categories t4 ON t1.shipid=t4.shipid and t1.PART_CAT_ID=t4.PART_CAT_ID LEFT JOIN \
		blm_emop_etl.t50_emop_equip_model t5 ON t4.EQUIP_MODEL_ID=t5.EQUIP_MODEL_ID \
		WHERE t1.SHIPID = '%s' and t1.PART_CAT_ID='%s' and t1.PART_MODEL_ID='%s'",\
		strShipId.c_str(),strEquipId.c_str(),strPartsId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int cnt = 0;

	if (psql->NextRow())
	{
		READMYSQL_STR(PART_CAT_ID, eqid);
		READMYSQL_STR(PART_MODEL_ID, id);//备件ID
		READMYSQL_STR(NAME_CN, snc);//
		READMYSQL_STR(NAME_EN, sne);
		READMYSQL_STR(MODEL, st);
		READMYSQL_STR(CHART_NO, pnu);
		READMYSQL_STR(UNIT, su);//
		READMYSQL_INT(LOW_REQUIRED, iLowl,0);//
		READMYSQL_INT(HIGH_REQUIRED, iHighl,0);//
		READMYSQL_INT(INSTOCK, iSto,0);// t2
		//		READMYSQL_STR(LOCATION_ID, remark);//t2,需要t3  T50_EMOP_SHIP_INVENTORY_LOCATION		
		//		READMYSQL_INT(IS_IMPORTANT, iflag, -1);//t2 si:是否是重大备件 ,0 不是，1是
		//
		READMYSQL_STR(EQNAME, en);// 设备表数据,t4
		//READMYSQL_STR(MANU_CODE, es);//
		READMYSQL_STR(EQMODE, et);//
		READMYSQL_STR(COMPANY_NAME, em);//			
		// 库位代码 
		/*READMYSQL_STR(DECK, slc_1);
		READMYSQL_STR(ROOM, slc_2);
		READMYSQL_STR(SHELF, slc_3);
		READMYSQL_STR(LAYER, slc_4);
		READMYSQL_STR(BOX, slc_5);
		strSlc+=slc_1;strSlc+=slc_2;strSlc+=slc_3;strSlc+=slc_4;strSlc+=slc_5;*/
		// 库位名称 
		/*READMYSQL_STR(DECK_NAME, sln_1);
		READMYSQL_STR(ROOM_NAME, sln_2);
		READMYSQL_STR(SHELF_NAME, sln_3);
		READMYSQL_STR(LAYER_NAME, sln_4);
		READMYSQL_STR(BOX_NAME, sln_5);
		strSln=sln_1;strSln+=sln_2;strSln+=sln_3;strSln+=sln_4;strSln+=sln_5;*/
		READMYSQL_STR(LOCATION_ID, slc_1);//
		READMYSQL_STR(LCNAME, sln_5);

		string name_cn = snc;
		name_cn=StrReplace(name_cn,"\"","\\\"");
		string name_en = sne;
		name_en=StrReplace(name_en,"\"","\\\""); 
		//{id:" ",snc:"",sne:" ",st:" ",pnu:" ",sto: ,su:" ",sln:" ",slc:" ",ls: ,hs: ,en:" ",et:" ",es:" ";em:" ",pics:[" "," ",....],rem:[{per:" ",sub:" ",cont:" ",dt:""},{},......]}}
		if (cnt++)
			out << ",";
		out << "{id:\"" << id << "\",snc:\"" << name_cn << "\",sne:\"" << name_en <<"\",st:\"" << st<< "\",pnu:\""<< pnu\
			<<"\",sto:"<< iSto << ",su:\"" << su << "\",sln:\"" << sln_5 << "\",slc:\"" << slc_1 << "\",ls:" << iLowl \
			<< ",hs:" << iHighl << ",en:\"" << en << "\",et:\"" << et<< "\",es:\"" << es << "\",em:\"" << em  << "\","<<"pics:[],rem:[]}";
	}
	out<<"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID: 0x61    SID: 0x07
//{seq: "ssssssss001",type:0,shipId:””,eqid:"",id:"",snc:"",sne:" ",st:" ",pnu:" ",sto: ,su:" ",sln:" ",slc:" ",ls: ,hs: ,pics:[" "," ",....],rem:[{per:" ",sub:" ",cont:" ",dt:""},{},......]}
//{ seq: "ssssssss001",eid:0,id: ""}
int eMOPEquipSvc::NewParts(const char* pUid, const char* jsonString, std::stringstream& out)
{
	/*
	说明：
	新建备件同时需要插入
	1.备件表 t50_emop_parts_models
	2.备件船舶关系表t50_emop_ship_parts   

	问题：
	图片和备注还没定，备注可能需要设计一个统一接口
	*/


	JSON_PARSE_RETURN("[eMOPEquipSvc::NewParts]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strShipId = root.getv("shipId", "");
	string strEquipId = root.getv("eqid", "");
	string strId = root.getv("id", "");//备件ID,新增备件时为空
	string strNc = root.getv("snc", "");//中文名
	string strNe = root.getv("sne", "");//英文名
	string strSt = root.getv("st", "");//备件编号/型号
	string strPnu = root.getv("pnu", "");//图号t50_emop_parts_models
	string strSu = root.getv("su", "");// 单位 t50_emop_parts_models	
	int iLs = root.getv("ls", 0);//低储值t50_emop_parts_models
	int iHs = root.getv("hs", 0);//高储值t50_emop_parts_models
	int iSto = root.getv("sto", 0);// 库存量 

	string locationid=root.getv("slc", ""); //库位ID
	string locationnm = root.getv("sln", ""); //库位名称

	int iType=root.getv("type", -1);//0添加 1修改

	string upddtStr=GetCurrentTmStr();

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 

	if(iType==0)
	{   
		//新建 
		string sequence="";
		GET_EMOP_SEQUENCE_STRING(P,sequence);
		strId="P"+CurLocalDateSeq()+sequence;

		// 备件表
		sprintf (sql, "insert into blm_emop_etl.t50_emop_parts_models(PART_MODEL_ID,NAME_CN,NAME_EN,MODEL,CHART_NO,UNIT,HIGH_REQUIRED,LOW_REQUIRED,LAST_UPD_DT) values ('%s','%s','%s','%s','%s','%s','%d','%d','%s')",\
			strId.c_str(),strNc.c_str(), strNe.c_str(), strSt.c_str(),strPnu.c_str(),strSu.c_str(),iHs ,iLs,upddtStr.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		// 备件设备船舶关系表
		sprintf (sql, "insert into blm_emop_etl.t50_emop_ship_parts(PART_CAT_ID,SHIPID, PART_MODEL_ID,INSTOCK,LOCATION_ID) values ('%s','%s','%s','%d','%s')",
			strEquipId.c_str(),strShipId.c_str(), strId.c_str(),iSto,locationid.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	}
	else  
	{ //修改备件信息 id确定
		sprintf (sql, "UPDATE blm_emop_etl.t50_emop_parts_models SET NAME_CN = '%s', NAME_EN = '%s',MODEL='%s',CHART_NO='%s',UNIT='%s',HIGH_REQUIRED='%d',LOW_REQUIRED='%d' WHERE PART_MODEL_ID = '%s'",\
			strNc.c_str(),strNe.c_str(),strSt.c_str(),strPnu.c_str(),strSu.c_str(),iHs,iLs ,strId.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


		sprintf (sql, "UPDATE blm_emop_etl.t50_emop_ship_parts SET LOCATION_ID='%s' where SHIPID='%s' AND PART_CAT_ID='%s' AND PART_MODEL_ID='%s'",
			locationid.c_str(),strShipId.c_str(),strEquipId.c_str(),strId.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	}

	//加入到缓存
	SparePartsInfo partInfo;
	partInfo.m_partid=strId;
	partInfo.m_name_en=StrReplace(strNe,"\"","\\\"");
	partInfo.m_name_cn=StrReplace(strNc,"\"","\\\"");
	partInfo.m_model=strSt;
	partInfo.m_charno=strPnu;
	partInfo.m_unit=strSu;
	partInfo.m_highreq=iHs;
	partInfo.m_lowreq=iLs;

	g_eMOPCommDataSvr::instance()->AddNewPartItem(partInfo);


	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",id:\""<<strId.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//Mid:0x60  SID: 0x0008
//{seq: "ssssssss001",shipId:””,id:" "}
//{seq: "ssssssss001",eid:0}
int eMOPEquipSvc::DelParts(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::DelMaterialsInfo]bad format:", jsonString, 1);
	string strShipId = root.getv("shipId", "");
	string strId = root.getv("id", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_ship_parts WHERE SHIPID = '%s' and PART_MODEL_ID= '%s'", strShipId.c_str(),strId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
//MID: 0x61    SID: 0x1f
//{seq:””,shipid:”324234”,equipid:”000001” }
int eMOPEquipSvc::GetPaths(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPaths]bad format:", jsonString, 1);
	string strShipId = root.getv("shipId", "");
	string equipid = root.getv("equipid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[2048] = ""; 

	sprintf (sql, "SELECT PATH FROM blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES WHERE SHIPID='%s' AND PATH LIKE '%s#%s'", strShipId.c_str(),equipid.c_str(),"%%");

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 

	out<<"{seq:\""<<equipid<<"\",data:["; 
	int cnt=0; 
	char path[1024]="";//:申请单号  申请单表
	while (psql->NextRow())
	{
		READMYSQL_STR(PATH, path);
		if (cnt++)
			out << ","; 
		out <<"\""<< path<<"\""; 
	}
	out<<"]}"; 
	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID: 0x61    SID: 0x1b
//{ seq: "ssssssss001",sid:" ",equipId:””,id:" ",name:" ",code:" "}
//{seq: "ssssssss001",eid:0}
int eMOPEquipSvc::SetStorePos(const char* pUid, const char* jsonString, std::stringstream& out)
{// 问题：更改现有备件位置？（数量不变）还是新增备件设置位置（需要存储数量）？  另外需要增加设备id
	// 库位名字不能修改
	JSON_PARSE_RETURN("[eMOPEquipSvc::SetStorePos]bad format:", jsonString, 1);
	string strShipID=root.getv("sid", "");
	string strPid=root.getv("id", "");
	string strEid=root.getv("equipId", "");
	string strName=root.getv("name", "");//库位名称
	string strCode=root.getv("code", "");//库位代码
	string strSeq = root.getv("seq", "");//
	// code解析
	Tokens svrList;	
	if(strCode.find("-") != string::npos)
	{					 
		svrList = StrSplit(strCode, "-");		
	}
	//	out<<"{seq:\""<<strSeq.c_str()<<"\",info:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//sprintf (sql, \
	//	"select * from t50_emop_ship_inventory_location  where SHIPID='%s' and DECK='%s' ROOM='%s' and SHELF='%s' and LAYER='%s' and BOX='%s'",\
	//	strShipID.c_str(),svrList[0].c_str(),svrList[1].c_str(),svrList[2].c_str(),svrList[3].c_str(),svrList[4].c_str());	
	//CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	//char szLid[128];
	//if (psql->NextRow())
	//{//t50_emop_ship_parts
	//	READMYSQL_STR(LOCATION_ID, szLid);//备件ID
	//	sprintf(sql,\
	//		"insert into blm_emop_etl.t50_emop_ship_parts(PART_CAT_ID,SHIPID, LOCATION_ID,PART_MODEL_ID) values ('%s','%s','%s','%s')",\
	//		strEid.c_str(),strShipID.c_str(),szLid,strPid.c_str() );	
	//}
	sprintf(sql,\
		"update blm_emop_etl.t50_emop_ship_parts set LOCATION_ID='%s' where SHIPID='%s' and PART_CAT_ID='%s' and PART_MODEL_ID='%s'",\
		strCode.c_str(),strShipID.c_str(),strEid.c_str(),strPid.c_str() );
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:\"0"<<"\",seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID: 0x61    SID: 0x09
//{ seq: "ssssssss001",id:" ",shipId:””,equipId:””,startTime:" "，endTime:””}
//{seq: "ssssssss001",eid:0,purchase:[{aon:" ",adt:" ",ast:" ",sto: ,aq:,unit:" ",price:,curr:" ",per:" ",aun: ,acn: ,slc:" ",sln:" "},{},.....]}
/*
这个接口功能很难理解和求取，只取了部分数据。
可能存在的问题：
1 只有待审核申请单，返回数据只有申请单号，日期，申请人，申请数量，其他数据全无。
2 以申请单为起点去向后查状态，直到入库表，不一定可达，后边数据无，申请状态不知道如何获取
我所理解采购历史是这样的：
1 采购入库了的备件，由后向前追溯历史。
2 其实时间和结束时间也为入库时间
3 检索条件为：shipid+partsid+starttime+endtime
4 数据可能需要过滤，流程不完整，到申请单，申请单流程表taskid stepid包含批复和询价，以申请单id查出的流程及其数量都是申请单的，
也就是询价数量不正确。询价无流程存储
5 我理解的接口：
[{aon:" ",adt:" ",ast:" ",sto: ,aq:,unit:" ",price:,curr:" ",per:" ",aun: ,acn: ,slc:" ",sln:" "},{},.....]
aun: 订购（最终）审核数量
去掉申请状态（或者单独一个接口查询这个时间段内所有申请单的数据，包括最新状态）
*/
int eMOPEquipSvc::GetPartsOrderHis(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsOrderHis]bad format:", jsonString, 1);
	string strID=root.getv("id", "");//备件的ID
	int iStartTime=root.getv("startTime", -1);
	int iEndTime=root.getv("endTime", -1);
	string strSeq = root.getv("seq", "");
	string strShipId = root.getv("shipId", "");
	string strEquipId = root.getv("equipId", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",purchase:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//
	//aon:申请单号  申请单表
	//adt:申请日期   申请单表
	//ast:申请状态  //taskid???
	//sto: 库存量   申请单备件关系表
	//aq:申请数量  T50_EMOP_PARTS_APPLY_DETAILS 
	//unit:单位     询价表 or 订购表
	//price:单价  订购？
	//curr:币种 询价表 or 订购表
	//per:申请人   申请单表
	//aun: 审核数量
	//acn: 验收数量  //入库表
	//slc:库位代码   //
	//sln:库位名称 

	sprintf (sql, \
		"select t1.QUATITY as appquantity,t2.PARTS_APPLY_ID,UNIX_TIMESTAMP(t2.APPLY_DATE) as t1apptm,t2.USER_ID as appuser,t3.INSTOCK,t3.LOCATION_ID \
		,t10.UNIT,t5.CURRENCY,t7.UNIT_PRICE,t7.QUATITY as odquantity,t8.QUATITY as inquantity,t9.NAME AS STOCKNM from \
		blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS t1 left join \
		blm_emop_etl.T50_EMOP_PARTS_APPLIES t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID left join \
		blm_emop_etl.T50_EMOP_SHIP_PARTS t3 on t1.PART_MODEL_ID=t3.PART_MODEL_ID and t2.SHIPID=t3.SHIPID left join \
		blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t4 on	t1.PARTS_APPLY_ID=t4.PARTS_APPLY_ID left join \
		blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t5 on t4.PARTS_APPLY_INQUERY_ID=t5.PARTS_APPLY_INQUERIED_QUOTED_ID left join \
		blm_emop_etl.T50_EMOP_PARTS_INSTOCK t6 on t5.PARTS_PURCHASE_ORDER_ID=t6.PARTS_PURCHASE_ORDER_ID left join \
		blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS t7 on t5.PARTS_PURCHASE_ORDER_ID=t7.PARTS_PURCHASE_ORDER_ID and t1.PART_MODEL_ID=t7.PART_MODEL_ID left join \
		blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS t8 on t6.PART_INSTOCK_ID=t8.PART_INSTOCK_ID and t1.PART_MODEL_ID=t8.PART_MODEL_ID LEFT JOIN \
		blm_emop_etl.t50_emop_ship_inventory_location t9 ON t3.LOCATION_ID=t9.LOCATION_ID left join\
		blm_emop_etl.T50_EMOP_PARTS_MODELS t10 ON t1.PART_MODEL_ID=t10.PART_MODEL_ID \
		where t1.PART_MODEL_ID='%s' and t2.SHIPID='%s' and t2.APPLY_DATE between FROM_UNIXTIME('%d') and FROM_UNIXTIME('%d') ",\
		strID.c_str(),strShipId.c_str(),iStartTime,iEndTime);	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char aon[100]="";//:申请单号  申请单表
	int iAdt=0;//:申请日期   申请单表
	char ast[100]="";//:申请状态  //taskid???
	int iSto=0;//: 库存量   申请单备件关系表
	int iAq=0;//:申请数量  T50_EMOP_PARTS_APPLY_DETAILS 
	char unit[100]="";//:单位     询价表 or 订购表
	double ffPrice=0;//:单价  订购？
	int iCurr=0;//:币种 询价表 or 订购表
	char per[100]="";//:申请人   申请单表
	int iAun=0;//: 审核数量
	int iAcn=0;//: 验收数量  //入库表
	char slc[100]="";//:库位代码   //
	char sln[100]="";//:库位名称 

	int cnt = 0;	
	while (psql->NextRow())
	{
		READMYSQL_STR(PARTS_APPLY_ID, aon);
		READMYSQL_INT(t1apptm, iAdt,0);
		READMYSQL_INT(INSTOCK, iSto,0);
		READMYSQL_INT(appquantity, iAq,0);
		READMYSQL_STR(UNIT, unit);
		READMYSQL_DOUBLE(UNIT_PRICE, ffPrice, 0.0);
		READMYSQL_INT(CURRENCY, iCurr, -1);
		READMYSQL_STR(appuser, per);
		READMYSQL_INT(odquantity, iAun, -1);
		READMYSQL_INT(inquantity, iAcn, -1);
		READMYSQL_STR(LOCATION_ID, slc);
		READMYSQL_STR(STOCKNM, sln);
		//{aon:" ",adt:" ",ast:" ",sto: ,aq:,unit:" ",price:,curr:" ",per:" ",aun: ,acn: ,slc:" ",sln:" "}
		if (cnt++)
			out << ",";
		out << "{aon:\"" << aon << "\",adt:" << iAdt << ",ast:\"\""<< "\",sto:" << iSto <<",aq:" << iAq<< ",unit:\""<<unit\
			<< "\",price:" << ffPrice << ",curr:" << iCurr << ",per:\"" << per << "\",aun:" << iAun  << ",acn:\"" << iAcn  \
			<< ",slc:\"" << slc << "\",sln:\"" << sln << "\"}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);	
}
//MID: 0x61    SID: 0x10
//{ seq: "ssssssss001",id:" ",shipId:””,equipId:””,startTime:，endTime:}
//{seq: "ssssssss001",eid:0,oistore:[{ooi: ,on:" ",dt:" ",ope:" ",type:" ",quan: ,cbs:" ",price:,curr:" "},{},....]}
int eMOPEquipSvc::GetPartsStoreHis(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsStoreHis]bad format:", jsonString, 1);
	string strID=root.getv("id", "");//备件的ID
	int iStartTime=root.getv("startTime", -1);
	int iEndTime=root.getv("endTime", -1);
	string strSeq = root.getv("seq", "");
	string strShipId = root.getv("shipId", "");
	string strEquipId = root.getv("equipId", "");
	//ooi:0代表出库，1代表入库
	//on:单号 t1
	//quan:数量 t1
	//cbs:当前批次库存（入库）t1
	//dt:日期 t2
	//ope:经办人 t2
	//type:类型，入库有（正常、盘点、其他），出库有（领用、保养、报废、丢失、盘点）	 t2
	//price:入库单价 t3
	//curr:入库币种 t4
	out<<"{seq:\""<<strSeq.c_str()<<"\",oistore:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.PART_INSTOCK_ID,t1.QUATITY,t1.INSTOCK_REST,UNIX_TIMESTAMP(t2.OP_DATE) AS OPDATE,t2.OPERATOR,t2.STOCK_STATUS,t3.UNIT_PRICE,t4.CURRENCY FROM\
				  blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS t1 left join\
				  blm_emop_etl.T50_EMOP_PARTS_INSTOCK t2 on t1.PART_INSTOCK_ID=t2.PART_INSTOCK_ID left join \
				  blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS t3 on t2.PARTS_PURCHASE_ORDER_ID=t3.PARTS_PURCHASE_ORDER_ID and t1.PART_MODEL_ID=t3.PART_MODEL_ID left join\
				  blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t4 on t2.PARTS_PURCHASE_ORDER_ID=t4.PARTS_PURCHASE_ORDER_ID\
				  where t1.PART_MODEL_ID='%s' and t2.SHIPID='%s' and t2.OP_DATE between FROM_UNIXTIME('%d') and FROM_UNIXTIME('%d') ORDER BY OP_DATE",strID.c_str(),strShipId.c_str(),iStartTime,iEndTime);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	//	ooi:0代表出库，1代表入库
	char on[100]="";//:单号 t1
	int iQuan=0;//:数量 t1
	int iCbs=0;//:当前批次库存（入库）t1
	int iDt=0;//:日期 t2
	char ope[100]="";//:经办人 t2
	int iType=0;//:类型，入库有（正常、盘点、其他），出库有（领用、保养、报废、丢失、盘点）	 t2
	double ffPrice=0;//:入库单价 t3
	int iCurr=0;//:入库币种 t4
	int cnt=0;
	while (psql->NextRow())
	{
		READMYSQL_STR(PART_INSTOCK_ID, on);
		READMYSQL_STR(OPERATOR, ope);
		READMYSQL_INT(QUATITY,iQuan ,0);
		READMYSQL_INT(INSTOCK_REST,iCbs ,0);
		READMYSQL_INT(OPDATE, iDt,0);
		READMYSQL_INT(STOCK_STATUS,iType ,0);
		READMYSQL_INT(CURRENCY,iCurr ,0);
		READMYSQL_DOUBLE(UNIT_PRICE,ffPrice ,0.0);
		if (cnt++)
			out << ",";
		//{ooi: ,on:" ",dt:" ",ope:" ",type:" ",quan: ,cbs:" ",price:,curr:" "}
		out<<"{ooi:1"<<",on:\""<<on<<"\",dt:"<<iDt<<",ope:\""<<ope<<"\",type:"<<iType<<",quan:"<<iQuan<<",cbs:"<<iCbs<<",price:"<<ffPrice<<",curr:"<<iCurr<<"}";
	}
	//出库历史
	sprintf (sql, "SELECT t1.PART_OUTSTOCK_ID,t1.QUATITY,UNIX_TIMESTAMP(t2.OP_DATE) AS OPDATE,t2.OPERATOR,t2.STOCK_STATUS FROM\
				  blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK_DETAILS t1 left join\
				  blm_emop_etl.T50_EMOP_PARTS_OUTSTOCK t2 on t1.PART_OUTSTOCK_ID=t2.PART_OUTSTOCK_ID \
				  where t1.PART_MODEL_ID='%s'and t2.SHIPID='%s' and t2.OP_DATE between FROM_UNIXTIME('%d') and FROM_UNIXTIME('%d') ORDER BY OP_DATE",strID.c_str(),strShipId.c_str(),iStartTime,iEndTime);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while (psql->NextRow())
	{
		READMYSQL_STR(PART_OUTSTOCK_ID, on);
		READMYSQL_STR(OPERATOR, ope);
		READMYSQL_INT(QUATITY,iQuan ,0);
		//		READMYSQL_INT(INSTOCK_REST,iCbs ,0);
		READMYSQL_INT(OPDATE, iDt,0);
		READMYSQL_INT(STOCK_STATUS,iType ,0);
		//		READMYSQL_INT(CURRENCY,iCurr ,0);
		//		READMYSQL_DOUBLE(UNIT_PRICE,ffPrice ,0.0);
		out << ",";
		//{ooi: ,on:" ",dt:" ",ope:" ",type:" ",quan: ,cbs:" ",price:,curr:" "}
		out<<"{ooi:0"<<",on:\""<<on<<"\",dt:"<<iDt<<",ope:\""<<ope<<"\",type:"<<iType<<",quan:"<<iQuan<<",cbs:"<<0<<",price:"<<0<<",curr:"<<0<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//MID: 0x61    SID: 0x1a
//{ seq: "ssssssss001",id:" "}	
//{seq: "ssssssss001",eid:0,shipId:””,equipId:””,stock:[{on:" ",type:" ",dt:" ",quan: ,req: ,price: ,curr:" "},{},.....]}
int eMOPEquipSvc::GetPartsInstock(const char* pUid, const char* jsonString, std::stringstream& out)
{ // 
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsStoreHis]bad format:", jsonString, 1);

	string strPartsID=root.getv("id", "");//备件的ID;
	string strSeq = root.getv("seq", "");
	string strShipId = root.getv("shipId", "");
	string strEquipId = root.getv("equipId", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",stock:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql,\
		"SELECT t1.PART_INSTOCK_ID,t1.QUATITY,t1.INSTOCK_REST,UNIX_TIMESTAMP(t2.OP_DATE) AS OPDATE,t3.UNIT_PRICE,t4.CURRENCY FROM\
		blm_emop_etl.T50_EMOP_PARTS_INSTOCK_DETAILS t1 left join\
		blm_emop_etl.T50_EMOP_PARTS_INSTOCK t2 on t1.PART_INSTOCK_ID=t2.PART_INSTOCK_ID left join \
		blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS t3 on t2.PARTS_PURCHASE_ORDER_ID=t3.PARTS_PURCHASE_ORDER_ID and t1.PART_MODEL_ID=t3.PART_MODEL_ID left join\
		blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t4 on t2.PARTS_PURCHASE_ORDER_ID=t4.PARTS_PURCHASE_ORDER_ID\
		where t1.PART_MODEL_ID='%s' and t1.REST>0 and t2.SHIPID='%s' ",strPartsID.c_str(),strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char on[100]="";//:入库单号
	int iType=0;//:入库类型
	int iDt=0;//:入库时间
	int iQuan=0;//: 入库数量
	int iReq=0;//: 剩余数量
	double ffPrice=0;//: 入库单价
	int iCurr=0;//:币种

	int cnt=0;
	while (psql->NextRow())
	{
		READMYSQL_STR(PART_INSTOCK_ID, on);
		//		READMYSQL_STR(OPERATOR, ope);
		READMYSQL_INT(QUATITY,iQuan ,0);
		READMYSQL_INT(INSTOCK_REST,iReq ,0);
		READMYSQL_INT(OPDATE, iDt,0);
		READMYSQL_INT(STOCK_STATUS,iType ,0);
		READMYSQL_INT(CURRENCY,iCurr ,0);
		READMYSQL_DOUBLE(MATERIALS_CAT_ID,ffPrice ,0.0);
		if (cnt++)
			out << ",";
		//{on:" ",type:" ",dt:" ",quan: ,req: ,price: ,curr:" "}
		out<<"{on:\""<<on<<"\",dt:"<<iDt<<",type:"<<iType<<",quan:"<<iQuan<<",req:"<<iReq<<",price:"<<ffPrice<<",curr:"<<iCurr<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//MID: 0x61    SID: 0x1c
//{ seq: "ssssssss001",sid:" ",key:" "}
//{seq:"ssssssss001",eid:0,data:[{eqid:"",id:"",snc:"",sne:"",st:"",pnu:"",sto: ,su:"",slc:"",ls: ,hs: ,si: ,en:" ",et:" ",es:" ",em:" "},{}......]}
int eMOPEquipSvc::SearchPartsOnShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::SearchPartsOnShip]bad format:", jsonString, 1);
	string strKeyword = root.getv("key", "");
	string strSeq= root.getv("seq", "");
	string strShipId=root.getv("sid", "");


	out<<"{seq:\""<<strSeq.c_str()<<"\",data:[";

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf(sql, 
		"SELECT T1.PART_MODEL_ID,T1.PART_CAT_ID,T1.IS_IMPORTANT,T1.INSTOCK,T1.LOCATION_ID,T2.EQUIP_MODEL_ID, T2.NAME \
		FROM blm_emop_etl.T50_EMOP_SHIP_PARTS T1,blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES T2 \
		WHERE T1.SHIPID='%s' and T1.SHIPID=T2.SHIPID AND T1.PART_CAT_ID=T2.PART_CAT_ID",
		strShipId.c_str());	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char partid[64]="";
	char partcatid[64]="";
	char equipmodelid[64]="";
	char isimport[3]="";
	char equipname[128]="";
	int instock=0;//库存量
	char locationid[32]="";

	int idx=0;
	while (psql->NextRow())
	{

		READMYSQL_STR(PART_MODEL_ID, partid);//备件ID
		READMYSQL_STR(PART_CAT_ID, partcatid);//
		READMYSQL_STR(EQUIP_MODEL_ID, equipmodelid);
		READMYSQL_STR(MOIS_IMPORTANTDEL, isimport);
		READMYSQL_STR(NAME, equipname);
		READMYSQL_INT(INSTOCK, instock,0);
		READMYSQL_STR(LOCATION_ID, locationid);

		if (strlen(isimport)==0)
		{
			strcpy(isimport,"0");
		}

		SparePartsInfo *partInfo=g_eMOPCommDataSvr::instance()->GetPartItemById(string(partid));

		if (!partInfo)
		{
			continue;
		}

		char keyUpper[128]=""; 
		char NameUper[128]="";   
		char ModelUper[128]=""; 
		strcpy(keyUpper,strKeyword.c_str());
		strcpy(NameUper,partInfo->m_name_en.c_str());
		strcpy(ModelUper,partInfo->m_model.c_str());

		StrUpr(keyUpper);
		StrUpr(NameUper);
		StrUpr(ModelUper);


		//搜索匹配
		if (strstr(partInfo->m_name_cn.c_str(),strKeyword.c_str())!=NULL||strstr(NameUper,keyUpper)!=NULL||strstr(ModelUper,keyUpper)!=NULL)
		{
			if (idx>0)
			{
				out<<",";
			}
			string equipmodel="";
			string equiptype="";
			string equipmaker="";

			EquipBaseInfo *pEquipInfo=g_eMOPCommDataSvr::instance()->GetEquipItemById(string(equipmodelid));
			if (pEquipInfo)
			{
				equipmodel=pEquipInfo->m_model;
				equiptype=pEquipInfo->m_typecode;
				equipmaker=pEquipInfo->m_cmpyname;
			}


			out << "{eqid:\"" << partcatid << "\",pid:\"" << equipmodelid << "\",id:\"" << partid << "\",snc:\"";
			out	<< partInfo->m_name_cn <<"\",sne:\"" << partInfo->m_name_en<< "\",st:\""<< partInfo->m_model<< "\",pnu:\"";
			out	<< partInfo->m_charno<<"\",sto:" << instock<< ",su:\""<< partInfo->m_unit<< "\",slc:\"";
			out	<<locationid<<"\",ls:" << partInfo->m_lowreq<< ",hs:"<< partInfo->m_highreq<< ",si:"<<isimport<<",en:\"";
			out	<<equipname<< "\",et:\"" << equipmodel << "\",es:\"" << equiptype <<"\",em:\"" << equipmaker<<"\"}";
			idx++;
		}
	}

	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);	

}
// 根据userid获取部门id
int eMOPEquipSvc::GetDptFromUserid(string &o_strDptid,const string &i_strUid)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT DEPARTMENT_CODE FROM blm_emop_etl.t50_emop_users WHERE USER_ID='%s'",i_strUid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char id[100] = "";
	if(psql->NextRow())
	{
		READMYSQL_STR(DEPARTMENT_CODE, id);
		o_strDptid=id;
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}
// MID： 0x0061    SID： 0x0020
//{ seq： "ssssssss001",uid："",shipid："",which:0,taskId: "",stepId: "",status:0}
//{seq：”sssss”,sid:”ssssss0001”,time:1233212}
int eMOPEquipSvc::NewApplyOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::NewApplyOrder]bad format:", jsonString, 1);
	string strShipId = root.getv("shipid", "");
	string strUid= root.getv("uid", "");
	string strSeq= root.getv("seq", "");
	int iType=root.getv("which", -1);
	string strTaskid=root.getv("taskId", "");//taskId:,stepId;
	string strStepid=root.getv("stepId", "");
	int iStatus=root.getv("status", -1);
	string strAOId,DptId;
	strAOId="BPX";
	char sztmp[32];
	int iT=time(0);
	sprintf(sztmp,"%d",iT);
	strAOId+=sztmp;
	//	newAppOrderNum(strAOId,strShipId);	
	GetDptFromUserid(DptId,strUid);
	int iTime=time(0);
	// 空订单 入t50_emop_parts_applies
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	// 申请单表
	sprintf (sql, "insert into blm_emop_etl.t50_emop_parts_applies(PARTS_APPLY_ID,DEPARTMENT_CODE,USER_ID,SHIPID, APPLY_DATE) values ('%s','%s','%s','%s',FROM_UNIXTIME(%d))",\
		strAOId.c_str(),DptId.c_str(),strUid.c_str(),strShipId.c_str() ,iTime);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	// 状态表
	//	if(iType==0)//船端
	//	{// 船端申请需要三步
	// work step id: 1/11 2/11 3/11
	//		sprintf (sql, "insert into blm_emop_etl.t50_emop_parts_apply_status(PARTS_APPLY_ID,OP_DATE,STATUS,WORKFLOW_STEP_ID,WORKFLOW_TASK_ID) values ('%s',FROM_UNIXTIME(%d),'0','1','11')",\
	strAOId.c_str(),iTime);
	//	}
	//	else
	//	{// work step id:1/21
	//		sprintf (sql, "insert into blm_emop_etl.t50_emop_parts_apply_status(PARTS_APPLY_ID,OP_DATE,STATUS,WORKFLOW_STEP_ID,WORKFLOW_TASK_ID) values ('%s',FROM_UNIXTIME(%d),'0','1','21')",\
	strAOId.c_str(),iTime);
	//	}
	sprintf (sql, "insert into blm_emop_etl.t50_emop_parts_apply_status(PARTS_APPLY_ID,OP_DATE,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,STATUS) values ('%s',FROM_UNIXTIME(%d),'%s','%s','%d')",\
		strAOId.c_str(),iTime,strTaskid.c_str(),strStepid.c_str(),iStatus);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out << "{seq:\"" << strSeq.c_str() << "\",sid:\"" << strAOId.c_str() << "\",time:\"" <<iTime<< "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);	
}
//MID：0x0061     SID:0x0021
//{seq:”sssss”,ship:”ssid”,part:”ddddd”,state:1,taskId:””,stepId:””}
//{seq:””,sid:”s0001”,data:[{aid:”sss”,time:12313,st:1},{sid:”s0001”,aid:”sss”,time:12313,st:1},....]}
int eMOPEquipSvc::SearchAppOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::SearchAppOrder]bad format:", jsonString, 1);
	string strShipId = root.getv("ship", "");
	//	string strUid= root.getv("uid", "");
	string strPartId= root.getv("part", "");
	string strSeq= root.getv("seq", "");
	string strTaskId=root.getv("taskId", "");
	string strStepId=root.getv("stepId", "");
	int iState= root.getv("state", -1);

	string strTmp="";
	Tokens svrList;	
	if(strStepId.find("|") != string::npos)
	{					 
		svrList = StrSplit(strStepId, "|");
		for(int i=0;i<svrList.size();i++)
		{
			if(svrList[i].empty())
				continue;
			strTmp+="'";
			strTmp+=svrList[i];
			strTmp+="'";
			if (i<((int)svrList.size()-1))
				strTmp+=",";
		}
	}
	else
	{
		strTmp+="'";
		strTmp+=strStepId;
		strTmp+="'";
	}

	out<<"{seq:\""<<strSeq.c_str()<<"\",data:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	if(strPartId.empty()&&(iState>3||iState<0))
	{ // 部门和状态都无限制
		sprintf (sql, \
			"select t1.PARTS_APPLY_ID,UNIX_TIMESTAMP(t1.APPLY_DATE) as apptm,t2.STATUS from \
			blm_emop_etl.t50_emop_parts_applies t1 left join blm_emop_etl.t50_emop_parts_apply_status t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID \
			where t1.SHIPID='%s'and t2.WORKFLOW_TASK_ID='%s' and t2.WORKFLOW_STEP_ID IN(%s) order by apptm desc",\
			strShipId.c_str(),strTaskId.c_str(),strTmp.c_str());
	}
	else if(strPartId.empty())
	{// 部门无限制
		sprintf (sql, \
			"select t1.PARTS_APPLY_ID,UNIX_TIMESTAMP(t1.APPLY_DATE) as apptm,t2.STATUS from \
			blm_emop_etl.t50_emop_parts_applies t1 left join blm_emop_etl.t50_emop_parts_apply_status t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID \
			where t1.SHIPID='%s' and t2.STATUS='%d' and t2.WORKFLOW_TASK_ID='%s' and t2.WORKFLOW_STEP_ID IN(%s) order by apptm desc",\
			strShipId.c_str(), iState,strTaskId.c_str(),strTmp.c_str());
	}
	else if(iState>3||iState<0)
	{ // 状态无限制
		sprintf (sql, \
			"select t1.PARTS_APPLY_ID,UNIX_TIMESTAMP(t1.APPLY_DATE) as apptm,t2.STATUS from \
			blm_emop_etl.t50_emop_parts_applies t1 left join blm_emop_etl.t50_emop_parts_apply_status t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID \
			where t1.SHIPID='%s'and t1.DEPARTMENT_CODE='%s' and t2.WORKFLOW_TASK_ID='%s' and t2.WORKFLOW_STEP_ID IN(%s) order by apptm desc",\
			strShipId.c_str(),strPartId.c_str(),strTaskId.c_str(),strTmp.c_str());
	}
	else
	{ // 都有限制
		sprintf (sql, \
			"select t1.PARTS_APPLY_ID,UNIX_TIMESTAMP(t1.APPLY_DATE) as apptm,t2.STATUS from \
			blm_emop_etl.t50_emop_parts_applies t1 left join blm_emop_etl.t50_emop_parts_apply_status t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID \
			where t1.SHIPID='%s'and t1.DEPARTMENT_CODE='%s' and t2.STATUS='%d' and t2.WORKFLOW_TASK_ID='%s' and t2.WORKFLOW_STEP_ID IN(%s) order by apptm desc",\
			strShipId.c_str(),strPartId.c_str(), iState,strTaskId.c_str(),strTmp.c_str());
	}

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char aid[100] = "";
	int itime=0;
	int iStatus=0;
	int cnt = 0;
	out << "[";	
	while (psql->NextRow())
	{
		READMYSQL_STR(PARTS_APPLY_ID, aid);
		READMYSQL_INT(apptm, itime,-1);
		READMYSQL_INT(STATUS, iStatus,-1);
		if (cnt++)
			out << ",";
		out << "{sid:\"" << strShipId << "\",aid:\"" << aid << "\",time:\"" << itime <<"\",st:" << iStatus << "}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);	
}
//MID：0x0061       SID：0x0022
//{seq:”ssss0001”,sid:”s000001”}
//{seq:"detail0000",sid:"",ship:"",dep:"",st:0,et:1234567,way:"aaaa",ad:"bbbb",isu:0,isl:0,flow:[{nm:"aa",url:"http://wwaidu.com/"}],rs:"reason",pro:[{fs:fn:"lucy",fc:"申请",ft:12312,fr:"备注",fs:0}],list:[{Id:”ssss00001”, bid:””,sid:"2-1232",name:"na",num:5,unit:"个",isi:0,has:10}]}
int eMOPEquipSvc::GetAppOrderDetails(const char* pUid, const char* jsonString, std::stringstream& out)
{//  根据sid获取单据详细信息
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetAppOrderDetails]bad format:", jsonString, 1);
	string strSId = root.getv("sid", "");// 订单id
	string strSeq= root.getv("seq", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\"";
	MySql* psql = CREATE_MYSQL;

	/*
	ship:""---t50_emop_parts_applies
	Sid:订单号
	Dep:部门
	Et:交货期限
	Way：交货方式
	Ad：交货地点
	Isu：是否为紧急申请
	Isl：是否船检/证书
	Flow：附件---T50_EMOP_PARTS_APPLIES_ATTACHMENTS
	Nm：附件名称
	Url：附件URL
	Pro：申请流程  ------------t50_emop_parts_apply_status
	taskId:任务ID
	stepId：步骤ID
	stateId:状态(当前流程状态)   状态 0待审核 1已审核 2 退单 3作废 4 所有	  
	Fn：操作人
	Ft：操作时间
	Fr：备注
	List：申请备件列表---T50_EMOP_PARTS_APPLY_DETAILS
	Id：备件ID
	Bid: 设备ID
	qid:”aaaaaaaaa”????
	Sid：备件编号
	Name：名称
	Num:申请数量
	Unit：单位
	Isi：是否为重大备件
	Has：库存
	Check:核发数量-----T50_EMOP_PARTS_APPLY_STATUS_DETAILS
	taskId:任务ID
	stepId：步骤ID
	Number：数量
	*/
	char sql[1024] = "";
	sprintf (sql, \
		"select t1.DEPARTMENT_CODE,t1.SHIPID,t1.EMERGENCY_FLAG,t1.CS_CERTIFIED_FLAG,UNIX_TIMESTAMP(t1.DELIVERY_DATE) as DLYDATA,DELIVERY_WAY,t1.DELIVERY_LOCATION,t2.NAME_CN AS DPTNAME from \
		blm_emop_etl.t50_emop_parts_applies t1 left join \
		blm_emop_etl.t50_emop_department_code t2 on t1.DEPARTMENT_CODE=t2.DEPARTMENT_CODE\
		where PARTS_APPLY_ID='%s'",strSId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char szShipid[100] = "";
	char szDptid[100]="";
	char szDpt[100]="";
	int iEt = 0;
	char szWay[100] = "";
	char szAd[100]="";
	int iIsu=0;
	int iIsl=0;	
	string strTmp;
	int cnt = 0;
	if(psql->NextRow())
	{//{seq:"detail0000",sid:"",ship:"",dep:"",et:1234567,way:"aaaa",ad:"bbbb",isu:0,isl:0,flow:[{nm:"aa",url:"http://wwaidu.com/"}],rs:"reason",pro:[{fs:fn:"lucy",fc:"申请",ft:12312,fr:"备注",fs:0}],list:[{Id:”ssss00001”, bid:””,sid:"2-1232",name:"na",num:5,unit:"个",isi:0,has:10}]}
		READMYSQL_STR(SHIPID, szShipid);
		READMYSQL_STR(DEPARTMENT_CODE, szDptid);	
		READMYSQL_STR(DPTNAME, szDpt);	
		READMYSQL_INT(DLYDATA, iEt,0);
		READMYSQL_STR(DELIVERY_WAY, szWay);
		READMYSQL_STR(DELIVERY_LOCATION, szAd);
		READMYSQL_INT(EMERGENCY_FLAG, iIsu,0);
		READMYSQL_INT(CS_CERTIFIED_FLAG, iIsl,0);
		//seq:"detail0000",sid:"",shipId:””,deptId:””deptName:"20",st:0,et:1234567,way:"aaaa",ad:"bbbb",isu:0,isl:0,flow:
		out <<",sid:\"" << strSId.c_str()<< "\",shipId:\""<< szShipid << "\",deptId:\""<< szDptid<< "\",deptName:\""<< szDpt <<"\",et:" << iEt \
			<< ",way:\"" << szWay <<"\",ad:\"" << szAd<<"\",isu:" << iIsu<<",isl:" << iIsl<<",flow:[";
	}
	//	out << "]}";
	// flow
	char szF_na[100]="";
	char szF_url[256]="";
	sprintf (sql, \
		"select *from blm_emop_etl.T50_EMOP_PARTS_APPLIES_ATTACHMENTS where Parts_Apply_ID='%s'",strSId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	cnt = 0;
	while(psql->NextRow())
	{//{nm:"aa",url:"http://wwaidu.com/"}
		READMYSQL_STR(NAME, szF_na);
		READMYSQL_STR(FILE_ID, szF_url); 
		if (cnt++)
			out << ",";		
		out << "{nm:\"" <<szF_na<< "\",url:\"" << szF_url << "\"}";
	}
	out<<"],pro:[";
	//pro
	char szP_taskid[100]="";
	char szP_stepid[100]="";
	int iStatus=0;
	char szP_fn[100]="";
	int iOpdate=0;
	char szP_fr[100]="";
	sprintf (sql, \
		"select WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,STATUS,OPERATOR,UNIX_TIMESTAMP(OP_DATE) AS OPDATE,REMARK from blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS where PARTS_APPLY_ID='%s'",strSId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	cnt = 0;
	while(psql->NextRow())
	{//{taskId:,stepId:,stateId:,fn:"lucy",fc:"",ft:12312,fr:"备注"}
		READMYSQL_STR(WORKFLOW_TASK_ID, szP_taskid);
		READMYSQL_STR(WORKFLOW_STEP_ID, szP_stepid);
		READMYSQL_INT(STATUS, iStatus,0);
		READMYSQL_STR(OPERATOR, szP_fn);
		READMYSQL_INT(OPDATE, iOpdate,0); 
		READMYSQL_STR(REMARK, szP_fr); 
		if (cnt++)
			out << ",";
		out << "{taskId:\"" <<szP_taskid<< "\",stepId:\"" << szP_stepid << "\",stateId:" <<iStatus<< ",fn:\"" <<szP_fn<< "\",ft:" <<iOpdate<< ",fr:\"" <<szP_fr <<"\"}";
	}
	out<<"],list:[";
	//list
	char szL_partsid[100]="";
	char szL_equipid[100]="";
	char szL_sid[100]="";
	char szL_na[100]="";
	int iAppCount=0;
	char szL_unit[100]="";
	int iIsi=0;
	int iStock=0;
	//Id：备件ID Bid: 设备ID Sid：备件编号 Name：名称 Num:申请数量 Unit：单位 Isi：是否为重大备件 Has：库存
	int iCheck=0;
	char szTaskid[100]="";
	char szStepid[100]="";
	// 属性id
	char szQid[100]="";//:”aaaaaaaaa”
	sprintf (sql, \
		"select t1.PART_MODEL_ID,t1.EQUIP_MODEL_ID as PART_CAT_ID,t1.QUATITY,t2.NAME_CN,t2.MODEL,t2.UNIT,t3.INSTOCK,t3.IS_IMPORTANT,t4.EQUIP_MODEL_ID from \
		blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS t1 left join \
		blm_emop_etl.t50_emop_parts_models t2 on t1.PART_MODEL_ID=t2.PART_MODEL_ID left join\
		blm_emop_etl.T50_EMOP_SHIP_PARTS t3 on t1.EQUIP_MODEL_ID=t3.PART_CAT_ID and t1.PART_MODEL_ID=t3.PART_MODEL_ID left join\
		blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t4 on t1.EQUIP_MODEL_ID=t4.PART_CAT_ID and t3.SHIPID=t4.SHIPID\
		where PARTS_APPLY_ID='%s'",strSId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	MySql* psqlTmp = CREATE_MYSQL;
	cnt = 0;
	int cnt1=0;
	while(psql->NextRow())
	{//{Id:”ssss00001”, bid:””,sid:"2-1232",name:"na",num:5,unit:"个",isi:0,has:10}
		READMYSQL_STR(PART_MODEL_ID, szL_partsid);
		READMYSQL_STR(PART_CAT_ID, szL_equipid);
		READMYSQL_STR(EQUIP_MODEL_ID, szQid);
		READMYSQL_STR(MODEL, szL_sid);
		READMYSQL_STR(NAME_CN, szL_na);
		READMYSQL_INT(QUATITY, iAppCount,0);
		READMYSQL_STR(UNIT, szL_unit); 
		READMYSQL_INT(INSTOCK, iStock,0); 
		READMYSQL_INT(IS_IMPORTANT, iIsi,0);  
		if (cnt++)
			out << ",";
		out << "{Id:\"" << szL_partsid << "\",bid:\"" << szL_equipid << "\",qid:\"" << szQid \
			<< "\",sid:\"" << szL_sid <<"\",name:\"" << szL_na << "\",num:" << iAppCount <<",unit:\"" << szL_unit<< "\",isi:" << iIsi<< ",has:" << iStock<< ",check:[";//"}";
		sprintf (sql, \
			"select WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,QUANTITY from blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS_DETAILS where PARTS_APPLY_ID='%s' and EQUIP_MODEL_ID='%s' and  PART_MODEL_ID='%s'",strSId.c_str(),szL_equipid,szL_partsid);

		CHECK_MYSQL_STATUS(psqlTmp->Query(sql), 3);
		cnt1=0;
		while(psqlTmp->NextRow())
		{//{Id:”ssss00001”, bid:””,sid:"2-1232",name:"na",num:5,unit:"个",isi:0,has:10}
			READMYSQL_STR1(WORKFLOW_TASK_ID, szTaskid,psqlTmp);
			READMYSQL_STR1(WORKFLOW_STEP_ID, szStepid,psqlTmp);
			READMYSQL_INT1(QUANTITY, iCheck,-1,psqlTmp);
			if (cnt1++)
				out << ",";
			out<<"{taskid:\""<<szTaskid<<"\",stepId:\""<<szStepid<<"\",number:"<<iCheck<<"}";
		}
		out<<"]}";		
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int eMOPEquipSvc::newAppOrderNum(string &o_strAppNum,string &i_strShipId)
{
	// 空订单 入t50_emop_parts_applies
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select PARTS_APPLY_ID from blm_emop_etl.t50_emop_parts_applies where SHIPID='%s' ORDER BY PARTS_APPLY_ID DESC",i_strShipId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	string strFlowNum;
	char szId[100]="";
	char szTmp[16]="";
	if(psql->NextRow())
	{
		READMYSQL_STR(PARTS_APPLY_ID, szId);
		string strTmp=szId;
		strTmp=strTmp.substr(strTmp.length()-7,7);
		int iNum=atoi(strTmp.c_str())+1;
		//		strFlowNum=itoa(iNum,szId,10);
		sprintf(szTmp,"%07d",iNum);
		strFlowNum=szTmp;
	}
	else
		strFlowNum="0000001";	
	//17位：'BP' + 标记位（1位 S/C) + 船舶ID（7位）+ 7位流水号  如：BPS10000010000001
	// 流水号,申请单一定来自于ship
	o_strAppNum="BP";
	o_strAppNum+="S";
	o_strAppNum+=i_strShipId;
	o_strAppNum+=strFlowNum;

	RELEASE_MYSQL_RETURN(psql, 0);	
}
//MID：0x0061         SID：0x0023
//{seq:"detail0000",data:{sid:"",ship:"s001",et:1234567,way:"",ad:"",isu:0,isl:0,flow:[{nm:"aa",url:""}],rs:"",list:[{id:””,bid:””,sid:"",name:"",num:5,unit:"",isi:0,has:10}]}}
//{seq:”sssss”,eid:0}
int eMOPEquipSvc::ModifyAppOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{
	/*
	Sid:订单号----t50_emop_parts_applies
	Et:交货期限 对应 表里 发货日期
	Way：交货方式
	Ad：交货地点
	Isu：是否为紧急申请
	Isl：是否船检/证书	
	State：状态 t50_emop_parts_apply_status
	Flow：附件 T50_EMOP_PARTS_APPLIES_ATTACHMENTS
	Nm：附件名称
	Url：附件URL	
	List：申请备件列表 T50_EMOP_PARTS_APPLY_DETAILS
	Id:备件ID
	Bid：设备ID
	Sid：备件编号
	Num:申请数量
	Check：核发数量
	*/
	JSON_PARSE_RETURN("[eMOPEquipSvc::ModifyAppOrder]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", "");
	//	Json* pJData = root.get("data");
	string strSid = root.getv("sid", "");//t50_emop_parts_applies
	string strShipid = root.getv("ship", "");//
	int iEt = root.getv("et", -1);//
	string strWay = root.getv("way", "");//
	string strAd = root.getv("ad", "");//
	int iIsu = root.getv("isu", -1);//
	int iIsl = root.getv("isl", -1);//
	string strRs = root.getv("rs", "");//
	int iState=root.getv("state", -1);//t50_emop_parts_apply_status
	// 新增taskid stepid
	string strTaskId=root.getv("taskId", "");
	string strStepId=root.getv("stepId", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	// 修改备件申请单表
	sprintf (sql,\
		"UPDATE blm_emop_etl.t50_emop_parts_applies SET \
		DELIVERY_DATE=FROM_UNIXTIME(%d),DELIVERY_WAY='%s',DELIVERY_LOCATION='%s',EMERGENCY_FLAG='%d',CS_CERTIFIED_FLAG='%d',REASON='%s' WHERE PARTS_APPLY_ID = '%s'",\
		iEt,strWay.c_str(),strAd.c_str(),iIsu,iIsl,strRs.c_str(),strSid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	// 修改状态表（申请原因）
	sprintf (sql, "SELECT APPLY_DATE FROM blm_emop_etl.t50_emop_parts_applies WHERE PARTS_APPLY_ID = '%s'",strSid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char szODate[100]="";
	if(psql->NextRow())
	{
		READMYSQL_STR(APPLY_DATE, szODate);
		sprintf (sql,\
			"UPDATE blm_emop_etl.t50_emop_parts_apply_status SET REMARK='%s'WHERE PARTS_APPLY_ID='%s' and OP_DATE='%s'",\
			strRs.c_str(),strSid.c_str(),szODate);		
	}
	else
	{
		sprintf (sql,\
			"insert into blm_emop_etl.t50_emop_parts_apply_status(REMARK,PARTS_APPLY_ID,OP_DATE)values('%s','%s',FROM_UNIXTIME(%d))",\
			strRs.c_str(),strSid.c_str(),time(0));
	}

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	// 修改附件列表
	sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_APPLIES_ATTACHMENTS WHERE PARTS_APPLY_ID= '%s'",strSid.c_str());				

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	Json* pJFlow = root.get("flow");//
	int iSize=pJFlow->size();
	string strNa,strFile;
	for(int i=0;i<iSize;i++)
	{
		Json* pJFChild=pJFlow->get(i);
		strNa=pJFChild->getv("nm","");
		strFile=pJFChild->getv("url","");
		sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_APPLIES_ATTACHMENTS(PARTS_APPLY_ID,NAME,FILE_ID) values ('%s','%s','%s')",\
			strSid.c_str(),strNa.c_str(),strFile.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	// 修改备件列表
	sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS WHERE PARTS_APPLY_ID= '%s'",strSid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	Json* pJList = root.get("list");//
	iSize=pJList->size();
	string strD1,strD2;
	int iCount=0;
	int iCheck=0;// 核发数量
	for(int i=0;i<iSize;i++)
	{
		Json* pJPChild=pJList->get(i);
		strD1=pJPChild->getv("id","");//备件id
		strD2=pJPChild->getv("bid","");//设备id
		//		strD3=pJPChild->getv("name","");
		iCount=pJPChild->getv("num",-1);//申请数量
		iCheck=pJPChild->getv("check",-1);
		// 如果审核阶段：更改某个单子的某个备件的核发数量
		if(strTaskId=="12"||strTaskId=="22")
		{
			sprintf (sql, "select QUANTITY from blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS_DETAILS where \
						  PARTS_APPLY_ID='%s' and WORKFLOW_TASK_ID='%s' and WORKFLOW_STEP_ID='%s' and EQUIP_MODEL_ID='%s' and PART_MODEL_ID='%s'",\
						  strSid.c_str(),strTaskId.c_str(),strStepId.c_str(),strD2.c_str(),strD1.c_str());
			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
			if(psql->NextRow())
			{// 如果T50_EMOP_PARTS_APPLY_STATUS_DETAILS能找到此条记录
				sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS_DETAILS SET QUATITY='%d' where \
							  PARTS_APPLY_ID='%s' and WORKFLOW_TASK_ID='%s' and WORKFLOW_STEP_ID='%s' and EQUIP_MODEL_ID='%s' and PART_MODEL_ID='%s'",\
							  iCheck,strSid.c_str(),strTaskId.c_str(),strStepId.c_str(),strD2.c_str(),strD1.c_str());
			}
			else
				sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS_DETAILS(PARTS_APPLY_ID,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,EQUIP_MODEL_ID,PART_MODEL_ID,QUANTITY) values ('%s','%s','%s','%s','%s','%d')",\
				strSid.c_str(),strTaskId.c_str(),strStepId.c_str(),strD2.c_str(),strD1.c_str(),iCheck);
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
		sprintf (sql, "select QUATITY from blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS where PARTS_APPLY_ID='%s' and PART_MODEL_ID='%s'",\
			strSid.c_str(),strD1.c_str());	
		CHECK_MYSQL_STATUS(psql->Query(sql), 3); // 客户端可能相同备件写两遍错误
		if(psql->NextRow())
		{
			sprintf (sql, "update blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS set EQUIP_MODEL_ID='%s',QUATITY='%d'where PARTS_APPLY_ID='%s'and PART_MODEL_ID='%s')",\
				strD2.c_str(),iCount,strSid.c_str(),strD1.c_str());
		}
		else
		{
			sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS(PARTS_APPLY_ID,PART_MODEL_ID,EQUIP_MODEL_ID,QUATITY) values ('%s','%s','%s','%d')",\
				strSid.c_str(),strD1.c_str(),strD2.c_str(),iCount);
		}

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}	

	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
int GetNext(string &o_strNext,vector <string> &i_vecStep,string &i_strKey,bool i_bNext)
{
	int iSize=i_vecStep.size();
	for(int i=0;i<iSize;i++)
	{
		if(i_strKey==i_vecStep.at(i))
		{
			if(i_bNext)
			{
				if(i<(iSize-1))
				{
					o_strNext=i_vecStep.at(i+1);
					return 1;
				}
				else
					return 2;
			}
			else
			{
				if(i>0)
				{
					o_strNext=i_vecStep.at(i-1);
					return 1;
				}
				else
					return 2;
			}
		}
	}
	return 3;
}
int eMOPEquipSvc::GetAllSteps(vector <string>&o_vecSteps,MySql* psql,string &i_strTaskid)
{
	char sql[1024] = "";
	o_vecSteps.clear();
	sprintf (sql, "select WORKFLOW_STEP_ID from blm_emop_etl.t50_emop_workflow_steps where WORKFLOW_TASK_ID='%s' ORDER BY WORKFLOW_STEP_ID ASC",i_strTaskid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char szSid[32];
	string strTmp;
	while(psql->NextRow())
	{
		READMYSQL_STR(WORKFLOW_STEP_ID, szSid);
		strTmp=szSid;
		o_vecSteps.push_back(strTmp);
	}
	return 0;
}

//MID：0x0061   SID:0x0024
//{seq:”ssss”,uid:””,oid:””,tid:””,sid:””,status:””,name:””,ctime:””,idea:””,remark:””}
//{seq:”sssss”,eid:0}
int eMOPEquipSvc::AppOrderOpt(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::AppOrderOpt]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strUid= root.getv("uid", "");//操作人
	string strOid=root.getv("oid", "");//订单ID
	string strTaskid=root.getv("tid", "");//任务ID
	string strStepid=root.getv("sid", "");//步骤ID
	string strRemark=root.getv("idea", "");
	int iStatus=root.getv("status", -1);
	//	int iTime=root.getv("status", -1);
	int iTime=time(0);// 操作时间
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select *from blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS where PARTS_APPLY_ID='%s'  and STATUS='0'",strOid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{// 
		sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS SET OP_DATE=FROM_UNIXTIME(%d),REMARK='%s',OPERATOR='%s',WORKFLOW_TASK_ID='%s',WORKFLOW_STEP_ID='%s', STATUS='%d' where PARTS_APPLY_ID='%s' and STATUS='0'",\
			iTime,strRemark.c_str(),strUid.c_str(),strTaskid.c_str(),strStepid.c_str(), iStatus,strOid.c_str());
	}
	else// 插入新的状态处理
	{
		sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS(PARTS_APPLY_ID,OP_DATE,REMARK,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID, STATUS) values ('%s',FROM_UNIXTIME(%d),'%s','%s','%s','%s','%d')",\
			strOid.c_str(),iTime,strRemark.c_str(),strUid.c_str(),strTaskid.c_str(),strStepid.c_str(), iStatus);
	}

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	// 取出所有步骤 升序排列
	vector <string>vecSteps;
	GetAllSteps(vecSteps,psql,strTaskid);
	// 根据状态判断下一步
	string strTmp;
	char szSid[32];
	int iRlt=0;
	switch(iStatus)
	{
	case 1: //已审核 step+1
		iRlt=GetNext(strTmp,vecSteps,strStepid,true);

		if(iRlt==2)// end生成下一个状态 判断路由表
		{
			sprintf (sql, "select NETX_TASK  from blm_emop_etl.T50_EMOP_WORKFLOW_ROUTES where CURRENT_TASK_ID='%s' and ROUTE_CONDITION='%s'",strTaskid.c_str(),strStepid.c_str());

			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
			if(psql->NextRow())
			{ // 如果找到下一个任务
				READMYSQL_STR(NETX_TASK, szSid); // 暂时写死 1
				// 取出所有步骤 升序排列
				strTmp=szSid;
				GetAllSteps(vecSteps,psql,strTmp);
				if(vecSteps.size()<=0)//error
					break;
				iTime=time(0)+100;
				sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS(PARTS_APPLY_ID,OP_DATE,REMARK,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID, STATUS) values ('%s',FROM_UNIXTIME(%d),'%s','%s','%s','%s','%d')",\
					strOid.c_str(),iTime,strRemark.c_str(),strUid.c_str(),szSid,vecSteps.at(0).c_str(), 0);

				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
			}
		}
		else if(iRlt==1) //生成新的待审核单
		{
			iTime=time(0)+100;
			sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS(PARTS_APPLY_ID,OP_DATE,REMARK,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID, STATUS) values ('%s',FROM_UNIXTIME(%d),'%s','%s','%s','%s','%d')",\
				strOid.c_str(),iTime,strRemark.c_str(),strUid.c_str(),strTaskid.c_str(),strTmp.c_str(), 0);

			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
		break; 
	case 2: // 2 退单step-1 
		iRlt=GetNext(strTmp,vecSteps,strStepid,false);	 
		if(iRlt==2)// end
		{ // 申请单第一步，无需处理
		}
		else if(iRlt==1)
		{
			iTime=time(0)+100;
			sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS(PARTS_APPLY_ID,OP_DATE,REMARK,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID, STATUS) values ('%s',FROM_UNIXTIME(%d),'%s','%s','%s','%s','%d')",\
				strOid.c_str(),iTime,strRemark.c_str(),strUid.c_str(),strTaskid.c_str(),strTmp.c_str(), 0);

			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}

		break;
	case 0: // 待审核 无需下一状态
	case 3: // 3作废 无需下一状态
	default:
		break;
	}
	out<<"{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);	
}
//MID:0x0061,      sid:0x002B
//{seq:””,shipId:””,taskId:””,stepId:””}
//{seq:””,not:,back:}
int eMOPEquipSvc::GetApporderNum(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetApporderNum]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strShipid=root.getv("shipId", "");//shipID
	string strTaskId=root.getv("taskId", "");
	string strStepId=root.getv("stepId", "");
	string strTmp="";
	Tokens svrList;	
	if(strStepId.find("|") != string::npos)
	{					 
		svrList = StrSplit(strStepId, "|");		
		for(int i=0;i<svrList.size();i++)
		{
			if(svrList[i].empty())
				continue;
			strTmp+="'";
			strTmp+=svrList[i];
			strTmp+="'";
			if (i<((int)svrList.size()-1))
				strTmp+=",";
		}
	}
	else
	{
		strTmp+="'";
		strTmp+=strStepId;
		strTmp+="'";
	}

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//not
	sprintf (sql,\
		"select t1.* from blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 left join blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID where \
		t1.SHIPID='%s' and t2.WORKFLOW_TASK_ID='%s' and t2.STATUS='0' and t2.WORKFLOW_STEP_ID IN(%s)",strShipid.c_str(),strTaskId.c_str(),strTmp.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iNot=psql->GetRowCount();
	sprintf (sql, \
		"select t1.* from blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 left join blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID where \
		t1.SHIPID='%s' and t2.WORKFLOW_TASK_ID='%s' and t2.STATUS='2' and t2.WORKFLOW_STEP_ID IN(%s)",strShipid.c_str(),strTaskId.c_str(),strTmp.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iBack=psql->GetRowCount();
	out<<"{seq:\""<<strSeq.c_str()<<"\",not:"<<iNot<<",back:"<<iBack<<"}";
	RELEASE_MYSQL_RETURN(psql, 0);	
}
//MID：0x0062   SID:0x002C;
//{seq:””,type:”’,which:””}
//{seq:””,task:[{taskId:””,taskName:””,detail:[{stepId:””,sname:””,roleId:””},{},.....]},{},.....]}
int eMOPEquipSvc::GetWorkfllow(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetWorkfllow]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	int iType= root.getv("type", -1);//
	int iWhich=root.getv("which", -1);//
	out<<"{seq:\""<<strSeq.c_str()<<"\",task:[";
	vector <KeyValue>vecT1;
	KeyValue kv;
	char na[100]="";
	char key[100] = "";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	if(iType==0)
	{// 备件
		if(iWhich==0)
		{//船端
			sprintf (sql, "select WORKFLOW_TASK_ID,NAME from blm_emop_etl.t50_emop_workflow_task where WORKFLOW_ID='1' order by WORKFLOW_TASK_ID asc");
		}else if(iWhich==1)
		{//岸端
			sprintf (sql, "select WORKFLOW_TASK_ID,NAME from blm_emop_etl.t50_emop_workflow_task where WORKFLOW_ID='2' order by WORKFLOW_TASK_ID asc");			
		}
	}else if(iType==1)
	{// 物料
		if(iWhich==0)
		{//船端
			sprintf (sql, "select WORKFLOW_TASK_ID,NAME from blm_emop_etl.t50_emop_workflow_task where WORKFLOW_ID='3' order by WORKFLOW_TASK_ID asc");
		}else if(iWhich==1)
		{//岸端
			sprintf (sql, "select WORKFLOW_TASK_ID,NAME from blm_emop_etl.t50_emop_workflow_task where WORKFLOW_ID='4' order by WORKFLOW_TASK_ID asc");			
		}
	}
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);			
	while(psql->NextRow())
	{
		READMYSQL_STR(WORKFLOW_TASK_ID, key);
		READMYSQL_STR(NAME, na);
		kv.m_strKey=key;
		kv.m_strValue=na;
		vecT1.push_back(kv);
	}
	int cnt=0;
	for(int i=0;i<vecT1.size();i++)
	{
		if (cnt++)
			out << ",";
		out << "{taskId:\"" <<vecT1.at(i).m_strKey.c_str()<< "\",taskName:\"" << vecT1.at(i).m_strValue.c_str()<<"\",detail:[";
		GetSubData(out,vecT1.at(i).m_strKey,psql);
		out<<"]}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//
int eMOPEquipSvc::GetSubData(std::stringstream& out,string &i_strKey,MySql* psql )
{
	char sql[1024] = "";	
	sprintf (sql, "select * from blm_emop_etl.t50_emop_workflow_steps where WORKFLOW_TASK_ID='%s' order by WORKFLOW_STEP_ID asc",i_strKey.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char stepid[100]="";
	char na[100]="";
	char roleid[100] = "";	
	int cnt = 0;
	while(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(WORKFLOW_STEP_ID, stepid);
		READMYSQL_STR(NAME, na);
		READMYSQL_STR(ROLE_ID, roleid);
		if (cnt++)
			out << ",";
		out << "{stepId:\"" << stepid << "\",sname:\"" << na<<"\",roleId:\"" << roleid<<"\"}";
	}


	return 0;
}
//MID：0x0061   SID:0x002A;
//{seq:””}
//{seq:””,data:[{id:””,name:””},{},...]}
int eMOPEquipSvc::GetAllPartsCorps(const char* pUid, const char* jsonString, std::stringstream& out)
{//T50_EMOP_PART_PROVIDER_LIST
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetAllPartsCorps]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");	 
	out<<"{seq:\""<<strSeq.c_str()<<"\",data:[";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select t1.COMPANY_KEY,t2.NAME from blm_emop_etl.T50_EMOP_PART_PROVIDER_LIST t1 left join blm_emop_etl.t41_company t2 on t1.COMPANY_KEY=t2.COMPANY_KEY");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char key[100] = "";
	char na[100]="";	
	int cnt = 0;
	while(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(COMPANY_KEY, key);
		READMYSQL_STR(NAME, na);
		if (cnt++)
			out << ",";
		out << "{id:\"" << key << "\",name:\"" << na<<"\"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID：0x0061   SID:0x0029;
//{seq:””,applyId:””,shipId:””,deptId:””,provider:””,spares:[{bid:””,sid:””,number:},{},...]}
//{seq:””,eid:””}
int eMOPEquipSvc::MkInquirySheete(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::MkInquirySheete]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strAppid= root.getv("applyId", "");
	string strProvider= root.getv("provider", "");
	string strShipid= root.getv("shipId", "");
	string strDptid= root.getv("deptId", "");

	// 生成报价单id(询价单id),一个供应商一个询价单
	string strId="IQ";
	char szTmp[32];
	long ltime=0;

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";
	Tokens svrList;	
	int iSize=1;
	string strTmp="";
	bool bFinded=false;
	out<<"{seq:\""<<strSeq<<"\",eid:0,data:[";
	if(strProvider.find("|") != string::npos)
	{					 
		svrList = StrSplit(strProvider, "|");
		iSize=svrList.size();
		bFinded=true;
	}
	else
	{
		iSize=1;
		strTmp=strProvider;
		bFinded=false;
	}
	int idx=0;

	for(int i=0;i<iSize;i++)
	{
		strId="IQ";
		ltime =time(0)+i;
		sprintf(szTmp,"%d",ltime);
		strId+=szTmp;
		
		//if(svrList[i].empty())
		//	continue;
		//T50_EMOP_PARTS_INQUERIED_QUOTED
		DEBUG_LOG("test2");
		if(bFinded)
			strTmp=svrList[i].c_str();
		sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED(PARTS_INQUERIED_QUOTED_ID,PARTS_APPLY_INQUERY_ID,COMPANY_KEY,STATUS) values ('%s','%s','%s',%d)",\
			strId.c_str(),strId.c_str(),strTmp.c_str(), 0);

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		DEBUG_LOG("test3");
		//T50_EMOP_PARTS_APPLY_INQUERY
		sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY(PARTS_APPLY_INQUERY_ID,PARTS_APPLY_ID,COMPANY_KEY,OP_DATE,SHIPID,DEPARTMENT_CODE) values ( '%s','%s','%s',FROM_UNIXTIME(%d),'%s','%s')",\
			strId.c_str(),strAppid.c_str(),strTmp.c_str(),ltime,strShipid.c_str(),strDptid.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		DEBUG_LOG("test4");
		if(idx++)
		{
			out<<",";
		}
		sprintf(sql,"SELECT NAME FROM blm_emop_etl.T41_COMPANY \
					WHERE COMPANY_KEY='%s'",strTmp.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);
		string tempname="";
		if(psql->NextRow())
		{
			READMYSQL_STRING(NAME,tempname );
		}
		out<<"{qid:\""<<strId<<"\",qname:\""<<tempname<<"\"}";
		//T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS	
		Json *child0=root.get("spares");
		DEBUG_LOG("jjfhjhj");
		DEBUG_LOG("jjfhjhj");
		int iSize=child0->size();
		string strBid;
		string strSid;
		int iNum=0;
		for(int i=0;i<iSize;i++)
		{
			Json* child1 = child0->get(i);
			strBid=child1->getv("bid","");
			strSid=child1->getv("sid","");
			iNum=child1->getv("number",-1);
			sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS(PARTS_INQUERIED_QUOTED_ID,PART_MODEL_ID, EQUIP_MODEL_ID,QUATITY) values ('%s','%s','%s','%d')",\
				strId.c_str(),strSid.c_str(), strBid.c_str(),iNum);

			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
			DEBUG_LOG("test5");
		}	
	}	

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6101  (mt=0x61,st=0x41)
//{seq:”ss”,sid :"S001",did:”d001”,st:0}
//{not:52,yet:30,sid:”S001”,orders:[{mt:1386579272,no:”S00001”,st:0},{..}]}
int eMOPEquipSvc::GetInquirySheeteList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetInquirySheeteList]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strSid= root.getv("sid", "");//船舶ID
	string strDid= root.getv("did", "");//部门ID
	int iSt= root.getv("st", -1);	//0未报价；1已报价
	//T50_EMOP_PARTS_APPLIES // 申请单
	//T50_EMOP_PARTS_APPLY_INQUERY  //备件询价表
	//T50_EMOP_PARTS_INQUERIED_QUOTED // 备件询价-报价表
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	if(strDid.empty())
		sprintf (sql, "select t2.PARTS_APPLY_INQUERY_ID,UNIX_TIMESTAMP(t2.OP_DATE) as optm from \
					  blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID left join \
					  blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t3 on t2.PARTS_APPLY_INQUERY_ID=t3.PARTS_APPLY_INQUERY_ID where \
					  t1.SHIPID='%s' and t3.STATUS='%d'",\
					  strSid.c_str(),0);
	else
		sprintf (sql, "select t2.PARTS_APPLY_INQUERY_ID,UNIX_TIMESTAMP(t2.OP_DATE) as optm from \
					  blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID left join \
					  blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t3 on t2.PARTS_APPLY_INQUERY_ID=t3.PARTS_APPLY_INQUERY_ID where \
					  t1.SHIPID='%s' and t1.DEPARTMENT_CODE='%s' and t3.STATUS='%d'",\
					  strSid.c_str(),strDid.c_str(),0);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iNot=psql->GetRowCount();
	if(strDid.empty())
		sprintf (sql, "select t2.PARTS_APPLY_INQUERY_ID,UNIX_TIMESTAMP(t2.OP_DATE) as optm from \
					  blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID left join \
					  blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t3 on t2.PARTS_APPLY_INQUERY_ID=t3.PARTS_APPLY_INQUERY_ID where \
					  t1.SHIPID='%s' and t3.STATUS='%d'",\
					  strSid.c_str(),1);
	else
		sprintf (sql, "select t2.PARTS_APPLY_INQUERY_ID,UNIX_TIMESTAMP(t2.OP_DATE) as optm from \
					  blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID left join \
					  blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t3 on t2.PARTS_APPLY_INQUERY_ID=t3.PARTS_APPLY_INQUERY_ID where \
					  t1.SHIPID='%s' and t1.DEPARTMENT_CODE='%s' and t3.STATUS='%d'",\
					  strSid.c_str(),strDid.c_str(),1);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iYet=psql->GetRowCount();
	out<<"{seq:\""<<strSeq.c_str()<<"\",not:"<<iNot<<",yet:"<<iYet<<",sid:\""<<strSid.c_str()<<"\",orders:[";
	if(strDid.empty())
		sprintf (sql, "select t2.PARTS_APPLY_INQUERY_ID,UNIX_TIMESTAMP(t2.OP_DATE) as optm from \
					  blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID left join \
					  blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t3 on t2.PARTS_APPLY_INQUERY_ID=t3.PARTS_APPLY_INQUERY_ID where \
					  t1.SHIPID='%s' and t3.STATUS='%d' order by optm  desc",\
					  strSid.c_str(),iSt);
	else
		sprintf (sql, "select t2.PARTS_APPLY_INQUERY_ID,UNIX_TIMESTAMP(t2.OP_DATE) as optm from \
					  blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID left join \
					  blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t3 on t2.PARTS_APPLY_INQUERY_ID=t3.PARTS_APPLY_INQUERY_ID where \
					  t1.SHIPID='%s' and t1.DEPARTMENT_CODE='%s' and t3.STATUS='%d' order by optm desc",\
					  strSid.c_str(),strDid.c_str(),iSt);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[100] = "";
	int iTime=0;
	int cnt = 0;
	while(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(PARTS_APPLY_INQUERY_ID, id);
		READMYSQL_INT(optm, iTime,0);
		if (cnt++)
			out << ",";
		out <<"{seq:\""<<strSeq.c_str()<< "\",mt:" << iTime << ",no:\"" << id<< "\",st:" << iSt<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6142  (mt=0x61,st=0x42)
//{seq:”sss”,no:"S001"} 
//{cno:”s001”,sn:”新厦门”,dpt:”轮机”,st:0,cd:1386579272,ct:”船送”,cd:”船上”,ty1:1,ty2:0,at:[{atid:”132”,atna:”附件1”},{..}],ino:”12345”,dt:1386579272,vdor:”上海供应商”,afino:”12345”,afdt:1386579272,predt:1386579272,scdt:1386579272,scad:”送船地点”,pty:”1”,curr:”1”,fare:”22.0”,othe:”0”,price:”120.0”,ist:0}
int eMOPEquipSvc::GetInquirySheetDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	/*
	------------T50_EMOP_PARTS_APPLY_INQUERY t2
	Cno:采购单号
	No：询价单号
	Dt：询价日期
	------------T50_EMOP_PARTS_INQUERIED_QUOTED t1
	Vdor：供应商
	Aino：确认单号
	Afdt：确认日期
	Predt：预抵达日期
	Scdt：送船日期
	Scad：送船地点
	Pty：付款方式：此处为代码
	Curr：币种：此处为代码
	Fare：运费
	Othe：其它费用
	Price：总价	
	Ist:询价单状态：1已报价，0未报价
	*/
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetInquirySheetDetail]bad format:", jsonString, 1);	
	string strSeq= root.getv("seq", "");
	string strIqsid= root.getv("no", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\"";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//	sprintf (sql, "select t1.*,t1.STATUS as XSTATUS,UNIX_TIMESTAMP(t1.QUOTE_DATE)as t1qttm,UNIX_TIMESTAMP(t1.ETA)as t1eta,UNIX_TIMESTAMP(t1.DELIVERY_DATE)as t1dvtm,\
	UNIX_TIMESTAMP(t2.OP_DATE)as t2optm,t3.*,UNIX_TIMESTAMP(t3.DELIVERY_DATE)as t3DLTM,t4.STATUS,t5.NAME_CN AS SHIPNAME,t6.NAME_CN AS DPTNAME from \
	blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t1 left join \
	blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 on t1.PARTS_APPLY_INQUERY_ID=t2.PARTS_APPLY_INQUERY_ID left join \
	blm_emop_etl.T50_EMOP_PARTS_APPLIES t3 on t2.PARTS_APPLY_ID=t3.PARTS_APPLY_ID left join\
	blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS t4 on t3.PARTS_APPLY_ID=t4.PARTS_APPLY_ID left join\
	blm_emop_etl.t41_emop_ship t5 on t3.SHIPID=t5.SHIPID left join\
	blm_emop_etl.t50_emop_department_code t6 on t3.DEPARTMENT_CODE=t6.DEPARTMENT_CODE\
	where t1.PARTS_INQUERIED_QUOTED_ID='%s'",\
	strIqsid.c_str());
	sprintf (sql, "select t1.*,t1.STATUS as XSTATUS,UNIX_TIMESTAMP(t1.QUOTE_DATE)as t1qttm,UNIX_TIMESTAMP(t1.ETA)as t1eta,UNIX_TIMESTAMP(t1.DELIVERY_DATE)as t1dvtm,\
				  UNIX_TIMESTAMP(t1.OP_DT)as t1optm,t1.OPERATOR as t1optor,UNIX_TIMESTAMP(t2.OP_DATE)as t2optm,t2.PARTS_APPLY_ID,t3.name as corpname from \
				  blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t1 left join \
				  blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 on t1.PARTS_APPLY_INQUERY_ID=t2.PARTS_APPLY_INQUERY_ID left join\
				  blm_emop_etl.t41_company t3 on t1.COMPANY_KEY=t3.COMPANY_KEY \
				  where t1.PARTS_INQUERIED_QUOTED_ID='%s'",\
				  strIqsid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	//T3
	char Cno[100];//：采购单号
	//char Sn[100];//：船舶名称    
	//char Dpt[100];//：部门名称   	
	//int  iCd=0;//:交货日期
	//char Ct[100];//：交货方式
	//char Ad[100];//：交货地点
	//int iTy1=0;//：紧急申请：0未选中，1选中
	//int iTy2=0;//：船检/产品证书：0未选中，1选中
	//int iSt=0;//：采购单状态：
	//T2
	char No[100];//：询价单号
	int iDt=0;//：询价日期
	char Vdor[100];//：供应商
	//T1
	char Aino[100];//：确认单号
	int iAfdt=0;//：确认日期
	int iPredt=0;//：预抵达日期
	int iScdt=0;//：送船日期
	char Scad[100];//：送船地点
	int iPty=0;//：付款方式：此处为代码
	int iCurr=0;//：币种：此处为代码
	double ffFare=0;//：运费
	double ffOthe=0;//：其它费用
	double ffPrice=0;//：总价	
	int iIst=0;//:询价单状态：1已报价，0未报价
	int iOpttm=0;
	char Optor[100]="";
	char vendorId[100]="";//:””,
	int cnt = 0;
	if(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(QUOTE_CODE, Aino);// 确认单号
		READMYSQL_STR(PARTS_INQUERIED_QUOTED_ID, No);//询价单号
		READMYSQL_INT(t2optm,iDt ,0);//询价日期
		READMYSQL_INT(t1qttm,iAfdt ,0);//确认日期
		READMYSQL_INT(t1eta,iPredt ,0);//预抵达日期
		READMYSQL_INT(t1dvtm,iScdt ,0);//送船日期
		READMYSQL_STR(DELIVERY_LOCATION, Scad);//送船地点
		READMYSQL_INT(PAY_METHOD,iPty ,0);//付款方式
		READMYSQL_INT(CURRENCY,iCurr ,0);//币种
		READMYSQL_INT(XSTATUS,iIst ,0);//询价单状态
		READMYSQL_DOUBLE(FREIGHT,ffFare ,0);//：运费
		READMYSQL_DOUBLE(OTHER_EXPENSE, ffOthe,0);//其它费用
		READMYSQL_DOUBLE(TOTAL_PRICE, ffPrice,0);//总价
		READMYSQL_STR(COMPANY_KEY, vendorId);//供应商
		READMYSQL_STR(corpname, Vdor);//供应商
		READMYSQL_STR(PARTS_APPLY_ID, Cno);//采购单号
		READMYSQL_STR(t1optor, Optor);
		READMYSQL_INT(t1optm,iOpttm ,0);
		//READMYSQL_STR(SHIPNAME, Sn);//船舶名称
		//READMYSQL_STR(DPTNAME, Dpt);//部门名称
		//READMYSQL_INT(t3DLTM, iCd,0);//交货日期
		//READMYSQL_STR(DELIVERY_WAY, Ct);//交货方式
		//READMYSQL_STR(DELIVERY_LOCATION, Ad);//交货地点
		//READMYSQL_INT(EMERGENCY_FLAG,iTy1 ,0);//紧急申请
		//READMYSQL_INT(CS_CERTIFIED_FLAG,iTy2 ,0);//船检/产品证书
		//READMYSQL_INT(STATUS,iSt ,0);//采购单状态
		//{seq:””,cno:”s001”no:”12345”,dt:1386579272,vdor:”上海供应商”,aino:”12345”,afdt:1386579272,predt:1386579272,\
		scdt:1386579272,scad:”送船地点”,pty:”1”,curr:”1”,fare:”22.0”,othe:”0”,price:”120.0”,ist:0}
		out << ",cno:\"" << Cno<< "\",no:\"" << strIqsid.c_str()<< "\",dt:" << iDt<< ",vendorId:\"" << vendorId<< "\",vdor:\"" << Vdor<< "\",aino:\"" << Aino\
			<< "\",afdt:" << iAfdt<< ",predt:" << iPredt<< ",scdt:" << iScdt<< ",scad:\"" << Scad<< "\",pty:" << iPty<< ",curr:" << iCurr\
			<< ",fare:" << ffFare<< ",othe:" << ffOthe<< ",price:" << ffPrice<< ",ist:" << iIst<< ",otime:" << iOpttm<< ",Optor:\"" << Optor<< "\"";
	}
	/*char szF_na[100]="";
	char szF_url[256]="";
	sprintf (sql, \
	"select *from blm_emop_etl.T50_EMOP_PARTS_APPLIES_ATTACHMENTS where Parts_Apply_ID='%s'",Cno);
	pf=fopen("/tmp/SQL_6142_2.log","wb");
	fwrite(sql,sizeof(char),strlen(sql),pf);
	fclose(pf);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	cnt = 0;
	while(psql->NextRow())
	{
	READMYSQL_STR(NAME, szF_na);
	READMYSQL_STR(FILE_ID, szF_url); 
	if (cnt++)
	out << ",";		
	out << "{atid:\"" << szF_url<< "\",atna:\"" <<szF_na << "\"}";
	}
	out<<"]}";*/
	out<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//CmdId:0x6143  (mt=0x61,st=0x43)
//{seq:”sss”,no:"S001"}
//{total:58,list:[{sno:”s001”,sn:”UPS”,con:”10”,unit:”100.0”,rate:”0.90”,price:”100.0”,note:”备注”},{..}]}
int eMOPEquipSvc::GetAppOrderDetail_Iqsid(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetAppOrderDetail_Iqsid]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strNo= root.getv("no", "");//询价单号	
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql,\
		"select t1.*,t1.EQUIP_MODEL_ID as PART_CAT_ID,t2.MODEL,t2.NAME_CN,t4.EQUIP_MODEL_ID from \
		blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS t1 left join\
		blm_emop_etl.T50_EMOP_PARTS_MODELS t2 on t1.PART_MODEL_ID=t2.PART_MODEL_ID left join\
		blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t3 on t1.PARTS_INQUERIED_QUOTED_ID=t3.PARTS_APPLY_INQUERY_ID left join\
		blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t4 on t1.EQUIP_MODEL_ID=t4.PART_CAT_ID and t3.SHIPID=t4.SHIPID\
		where t1.PARTS_INQUERIED_QUOTED_ID='%s'",strNo.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	//	Total：备件数量
	int iTotal=psql->GetRowCount();
	out<<"{seq:\""<<strSeq.c_str()<<"\",total:"<<iTotal<<",list:[";
	char sno[100] = "";//Sno：备件编号
	char sn[100]="";//Sn：备件名称：此处不知是中文名还是英文名
	int iCon=0;//Con：询价数量
	double ffUnit=0;//：单价
	double ffRate=0;//：折扣率
	double ffPrice=0;//：总价
	char Note[100]="";//：备注	
	char Spid[100]="";//:备件ID
	char Eqid[100]="";//:设备ID
	char qid[100]="";

	int cnt = 0;
	while(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(MODEL, sno);
		READMYSQL_STR(NAME_CN, sn);
		READMYSQL_STR(REMARK, Note);
		READMYSQL_STR(PART_MODEL_ID, Spid);
		READMYSQL_STR(PART_CAT_ID, Eqid);
		READMYSQL_STR(EQUIP_MODEL_ID, qid);
		READMYSQL_INT(QUATITY,iCon ,0);//询价数量
		READMYSQL_DOUBLE(UNIT_PRICE,ffUnit ,0);//：单价
		READMYSQL_DOUBLE(DISCOUNTS, ffRate,0);//折扣率
		READMYSQL_DOUBLE(TOTAL_PRICE_DISCOUNTED, ffPrice,0);//总价
		if (cnt++)
			out << ",";
		//{sno:”s001”,sn:”UPS”,con:”10”,unit:”100.0”,rate:”0.90”,price:”100.0”,note:”备注”}
		out << "{sno:\"" << sno<< "\",Spid:\"" << Spid << "\",Eqid:\"" << Eqid<< "\",qid:\"" << qid<< "\",sn:\"" << sn<< "\",note:\"" << Note<< "\",con:" << iCon<< ",unit:" << ffUnit\
			<< ",rate:" << ffRate<< ",price:" << ffPrice<< "}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6144  (mt=0x61,st=0x44)
//{seq:”sss”,spid:"2-3306",eqid:”232”}
//{sn:””,sno:””,cn:””,en:””,list:[{vdor:””,md:”2013-11-01”,ino:””,con:””,unit:””,curr:””},{....}]}
int eMOPEquipSvc::GetIqsHis_Partsid(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetIqsHis_Partsid]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strSpid= root.getv("spid", "");
	string strEqid= root.getv("eqid", "");//

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql,\
		"select MODEL,NAME_CN,NAME_EN from blm_emop_etl.T50_EMOP_PARTS_MODELS \
		where PART_MODEL_ID='%s'",strSpid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	//	char Sn[100]="";//：船舶
	char Sno[100]="";//：备件编号
	char Cn[100]="";//：备件名称-中文
	char En[100]="";//：备件名称-英文

	if(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(MODEL, Sno);
		READMYSQL_STR(NAME_CN, Cn);
		READMYSQL_STR(NAME_EN, En);
	}
	out<<"{seq:\""<<strSeq.c_str()<< "\",Sno:\"" << Sno<< "\",Cn:\"" << Cn<< "\",En:\"" << En<<"\",list:[";
	sprintf (sql,\
		"select t1.PARTS_INQUERIED_QUOTED_ID,t1.QUATITY,t1.UNIT_PRICE,t2.COMPANY_KEY,UNIX_TIMESTAMP(t2.QUOTE_DATE) as QTDATE,t2.CURRENCY from blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS t1 left join\
		blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t2 on t1.PARTS_INQUERIED_QUOTED_ID=t2.PARTS_INQUERIED_QUOTED_ID \
		where t1.PART_MODEL_ID='%s' and t1.EQUIP_MODEL_ID='%s' ORDER BY QTDATE",strSpid.c_str(),strEqid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char Vdor[100]="";//：供应商
	int iMd=0;//：制单时间???
	char Ino[100]="";//：询价单号
	int iCon=0;//：询价数量
	double ffUnit=0;//：单价
	int iCurr=0;//：币种，此为名称
	int cnt = 0;
	while(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(PARTS_INQUERIED_QUOTED_ID, Ino);
		READMYSQL_INT(QUATITY, iCon,0);
		READMYSQL_DOUBLE(UNIT_PRICE, ffUnit,0);
		READMYSQL_STR(COMPANY_KEY, Vdor);
		READMYSQL_INT(QTDATE, iMd,0);
		READMYSQL_INT(CURRENCY, iCurr,0);
		if (cnt++)
			out << ",";
		//{vdor:””,md:”2013-11-01”,ino:””,con:””,unit:””,curr:””}
		out << "{vdor:\"" << Vdor << "\",md:" << iMd<< ",ino:\"" << Ino<< "\",Con:" << iCon<< ",unit:" << ffUnit<< ",curr:" << iCurr<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6145  (mt=0x61,st=0x45)
//{seq:”sss”,pno:"s50301"}
//{ship:”新厦门”,dept:”轮机”,pno:”S50301”,list:[{ino:”s001”,vdor:”上海供应商”,price:”100”,curr:”人民币”,md:1386579272},{...}]}
int eMOPEquipSvc::GetIqsList_Appo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetIqsList_Appo]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strPno= root.getv("pno", "");//采购单号(申请单)

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	// 取船舶名称部门名称
	sprintf (sql, "select t2.NAME_CN as sname_cn,t3.NAME_CN as dname_cn from blm_emop_etl.T50_EMOP_PARTS_APPLIES t1 left join \
				  blm_emop_etl.t41_emop_ship t2 on t1.SHIPID=t2.SHIPID left join \
				  blm_emop_etl.t50_emop_department_code t3 on t1.DEPARTMENT_CODE=t3.DEPARTMENT_CODE\
				  where t1.PARTS_APPLY_ID='%s'",strPno.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char shipname[100]="";
	char dptname[100]="";
	if(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(sname_cn, shipname);
		READMYSQL_STR(dname_cn, dptname);
	}
	out<<"{seq:\""<<strSeq.c_str()<< "\",ship:\"" << shipname<< "\",dept:\"" << dptname<< "\",pno:\"" << strPno.c_str()<<"\",list:[";
	// 取数组信息
	sprintf (sql, "select t1.PARTS_APPLY_INQUERY_ID,UNIX_TIMESTAMP(t1.OP_DATE) as OPDATE,t1.COMPANY_KEY,t2.TOTAL_PRICE,t2.CURRENCY\
				  from blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t1 left join \
				  blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t2 on t1.PARTS_APPLY_INQUERY_ID=t2.PARTS_APPLY_INQUERY_ID \
				  where t1.PARTS_APPLY_ID='%s'",strPno.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char Ino[100] = "";//：询价单号
	char Vdor[100] = "";//:供应商
	double ffPrice=0;//：总金额
	int iCurr=0;//：币种
	int iMd=0;//：制单日期

	int cnt = 0;
	while(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(PARTS_APPLY_INQUERY_ID, Ino);
		READMYSQL_STR(COMPANY_KEY, Vdor);
		READMYSQL_INT(CURRENCY, iCurr,0);
		READMYSQL_INT(OPDATE, iMd,0);
		READMYSQL_DOUBLE(TOTAL_PRICE, ffPrice,0);
		if (cnt++)
			out << ",";
		//list:[{ino:”s001”,vdor:”上海供应商”,price:”100”,curr:”人民币”,md:1386579272}
		out << "{ino:\"" << Ino << "\",Vdor:\"" << Vdor<< "\",price:" << ffPrice<< ",curr:" << iCurr<< ",md:" << iMd<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6146  (mt=0x61,st=0x46)
//{seq:”ss”,no:"s50301"}
//{seq:””,orders:[{sno:”S001”,spid:”s001”,eqid:”s001”,sn:”电源”,unit:”10.0”,con:”12”,price:”120.0”},{...}]}
int eMOPEquipSvc::GetPartsList_Iqs(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsList_Iqs]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strNo= root.getv("no", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",orders:[";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select t1.*,t1.EQUIP_MODEL_ID as PART_CAT_ID,t2.NAME_CN,t2.MODEL,t4.EQUIP_MODEL_ID from \
				  blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS t1 left join \
				  blm_emop_etl.T50_EMOP_PARTS_MODELS t2 on t1.PART_MODEL_ID=t2.PART_MODEL_ID left join\
				  blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t3 on t1.PARTS_INQUERIED_QUOTED_ID=t3.PARTS_APPLY_INQUERY_ID left join\
				  blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t4 on t1.EQUIP_MODEL_ID=t4.PART_CAT_ID and t3.SHIPID=t4.SHIPID\
				  where t1.PARTS_INQUERIED_QUOTED_ID='%s'",\
				  strNo.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char Sno[100] = "";//：备件编号
	char Sn[100] = "";//：备件名称
	double ffUnit=0;//：单价
	int iCon=0;//：询价数量
	double ffPrice=0;//：总价
	char Partsid[100] = "";// 备件id
	char Equipid[100] = "";// 设备id
	char qid[100] = "";//
	double ffDiscount;
	int cnt = 0;
	while(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(NAME_CN, Sn);
		READMYSQL_STR(MODEL, Sno);
		READMYSQL_INT(QUATITY, iCon,0);
		READMYSQL_DOUBLE(UNIT_PRICE, ffUnit,0);
		READMYSQL_DOUBLE(DISCOUNTS, ffDiscount,0);
		READMYSQL_DOUBLE(TOTAL_PRICE_DISCOUNTED, ffPrice,0);
		READMYSQL_STR(PART_MODEL_ID, Partsid);
		READMYSQL_STR(PART_CAT_ID, Equipid);
		READMYSQL_STR(EQUIP_MODEL_ID, qid);
		if (cnt++)
			out << ",";
		out << "{sno:\"" << Sno << "\",spid:\"" << Partsid<< "\",eqid:\"" << Equipid<< "\",qid:\"" << qid<< "\",sn:\"" << Sn<< "\",unit:" << ffUnit<< ",discount:" << ffDiscount<< ",con:" << iCon<< ",price:" << ffPrice<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6147  (mt=0x61,st=0x47)
//{seq:”sss”,pno:"s50301"}
//{seq:””,orders:[{sno:”23-1234”,spid:”s001”,eqid:”s001”,sn:”电源”,con:”12”},{...}]}
int eMOPEquipSvc::GetPartsList_Appo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsList_Appo]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strPno= root.getv("pno", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",orders:[";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select t1.PART_MODEL_ID,t1.EQUIP_MODEL_ID as PART_CAT_ID,t2.NAME_CN,t2.MODEL,t4.EQUIP_MODEL_ID from \
				  blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS t1 left join \
				  blm_emop_etl.T50_EMOP_PARTS_MODELS t2 on t1.PART_MODEL_ID=t2.PART_MODEL_ID left join \
				  blm_emop_etl.T50_EMOP_PARTS_APPLIES t3 on t1.PARTS_APPLY_ID=t3.PARTS_APPLY_ID left join\
				  blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t4 on t1.EQUIP_MODEL_ID=t4.PART_CAT_ID and t3.SHIPID=t4.SHIPID\
				  where t1.PARTS_APPLY_ID='%s'",\
				  strPno.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char Sno[100] = "";//：备件编号
	char Sn[100] = "";//：备件名称
	int iCon=0;//：询价数量
	char Partsid[100] = "";// 备件id
	char Equipid[100] = "";// 设备id
	char qid[100] = "";
	int cnt = 0;
	while(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(NAME_CN, Sn);
		READMYSQL_STR(MODEL, Sno);
		READMYSQL_INT(QUATITY, iCon,0);
		READMYSQL_STR(PART_MODEL_ID, Partsid);
		READMYSQL_STR(PART_CAT_ID, Equipid);
		READMYSQL_STR(EQUIP_MODEL_ID, qid);
		if (cnt++)
			out << ",";
		out << "{sno:\"" << Sno << "\",spid:\"" << Partsid<< "\",eqid:\"" << Equipid<< "\",qid:\"" << qid<< "\",sn:\"" << Sn<< "\",con:" << iCon<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6148  (mt=0x61,st=0x48)
//{seq:”ssss”,id:"s50301"}
//{seq:””,orders:[{ino:”s001”,vdor:”上海供应商”,unit:”10.0”,price:”120.0”,curr:”人民币”},{...}]}
int eMOPEquipSvc::GetIqsList_Partsid(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetIqsList_Partsid]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strId= root.getv("id", "");//备件ID
	out<<"{seq:\""<<strSeq.c_str()<<"\",orders:[";		
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, \
		"select t1.PARTS_INQUERIED_QUOTED_ID,t1.UNIT_PRICE,t1.TOTAL_PRICE_DISCOUNTED,t2.COMPANY_KEY,t2.CURRENCY,t3.name from \
		blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS t1 left join \
		blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t2 on t1.PARTS_INQUERIED_QUOTED_ID=t2.PARTS_INQUERIED_QUOTED_ID left join\
		blm_emop_etl.t41_company t3 on t2.COMPANY_KEY=t3.COMPANY_KEY\
		where t1.PART_MODEL_ID='%s'",\
		strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char Ino[100]="";//：询价单号
	char Vdor[100]= "";//：供应商
	double ffUnit=0;//：单价
	double ffPrice=0;//：总价
	int iCurr=0;//:币种	
	int cnt = 0;
	while(psql->NextRow())
	{//{stepId:””,sname:””,roleId:””}
		READMYSQL_STR(PARTS_INQUERIED_QUOTED_ID, Ino);
		READMYSQL_STR(name, Vdor);
		READMYSQL_INT(CURRENCY, iCurr,0);
		READMYSQL_DOUBLE(UNIT_PRICE, ffUnit,0);
		READMYSQL_DOUBLE(TOTAL_PRICE_DISCOUNTED, ffPrice,0);
		if (cnt++)
			out << ",";
		//{ino:”s001”,vdor:”上海供应商”,unit:”10.0”,price:”120.0”,curr:”人民币”}
		out << "{ino:\"" << Ino << "\",vdor:\"" << Vdor<< "\",unit:" << ffUnit<< ",price:" << ffPrice<< ",iCurr:" << iCurr<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x6149  (mt=0x61,st=0x49)
//{seq:”ssss”,id:"s50301"}
//{seq:””,orders:[{unit:”12.0”,con:”10”,vdor:”上海供应商”,ship:”新厦门”,ot:1386579272},{...}]}
int eMOPEquipSvc::GetAppoList_Partsid(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetAppoList_Partsid]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strId= root.getv("id", "");
	//	string strDid= root.getv("did", "");
	//	int iSt= root.getv("st", -1);
	out<<"{seq:\""<<strSeq.c_str()<<"\",orders:[";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t5 on t2.PARTS_INQUERIED_QUOTED_ID=t5.PARTS_INQUERIED_QUOTED_ID left join\
	blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t6 on t5.PARTS_APPLY_INQUERY_ID=t6.PARTS_APPLY_INQUERY_ID left join \
	blm_emop_etl.T50_EMOP_PARTS_APPLIES t7 on t6. 
	sprintf (sql, "select t1.UNIT_PRICE,t1.QUATITY,UNIX_TIMESTAMP(t3.OP_DATE) as OPDATE,t4.name AS CORPNAME,t5.NAME_CN as SHIPNAME_CN from \
				  blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS t1 left join \
				  blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t2 on t1.PARTS_PURCHASE_ORDER_ID=t2.PARTS_PURCHASE_ORDER_ID left join\
				  blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS t3 on t2.PARTS_PURCHASE_ORDER_ID=t3.PARTS_PURCHASE_ORDER_ID left join\
				  blm_emop_etl.t41_company t4 on t2.COMPANY_KEY=t4.COMPANY_KEY left join\
				  blm_emop_etl.t41_emop_ship t5 on t2.SHIPID=t5.SHIPID\
				  where t1.PART_MODEL_ID='%s' AND t3.STATUS='1'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	double ffUnit=0;//：单价
	int iCon=0;//：订购数量
	char Vdor[100]="";//：供应商
	char Ship[100]="";//：船舶
	int iOt=0;//：订购时间

	int cnt = 0;
	while(psql->NextRow())
	{//{unit:”12.0”,con:”10”,vdor:”上海供应商”,ship:”新厦门”,ot:1386579272}
		READMYSQL_STR(CORPNAME, Vdor);
		READMYSQL_STR(SHIPNAME_CN, Ship);
		READMYSQL_INT(QUATITY, iCon,0);
		READMYSQL_DOUBLE(UNIT_PRICE, ffUnit,0);
		READMYSQL_INT(OPDATE, iOt,0);
		if (cnt++)
			out << ",";
		out << "{unit:" << ffUnit << ",con:" << iCon<< ",vdor:\"" << Vdor<< "\",ship:\"" << Ship<< "\",ot:" << iOt<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x614a  (mt=0x61,st=0x4a)
//{seq:”ssss”,id:"s001"} 
//{seq:””,eid:0}
int eMOPEquipSvc::DeleteIqs(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::DeleteIqs]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strId= root.getv("id", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";
	sprintf (sql, "select STATUS from blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED \
				  where PARTS_INQUERIED_QUOTED_ID='%s'",\
				  strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iStatus=-1;
	if(psql->NextRow())
	{
		READMYSQL_INT(STATUS, iStatus,-1);
	}
	if(iStatus==0)
	{
		sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED WHERE PARTS_APPLY_INQUERY_ID = '%s'",strId.c_str());				
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY WHERE PARTS_APPLY_INQUERY_ID = '%s'",strId.c_str());				
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS WHERE PARTS_INQUERIED_QUOTED_ID = '%s'",strId.c_str());				
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_ATTACHS WHERE PARTS_APPLY_INQUERIED_QUOTED_ID = '%s'",strId.c_str());				

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//CmdId:0x614b  (mt=0x61,st=0x4b)
//{seq:”ssss”,id:"s001",oid:”121212”,t1:1386579272,t2:1386579272,t3:1386579272,addr:”送船地点”,pty:0,curr:0,fare:12,othe:12,price:23,list:[{spid:”s001”,eqid:”s003”,qu:10,price:12,rate:0.1},{..}]}
//{seq:””,eid:0}
int eMOPEquipSvc::CompleteIqs(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::CompleteIqs]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strId= root.getv("id", "");//id:询价单ID	
	string strOid=root.getv("oid", "");//:确认单号
	int iT1=root.getv("t1", -1);//:确认日期
	int iT2=root.getv("t2", -1);//:预抵日期
	int iT3=root.getv("t3", -1);//:送船日期
	string strAddr=root.getv("addr", "");//：送船地点
	int iPty=root.getv("pty", -1);//:付款方式
	int iCuur=root.getv("curr", -1);//：币种
	double ffFare=root.getv("fare", 0.0);//：运费
	double ffOthe=root.getv("oth", 0.0);//：其它费用
	double ffPrice=root.getv("price", 0.0);//：总价
	string strOptor= root.getv("operator", "");// 操作员
	int iOpdate=time(0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED SET QUOTE_CODE='%s',QUOTE_DATE=FROM_UNIXTIME(%d),ETA=FROM_UNIXTIME(%d),DELIVERY_DATE=FROM_UNIXTIME(%d), \
				  DELIVERY_LOCATION='%s',PAY_METHOD='%d',CURRENCY='%d',FREIGHT='%f',OTHER_EXPENSE='%f',TOTAL_PRICE='%f',STATUS='%d',OPERATOR='%s',OP_DT=FROM_UNIXTIME(%d)\
				  where PARTS_INQUERIED_QUOTED_ID='%s'",\
				  strOid.c_str(),iT1,iT2,iT3,strAddr.c_str(),iPty,iCuur,ffFare,ffOthe,ffPrice,1,strOptor.c_str(),iOpdate,strId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	Json *pList=root.get("list");//:备件、设备列表
	string strSpid;//:备件ID
	string strEqid;//:设备ID
	int iQu=0;//:申请数量
	double ffUnitp=0.0;//:单价
	double ffRate=0.0;//:折扣率
	double ffTotal=0.0;
	string cty="";
	int iSize=pList->size();
	for(int i=0;i<iSize;i++)
	{//{spid:”s001”,eqid:”s003”,qu:10,price:12,rate:0.1}
		Json *pParts=pList->get(i);
		strSpid=pParts->getv("spid","");//备件id
		strEqid=pParts->getv("eqid","");
		iQu=pParts->getv("qu",0);
		ffUnitp=pParts->getv("price",0.01);
		ffRate=pParts->getv("rate",0.01);
		ffTotal=pParts->getv("disp",0.01);//：折扣价
		cty=pParts->getv("cty","");

		sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS SET QUATITY=%d,UNIT_PRICE=%f,DISCOUNTS=%f,TOTAL_PRICE_DISCOUNTED=%f,CURRENCY='%s'\
					  where PARTS_INQUERIED_QUOTED_ID='%s' and PART_MODEL_ID='%s'",\
					  iQu,ffUnitp,ffRate,ffTotal,cty.c_str(),strId.c_str(),strSpid.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);	
}

//CmdId:0x614b  (mt=0x61,st=0x58)
//{seq:”ssss”,qid:""}
//{seq:””,id:””,qid:””,status:0,time:,provider:””,confrimId:””,ctime:””,etime:””,stime:””,site:””,pay:””,currey:””,sparePrice:””,fare:””,other:””,total:””,discount:”,disPrice:””}
int eMOPEquipSvc::GetIqsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::CompleteIqs]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strId= root.getv("qid", "");//id:询价单ID	


	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,"SELECT t1.PARTS_APPLY_INQUERY_ID,t1.DISCOUNTS,t1.STATUS,t1.QUOTE_CODE,UNIX_TIMESTAMP(t1.QUOTE_DATE) AS QUOTE_DATE,UNIX_TIMESTAMP(t1.ETA) AS ETA, \
				UNIX_TIMESTAMP(t1.DELIVERY_DATE) AS DELIVERY_DATE,t1.DELIVERY_LOCATION,t1.PAY_METHOD, \
				t1.CURRENCY,t1.FREIGHT,t1.OTHER_EXPENSE,t1.TOTAL_PRICE,t2.NAME,t3.PARTS_APPLY_ID,UNIX_TIMESTAMP(t3.OP_DATE) AS OP_DATE FROM \
				blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t1 \
				LEFT JOIN blm_emop_etl.T41_COMPANY t2 \
				ON t1.COMPANY_KEY=t2.COMPANY_KEY \
				JOIN blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t3 \
				ON t1.PARTS_APPLY_INQUERY_ID=t3.PARTS_APPLY_INQUERY_ID \
				WHERE t1.PARTS_INQUERIED_QUOTED_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);

	string parts_apply_inquery_id="";
	int status=0;
	double discounts=0.0;
	int op_date=0;
	string quote_code="";
	int quote_date=0;
	int eta=0;
	int delivery_date=0;
	string delivery_location="";
	int pay_method=0;
	int currency=0;
	double freight=0.0;
	double other_expense=0.0;
	double total_price=0.0;
	string name="";
	string parts_apply_id="";

	if(psql->NextRow())
	{
		READMYSQL_STRING(PARTS_APPLY_INQUERY_ID,parts_apply_inquery_id);
		READMYSQL_INT(STATUS,status,0);
		READMYSQL_DOUBLE(DISCOUNTS,discounts,0.0);
		READMYSQL_INT(OP_DATE,op_date,0);
		READMYSQL_INT(QUOTE_DATE,quote_date,0);
		READMYSQL_INT(ETA,eta,0);
		READMYSQL_INT(DELIVERY_DATE,delivery_date,0);
		READMYSQL_INT(PAY_METHOD,pay_method,0);
		READMYSQL_INT(CURRENCY,currency,0);
		READMYSQL_DOUBLE(FREIGHT,freight,0.0);
		READMYSQL_DOUBLE(OTHER_EXPENSE,other_expense,0.0);
		READMYSQL_DOUBLE(TOTAL_PRICE,total_price,0.0);
		READMYSQL_STRING(QUOTE_CODE,quote_code);
		READMYSQL_STRING(DELIVERY_LOCATION,delivery_location);
		READMYSQL_STRING(NAME,name);
		READMYSQL_STRING(PARTS_APPLY_ID,parts_apply_id);
	}

	sprintf(sql,"SELECT SUM(TOTAL_PRICE_DISCOUNTED+0) AS TOTAL FROM \
				blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS \
				WHERE PARTS_INQUERIED_QUOTED_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);
	
	double total=0.0;

	if(psql->NextRow())
	{
		READMYSQL_DOUBLE(TOTAL,total,0.0);
	}

	out<<"{seq:\""<<strSeq.c_str()<<"\",id:\""<<parts_apply_id<<"\",qid:\""<<parts_apply_inquery_id<<"\",status:"<<status<<",time:"<<op_date<<",provider:\""<<name<<"\",confrimId:\""<<quote_code<<"\",ctime:\""<<quote_date<<"\",etime:\""<<eta<<"\",stime:\""<<delivery_date<<"\",site:\""<<delivery_location<<"\",pay:\""<<pay_method<<"\",currey:\""<<currency<<"\",sparePrice:\""<<total<<"\",fare:\""<<freight<<"\",other:\""<<other_expense<<"\",total:\""<<total_price<<"\",discount:\""<<discounts<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);	
}

//CmdId:0x614b  (mt=0x61,st=0x59)
//{seq:”ssss”,qid:""}
//{seq:””,data:[{id:””,eid:””,qid:””,cn:””,en:””,tp:””,num:””,mf:””,img:””,rep:””,cu:””,pre:””,dc:””,dp:””,chn:},{},...]}
int eMOPEquipSvc::GetIqsEquipList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::CompleteIqs]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strId= root.getv("qid", "");//id:询价单ID	


	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	
	sprintf(sql,"SELECT t3.PART_MODEL_ID,t3.PART_CAT_ID,t3.EQUIP_MODEL_ID,t3.UNIT_PRICE,t3.DISCOUNTS,t3.QUATITY,t3.TOTAL_PRICE_DISCOUNTED,t3.CURRENCY,t4.INSTOCK,t5.NAME_CN, \
				t5.NAME_EN,t5.MODEL,t5.COMPANY_KEY,t5.CHART_NO \
				FROM blm_emop_etl.T50_EMOP_PARTS_INQUERIED_QUOTED t1 \
				JOIN blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t2 \
				ON t2.PARTS_APPLY_INQUERY_ID=t1.PARTS_APPLY_INQUERY_ID \
				JOIN  blm_emop_etl.T50_EMOP_PARTS_INQUERY_QUOTED_DETAILS t3 \
				ON t3.PARTS_INQUERIED_QUOTED_ID=t1.PARTS_INQUERIED_QUOTED_ID \
				JOIN blm_emop_etl.T50_EMOP_SHIP_PARTS t4 \
				ON t4.PART_CAT_ID=t3.PART_CAT_ID AND t4.SHIPID=t2.SHIPID AND t4.PART_MODEL_ID=t3.PART_MODEL_ID \
				JOIN blm_emop_etl.T50_EMOP_PARTS_MODELS t5 \
				ON t5.PART_MODEL_ID=t4.PART_MODEL_ID WHERE t1.PARTS_INQUERIED_QUOTED_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);

	out<<"seq:\""<<strSeq<<"\",data:[";

	string part_model_id="";
	string equip_model_id="";
	string part_cat_id="";
	string name_cn="";
	string name_en="";
	string model="";
	int quatity=0;
	string company_key="";
	string chart_no="";
	int instock=0;
	int currency=0;
	double unit_price=0.0;
	double discounts=0.0;
	double total_price_discounted;

	int idx=0;

	while(psql->NextRow())
	{
		if(idx>0)
		{
			out<<",";
		}
		idx++;
		READMYSQL_STRING(PART_MODEL_ID,part_model_id);
		READMYSQL_STRING(EQUIP_MODEL_ID,equip_model_id);
		READMYSQL_STRING(PART_CAT_ID,part_cat_id);
		READMYSQL_STRING(NAME_CN,name_cn);
		READMYSQL_STRING(NAME_EN,name_en);
		READMYSQL_STRING(MODEL,model);
		READMYSQL_INT(QUATITY,quatity,0);
		READMYSQL_STRING(COMPANY_KEY,company_key);
		READMYSQL_STRING(CHART_NO,chart_no);
		READMYSQL_INT(INSTOCK,instock,0);
		READMYSQL_INT(CURRENCY,currency,0);
		READMYSQL_DOUBLE(UNIT_PRICE,unit_price,0.0);
		READMYSQL_DOUBLE(DISCOUNTS,discounts,0.0);
		READMYSQL_DOUBLE(TOTAL_PRICE_DISCOUNTED,total_price_discounted,0.0);
			
		out<<"{id:\""<<part_model_id<<"\",eid:\""<<equip_model_id<<"\",qid:\""<<part_cat_id<<"\",cn:"<<name_cn<<",en:"<<name_en<<",tp:\""<<model<<"\",num:\""<<quatity<<"\",mf:\""<<company_key<<"\",img:\""<<chart_no<<"\",rep:\""<<instock<<"\",cu:\""<<currency<<"\",pre:\""<<unit_price<<"\",dc:\""<<discounts<<"\",dp:\""<<total_price_discounted<<"\"}";
	}	
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);	
}

//CmdId:0x615a  (mt=0x61,st=0x5a)
//{seq:”ssss”,qid:""}
//{seq:””,data:[{id:””,eid:””,qid:””,cn:””,en:””,tp:””,num:””,mf:””,img:””,rep:””,cu:””,pre:””,dc:””,dp:””,chn:},{},...]}
int eMOPEquipSvc::GetPurchaseEquipList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPurchaseEquipList]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strId= root.getv("qid", "");//id:询价单ID	


	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	
	sprintf(sql,"SELECT t3.PART_MODEL_ID,t3.PART_CAT_ID,t3.EQUIP_MODEL_ID,t3.UNIT_PRICE,t3.DISCOUNTS,t3.QUATITY,t3.TOTAL_PRICE_DISCOUNTED,t4.INSTOCK,t5.NAME_CN, \
				t5.NAME_EN,t5.MODEL,t5.COMPANY_KEY,t5.CHART_NO,t5.UNIT \
				FROM blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t1 \
				JOIN  blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS t3 \
				ON t3.PARTS_PURCHASE_ORDER_ID=t1.PARTS_PURCHASE_ORDER_ID \
				JOIN blm_emop_etl.T50_EMOP_SHIP_PARTS t4 \
				ON t4.PART_CAT_ID=t3.PART_CAT_ID AND t4.SHIPID=t1.SHIPID AND t4.PART_MODEL_ID=t3.PART_MODEL_ID \
				JOIN blm_emop_etl.T50_EMOP_PARTS_MODELS t5 \
				ON t5.PART_MODEL_ID=t4.PART_MODEL_ID WHERE t1.PARTS_APPLY_INQUERIED_QUOTED_ID='%s'",strId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);

	out<<"seq:\""<<strSeq<<"\",data:[";

	string part_model_id="";
	string equip_model_id="";
	string part_cat_id="";
	string name_cn="";
	string name_en="";
	string model="";
	int quatity=0;
	string company_key="";
	string chart_no="";
	int instock=0;
	//int currency=0;
	double unit_price=0.0;
	double discounts=0.0;
	double total_price_discounted;
	string unit="";

	int idx=0;

	while(psql->NextRow())
	{
		if(idx>0)
		{
			out<<",";
		}
		idx++;
		READMYSQL_STRING(PART_MODEL_ID,part_model_id);
		READMYSQL_STRING(EQUIP_MODEL_ID,equip_model_id);
		READMYSQL_STRING(PART_CAT_ID,part_cat_id);
		READMYSQL_STRING(NAME_CN,name_cn);
		READMYSQL_STRING(NAME_EN,name_en);
		READMYSQL_STRING(MODEL,model);
		READMYSQL_INT(QUATITY,quatity,0);
		READMYSQL_STRING(COMPANY_KEY,company_key);
		READMYSQL_STRING(CHART_NO,chart_no);
		READMYSQL_INT(INSTOCK,instock,0);
		//READMYSQL_INT(CURRENCY,currency,0);
		READMYSQL_DOUBLE(UNIT_PRICE,unit_price,0.0);
		READMYSQL_DOUBLE(DISCOUNTS,discounts,0.0);
		READMYSQL_DOUBLE(TOTAL_PRICE_DISCOUNTED,total_price_discounted,0.0);
		READMYSQL_STRING(UNIT,unit);
			
		out<<"{id:\""<<part_model_id<<"\",eid:\""<<equip_model_id<<"\",qid:\""<<part_cat_id<<"\",cn:"<<name_cn<<",en:"<<name_en<<",tp:\""<<model<<"\",num:\""<<quatity<<"\",mf:\""<<company_key<<"\",img:\""<<chart_no<<"\",rep:\""<<instock<<"\",pre:\""<<unit_price<<"\",dc:\""<<discounts<<"\",dp:\""<<total_price_discounted<<"\",unit:\""<<unit<<"\"}";
	}	
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);	
}

//CmdId:0x614c  (mt=0x61,st=0x4c)
//{seq:”ssss”,id:"s001",operator:””,taskId:””,stepId:””,shipId:””,deptId:””,list:[{spid:”s001”,eqid:”s001”,number:””},{}]}
//{seq:””,eid:0}
int eMOPEquipSvc::MkPurchaseSheete(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::MkPurchaseSheete]bad format:", jsonString, 1);	
	string strSeq= root.getv("seq", "");
	string strIqsid= root.getv("id", "");
	string strOptor= root.getv("operator", "");
	string strTaskid= root.getv("taskId", "");
	string strStepid= root.getv("stepId", "");
	string strShipid= root.getv("shipId", "");
	string strDptid= root.getv("deptId", "");
	string strCorpid= root.getv("vendorId", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	string strPchid="PO";
	long ltime =time(0);
	char szTmp[100];
	sprintf(szTmp,"%d",ltime);
	strPchid+=szTmp;
	// 订购单表
	sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS(PARTS_PURCHASE_ORDER_ID,PARTS_APPLY_INQUERIED_QUOTED_ID,ORDER_DATE,SHIPID,DEPARTMENT_CODE,COMPANY_KEY) values ('%s','%s',FROM_UNIXTIME(%d),'%s','%s','%s')",\
		strPchid.c_str(),strIqsid.c_str(),ltime,strShipid.c_str(),strDptid.c_str(),strCorpid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	// 订购单状态表
	sprintf (sql, \
		"insert into blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS(PARTS_PURCHASE_ORDER_ID,OP_DATE,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,STATUS,OPERATOR)\
		values ('%s',FROM_UNIXTIME(%d),'%s','%s','%d','%s')",\
		strPchid.c_str(),ltime,strTaskid.c_str(),strStepid.c_str(),0,strOptor.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	// 订购单备件列表
	sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS WHERE PARTS_PURCHASE_ORDER_ID= '%s'",strPchid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	Json* pJList = root.get("list");//
	// {spid:”s001”,eqid:”s001”,number:””}
	int iSize=pJList->size();
	string strPartsid,strEqid;
	int iCount=0;
	double ffUnit=0;
	double ffDiscount=0;
	double ffTPrice=0;
	for(int i=0;i<iSize;i++)
	{
		Json* pJPChild=pJList->get(i);
		strPartsid=pJPChild->getv("spid","");//备件id
		strEqid=pJPChild->getv("eqid","");//设备id
		iCount=pJPChild->getv("number",-1);//订购数量
		ffUnit=pJPChild->getv("unit",0.01);//：单价
		ffDiscount=pJPChild->getv("discount",0.01);//：折扣率
		ffTPrice=pJPChild->getv("price",0.01);//：折扣后总结
		sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS(PARTS_PURCHASE_ORDER_ID,PART_MODEL_ID,EQUIP_MODEL_ID,QUATITY,UNIT_PRICE,DISCOUNTS,TOTAL_PRICE_DISCOUNTED)\
					  values ('%s','%s','%s','%d','%f','%f','%f')",\
					  strPchid.c_str(),strPartsid.c_str(),strEqid.c_str(),iCount,ffUnit,ffDiscount,ffTPrice);

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);		 
	}	
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);	
}
//MID： 0x0061    SID： 0x0025
//{ seq： "ssssssss001",shipId："",depart：””,status:””,taskId:””,stepId:””}
//{seq：”s1”,shipId：”” ,data：[{aid：”s000001”,time：”12321231”, status：1},{},....]}
int eMOPEquipSvc::SearchPurchase(const char* pUid, const char* jsonString, std::stringstream& out){
	JSON_PARSE_RETURN("[eMOPEquipSvc::SearchPurchase]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", "");
	string strShipId = root.getv("shipId", "");
	string strPartId= root.getv("part", "");
	string strTaskId=root.getv("taskId", "");
	string strStepId=root.getv("stepId", "");
	int iState= root.getv("status", -1);

	string strTmp="";
	Tokens svrList;	
	if(strStepId.find("|") != string::npos)
	{					 
		svrList = StrSplit(strStepId, "|");		
		for(int i=0;i<svrList.size();i++)
		{
			if(svrList[i].empty())
				continue;
			strTmp+="'";
			strTmp+=svrList[i];
			strTmp+="'";
			if (i<((int)svrList.size()-1))
				strTmp+=",";
		}
	}
	else
	{
		strTmp+="'";
		strTmp+=strStepId;
		strTmp+="'";
	}

	out<<"{seq:\""<<strSeq.c_str()<<"\",shipId:\""<<strShipId.c_str()<<"\",data:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	if(strPartId.empty()&&(iState>3||iState<0))
	{ // 部门和状态都无限制
		sprintf (sql, \
			"select t1.PARTS_PURCHASE_ORDER_ID,UNIX_TIMESTAMP(t1.ORDER_DATE) as apptm,t2.STATUS from \
			blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t1 left join \
			blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS t2 on t1.PARTS_PURCHASE_ORDER_ID=t2.PARTS_PURCHASE_ORDER_ID left join\
			blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t3 on t1.PARTS_APPLY_INQUERIED_QUOTED_ID=t3.PARTS_APPLY_INQUERY_ID left join\
			blm_emop_etl.T50_EMOP_PARTS_APPLIES t4 on t3.PARTS_APPLY_ID=t4.PARTS_APPLY_ID\
			where t4.SHIPID='%s'and t2.WORKFLOW_TASK_ID='%s' and t2.WORKFLOW_STEP_ID IN(%s) order by apptm  desc",\
			strShipId.c_str(),strTaskId.c_str(),strTmp.c_str());
	}
	else if(strPartId.empty())
	{// 部门无限制
		sprintf (sql, \
			"select t1.PARTS_PURCHASE_ORDER_ID,UNIX_TIMESTAMP(t1.ORDER_DATE) as apptm,t2.STATUS from \
			blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t1 left join \
			blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS t2 on t1.PARTS_PURCHASE_ORDER_ID=t2.PARTS_PURCHASE_ORDER_ID left join\
			blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t3 on t1.PARTS_APPLY_INQUERIED_QUOTED_ID=t3.PARTS_APPLY_INQUERY_ID left join\
			blm_emop_etl.T50_EMOP_PARTS_APPLIES t4 on t3.PARTS_APPLY_ID=t4.PARTS_APPLY_ID\
			where t4.SHIPID='%s'and t2.status='%d' and t2.WORKFLOW_TASK_ID='%s' and t2.WORKFLOW_STEP_ID IN(%s) order by apptm  desc",\
			strShipId.c_str(), iState,strTaskId.c_str(),strTmp.c_str());
	}
	else if(iState>3||iState<0)
	{ // 状态无限制
		sprintf (sql, \
			"select t1.PARTS_PURCHASE_ORDER_ID,UNIX_TIMESTAMP(t1.ORDER_DATE) as apptm,t2.STATUS from \
			blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t1 left join \
			blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS t2 on t1.PARTS_PURCHASE_ORDER_ID=t2.PARTS_PURCHASE_ORDER_ID left join\
			blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t3 on t1.PARTS_APPLY_INQUERIED_QUOTED_ID=t3.PARTS_APPLY_INQUERY_ID left join\
			blm_emop_etl.T50_EMOP_PARTS_APPLIES t4 on t3.PARTS_APPLY_ID=t4.PARTS_APPLY_ID\
			where t4.SHIPID='%s'and t4.DEPARTMENT_CODE='%s' and t2.WORKFLOW_TASK_ID='%s' and t2.WORKFLOW_STEP_ID IN(%s) order by apptm  desc",\
			strShipId.c_str(),strPartId.c_str(),strTaskId.c_str(),strTmp.c_str());
	}
	else
	{ // 都有限制
		sprintf (sql, \
			"select t1.PARTS_PURCHASE_ORDER_ID,UNIX_TIMESTAMP(t1.ORDER_DATE) as apptm,t2.STATUS from \
			blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t1 left join \
			blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS t2 on t1.PARTS_PURCHASE_ORDER_ID=t2.PARTS_PURCHASE_ORDER_ID left join\
			blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t3 on t1.PARTS_APPLY_INQUERIED_QUOTED_ID=t3.PARTS_APPLY_INQUERY_ID left join\
			blm_emop_etl.T50_EMOP_PARTS_APPLIES t4 on t3.PARTS_APPLY_ID=t4.PARTS_APPLY_ID\
			where t4.SHIPID='%s'and t4.DEPARTMENT_CODE='%s' and t2.STATUS='%d' and t2.WORKFLOW_TASK_ID='%s' and t2.WORKFLOW_STEP_ID IN(%s) order by apptm desc",\
			strShipId.c_str(),strPartId.c_str(), iState,strTaskId.c_str(),strTmp.c_str());
	}

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char aid[100] = "";
	int itime=0;
	int iStatus=0;
	int cnt = 0;
	out << "[";	
	while (psql->NextRow())
	{
		READMYSQL_STR(PARTS_PURCHASE_ORDER_ID, aid);
		READMYSQL_INT(apptm, itime,-1);
		READMYSQL_INT(STATUS, iStatus,-1);
		if (cnt++)
			out << ",";
		//{aid：”s000001”,time：”12321231”, status：1}
		out << "{aid:\"" << aid << "\",time:\"" << itime <<"\",st:" << iStatus << "}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);	
}
//MID： 0x0061    SID： 0x0026
//{seq：”ss00001”,aid：”s0000001”}
//{seq:”sss”,id:””,sid:””state:1,process[{taskId:””,stepId:””,stateId:””,fn:””,ft:””,fr:””},{},...],spare[{id:””,bid:””,sid:””,,nm:””,num:,unit:””,pr:,cu:,to:,rm:””,check:[{taskId:””,stepId:””,number:””},{},...]””},{},...]}
int eMOPEquipSvc::GetPurchaseDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPurchaseDetail]bad format:", jsonString, 1);
	string strAid = root.getv("aid", "");// 订单id
	string strSeq= root.getv("seq", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\"";
	MySql* psql = CREATE_MYSQL;

	/*
	eid：询价单号
	Sid：订购单号
	Process：流程信息
	taskId:任务ID
	stepId：步骤ID
	stateId:状态(当前流程状态)   状态 0待审核 1已审核 2 退单 3作废 4 所有 
	Fn：操作人
	Ft：操作时间
	Fr：备注
	Spare：订购订单内容
	Id:备件ID
	Bid：设备ID
	sid: 备件编号/型号
	nm: 备件名称
	num:订购数量
	unit: 单位
	pr:单价
	cu:折扣率
	to:折扣总价
	Check:核发数量
	taskId:任务ID
	stepId：步骤ID
	Number：数量
	*/
	char sql[1024] = "";
	sprintf (sql, \
		"select PARTS_APPLY_INQUERIED_QUOTED_ID from blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS \
		where PARTS_PURCHASE_ORDER_ID='%s'",strAid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char szIqsid[100] = "";
	string strTmp;
	int cnt = 0;
	if(psql->NextRow())
	{//{seq:"detail0000",sid:"",ship:"",dep:"",et:1234567,way:"aaaa",ad:"bbbb",isu:0,isl:0,flow:[{nm:"aa",url:"http://wwaidu.com/"}],rs:"reason",pro:[{fs:fn:"lucy",fc:"申请",ft:12312,fr:"备注",fs:0}],list:[{Id:”ssss00001”, bid:””,sid:"2-1232",name:"na",num:5,unit:"个",isi:0,has:10}]}
		READMYSQL_STR(PARTS_APPLY_INQUERIED_QUOTED_ID, szIqsid);
		//seq:"detail0000",sid:"",ship:"",dep:"",st:0,et:1234567,way:"aaaa",ad:"bbbb",isu:0,isl:0,flow:
		out <<",sid:\"" << strAid.c_str()<< "\",eid:\""<< szIqsid << "\",process:[";
	}
	//process
	char szP_taskid[100]="";
	char szP_stepid[100]="";
	int iStatus=0;
	char szP_fn[100]="";//操作人
	int iOpdate=0;//操作时间
	char szP_fr[100]="";//备注
	sprintf (sql, \
		"select WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,STATUS,OPERATOR,UNIX_TIMESTAMP(OP_DATE) AS OPDATE,REMARK from \
		blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS where PARTS_PURCHASE_ORDER_ID='%s'",strAid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	cnt = 0;
	while(psql->NextRow())
	{//{taskId:,stepId:,stateId:,fn:"lucy",fc:"",ft:12312,fr:"备注"}
		READMYSQL_STR(WORKFLOW_TASK_ID, szP_taskid);
		READMYSQL_STR(WORKFLOW_STEP_ID, szP_stepid);
		READMYSQL_INT(STATUS, iStatus,0);
		READMYSQL_STR(OPERATOR, szP_fn);
		READMYSQL_INT(OPDATE, iOpdate,0); 
		READMYSQL_STR(REMARK, szP_fr); 
		if (cnt++)
			out << ",";
		out << "{taskId:\"" <<szP_taskid<< "\",stepId:\"" << szP_stepid << "\",stateId:" <<iStatus<< ",fn:\"" \
			<<szP_fn<< "\",ft:"<<iOpdate<<",fr:\""<<szP_fr<<"\"}";
	}
	out<<"],spare:[";
	//spare
	char szL_partsid[100]="";//备件ID
	char szL_equipid[100]="";//设备ID
	char szL_sid[100]="";//备件编号/型号
	char szL_na[100]="";//备件名称
	int iAppCount=0;//订购数量
	char szL_unit[100]="";//单位
	double ffPr=0;//pr:单价
	double ffCu=0;//cu:折扣率
	double ffTo=0;//to:折扣总价
	char szQid[100]="";//设备属性id

	//Id：备件ID Bid: 设备ID Sid：备件编号 Name：名称 Num:申请数量 Unit：单位 Isi：是否为重大备件 Has：库存
	int iCheck=0;
	char szTaskid[100]="";
	char szStepid[100]="";
	sprintf (sql, \
		"select t1.*,t1.EQUIP_MODEL_ID as PART_CAT_ID,t2.NAME_CN,t2.MODEL,t2.UNIT,t4.EQUIP_MODEL_ID from \
		blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS t1 left join \
		blm_emop_etl.t50_emop_parts_models t2 on t1.PART_MODEL_ID=t2.PART_MODEL_ID left join\
		blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t3 on t1.PARTS_PURCHASE_ORDER_ID=t3.PARTS_PURCHASE_ORDER_ID left join\
		blm_emop_etl.T50_EMOP_PART_SHIP_CATEGORIES t4 on t1.EQUIP_MODEL_ID=t4.PART_CAT_ID and t3.SHIPID=t4.SHIPID\
		where t1.PARTS_PURCHASE_ORDER_ID='%s'",strAid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	MySql* psqlTmp = CREATE_MYSQL;
	cnt = 0;
	int cnt1=0;
	while(psql->NextRow())
	{//{id:””,bid:””,sid:””,,nm:””,num:,unit:””,pr:,cu:,to:,check:[{taskId:””,stepId:””,number:””},{},...]””}
		READMYSQL_STR(PART_MODEL_ID, szL_partsid);
		READMYSQL_STR(PART_CAT_ID, szL_equipid);
		READMYSQL_STR(MODEL, szL_sid);
		READMYSQL_STR(NAME_CN, szL_na);
		READMYSQL_INT(QUATITY, iAppCount,0);
		READMYSQL_STR(UNIT, szL_unit); 

		READMYSQL_DOUBLE(UNIT_PRICE, ffPr,0); 
		READMYSQL_DOUBLE(DISCOUNTS, ffCu,0); 
		READMYSQL_DOUBLE(TOTAL_PRICE_DISCOUNTED, ffTo,0); 
		READMYSQL_STR(EQUIP_MODEL_ID, szQid);
		if (cnt++)
			out << ",";
		out << "{Id:\"" << szL_partsid << "\",bid:\"" << szL_equipid<< "\",qid:\"" << szQid \
			<< "\",sid:\"" << szL_sid <<"\",name:\"" << szL_na << "\",num:" << iAppCount\
			<<",unit:\"" << szL_unit<< "\",pr:" << ffPr<< ",cu:" << ffCu<< ",to:" << ffTo<< ",check:[";//"}";
		sprintf (sql, \
			"select WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,QUANTITY from blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_STATUS_DETAILS\
			where PARTS_PURCHASE_ORDER_ID='%s' and EQUIP_MODEL_ID='%s' and  PART_MODEL_ID='%s'",strAid.c_str(),szL_equipid,szL_partsid);
		CHECK_MYSQL_STATUS(psqlTmp->Query(sql), 3);
		cnt1=0;
		while(psqlTmp->NextRow())
		{//{Id:”ssss00001”, bid:””,sid:"2-1232",name:"na",num:5,unit:"个",isi:0,has:10}
			READMYSQL_STR1(WORKFLOW_TASK_ID, szTaskid,psqlTmp);
			READMYSQL_STR1(WORKFLOW_STEP_ID, szStepid,psqlTmp);
			READMYSQL_INT1(QUANTITY, iCheck,-1,psqlTmp);
			if (cnt1++)
				out << ",";
			out<<"{taskid:\""<<szTaskid<<"\",stepId:\""<<szStepid<<"\",number:"<<iCheck<<"}";
		}
		out<<"]}";		
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}	
//MID: 0x0061   SID: 0x0027
//{seq:”ssss”,oid:””,tid:””,sid:””,total:,list:[{id:””,bid:””,num:””},{},.....]}
//{seq:””,eid:””}
int eMOPEquipSvc::ModifyPurchase(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::ModifyPurchase]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", "");
	string strOid= root.getv("oid", "");//：订单ID
	string strTaskId= root.getv("tid", "");//: 任务ID
	string strStepId= root.getv("sid", "");//: 步骤ID
	double ffTotal=root.getv("total", 0.0);//：总价

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	// 修改备件订购单表
	sprintf (sql,\
		"UPDATE blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS SET \
		TOTAL_PRICE='%f' WHERE PARTS_PURCHASE_ORDER_ID = '%s'",\
		ffTotal,strOid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	// 修改备件列表
	//	sprintf (sql, "delete from blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_STATUS_DETAILS WHERE PARTS_PURCHASE_ORDER_ID= '%s'",strOid.c_str());
	//	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	Json* pJList = root.get("list");//
	int iSize=pJList->size();
	string strId,strBid;
	int iCount=0;
	int iCheck=0;// 核发数量
	int iTime=time(0);
	for(int i=0;i<iSize;i++)
	{
		Json* pJPChild=pJList->get(i);
		strId=pJPChild->getv("id","");//备件id
		strBid=pJPChild->getv("bid","");//设备id
		iCheck=pJPChild->getv("num",-1);//核查数量
		//		iCheck=pJPChild->getv("check",-1);
		// 如果审核阶段：更改某个单子的某个备件的核发数量
		{
			sprintf (sql, "select QUANTITY from blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_STATUS_DETAILS where \
						  PARTS_PURCHASE_ORDER_ID='%s' and WORKFLOW_TASK_ID='%s' and WORKFLOW_STEP_ID='%s' and EQUIP_MODEL_ID='%s' and PART_MODEL_ID='%s'",\
						  strOid.c_str(),strTaskId.c_str(),strStepId.c_str(),strBid.c_str(),strId.c_str());

			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
			if(psql->NextRow())
			{// 如果T50_EMOP_PARTS_APPLY_STATUS_DETAILS能找到此条记录
				sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_STATUS_DETAILS SET QUATITY='%d',FROM_UNIXTIME(%d) where \
							  PARTS_PURCHASE_ORDER_ID='%s' and WORKFLOW_TASK_ID='%s' and WORKFLOW_STEP_ID='%s' and EQUIP_MODEL_ID='%s' and PART_MODEL_ID='%s'",\
							  iCheck,iTime,strOid.c_str(),strTaskId.c_str(),strStepId.c_str(),strBid.c_str(),strId.c_str());
			}
			else
				sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_STATUS_DETAILS(PARTS_PURCHASE_ORDER_ID,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,EQUIP_MODEL_ID,PART_MODEL_ID,QUANTITY,OP_DATE) values ('%s','%s','%s','%s','%s','%d',FROM_UNIXTIME(%d))",\
				strOid.c_str(),strTaskId.c_str(),strStepId.c_str(),strBid.c_str(),strId.c_str(),iCheck,iTime);

			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
		//sprintf (sql, "select QUATITY from blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS where PARTS_APPLY_ID='%s' and PART_MODEL_ID='%s'",\
		//			  strSid.c_str(),strD1.c_str());	
		//CHECK_MYSQL_STATUS(psql->Query(sql), 3); // 客户端可能相同备件写两遍错误
		//if(psql->NextRow())
		//{
		//	sprintf (sql, "update blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS set EQUIP_MODEL_ID='%s',QUATITY='%d'where PARTS_APPLY_ID='%s'and PART_MODEL_ID='%s')",\
		//		strD2.c_str(),iCount,strSid.c_str(),strD1.c_str());
		//}
		//else
		//{
		//	sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS(PARTS_APPLY_ID,PART_MODEL_ID,EQUIP_MODEL_ID,QUATITY) values ('%s','%s','%s','%d')",\
		//		strSid.c_str(),strD1.c_str(),strD2.c_str(),iCount);
		//}

		//CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}	

	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID:0x0061,      sid:0x0028
//{seq:””,shipId:””,taskId:””,stepId:””}
//{seq:””,not:,back:}
int eMOPEquipSvc::GetPchStatusCount(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPchStatusCount]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strShipid=root.getv("shipId", "");//shipID
	string strTaskId=root.getv("taskId", "");
	string strStepId=root.getv("stepId", "");
	string strTmp="";
	Tokens svrList;	
	if(strStepId.find("|") != string::npos)
	{					 
		svrList = StrSplit(strStepId, "|");
		for(int i=0;i<svrList.size();i++)
		{
			if(svrList[i].empty())
				continue;
			strTmp+="'";
			strTmp+=svrList[i];
			strTmp+="'";
			if (i<((int)svrList.size()-1))
				strTmp+=",";
		}
	}
	else
	{
		strTmp+="'";
		strTmp+=strStepId;
		strTmp+="'";
	}


	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//not
	sprintf (sql,\
		"select t1.* from blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t1 left join \
		blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS t2 on t1.PARTS_PURCHASE_ORDER_ID=t2.PARTS_PURCHASE_ORDER_ID left join\
		blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t3 on t1.PARTS_APPLY_INQUERIED_QUOTED_ID=t3.PARTS_APPLY_INQUERY_ID left join\
		blm_emop_etl.T50_EMOP_PARTS_APPLIES t4 on t3.PARTS_APPLY_ID=t4.PARTS_APPLY_ID\
		where t4.SHIPID='%s' and t2.WORKFLOW_TASK_ID='%s' and t2.STATUS='0' and t2.WORKFLOW_STEP_ID IN(%s)",strShipid.c_str(),strTaskId.c_str(),strTmp.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iNot=psql->GetRowCount();
	// back
	sprintf (sql, \
		"select t1.* from blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t1 left join \
		blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS t2 on t1.PARTS_PURCHASE_ORDER_ID=t2.PARTS_PURCHASE_ORDER_ID left join\
		blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t3 on t1.PARTS_APPLY_INQUERIED_QUOTED_ID=t3.PARTS_APPLY_INQUERY_ID left join\
		blm_emop_etl.T50_EMOP_PARTS_APPLIES t4 on t3.PARTS_APPLY_ID=t4.PARTS_APPLY_ID\
		where t4.SHIPID='%s' and t2.WORKFLOW_TASK_ID='%s' and t2.STATUS='2' and t2.WORKFLOW_STEP_ID IN(%s)",strShipid.c_str(),strTaskId.c_str(),strTmp.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int iBack=psql->GetRowCount();
	out<<"{seq:\""<<strSeq.c_str()<<"\",not:"<<iNot<<",back:"<<iBack<<"}";
	RELEASE_MYSQL_RETURN(psql, 0);	
}
//MID:0x0061      SID:0x002D
//{seq:”ssss”,uid:””,oid:””,tid:””,sid:””,status:””,idea:””}
//{seq:”sssss”,eid:0}
int eMOPEquipSvc::PurchaseOpt(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPEquipSvc::PurchaseOpt]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strUid= root.getv("uid", "");//操作人
	string strOid=root.getv("oid", "");//订单ID
	string strTaskid=root.getv("tid", "");//任务ID
	string strStepid=root.getv("sid", "");//步骤ID
	string strRemark=root.getv("idea", "");
	int iStatus=root.getv("status", -1);

	int iTime=time(0);// 操作时间
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select *from blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS where PARTS_PURCHASE_ORDER_ID='%s'  and STATUS='0'",strOid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{// 
		sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS SET OP_DATE=FROM_UNIXTIME(%d),REMARK='%s',OPERATOR='%s',WORKFLOW_TASK_ID='%s',WORKFLOW_STEP_ID='%s', STATUS='%d' where PARTS_PURCHASE_ORDER_ID='%s' and STATUS='0'",\
			iTime,strRemark.c_str(),strUid.c_str(),strTaskid.c_str(),strStepid.c_str(), iStatus,strOid.c_str());
	}
	else// 插入新的状态处理
	{
		sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS(PARTS_PURCHASE_ORDER_ID,OP_DATE,REMARK,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID, STATUS) values ('%s',FROM_UNIXTIME(%d),'%s','%s','%s','%s','%d')",\
			strOid.c_str(),iTime,strRemark.c_str(),strUid.c_str(),strTaskid.c_str(),strStepid.c_str(), iStatus);
	}

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	// 取出所有步骤 升序排列
	vector <string>vecSteps;
	GetAllSteps(vecSteps,psql,strTaskid);
	// 根据状态判断下一步
	string strTmp;
	char szSid[32];
	int iRlt=0;
	switch(iStatus)
	{
	case 1: //已审核 step+1
		iRlt=GetNext(strTmp,vecSteps,strStepid,true);
		if(iRlt==2)// end生成下一个状态 判断路由表
		{
			sprintf (sql, "select NETX_TASK  from blm_emop_etl.T50_EMOP_WORKFLOW_ROUTES where CURRENT_TASK_ID='%s' and ROUTE_CONDITION='%s'",strTaskid.c_str(),strStepid.c_str());

			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
			if(psql->NextRow())
			{ // 如果找到下一个任务
				READMYSQL_STR(NETX_TASK, szSid); // 暂时写死 1
				// 取出所有步骤 升序排列
				strTmp=szSid;
				GetAllSteps(vecSteps,psql,strTmp);
				if(vecSteps.size()<=0)//error
					break;
				iTime=time(0)+100;
				sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS(PARTS_PURCHASE_ORDER_ID,OP_DATE,REMARK,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID, STATUS) values ('%s',FROM_UNIXTIME(%d),'%s','%s','%s','%s','%d')",\
					strOid.c_str(),iTime,strRemark.c_str(),strUid.c_str(),szSid,vecSteps.at(0).c_str(), 0);

				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
			}
		}
		else if(iRlt==1) //生成新的待审核单
		{
			iTime=time(0)+100;
			sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS(PARTS_PURCHASE_ORDER_ID,OP_DATE,REMARK,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID, STATUS) values ('%s',FROM_UNIXTIME(%d),'%s','%s','%s','%s','%d')",\
				strOid.c_str(),iTime,strRemark.c_str(),strUid.c_str(),strTaskid.c_str(),strTmp.c_str(), 0);

			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
		break; 
	case 2: // 2 退单step-1 
		iRlt=GetNext(strTmp,vecSteps,strStepid,false);	 
		if(iRlt==2)// end
		{ // 申请单第一步，无需处理
		}
		else if(iRlt==1)
		{
			iTime=time(0)+100;
			sprintf (sql, "insert into blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS(PARTS_PURCHASE_ORDER_ID,OP_DATE,REMARK,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID, STATUS) values ('%s',FROM_UNIXTIME(%d),'%s','%s','%s','%s','%d')",\
				strOid.c_str(),iTime,strRemark.c_str(),strUid.c_str(),strTaskid.c_str(),strTmp.c_str(), 0);

			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}

		break;
	case 0: // 待审核 无需下一状态
	case 3: // 3作废 无需下一状态
	default:
		break;
	}
	out<<"{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID：0x0061   SID:0x002E
//{seq:””,oid:””,bid:””,sid:””,type:}
//{seq:””,data:[{taskId:””,stepId:””,apply:””,check:””},{},....]}
int eMOPEquipSvc::GetPartsHis(const char* pUid, const char* jsonString, std::stringstream& out)
{ // 订购和批复历史
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetPartsHis]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");
	string strOid=root.getv("oid", "");//订单ID or 申请单id
	string strEquipid=root.getv("bid", "");//ID
	string strPartsid=root.getv("sid", "");//
	int iType=root.getv("type",-1);//type:0采购批复，1订购
	out <<"{seq:\""<<strSeq<<"\",data:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char szIqsid[100]="";
	char szTaskid[100]="";
	char szStepid[100]="";
	int iApponum=0;
	int iChecknum=0;
	int cnt=0;

	if(iType==0)
	{
		sprintf (sql, "select t1.QUATITY as IQCOUNT,t2.WORKFLOW_TASK_ID,t2.WORKFLOW_STEP_ID,t3.QUANTITY from \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS t1 left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS_DETAILS t3 on t1.PARTS_APPLY_ID=t3.PARTS_APPLY_ID and t1.PART_MODEL_ID=t3.PART_MODEL_ID and t1.EQUIP_MODEL_ID=t3.EQUIP_MODEL_ID \
					  where t1.PARTS_APPLY_ID='%s' and t1.PART_MODEL_ID='%s' and t1.EQUIP_MODEL_ID='%s'",strOid.c_str(),strPartsid.c_str(),strEquipid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		while(psql->NextRow())
		{//{taskId:””,stepId:””,apply:””,check:””}
			READMYSQL_STR(WORKFLOW_TASK_ID, szTaskid);
			READMYSQL_STR(WORKFLOW_STEP_ID, szStepid);
			READMYSQL_INT(IQCOUNT, iApponum,0);
			READMYSQL_INT(QUANTITY, iChecknum,0);
			if (cnt++)
				out << ",";
			out<<"{taskId:\""<<szTaskid<<"\",stepId:\""<<szStepid<<"\",apply:"<<iApponum<<",check:"<<iChecknum<<"}";
		}
	}
	else if(iType==1)
	{
		// 订购数量批复数量
		// T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS 订购数量表	
		// T50_EMOP_PARTS_PURCHASE_ORDER_STATUS_DETAILS 批复数量表
		// T50_EMOP_PURCHASE_ORDER_STATUS 批复数量表
		// T50_EMOP_PARTS_PURCHASE_ORDERS 订购单表	 
		sprintf (sql, "select t1.QUATITY as IQCOUNT,t2.PARTS_APPLY_INQUERIED_QUOTED_ID,t3.WORKFLOW_TASK_ID,t3.WORKFLOW_STEP_ID,t4.QUANTITY from \
					  blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_DETAILS t1 left join \
					  blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDERS t2 on t1.PARTS_PURCHASE_ORDER_ID=t2.PARTS_PURCHASE_ORDER_ID left join \
					  blm_emop_etl.T50_EMOP_PURCHASE_ORDER_STATUS t3 on t1.PARTS_PURCHASE_ORDER_ID=t3.PARTS_PURCHASE_ORDER_ID left join \
					  blm_emop_etl.T50_EMOP_PARTS_PURCHASE_ORDER_STATUS_DETAILS t4 on t1.PARTS_PURCHASE_ORDER_ID=t4.PARTS_PURCHASE_ORDER_ID and t1.PART_MODEL_ID=t4.PART_MODEL_ID and t1.EQUIP_MODEL_ID=t4.EQUIP_MODEL_ID\
					  where t1.PARTS_PURCHASE_ORDER_ID='%s' and t1.PART_MODEL_ID='%s' and t1.EQUIP_MODEL_ID='%s'",strOid.c_str(),strPartsid.c_str(),strEquipid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		while(psql->NextRow())
		{//{taskId:””,stepId:””,apply:””,check:””}
			READMYSQL_STR(PARTS_APPLY_INQUERIED_QUOTED_ID, szIqsid);
			READMYSQL_STR(WORKFLOW_TASK_ID, szTaskid);
			READMYSQL_STR(WORKFLOW_STEP_ID, szStepid);
			READMYSQL_INT(IQCOUNT, iApponum,0);
			READMYSQL_INT(QUANTITY, iChecknum,0);
			if (cnt++)
				out << ",";
			out<<"{taskId:\""<<szTaskid<<"\",stepId:\""<<szStepid<<"\",apply:"<<iApponum<<",check:"<<iChecknum<<"}";
		}
		//申请批复数量
		sprintf (sql, "select t2.QUATITY as IQCOUNT,t3.WORKFLOW_TASK_ID,t3.WORKFLOW_STEP_ID,t4.QUANTITY from \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_INQUERY t1 left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_DETAILS t2 on t1.PARTS_APPLY_ID=t2.PARTS_APPLY_ID left join \
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS t3 on t2.PARTS_APPLY_ID=t3.PARTS_APPLY_ID left join\
					  blm_emop_etl.T50_EMOP_PARTS_APPLY_STATUS_DETAILS t4 on t2.PARTS_APPLY_ID=t4.PARTS_APPLY_ID and t2.PART_MODEL_ID=t4.PART_MODEL_ID and t2.EQUIP_MODEL_ID=t4.EQUIP_MODEL_ID \
					  where t1.PARTS_APPLY_INQUERY_ID='%s' and t2.PART_MODEL_ID='%s' and t2.EQUIP_MODEL_ID='%s'",szIqsid,strPartsid.c_str(),strEquipid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		while(psql->NextRow())
		{//{taskId:””,stepId:””,apply:””,check:””}
			READMYSQL_STR(WORKFLOW_TASK_ID, szTaskid);
			READMYSQL_STR(WORKFLOW_STEP_ID, szStepid);
			READMYSQL_INT(IQCOUNT, iApponum,0);
			READMYSQL_INT(QUANTITY, iChecknum,0);
			out<<",";
			out<<"{taskId:\""<<szTaskid<<"\",stepId:\""<<szStepid<<"\",apply:"<<iApponum<<",check:"<<iChecknum<<"}";
		}
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID: 0x61    SID: 0x1d
//{ seq: "ssssssss001",sid:" "，type: 0}
//{seq:"ssssssss001",eid:0,data:[{eqid:"",pid:””,id:"",snc:"",sne:"",st:"",pnu:"",sto: ,su:"",slc:"",ls: ,hs: ,si: ,en:" ",et:" ",es:" ",em:" "},{}......]}
int eMOPEquipSvc::GetWarndata(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetWarndata]bad format:", jsonString, 1);
	string strShipId=root.getv("sid", "");
	string strSeq= root.getv("seq", "");
	int iType=root.getv("type", -1);
	/*
	eqid:备件所属设备的ID
	pid：设备属性ID
	id:备件ID
	snc:中文名称
	sne:英文名称
	st:备件编号/型号
	pnu:图号
	sto:库存量
	su:单位
	slc:库位代码
	ls:低储值
	hs:高储值
	si:是否是重大备件 ,0 不是，1是
	en:所属设备名称
	et:设备型号
	es:设备系列号
	em:设备制造商
	*/
	out<<"{eid:0,seq:\""<<strSeq.c_str()<<"\",data:[";
	char sql[1024] = "";
	MySql* psql = CREATE_MYSQL;
	char eqid[100] = "";
	char pid[100] = "";
	char id[100] = "";
	char snc[100] = "";
	char sne[128]="";
	char st[128]="";
	char pnu[128]="";
	//	int iSto=0;
	char su[128]="";
	char slc_1[16]="";
	char slc_2[16]="";
	char slc_3[16]="";
	char slc_4[16]="";
	char slc_5[16]="";
	//char sln_1[16]="";
	//char sln_2[16]="";
	//char sln_3[16]="";
	//char sln_4[16]="";
	//char sln_5[16]="";
	//	char ls[128]="";
	//	char si[128]="";
	char en[128]="";
	char et[128]="";
	char es[128]="";
	char em[128]="";
	int iHighl=0;//高储值
	int iLowl=0;//低储值
	int iStock=0;//库存量
	int iIsi=-1;
	string strSlc;
	string strSln;

	//t4.MANU_CODE,
	sprintf (sql, \
		"SELECT t1.PART_CAT_ID,t1.INSTOCK,t1.IS_IMPORTANT ,t2.PART_MODEL_ID,t2.NAME_CN,t2.NAME_EN,t2.MODEL,t2.CHART_NO,t2.UNIT,t2.HIGH_REQUIRED,t2.LOW_REQUIRED,\
		t3.*,t4.NAME AS EQNAME,t4.EQUIP_MODEL_ID as PID,t5.MODE as EQMODE,t5.COMPANY_NAME FROM \
		blm_emop_etl.t50_emop_ship_parts t1 LEFT JOIN \
		blm_emop_etl.t50_emop_parts_models t2 ON t1.PART_MODEL_ID=t2.PART_MODEL_ID LEFT JOIN \
		blm_emop_etl.t50_emop_ship_inventory_location t3 ON t1.LOCATION_ID=t3.LOCATION_ID LEFT JOIN \
		blm_emop_etl.t50_emop_part_ship_categories t4 ON t1.shipid=t4.shipid and t1.PART_CAT_ID=t4.PART_CAT_ID LEFT JOIN \
		blm_emop_etl.t50_emop_equip_model t5 ON t4.EQUIP_MODEL_ID=t5.EQUIP_MODEL_ID \
		WHERE t1.SHIPID = '%s'",\
		strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int cnt = 0;
	string strSnc,strSne;
	while (psql->NextRow())
	{
		READMYSQL_STR(PART_CAT_ID, eqid);
		READMYSQL_INT(INSTOCK, iStock,0);// 
		READMYSQL_INT(IS_IMPORTANT, iIsi, -1);//t2 si:是否是重大备件 ,0 不是，1是
		READMYSQL_STR(PART_MODEL_ID, id);//备件ID
		READMYSQL_STR(NAME_CN, snc);//
		READMYSQL_STR(NAME_EN, sne);
		READMYSQL_STR(MODEL, st);
		READMYSQL_STR(CHART_NO, pnu);
		READMYSQL_STR(UNIT, su);//
		READMYSQL_INT(LOW_REQUIRED, iLowl,0);//
		READMYSQL_INT(HIGH_REQUIRED, iHighl,0);//		
		//		READMYSQL_STR(LOCATION_ID, remark);//t2,需要t3  T50_EMOP_SHIP_INVENTORY_LOCATION				
		//
		READMYSQL_STR(EQNAME, en);// 设备表数据,t4
		//READMYSQL_STR(MANU_CODE, es);//
		READMYSQL_STR(EQMODE, et);//
		READMYSQL_STR(COMPANY_NAME, em);//
		READMYSQL_STR(PID, pid);//
		// 库位代码 
		READMYSQL_STR(DECK, slc_1);
		READMYSQL_STR(ROOM, slc_2);
		READMYSQL_STR(SHELF, slc_3);
		READMYSQL_STR(LAYER, slc_4);
		READMYSQL_STR(BOX, slc_5);
		strSlc+=slc_1;strSlc+=slc_2;strSlc+=slc_3;strSlc+=slc_4;strSlc+=slc_5;
		// 库位名称 
		//READMYSQL_STR(DECK_NAME, sln_1);
		//READMYSQL_STR(ROOM_NAME, sln_2);
		//READMYSQL_STR(SHELF_NAME, sln_3);
		//READMYSQL_STR(LAYER_NAME, sln_4);
		//READMYSQL_STR(BOX_NAME, sln_5);
		//strSln=sln_1;strSln+=sln_2;strSln+=sln_3;strSln+=sln_4;strSln+=sln_5;
		strSnc=snc;
		strSne=sne;
		StrReplace(strSnc, "\"", "\\\"");
		StrReplace(strSne, "\"", "\\\"");
		if(iType==0)
		{
			if(iStock>iLowl)
				continue;
		}
		else if(iType==1)
		{
			if(iStock<iHighl)
				continue;
		}
		else
			continue;
		//{eqid:"",pid:””,id:"",snc:"",sne:"",st:"",pnu:"",sto: ,su:"",slc:"",ls: ,hs: ,si: ,en:" ",et:" ",es:" ",em:" "}
		if (cnt++)
			out << ",";
		out << "{eqid:\"" << eqid<< "\",pid:\"" << pid<< "\",id:\"" << id << "\",snc:\"" << strSnc.c_str() << "\",sne:\"" << strSne.c_str() <<"\",st:\"" << st\
			<< "\",pnu:\""<< pnu<<"\",sto:"<< iStock << ",su:\"" << su << "\",slc:\"" << strSlc.c_str() << "\",ls:" << iLowl \
			<< ",hs:" << iHighl<< ",si:" << iIsi  << ",en:\"" << en << "\",et:\"" << et<< "\",es:\"" << es << "\",em:\"" << em  << "\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}



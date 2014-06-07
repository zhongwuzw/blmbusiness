#include "eMOPRepairSvc.h"
#include "blmcom_head.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"



IMPLEMENT_SERVICE_MAP(eMOPRepairSvc)

eMOPRepairSvc::eMOPRepairSvc()
{

}

eMOPRepairSvc::~eMOPRepairSvc()
{

}

bool eMOPRepairSvc::Start()
{
	if(!g_MessageService::instance()->RegisterCmd(MID_EMOP_REPAIR, this))
		return false;

	SERVICE_MAP(0x01,eMOPRepairSvc,GetAllRepairCat); 
	SERVICE_MAP(0x02,eMOPRepairSvc,GetCatItems); 
	SERVICE_MAP(0x03,eMOPRepairSvc,GetItemDetail); 
	SERVICE_MAP(0x04,eMOPRepairSvc,NewOrUpdateRepairItem); 
	SERVICE_MAP(0x05,eMOPRepairSvc,GetAllShipWorkCard); 
	SERVICE_MAP(0x06,eMOPRepairSvc,NewOrDelItemShips); 
	SERVICE_MAP(0x07,eMOPRepairSvc,NewOrDelItemEquips);
	SERVICE_MAP(0x08,eMOPRepairSvc,NewOrDelWorkCard);
	SERVICE_MAP(0x09,eMOPRepairSvc,DeleteRepairItem);

	SERVICE_MAP(0x11,eMOPRepairSvc,GetRepairApplyList);
	SERVICE_MAP(0x12,eMOPRepairSvc,GetRepairApplyDetail);
	SERVICE_MAP(0x13,eMOPRepairSvc,AddOrUpdateRepairApply);
	SERVICE_MAP(0x14,eMOPRepairSvc,GetShipRepairItem);
	SERVICE_MAP(0x15,eMOPRepairSvc,UpdateRepairApplyStatus);
	SERVICE_MAP(0x16,eMOPRepairSvc,DeleteRepairApply);
 
	SERVICE_MAP(0x21,eMOPRepairSvc,GetEngineering); 
	SERVICE_MAP(0x22,eMOPRepairSvc,GetEngineeringDetail); 
	SERVICE_MAP(0x23,eMOPRepairSvc,NewOrUpdateEngineering); 
	SERVICE_MAP(0x24,eMOPRepairSvc,GetNotStartEngineering); 
	SERVICE_MAP(0x25,eMOPRepairSvc,FinishRepairApply); 
	SERVICE_MAP(0x26,eMOPRepairSvc,DeleteEngineering);  

	SERVICE_MAP(0x31,eMOPRepairSvc,GetFactoryRepair); 
	SERVICE_MAP(0x32,eMOPRepairSvc,GetShipEngineeringNotBegin); 
	SERVICE_MAP(0x33,eMOPRepairSvc,GetShipSurvey); 
	SERVICE_MAP(0x34,eMOPRepairSvc,AddOrUpdateFactoryRepair); 
	SERVICE_MAP(0x35,eMOPRepairSvc,DeleteFactoryRepair); 
	SERVICE_MAP(0x36,eMOPRepairSvc,AddOrDelEnginerringToFactory);

	DEBUG_LOG("[eMOPRepairSvc::Start] OK......................................");
	return true;
}
//{seq:"",uid:"",itemid:""}
int eMOPRepairSvc::DeleteRepairItem(const char* pUid, const char* jsonString, std::stringstream& out)
{ 
	JSON_PARSE_RETURN("[eMOPRepairSvc::DeleteRepairItem]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", ""); 
	string strItemId= root.getv("itemid", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[500] = ""; 
	sprintf(sql,"SELECT * FROM  blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS t1,blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS t2 \
				WHERE t1.REPAIR_ITEM_CODE='%s' OR t2.REPAIR_ITEM_CODE='%s'",strItemId.c_str(),strItemId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);   

	if(psql->NextRow()){ 
		out<< "{seq:\""<<strSeq.c_str()<<"\",eid:0,etype:1}";
	}else{
		sprintf(sql,"DELETE  FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS WHERE REPAIR_ITEM_CODE='%s'",strItemId.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
		out<< "{seq:\""<<strSeq.c_str()<<"\",eid:0,etype:0}";
	}	

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//请求全部修理类别
//{seq:"",uid:""}
//{seq:"",info:[{cateid:"",caten:""} {},{}]}
int eMOPRepairSvc::GetAllRepairCat(const char* pUid, const char* jsonString, std::stringstream& out){

	JSON_PARSE_RETURN("[eMOPRepairSvc::GetAllRepairItem]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "SELECT  t1.SEQ,t1.REPAIR_ITEM_CAT_CODE,t1.NAME_CN,t2.ITEM_COUNT FROM \
					 blm_emop_etl.T50_EMOP_REPAIR_ITEMS_CAT t1\
					 LEFT JOIN (SELECT COUNT(*) AS ITEM_COUNT,REPAIR_ITEM_CAT_CODE AS REPAIR_ITEM_CAT_CODE2\
					 FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS \
					 GROUP BY REPAIR_ITEM_CAT_CODE  ) t2 ON t1.REPAIR_ITEM_CAT_CODE=t2.REPAIR_ITEM_CAT_CODE2\
					 ORDER BY SEQ ASC";  
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	string catName="";
	string catCode="";
	int childNum=0;

	out<< "{seq:\""<<strSeq.c_str()<<"\",info:[";
	int idx=0;
	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;

		READMYSQL_STRING(NAME_CN,catName);
		READMYSQL_STRING(REPAIR_ITEM_CAT_CODE,catCode);
		READMYSQL_INT(ITEM_COUNT,childNum,0);
		out<<"{cateid:\""<<catCode<<"\",caten:\""<<catName<<"\",nchild:"<<childNum<<"}";
	}

	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//根据类别请求修理项的所有数据
//{seq:"",info:[{cateid:"",caten:"",item:[{itemid:"",itemn:""},{}]},{},{}]};
int eMOPRepairSvc::GetCatItems(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetCatItems]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", ""); 
	string strCateId= root.getv("cateid", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[500] = "";
	stringstream whereSql;
	whereSql<<"";
	if(!strCateId.empty()){
		Tokens catIds=StrSplit(strCateId,"|");
		whereSql<<"WHERE REPAIR_ITEM_CAT_CODE IN (";
		int size=catIds.size();
		for(int i=0;i<size;i++){
			if(i>0)
				whereSql<<",";
			whereSql<<"'"<<catIds[i]<<"'";
		}
		whereSql<<")";
	}

	sprintf(sql,"SELECT  REPAIR_ITEM_CAT_CODE,NAME_CN AS CAT_NAME_CN \
				FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_CAT \
				%s \
				ORDER BY SEQ ASC",whereSql.str().c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	string catName="";
	string catCode="";

	vector<RepairCategory> catsVce;
	while(psql->NextRow()){ 
		RepairCategory cat; 
		READMYSQL_STRING(CAT_NAME_CN,cat.m_name);
		READMYSQL_STRING(REPAIR_ITEM_CAT_CODE,cat.m_code);  
		catsVce.push_back(cat); 
	}

	out<< "{seq:\""<<strSeq.c_str()<<"\",info:["; 
	string itemCode="";
	string itemName="";
	string itemContent="";
	int size=catsVce.size();
	for(int i=0;i<size;i++)
	{
		if(i>0)
			out<<",";

		vector<RepairCategory>::iterator it=catsVce.begin()+i;
		out<<"{cateid:\""<<it->m_code<<"\",caten:\""<<it->m_name<<"\",item:["; 
		sprintf(sql,"SELECT  REPAIR_ITEM_CODE,NAME_CN AS ITEM_NAME_CN,CONTENTS FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS WHERE REPAIR_ITEM_CAT_CODE='%s'",it->m_code.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		int idx=0;
		while(psql->NextRow()){ 
			if(idx>0)
				out<<",";
			idx++;
			READMYSQL_STRING(ITEM_NAME_CN,itemName);
			READMYSQL_STRING(REPAIR_ITEM_CODE,itemCode); 
			READMYSQL_STRING_JSON(CONTENTS,itemContent); 
			out<<"{itemid:\""<<itemCode<<"\",itemn:\""<<itemName<<"\",content:\""<<itemContent<<"\"}"; 
		}
		out<<"]}";
	} 

	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0; 
}
//增加或者删除设备
//{seq:"",itemid:"",shipid:"",type:1"",cwbts:"e01|e02"}
int  eMOPRepairSvc::NewOrDelItemEquips(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::NewOrDelItemEquips]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");  
	string strItemId = root.getv("itemid", "");  
	string strShipId = root.getv("shipid", ""); 
	string strCwbts = root.getv("cwbts", ""); 
	int type = root.getv("type",0);

	Tokens cwbtVec = StrSplit(strCwbts, "|");
	MySql* psql = CREATE_MYSQL;
	char sql[500] = "";    
	for(Tokens::iterator it=cwbtVec.begin();it!=cwbtVec.end();it++){
		if(type==1){//new 
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS_WORCARD(REPAIR_ITEM_CODE,SHIP_ID,EQUIP_CWBT) \
						VALUES('%s','%s','%s');",strItemId.c_str(),strShipId.c_str(),it->c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}else if(type==2)//delete 
		{
			sprintf(sql,"DELETE  FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS_WORCARD WHERE REPAIR_ITEM_CODE='%s' AND SHIP_ID='%s' AND EQUIP_CWBT='%s'",
				strItemId.c_str(),strShipId.c_str(),it->c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
	}

	sprintf(sql,"SELECT NAME_CN FROM blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD WHERE CWBT='%s'",
		strCwbts.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);

	string name="";
	if(psql->NextRow())
		READMYSQL_STRING(NAME_CN,name);

	out<<"{seq:\""<<strSeq<<"\",eid:0,na:\""<<name<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//增加或者删除工作卡
//{seq:"",itemid:"",shipid:"",cwbt:"",type:1"",wcsids:"w01|w02"}
int  eMOPRepairSvc::NewOrDelWorkCard(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::NewOrDelWorkCard]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");  
	string strItemId = root.getv("itemid", "");  
	string strShipId = root.getv("shipid", ""); 
	string strCwbt = root.getv("cwbt", ""); 
	string strWcsids = root.getv("wcsids", ""); 
	int type = root.getv("type",0);

	Tokens wordIdVec = StrSplit(strWcsids, "|");
	MySql* psql = CREATE_MYSQL;
	char sql[500] = "";    
	for(Tokens::iterator it=wordIdVec.begin();it!=wordIdVec.end();it++){
		if(type==1){//new 
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS_WORCARD(REPAIR_ITEM_CODE,SHIP_ID,EQUIP_CWBT,WORK_CARD_ID) \
						VALUES('%s','%s','%s','%s')",strItemId.c_str(),strShipId.c_str(),strCwbt.c_str(),it->c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}else if(type==2)//delete 
		{
			sprintf(sql,"DELETE  FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS_WORCARD WHERE REPAIR_ITEM_CODE='%s' AND SHIP_ID='%s' AND EQUIP_CWBT='%s' AND WORK_CARD_ID='%s'",
				strItemId.c_str(),strShipId.c_str(),strCwbt.c_str(),it->c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
	}
	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//{seq:"",applyid:""}
int eMOPRepairSvc::DeleteRepairApply(const char* pUid, const char* jsonString, std::stringstream& out)
{	
	JSON_PARSE_RETURN("[eMOPRepairSvc::DeleteRepairApply]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");  
	string strApplyId = root.getv("applyid", "");  

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_REPAIR_APPLIES WHERE REPAIR_APPLY_ID='%s'",strApplyId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_PEPAIR_APPLY_STATUS WHERE REPAIR_APPLY_ID='%s'",strApplyId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS WHERE REPAIR_APPLY_ID='%s'",strApplyId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<<"{eid:0,seq:\""<<strSeq<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}
//获取船舶修理项列表
//{seq:"",shipid:""}
//{seq:"",info:[{itemid:"",itemn:"" },{ itemid:"",itemn:"", }]}
int eMOPRepairSvc::GetShipRepairItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetShipRepairItem]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");  
	string strShipId = root.getv("shipid", "");  

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	sprintf(sql,"SELECT t1.REPAIR_ITEM_CODE,t2.NAME_CN FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS t1 \
				LEFT JOIN blm_emop_etl.T50_EMOP_REPAIR_ITEMS t2 ON t1.REPAIR_ITEM_CODE=t2.REPAIR_ITEM_CODE \
				WHERE t1.SHIP_ID='%s'",strShipId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);

	string code="";
	string name="";
	out<<"{seq:\""<<strSeq<<"\",info:[";
	int idx=0;
	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(REPAIR_ITEM_CODE,code);
		READMYSQL_STRING(NAME_CN,name);
		out<<"{itemid:\""<<code<<"\",itemn:\""<<name<<"\"}";
	}
	out<<"]}"; 
	RELEASE_MYSQL_RETURN(psql, 0);
}

//增加或者删除船舶
//{seq:"",itemid:"",type:,shipids:"s01|s02"} 
int eMOPRepairSvc::NewOrDelItemShips(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::NewOrDelItemShips]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");  
	string strItemId = root.getv("itemid", "");  
	string strShipId = root.getv("shipids", ""); 
	int type = root.getv("type",0);

	Tokens shipVec = StrSplit(strShipId, "|");
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	for(Tokens::iterator it=shipVec.begin();it!=shipVec.end();it++){
		if(type==1){//new ship
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS(REPAIR_ITEM_CODE,SHIP_ID) VALUES('%s','%s');",strItemId.c_str(),it->c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}else if(type==2)//delete ship
		{
			sprintf(sql,"DELETE  FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS WHERE REPAIR_ITEM_CODE='%s' AND SHIP_ID='%s'; ",
				strItemId.c_str(),it->c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

			sprintf(sql,"DELETE  FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS_WORCARD WHERE REPAIR_ITEM_CODE='%s' AND SHIP_ID='%s'",
				strItemId.c_str(),it->c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
	}
	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",shipid:"",cwbt:""}
//{seq:"",wcs:[{ id:"",level:"",owner:""},{}]}
int eMOPRepairSvc::GetAllShipWorkCard(const char* pUid, const char* jsonString, std::stringstream& out)
{ 
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetAllShipWorkCard]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", "");  
	string strShipId= root.getv("shipid", "");  
	string strCwbt=root.getv("cwbt", "");  

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   

	sprintf(sql,"SELECT t1.WORK_CARD_ID,t1.LEVEL,t1.OWNER FROM blm_emop_etl.T50_EMOP_WORK_CARD  t1 \
				LEFT JOIN blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD t2 ON t1.EQUIP_ID=t2. EQUIP_ID \
				WHERE t2.SHIPID='%s' AND t2.CWBT='%s' ORDER BY  t1.OWNER  ASC ",strShipId.c_str(),strCwbt.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
	out<<"{seq:\""<<strSeq<<"\",wcs:[";
	int idx=0;
	char wid[20]="";
	char level[10]="";
	char owner[20]="";
	while(psql->NextRow())
	{ 
		READMYSQL_STR(WORK_CARD_ID,wid);
		READMYSQL_STR(LEVEL,level);
		READMYSQL_STR(OWNER,owner);
		if(idx>0)
			out<<",";
		idx++;
		out<<"{id:\""<<wid<<"\",level:\""<<level<<"\",owner:\""<<owner<<"\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",type:0,facid:"",proid:"p01|p02"}
int eMOPRepairSvc::AddOrDelEnginerringToFactory(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::DelRepairItem]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", "");    
	string strFacid= root.getv("facid", "");
	string strProid= root.getv("proid", "");  
	int type= root.getv("type",0);
	if(strProid.empty()){
		out<<"{eid:0,seq:\""<<strSeq<<"\",type:"<<type<<"}";
		return 0;
	}
	MySql* psql = CREATE_MYSQL;
	char sql[500] = "";   
	Tokens pidVec = StrSplit(strProid,"|");
	std::stringstream proidsStream;
	proidsStream<<"(";
	for(Tokens::iterator it=pidVec.begin();it!=pidVec.end();it++){
		if(it!=pidVec.begin())
			proidsStream<<",";
		proidsStream<<"'"<<it->c_str()<<"'";
	}
	proidsStream<<")";
	if(type==0){
		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING SET DOCKED_REPAIR_ID='%s' WHERE REPAIR_ENGINEERING_ID IN %s;",strFacid.c_str(),proidsStream.str().c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 	
	}else if(type==1){
		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING SET DOCKED_REPAIR_ID='' WHERE REPAIR_ENGINEERING_ID IN %s;",proidsStream.str().c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 	
	}
	DEBUG_LOG(sql);
	out<<"{eid:0,seq:\""<<strSeq<<"\",type:"<<type<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{seq:"",uid:"",itemid:""}
int eMOPRepairSvc::DelRepairItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::DelRepairItem]bad format:", jsonString, 1);
	string strSeq= root.getv("seq", "");    
	string strItemId= root.getv("itemid", "");  

	MySql* psql = CREATE_MYSQL;
	char sql[500] = "";   

	int etype=0;

	sprintf(sql,"SELECT * FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS WHERE REPAIR_ITEM_CODE='%s'",strItemId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
	if(psql->NextRow()){
		etype=1;
	}else{ 
		sprintf(sql,"delete  FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS WHERE REPAIR_ITEM_CODE='%s'",strItemId.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
	} 

	out<<"{eid:0,etype:"<<etype<<",seq:\""<<strSeq<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}


//{seq:"",uid:"",type:, cateid:"",itemid:"",itemn:"",content:""}
int eMOPRepairSvc::NewOrUpdateRepairItem(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPRepairSvc::NewOrUpdateRepairItem]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", ""); 
	int type=root.getv("type", 0);
	string strCateId= root.getv("cateid", "");  
	string strItemId= root.getv("itemid", ""); 
	string strItemName= root.getv("itemn", ""); 
	string strItemContent= root.getv("content", ""); 
	strItemId = strItemId.substr(0,5);

	MySql* psql = CREATE_MYSQL;
	char sql[2048] = ""; 

	int etype=0;
	if(type==1){//new item
		sprintf(sql,"SELECT * FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS WHERE REPAIR_ITEM_CODE='%s'",strItemId.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
		if(psql->NextRow()){
			etype=1;
		}else{ 
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_REPAIR_ITEMS(REPAIR_ITEM_CAT_CODE,REPAIR_ITEM_CODE,NAME_CN,CONTENTS) \
						VALUES('%s','%s','%s','%s')",strCateId.c_str(),strItemId.c_str(),strItemName.c_str(),strItemContent.c_str());

			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
	}else if(type==2){
		sprintf(sql,"UPDATE  blm_emop_etl.T50_EMOP_REPAIR_ITEMS SET REPAIR_ITEM_CAT_CODE='%s',NAME_CN='%s',CONTENTS='%s' WHERE REPAIR_ITEM_CODE='%s';",
			strCateId.c_str(),strItemName.c_str(),strItemContent.c_str(),strItemId.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
	}

	out<<"{eid:0,etype:"<<etype<<",seq:\""<<strSeq<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",itemid:""}
//{seq:"",itemid:"",itemna:"",content:"",ships:[{id:"",name:"",equips:[{idcwbt:"",na:""wcs:[{id:"",level:"",owner:""},{}]}, {}] },{}};
int eMOPRepairSvc::GetItemDetail(const char* pUid, const char* jsonString, std::stringstream& out){
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetItemDetail]bad format:", jsonString, 1);

	string strSeq= root.getv("seq", ""); 
	string stritemId= root.getv("itemid", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[500] = "";   

	string itemName="";
	string itemContent="";

	//查询修理项
	sprintf(sql,"SELECT REPAIR_ITEM_CODE,NAME_CN,CONTENTS FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS WHERE REPAIR_ITEM_CODE='%s'",stritemId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	if(psql->NextRow()){ 
		READMYSQL_STRING(NAME_CN,itemName);
		READMYSQL_STRING(CONTENTS,itemContent);
	}
	//查询船舶
	sprintf(sql,"SELECT t2.SHIPID,t2.NAME_CN FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS t1 \
				INNER JOIN blm_emop_etl.T41_EMOP_SHIP t2 ON t1.SHIP_ID=t2.SHIPID \
				WHERE t1.REPAIR_ITEM_CODE='%s'",stritemId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	vector<RepairShip> shipVce;
	while(psql->NextRow()){ 
		RepairShip ship; 
		READMYSQL_STRING(SHIPID,ship.m_id);
		READMYSQL_STRING(NAME_CN,ship.m_name_cn);  
		shipVce.push_back(ship); 
	}

	out<< "{seq:\""<<strSeq.c_str()<<"\",itemid:\""<<stritemId<<"\",itemna:\""<<itemName<<"\",content:\""<<itemContent<<"\",ships:[";

	//查询设备卡	
	for(int i=0;i<shipVce.size();i++){
		RepairShip ship = shipVce[i];
		if(i>0)
			out<<",";

		out<<"{id:\""<<ship.m_id<<"\",name:\""<<ship.m_name_cn<<"\",equips:[";
		sprintf(sql,"SELECT t2.EQUIP_ID,t2.CWBT,t2.NAME_CN FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS_WORCARD t1 \
					INNER JOIN blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD t2 ON t1.EQUIP_CWBT=t2.CWBT \
					WHERE t1.SHIP_ID='%s' AND t1.REPAIR_ITEM_CODE='%s' AND t2.SHIPID='%s' ",ship.m_id.c_str(),stritemId.c_str(),ship.m_id.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

		int idx=0;
		string equipId="";
		string equipCWBT="";
		string equipName_cn="";

		while(psql->NextRow()){
			if(idx>0)
				out<<",";

			idx++; 
			READMYSQL_STRING(EQUIP_ID,equipId);
			READMYSQL_STRING(CWBT,equipCWBT);
			READMYSQL_STRING(NAME_CN,equipName_cn);   
			string workCard="";
			getEquipWorkCard(equipId,workCard);
			out<<"{idcwbt:\""<<equipCWBT<<"\",na:\""<<equipName_cn<<"\",wcs:["<<workCard<<"]}";
		}
		out<<"]}";
	}  
	out<<"]}";  
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0; 
}


int eMOPRepairSvc::getRepairEquipWorkCard(std::string &shipId,std::string &itemCode,std::string &cwbt,std::string &cardArrStr)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";     
	sprintf(sql,"SELECT t2.WORK_CARD_ID,t2.LEVEL,t2.OWNER FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS_WORCARD t1 \
				RIGHT JOIN blm_emop_etl.T50_EMOP_WORK_CARD t2 ON t1.WORK_CARD_ID=t2.WORK_CARD_ID \
				WHERE  t1.SHIP_ID='%s' AND t1.REPAIR_ITEM_CODE='%s' AND t1.EQUIP_CWBT='%s'",shipId.c_str(),itemCode.c_str(),cwbt.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	char cardId[100]="";
	char cardLevel[100]="";
	char cardOwner[100]="";

	stringstream out;
	out<<"";
	int idx=0;
	while(psql->NextRow()){ 
		if(idx>0)
			out<<","; 
		idx++;

		READMYSQL_STR(WORK_CARD_ID,cardId);
		READMYSQL_STR(LEVEL,cardLevel);
		READMYSQL_STR(OWNER,cardOwner);
		out<<"{id:\""<<cardId<<"\",evel:\""<<cardLevel<<"\",owner:\""<<cardOwner<<"\"}"; 
	}
	cardArrStr = out.str();

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

int eMOPRepairSvc::getEquipWorkCard(string &equipCardId,string &cardArrStr)
{
	MySql* psql = CREATE_MYSQL;
	char sql[500] = "";     
	sprintf(sql,"SELECT t1.WORK_CARD_ID,t1.LEVEL,t1.OWNER FROM blm_emop_etl.T50_EMOP_WORK_CARD t1 \
				INNER JOIN blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS_WORCARD t2 ON t1.WORK_CARD_ID=t2.WORK_CARD_ID \
				WHERE t1.EQUIP_ID='%s'",equipCardId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	char cardId[100]="";
	char cardLevel[100]="";
	char cardOwner[100]="";

	stringstream out;
	out<<"";
	int idx=0;
	while(psql->NextRow()){ 
		if(idx>0)
			out<<","; 
		idx++;

		READMYSQL_STR(WORK_CARD_ID,cardId);
		READMYSQL_STR(LEVEL,cardLevel);
		READMYSQL_STR(OWNER,cardOwner);
		out<<"{id:\""<<cardId<<"\",evel:\""<<cardLevel<<"\",owner:\""<<cardOwner<<"\"}"; 
	}
	cardArrStr = out.str();

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//{seq:"",uid:"",shipid:"",st:"",tp:""}
//{seq:"",info:[{proid:"",shipid:"",shipna:"",tp:,st:,loc:"",  psday:1000,pfday:1000, asday:1000,afday:1000,comid:"",comna:"", facid:"", facna:"",,],{}]}
int eMOPRepairSvc::GetEngineering(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetEngineering]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strShipId = root.getv("shipid", ""); 
	string strUid = root.getv("uid", ""); 
	string strStatus = root.getv("st", ""); 
	string strType = root.getv("tp", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶id
	std::stringstream shipsSql;
	shipsSql<<"WHERE t1.SHIP_ID IN ("; 
	if(strShipId.empty()){
		sprintf(sql,"SELECT SHIPID FROM  blm_emop_etl.T50_EMOP_REGISTERED_USER_FLEET WHERE USER_ID='%s'",strUid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		int idx=0;
		char shipid[20];
		while(psql->NextRow()){
			if(idx>0)
				shipsSql<<",";
			idx++;
			READMYSQL_STR(SHIPID,shipid);
			shipsSql<<"'"<<shipid<<"'";
		}
	}
	else
	{
		Tokens shipIdVec = StrSplit(strShipId,"|"); 
		int idx=0;
		for(Tokens::iterator it=shipIdVec.begin();it!=shipIdVec.end();it++)
		{
			if(idx>0)
				shipsSql<<",";
			idx++; 
			shipsSql<<"'"<<it->c_str()<<"'";
		}
	} 
	shipsSql<<") ";

	//处理状态strStatus
	std::stringstream statusSql;
	statusSql<<""; 
	if(!strStatus.empty()){ 
		statusSql<<"AND t1.STATUS IN (";
		Tokens statusVec = StrSplit(strStatus,"|"); 
		int idx=0;
		for(Tokens::iterator it=statusVec.begin();it!=statusVec.end();it++)
		{
			if(idx>0)
				statusSql<<",";
			idx++; 
			statusSql<<it->c_str();
		} 
		statusSql<<") ";
	} 

	//处理状态strType
	std::stringstream typeSql;
	typeSql<<""; 
	if(!strType.empty()){ 
		typeSql<<"AND t1.type IN (";
		Tokens typeVec = StrSplit(strType,"|"); 
		int idx=0;
		for(Tokens::iterator it=typeVec.begin();it!=typeVec.end();it++)
		{
			if(idx>0)
				typeSql<<",";
			idx++; 
			typeSql<<it->c_str();
		} 
		typeSql<<") ";
	} 

	sprintf(sql,"SELECT t1.REPAIR_ENGINEERING_ID,t1.TYPE,t1.STATUS, \
				UNIX_TIMESTAMP(t1.SCHEDULED_START_DATE) AS SCHEDULED_START_DATE,UNIX_TIMESTAMP(t1.SCHEDULED_END_DATE) AS SCHEDULED_END_DATE,UNIX_TIMESTAMP(t1.ACTUAL_START_DATE) AS ACTUAL_START_DATE,UNIX_TIMESTAMP(t1.ACTUAL_END_DATE) AS ACTUAL_END_DATE,t1.REPAIR_LOCATION,\
				t1.SHIP_ID,t_ship.NAME_CN AS SHIPNAME,\
				t2.DOCKED_REPAIR_ID,t1.REPAIR_COMPANY_ID,t3.NAME AS COMPANYNAME \
				FROM blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING t1\
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP t_ship ON t_ship.SHIPID=t1.SHIP_ID \
				LEFT JOIN blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCHEDULES t2 ON t2.DOCKED_REPAIR_ID=t1.DOCKED_REPAIR_ID \
				LEFT JOIN blm_emop_etl.T41_COMPANY t3 ON t3.COMPANY_KEY=t1.REPAIR_COMPANY_ID \
				%s %s %s",shipsSql.str().c_str(),statusSql.str().c_str(),typeSql.str().c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	string engId=""; //工程单id 
	string itemContent="";
	string shipId="";
	string shipName="";
	int type=0;
	int status=0;
	int pStartDate=0;
	int pEndDate=0;
	int aStartDate=0;
	int aEndDate=0;
	string location="";
	string dockedId="";//场修计划id
	string companyId="";
	string companyName="";
	out<<"{seq:\""<<strSeq<<"\",info:["; 
	int idx=0;
	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(REPAIR_ENGINEERING_ID,engId);
		READMYSQL_INT(TYPE,type,0);
		READMYSQL_INT(STATUS,status,0);
		READMYSQL_INT(SCHEDULED_START_DATE,pStartDate,0);
		READMYSQL_INT(SCHEDULED_END_DATE,pEndDate,0);
		READMYSQL_INT(ACTUAL_START_DATE,aStartDate,0);
		READMYSQL_INT(ACTUAL_END_DATE,aEndDate,0);
		READMYSQL_STRING(SHIP_ID,shipId);
		READMYSQL_STRING(SHIPNAME,shipName);
		READMYSQL_STRING(REPAIR_LOCATION,location);
		READMYSQL_STRING(DOCKED_REPAIR_ID,dockedId);
		READMYSQL_STRING(COMPANY_KEY,companyId);
		READMYSQL_STRING(COMPANYNAME,companyName);

		string repairContent="";
		getFactoryRepairContent(dockedId,repairContent);
		out<<"{proid:\""<<engId<<"\",shipid:\""<<shipId<<"\",shipna:\""<<shipName<<"\",tp:"<<type<<",st:"<<status<<",loc:\"";
		out<<location<<"\",psday:"<<pStartDate<<",pfday:"<<pEndDate<<",asday:"<<aStartDate<<",afday:"<<aEndDate;
		out<<",comid:\""<<companyId<<"\",comna:\""<<companyName<<"\",facid:\""<<dockedId<<"\", facna:\""<<repairContent<<"\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//请求某船舶的未派工程单的且状态为未开始的修理单列表
//{seq:"",shipid:"",tp:0}
//{eid:0,info:[applyid:"",itemid:"",itemna:""]}
int eMOPRepairSvc::GetNotStartEngineering(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetNotStartEngineering]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", ""); 
	string strShipId = root.getv("shipid", ""); 
	int type = root.getv("tp", 0); 

	MySql* psql = CREATE_MYSQL; 
	char sql[1024] = "";     
	sprintf(sql,"SELECT t1.REPAIR_APPLY_ID,t2.REPAIR_ITEM_CODE,t3.NAME_CN, \
				UNIX_TIMESTAMP(t1.APPLY_DATE) AS APPLY_DATE,UNIX_TIMESTAMP(t1.SCHEDULED_START_DATE) AS SCHEDULED_START_DATE,UNIX_TIMESTAMP(t1.SCHEDULED_END_DATE) AS SCHEDULED_END_DATE,UNIX_TIMESTAMP(t1.ACTUAL_START_DATE) AS ACTUAL_START_DATE,UNIX_TIMESTAMP(t1.ACTUAL_END_DATE) AS ACTUAL_END_DATE\
				FROM blm_emop_etl.T50_EMOP_REPAIR_APPLIES t1\
				LEFT JOIN blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS t2 ON t1.REPAIR_APPLY_ID = t2.REPAIR_APPLY_ID\
				LEFT JOIN blm_emop_etl.T50_EMOP_REPAIR_ITEMS t3 ON t2.REPAIR_ITEM_CODE=t3.REPAIR_ITEM_CODE\
				WHERE  SHIP_ID='%s' AND TYPE=%d AND (REPAIR_ENGINEERING_ID IS NULL OR REPAIR_ENGINEERING_ID='') ",strShipId.c_str(),type);
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	out<<"{eid:0,seq:\""<<strSeq<<"\",info:[";
	int idx=0;
	string appid="";
	string itemId="";
	string itemName="";
	int appDate=0;
	int sStartDate=0;
	int sEndDate=0;
	int aStartDate=0;
	int aEndDate=0;

	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(REPAIR_APPLY_ID,appid);
		READMYSQL_STRING(REPAIR_ITEM_CODE,itemId);
		READMYSQL_STRING(NAME_CN,itemName);
		READMYSQL_INT(SCHEDULED_START_DATE,sStartDate,0);
		READMYSQL_INT(SCHEDULED_END_DATE,sEndDate,0);
		READMYSQL_INT(ACTUAL_START_DATE,aStartDate,0);
		READMYSQL_INT(ACTUAL_END_DATE,aEndDate,0);
		READMYSQL_INT(APPLY_DATE,appDate,0);
		out<<"{applyid:\""<<appid<<"\",itemid:\""<<itemId<<"\",itemna:\""<<itemName;
		out<<"\",adate:"<<appDate<<",bdate:"<<sStartDate<<",fdate:"<<sEndDate<<",rbdate:"<<aStartDate<<",rfdate:"<<aEndDate<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//{seq:"",uid:"",applyid:"",proc:"",link:"",st:1,remark:""}
int eMOPRepairSvc::UpdateRepairApplyStatus(const char* pUid, const char* jsonString, std::stringstream& out){
	JSON_PARSE_RETURN("[eMOPRepairSvc::UpdateRepairApplyStatus]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", ""); 
	string strUid = root.getv("uid", "");  
	string strApplyId= root.getv("applyid", ""); 
	string strTask = root.getv("proc", ""); 
	string strStep = root.getv("link", ""); 
	string strRemark = root.getv("remark", ""); 
	int status = root.getv("st", 0); 

	string strOpDay=CurLocalTime();

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";  
	sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_PEPAIR_APPLY_STATUS(REPAIR_APPLY_ID,OP_DATE,REMARK,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID,STATUS) \
				VALUES('%s','%s','%s','%s','%s','%s',%d)",strApplyId.c_str(),strOpDay.c_str(),strRemark.c_str(),strUid.c_str(),strTask.c_str(),strStep.c_str(),status);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);   
	out<<"{eid:0,seq:\""<<strSeq<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//增加或者修改工程单
//{seq:"",uid:"",dis:"",proid:"",shipid:"",type:0,loc:"",psday:,pfday:,asday:1000,afday:1000,comid:"",comna:"",alert:"",illu:"1|2|3|4",applyid:"a01|a02|a03" }
int eMOPRepairSvc::NewOrUpdateEngineering(const char* pUid, const char* jsonString, std::stringstream& out){

	JSON_PARSE_RETURN("[eMOPRepairSvc::NewOrUpdateEngineering]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", ""); 
	string strUid = root.getv("uid", "");  
	string strDis = root.getv("dis", "");  //类型 
	string strProid = root.getv("proid", "");  
	string strShipid = root.getv("shipid", "");  
	int type = root.getv("type", 0);  
	//int state = root.getv("state", 0);  
	string strLocation = root.getv("loc", "");  
	int psday = root.getv("psday", 0);  
	int pfday = root.getv("pfday", 0);  
	int asday = root.getv("asday", 0);  
	int afday = root.getv("afday", 0);  
	string strCompanyId = root.getv("comid", "");  
	string strCcompanyName = root.getv("comna", "");
	string strAlert = root.getv("alert", "");
	string strOtherDrections = root.getv("illu", "");
	string strApplyids = root.getv("applyid", "");

	string strPsday=TimeToTimestampStr(psday);
	string strPfday=TimeToTimestampStr(pfday);
	string strAsday=TimeToTimestampStr(asday);
	string strAfday=TimeToTimestampStr(afday);
	string strApplyDay=CurLocalTime();

	//实际开始日期和实际结束日期
	//实际开始日期 非0 实际结束日期为 0 修改工程单状态为1（进行中）
	//实际结束日期非0，修改工程单状态为2 （已完成）
	//实际开始日期和实际结束日期都为0，修改状态 为 0（未开始）
	int enstatus=0;
	if(asday>0 && afday<=0)
		enstatus=1;
	if(asday>0 && afday>0)
		enstatus=2;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	checkNewCompany(strCompanyId,strCcompanyName);

	if(strDis=="0"){
		//插入工程单
		//获取新工程单id
		sprintf(sql,"SELECT MAX(ENGINEERING_NUM) AS NEW_ENID FROM (SELECT SUBSTRING(REPAIR_ENGINEERING_ID,2)+0 AS ENGINEERING_NUM FROM blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING) t");
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3); 
		int newEnNum=0;
		if(psql->NextRow())
		{
			READMYSQL_INT(NEW_ENID,newEnNum,0);
		}
		newEnNum++;

		char newEnid[10]="";
		sprintf(newEnid, "R%09d",newEnNum);
		strProid = newEnid; 
		//插入工程单
		sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING(REPAIR_ENGINEERING_ID,SHIP_ID,REPAIR_COMPANY_ID,TYPE,STATUS,SCHEDULED_START_DATE,SCHEDULED_END_DATE,REPAIR_LOCATION,ENG_DESC,APPLY_DATE) \
					VALUES('%s','%s','%s',%d,%d,'%s','%s','%s','%s','%s')",strProid.c_str(),strShipid.c_str(),strCompanyId.c_str(),type,enstatus,strPsday.c_str(),strPfday.c_str(),strLocation.c_str(),strAlert.c_str(),strApplyDay.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	}else if(strDis=="1"){  
		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING \
					SET SHIP_ID='%s',REPAIR_COMPANY_ID='%s',TYPE=%d,STATUS=%d,SCHEDULED_START_DATE='%s',SCHEDULED_END_DATE='%s',ACTUAL_START_DATE='%s',ACTUAL_END_DATE='%s',REPAIR_LOCATION='%s',ENG_DESC='%s' \
					WHERE REPAIR_ENGINEERING_ID='%s'",
					strShipid.c_str(),strCompanyId.c_str(),type,enstatus,strPsday.c_str(),strPfday.c_str(),strAsday.c_str(),strAfday.c_str(),strLocation.c_str(),strAlert.c_str(),
					strProid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);   
	} 
	//其他说明处理
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_REPAIR_ENG_DIRECTIONS WHERE REPAIR_ENGINEERING_ID='%s'",strProid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);   
	Tokens otherDrectionsVec = StrSplit(strOtherDrections,"|");
	for(Tokens::iterator it=otherDrectionsVec.begin();it!=otherDrectionsVec.end();it++)
	{
		sprintf(sql,"INSERT INTO  blm_emop_etl.T50_EMOP_REPAIR_ENG_DIRECTIONS(REPAIR_ENGINEERING_ID,ENG_DIRCTIONS_ID) VALUES('%s','%s');",strProid.c_str(),it->c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);   
	}

	//修理单处理
	sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_REPAIR_APPLIES SET REPAIR_ENGINEERING_ID='' WHERE REPAIR_ENGINEERING_ID='%s'",strProid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	Tokens applyidVec = StrSplit(strApplyids,"|"); 
	for(Tokens::iterator it=applyidVec.begin();it!=applyidVec.end();it++)
	{
		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_REPAIR_APPLIES SET REPAIR_ENGINEERING_ID='%s' WHERE REPAIR_APPLY_ID='%s';",strProid.c_str(),it->c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);   
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0,proid:\""<<strProid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}


//{seq:"",proid:""}
//{seq:"",proid:"",shipid:"",shipna:"",type:0,state:0,loc:"",psday:1000,pfday:1000,asday:1000,afday:1000, pdays:1.1,adays:2.1,comid:"",comna:"", facid:"", facna:"",alert:"",illu:"1|2|3|4",applyinfo:[{applyid:"",itemid:"",itemn:"",,adate:,bdate:,fdate:,st:}] }
int eMOPRepairSvc::GetEngineeringDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetEngineeringDetail]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strProid = root.getv("proid", "");  

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   

	sprintf(sql,"SELECT t1.REPAIR_ENGINEERING_ID,t1.TYPE,t1.STATUS,t1.REPAIR_LOCATION,\
				UNIX_TIMESTAMP(t1.SCHEDULED_START_DATE) AS SCHEDULED_START_DATE,UNIX_TIMESTAMP(t1.SCHEDULED_END_DATE) AS SCHEDULED_END_DATE,UNIX_TIMESTAMP(t1.ACTUAL_START_DATE) AS ACTUAL_START_DATE,UNIX_TIMESTAMP(t1.ACTUAL_END_DATE) AS ACTUAL_END_DATE,\
				t1.SCHEDULED_PERIOD,t1.ACTUAL_PERIOD,t1.ENG_DESC,t1.SHIP_ID,t_ship.NAME_CN AS SHIP_NAME,\
				t2.DOCKED_REPAIR_ID,t1.REPAIR_COMPANY_ID,t3.NAME AS COMPANY_NAME \
				FROM blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING t1\
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP t_ship ON t_ship.SHIPID=t1.SHIP_ID\
				LEFT JOIN blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCHEDULES t2 ON t2.DOCKED_REPAIR_ID=t1.DOCKED_REPAIR_ID\
				LEFT JOIN blm_emop_etl.T41_COMPANY t3 ON t3.COMPANY_KEY=t1.REPAIR_COMPANY_ID \
				WHERE t1.REPAIR_ENGINEERING_ID='%s'",strProid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	string engId=""; //工程单id 
	string shipId="";
	string shipName="";
	int type=0;
	int status=0;
	int pStartDate=0;
	int pEndDate=0;
	int aStartDate=0;
	int aEndDate=0;
	float sPeriod=0; //计划周期
	float aPeriod=0; //实际周期
	string location="";
	string dockedId="";//场修计划id
	string companyId=""; 
	string companyName=""; 
	string alert;//注意事项

	if(psql->NextRow())
	{ 
		READMYSQL_STRING(REPAIR_ENGINEERING_ID,engId);
		READMYSQL_INT(TYPE,type,0);
		READMYSQL_INT(STATUS,status,0);
		READMYSQL_INT(SCHEDULED_START_DATE,pStartDate,0);
		READMYSQL_INT(SCHEDULED_END_DATE,pEndDate,0);
		READMYSQL_INT(ACTUAL_START_DATE,aStartDate,0);
		READMYSQL_INT(ACTUAL_END_DATE,aEndDate,0);
		READMYSQL_FLOAT(SCHEDULED_PERIOD,sPeriod,0);
		READMYSQL_FLOAT(ACTUAL_PERIOD,aPeriod,0);
		READMYSQL_STRING(SHIP_ID,shipId);
		READMYSQL_STRING(SHIP_NAME,shipName);
		READMYSQL_STRING(REPAIR_LOCATION,location);
		READMYSQL_STRING(DOCKED_REPAIR_ID,dockedId);
		READMYSQL_STRING(REPAIR_COMPANY_ID,companyId); 
		READMYSQL_STRING(COMPANY_NAME,companyName); 
		READMYSQL_STRING(ENG_DESC,alert); 

		string repairContent="";
		getFactoryRepairContent(dockedId,repairContent);

	}

	string repairContent="";
	getFactoryRepairContent(dockedId,repairContent);
	string otherDrections="";
	getOtherDrections(engId,otherDrections);
	string applyinfo="";
	getRepairApplies(engId,applyinfo);

	out<<"{seq:\""<<strSeq<<"\",proid:\""<<engId<<"\",shipid:\""<<shipId<<"\",shipna:\""<<shipName<<"\",type:"<<type<<",state:"<<status;
	out<<",loc:\""<<location<<"\",psday:"<<pStartDate<<",pfday:"<<pEndDate<<",asday:"<<aStartDate<<",afday:"<<aEndDate<<", pdays:"<<sPeriod<<",adays:"<<aPeriod;
	out<<",comid:\""<<companyId<<"\",comna:\""<<companyName<<"\", facid:\""<<dockedId<<"\", facna:\""<<repairContent<<"\",alert:\""<<alert;
	out<<"\",illu:\""<<otherDrections<<"\",applyinfo:"<<applyinfo<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;

}

//完成修理单
//{seq:"",applyid:"",startdt:"",enddt:""}
int eMOPRepairSvc::FinishRepairApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetEngineeringDetail]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strApplyid= root.getv("applyid", "");  
	string strStartdt= root.getv("startdt", "");  
	string strEnddt = root.getv("enddt", "");  

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   

	sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_REPAIR_APPLIES SET ACTUAL_START_DATE='%s',STATUS=2,ACTUAL_END_DATE='%s' WHERE REPAIR_APPLY_ID='%s'",
		strStartdt.c_str(),strEnddt.c_str(),strApplyid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS SET STATUS=2 WHERE REPAIR_APPLY_ID='%s'",strApplyid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  

	out<<"{eid:0,seq:\""<<strSeq<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//删除工程单
//{seq:"",proid:""}
int eMOPRepairSvc::DeleteEngineering(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetEngineeringDetail]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strProid= root.getv("proid", "");    
	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";    
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING WHERE REPAIR_ENGINEERING_ID='%s'",
		strProid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	out<<"{eid:0,seq:\""<<strSeq<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//{seq:"",uid:"",facid:""}
//{seq:"",eid:0}
int eMOPRepairSvc::DeleteFactoryRepair(const char* pUid, const char* jsonString, std::stringstream& out){
	JSON_PARSE_RETURN("[eMOPRepairSvc::AddOrUpdateFactoryRepair]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");   
	string strFacid=root.getv("facid","");    

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCHEDULES WHERE DOCKED_REPAIR_ID='%s';",
		strFacid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCHEDULES WHERE DOCKED_REPAIR_ID='%s';",
		strFacid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING SET DOCKED_REPAIR_ID='' WHERE DOCKED_REPAIR_ID='%s';",
		strFacid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

	out<<"{eid:0,seq:\""<<strSeq<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//{seq:"",type:,facid:"",shipid:"",state:1,pin:,pout:,rin:,rout:,cont:id|id|..,comid:"",comna:"", ptime:,rtime:,port:"",ccs:"",certdt:}
//{seq:"",eid:0,type:0,facid:""}
int eMOPRepairSvc::AddOrUpdateFactoryRepair(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::AddOrUpdateFactoryRepair]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");  
	int type = root.getv("type", -1);
	string strFacid=root.getv("facid",""); 
	string strShipid=root.getv("shipid","");  
	int state = root.getv("state",0);
	int sStartDate = root.getv("pin",0);
	int sEndDate = root.getv("pout",0);
	int aStartDate = root.getv("rin",0); 	
	int aEndDate = root.getv("rout",0);   	
	float sPeriod = root.getv("sPeriod",0);
	float aPeriod = root.getv("aPeriod",0);

	string strCompanyId=root.getv("comid",""); 
	string strCompanyName=root.getv("comna","");  
	string strPort=root.getv("port",""); 	
	string strCcs=root.getv("ccs",""); 
	int certdt = root.getv("certdt",0);  

	string strCon=root.getv("cont",""); 

	string strSStartDate = TimeToTimestampStr(sStartDate);
	string strSEndDate = TimeToTimestampStr(sEndDate);
	string strAStartDate = TimeToTimestampStr(aStartDate); 	
	string strAEndDate = TimeToTimestampStr(aEndDate);  
	string strCertdt = TimeToTimestampStr(certdt);

	MySql* psql = CREATE_MYSQL;
	char sql[2048] = "";
	//处理船舶id
	this->checkNewCompany(strCompanyId,strCompanyName);
	if(type==0){
		time_t t = time(NULL);
		tm* aTm = localtime(&t); 
		sprintf(sql,"SELECT MAX(KEY_NUM) AS MAX_DOCKED_NUM FROM (SELECT SUBSTRING(DOCKED_REPAIR_ID,10)+0 AS KEY_NUM  \
					FROM blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCHEDULES WHERE DOCKED_REPAIR_ID LIKE 'D%04d%02d%02d%%') t",aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday); 
		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		int nextNum=0;
		if(psql->NextRow())
			READMYSQL_INT(MAX_DOCKED_NUM,nextNum,0);
		 
		char newKey[40];
		sprintf(newKey, "D%04d%02d%02d%010d", aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday,nextNum+1);  
		sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCHEDULES(DOCKED_REPAIR_ID,SHIP_ID,REPAIR_COMPANY_ID,STATUS,\
					SCHEDULED_START_DATE,SCHEDULED_END_DATE,ACTUAL_START_DATE,ACTUAL_END_DATE,\
					SCHEDULED_PERIOD,ACTUAL_PERIOD,REPAIR_PORT,CCS,CERT_DUE_DATE) VALUES('%s','%s','%s',%d,'%s','%s','%s','%s',%f,%f,'%s','%s','%s');",
					newKey,strShipid.c_str(),strCompanyId.c_str(),state,
					strSStartDate.c_str(),strSEndDate.c_str(),strAStartDate.c_str(),strAEndDate.c_str(),sPeriod,aPeriod,strPort.c_str(),strCcs.c_str(),strCertdt.c_str());
		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
		strFacid=newKey;
	}else  if(type==1){ //修改
		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCHEDULES \
					SET SHIP_ID='%s',REPAIR_COMPANY_ID='%s',STATUS=%d,\
					SCHEDULED_START_DATE='%s',SCHEDULED_END_DATE='%s',ACTUAL_START_DATE='%s',ACTUAL_END_DATE='%s', \
					SCHEDULED_PERIOD=%f,ACTUAL_PERIOD=%f,REPAIR_PORT='%s',CCS='%s',CERT_DUE_DATE='%s'\
					WHERE DOCKED_REPAIR_ID='%s'",strShipid.c_str(),strCompanyId.c_str(),state,
					strSStartDate.c_str(),strSEndDate.c_str(),strAStartDate.c_str(),strAEndDate.c_str(),sPeriod,aPeriod,strPort.c_str(),strCcs.c_str(),strCertdt.c_str(),strFacid.c_str());
		DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
	}

	//处理检验 
	sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCH_SURVEY WHERE DOCKED_REPAIR_ID='%s'",strFacid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  

	if(!strCon.empty()){
		Tokens conVec = StrSplit(strCon,"|");
		for(Tokens::iterator it=conVec.begin();it!=conVec.end();it++)
		{
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCH_SURVEY(DOCKED_REPAIR_ID,ID) VALUES('%s','%s');",strFacid.c_str(),it->c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
	}
	out<<"{seq:\""<<strSeq<<"\",eid:0,type:"<<type<<",facid:\""<<strFacid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);	
	return 0;
}

//请求某船舶下检验列表
int eMOPRepairSvc::GetShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetShipSurvey]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");  
	string strShipId = root.getv("shipid", "");
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶id
	std::stringstream shipsSql; 
	sprintf(sql,"SELECT t1.ID,t2.SURVEY_NAME  FROM blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY t1 \
				LEFT JOIN blm_emop_etl.T50_EMOP_SC_SURVEY_CODE t2 ON t1.SURVEY_CODE=t2.SURVEY_CODE \
				WHERE t1.SHIP_ID='%s'",strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int idx=0;
	string id="";
	string name="";
	out<<"{seq:\""<<strSeq<<"\",info:[";
	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;  
		READMYSQL_STRING(ID,id);
		READMYSQL_STRING(SURVEY_NAME,name);
		out<<"{suvid:\""<<id<<"\",suvna:\""<<name<<"\"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}


//请求某船舶下所有未开始的未被分配的的工程单
//{seq:"",shipid:""}
//{seq:"",info:[{proid:"",psday:10,pfday:11, asday:10,afday:11},{}]}
int eMOPRepairSvc::GetShipEngineeringNotBegin(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetShipEngineeringNotBegin]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");  
	string strShipId = root.getv("shipid", "");  
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶id
	std::stringstream shipsSql; 
	sprintf(sql,"SELECT REPAIR_ENGINEERING_ID,\
				UNIX_TIMESTAMP(SCHEDULED_START_DATE) AS SCHEDULED__START_DATE,UNIX_TIMESTAMP(SCHEDULED_END_DATE) AS SCHEDULED_START_DATE,UNIX_TIMESTAMP(ACTUAL_START_DATE) AS ACTUAL_START_DATE,UNIX_TIMESTAMP(ACTUAL_END_DATE) AS ACTUAL_START_DATE \
				FROM blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING WHERE SHIP_ID='%s' AND STATUS=0 AND (DOCKED_REPAIR_ID='' OR DOCKED_REPAIR_ID=NULL)",
				strShipId.c_str());

	out<<"{seq:\""<<strSeq<<"\",info:[";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int idx=0;
	string pid=""; 
	int sStartDate=0;
	int sEndDate=0;
	int aStartDate=0;
	int aEndDate=0;

	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;

		READMYSQL_STRING(REPAIR_ENGINEERING_ID,pid); 
		READMYSQL_INT(SCHEDULED__START_DATE,sStartDate,0);
		READMYSQL_INT(SCHEDULED_END_DATE,sEndDate,0);
		READMYSQL_INT(ACTUAL_START_DATE,aStartDate,0);
		READMYSQL_INT(ACTUAL_END_DATE,aStartDate,0);

		out<<"{proid:\""<<pid<<"\",psday:"<<sStartDate<<",pfday:"<<sEndDate<<",asday:"<<aStartDate<<",afday:"<<aStartDate<<"}";
	}
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//{seq:"",uid:"",shipid:"",year:"",state:""}
//{seq:"",info:[{facid:"",shipid:"",shipna:"",state:"",pin:,pout:,rin:,rout:,cont:[{id:"",na:""},{}…],comid:"",comna:"",ptime:"",rtime:"",port:"",ccs:"",certdt:pro:[{proid:"",state:"",sday:10,fday:11},{}], },{},{}]}
int eMOPRepairSvc::GetFactoryRepair(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetFactoryRepair]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strUid = root.getv("uid", ""); 
	string strShipId = root.getv("shipid", ""); 
	string strYear = root.getv("year", "");
	string strStatus = root.getv("state", "");
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶id
	std::stringstream shipsSql;
	shipsSql<<"WHERE t1.SHIP_ID IN ("; 
	if(strShipId.empty()){
		sprintf(sql,"SELECT SHIPID FROM  blm_emop_etl.T50_EMOP_REGISTERED_USER_FLEET WHERE USER_ID='%s'",strUid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		int idx=0;
		char shipid[20];
		while(psql->NextRow()){
			if(idx>0)
				shipsSql<<",";
			idx++;
			READMYSQL_STR(SHIPID,shipid);
			shipsSql<<"'"<<shipid<<"'";
		}
	}
	else
	{
		Tokens shipIdVec = StrSplit(strShipId,"|"); 
		int idx=0;
		for(Tokens::iterator it=shipIdVec.begin();it!=shipIdVec.end();it++)
		{
			if(idx>0)
				shipsSql<<",";
			idx++; 
			shipsSql<<"'"<<it->c_str()<<"'";
		}
	} 
	shipsSql<<") ";

	//处理状态strStatus
	std::stringstream statusSql;
	statusSql<<""; 
	if(!strStatus.empty()){ 
		statusSql<<"AND t1.STATUS IN (";
		Tokens statusVec = StrSplit(strStatus,"|"); 
		int idx=0;
		for(Tokens::iterator it=statusVec.begin();it!=statusVec.end();it++)
		{
			if(idx>0)
				statusSql<<",";
			idx++; 
			statusSql<<it->c_str();
		} 
		statusSql<<") ";
	} 
	//处理年 
	std::stringstream YearSql;
	YearSql<<""; 
	if(!strYear.empty()){
		YearSql<<"AND SUBSTRING(SCHEDULED_START_DATE,1,4) IN (";
		Tokens yearVec = StrSplit(strYear,"|"); 
		int idx=0;
		for(Tokens::iterator it=yearVec.begin();it!=yearVec.end();it++)
		{
			if(idx>0)
				YearSql<<",";
			idx++; 
			YearSql<<it->c_str();
		} 
		YearSql<<") ";
	} 

	sprintf(sql,"SELECT t1.DOCKED_REPAIR_ID,t1.SHIP_ID,t2.NAME_CN AS SHIPNAME_CN,t1.STATUS,\
				UNIX_TIMESTAMP(t1.SCHEDULED_START_DATE) AS SCHEDULED_START_DATE,UNIX_TIMESTAMP(t1.SCHEDULED_END_DATE) AS SCHEDULED_END_DATE,UNIX_TIMESTAMP(t1.ACTUAL_START_DATE) AS ACTUAL_START_DATE,UNIX_TIMESTAMP(t1.ACTUAL_END_DATE) AS ACTUAL_END_DATE,\
				t1.SCHEDULED_PERIOD,t1.ACTUAL_PERIOD,t1.REPAIR_PORT,t1.CCS,UNIX_TIMESTAMP(t1.CERT_DUE_DATE) AS CERT_DUE_DATE,\
				t1.REPAIR_COMPANY_ID,t3.NAME AS COMPANY_NAME\
				FROM blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCHEDULES t1 \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP t2 ON  t2.SHIPID=t1.SHIP_ID \
				LEFT JOIN blm_emop_etl.T41_COMPANY t3 ON t1.REPAIR_COMPANY_ID=t3.COMPANY_KEY %s %s %s",
				shipsSql.str().c_str(),YearSql.str().c_str(),statusSql.str().c_str()); 
	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	string dockedId="";
	string shipid="";
	string shipname="";
	int status=0;
	int sStartDate=0;
	int sEndDate=0;
	int aStartDate=0;
	int aEndDate=0;
	float sPeroid=0;
	float aPeriod=0;
	string port="";
	string ccs="";
	int certDate=0;
	string companyKey="";
	string companyName="";
	out<<"{seq:\""<<strSeq<<"\",info:["; 
	int idx=0;
	while(psql->NextRow()){
		READMYSQL_STRING(DOCKED_REPAIR_ID,dockedId);
		READMYSQL_STRING(SHIP_ID,shipid);
		READMYSQL_STRING(SHIPNAME_CN,shipname);
		READMYSQL_INT(STATUS,status,0);
		READMYSQL_INT(SCHEDULED_START_DATE,sStartDate,0);
		READMYSQL_INT(SCHEDULED_END_DATE,sEndDate,0);
		READMYSQL_INT(ACTUAL_START_DATE,aStartDate,0);
		READMYSQL_INT(ACTUAL_END_DATE,aEndDate,0); 
		READMYSQL_FLOAT(SCHEDULED_PERIOD,sPeroid,0); 
		READMYSQL_FLOAT(ACTUAL_PERIOD,aPeriod,0);  
		READMYSQL_STRING(REPAIR_PORT,port);
		READMYSQL_STRING(CCS,ccs); 
		READMYSQL_INT(CERT_DUE_DATE,certDate,0); 
		READMYSQL_STRING(REPAIR_COMPANY_ID,companyKey); 
		READMYSQL_STRING(COMPANY_NAME,companyName); 

		string repairContent="";
		getFactoryRepairContent2(dockedId,repairContent);
		string repairEn="";  //工程单
		getFactoryEngineer(dockedId,repairEn);
		if(idx>0)
			out<<",";
		idx++;
		out<<"{facid:\""<<dockedId<<"\",shipid:\""<<shipid<<"\",shipna:\""<<shipname<<"\",state:\""<<status;
		out<<"\",pin:"<<sStartDate<<",pout:"<<sEndDate<<",rin:"<<aStartDate<<",rout:"<<aEndDate;		
		out<<",comid:\""<<companyKey<<"\",comna:\""<<companyName<<"\",ptime:"<<sPeroid<<",rtime:"<<aPeriod<<",port:\""<<port<<"\",ccs:\""<<ccs<<"\",certdt:"<<certDate;
		out<<",pro:["<<repairEn<<"],cont:["<<repairContent<<"]}";
	}	
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//请求修理单列表
//{seq:"",uid:"",shipid:"", tp:"",deparid:"",proc:"",link:"",st:""}
//{seq:"",info:[{applyid:"",shipid:"",shipn:"",deparid:"",deparn:"",type:"",proc:"",link:"",st:"",self: 0,comid:"",comna:"", adate:,bdate:,fdate:,loc:"",proid:"",rbdate:,rfdate:,maded:,pland:,pstate:"",ldate:,lname:""},{},{}]}
int eMOPRepairSvc::GetRepairApplyList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetRepairApplyList]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");
	string strUid = root.getv("uid", "");
	string strShipId = root.getv("shipid", "");    
	string strType = root.getv("tp", "");    
	string strDeparid = root.getv("deparid", "");   
	string strTasks = root.getv("proc", "");    
	string strSteps = root.getv("link", "");    
	string strStatus = root.getv("st", ""); 

	MySql* psql = CREATE_MYSQL;
	char sql[2048] = "";   
	//处理船舶id
	std::stringstream shipsSql;
	shipsSql<<"WHERE t1.SHIP_ID IN ("; 
	if(strShipId.empty()){
		sprintf(sql,"SELECT SHIPID FROM  blm_emop_etl.T50_EMOP_REGISTERED_USER_FLEET WHERE USER_ID='%s'",strUid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		int idx=0;
		char shipid[20];
		while(psql->NextRow()){
			if(idx>0)
				shipsSql<<",";
			idx++;
			READMYSQL_STR(SHIPID,shipid);
			shipsSql<<"'"<<shipid<<"'";
		}
	}
	else
	{
		Tokens shipIdVec = StrSplit(strShipId,"|"); 
		int idx=0;
		for(Tokens::iterator it=shipIdVec.begin();it!=shipIdVec.end();it++)
		{
			if(idx>0)
				shipsSql<<",";
			idx++; 
			shipsSql<<"'"<<it->c_str()<<"'";
		}
	} 
	shipsSql<<") ";
	//处理类型strType
	std::stringstream typeSql;
	typeSql<<""; 
	if(!strType.empty()){ 
		typeSql<<"AND t1.TYPE IN (";
		Tokens typeVec = StrSplit(strType,"|"); 
		int idx=0;
		for(Tokens::iterator it=typeVec.begin();it!=typeVec.end();it++)
		{
			if(idx>0)
				typeSql<<",";
			idx++; 
			typeSql<<it->c_str();
		} 
		typeSql<<") ";
	}  

	//处理部门 strDeparid
	std::stringstream departSql;
	departSql<<""; 
	if(!strStatus.empty()){ 
		departSql<<"AND t1.DEPARTMENT_CODE IN (";
		Tokens departVec = StrSplit(strDeparid,"|"); 
		int idx=0;
		for(Tokens::iterator it=departVec.begin();it!=departVec.end();it++)
		{
			if(idx>0)
				departSql<<",";
			idx++; 
			departSql<<"'"<<it->c_str()<<"'";
		} 
		departSql<<") ";
	} 
	//处理业务strTasks
	std::stringstream taskSql;
	taskSql<<""; 
	if(!strTasks.empty()){ 
		taskSql<<"AND t2.WORKFLOW_TASK_ID IN (";
		Tokens taskVec = StrSplit(strTasks,"|"); 
		int idx=0;
		for(Tokens::iterator it=taskVec.begin();it!=taskVec.end();it++)
		{
			if(idx>0)
				taskSql<<",";
			idx++; 
			taskSql<<"'"<<it->c_str()<<"'";
		} 
		taskSql<<") ";
	} 

	//处理步骤strSteps
	std::stringstream stepSql;
	stepSql<<""; 
	if(!strSteps.empty()){ 
		stepSql<<"AND t2.WORKFLOW_STEP_ID IN (";
		Tokens stepVec = StrSplit(strSteps,"|"); 
		int idx=0;
		for(Tokens::iterator it=stepVec.begin();it!=stepVec.end();it++)
		{
			if(idx>0)
				stepSql<<",";
			idx++; 
			stepSql<<"'"<<it->c_str()<<"'";
		} 
		stepSql<<") ";
	} 
	//处理状态strStatus
	std::stringstream statusSql;
	statusSql<<""; 
	if(!strStatus.empty()){ 
		statusSql<<"AND t2.STATUS IN (";
		Tokens statusVec = StrSplit(strStatus,"|"); 
		int idx=0;
		for(Tokens::iterator it=statusVec.begin();it!=statusVec.end();it++)
		{
			if(idx>0)
				statusSql<<",";
			idx++; 
			statusSql<<it->c_str();
		} 
		statusSql<<") ";
	} 

	sprintf(sql,"SELECT t1.REPAIR_APPLY_ID,t1.SHIP_ID,t3.NAME_CN AS SHIP_NAME_CN,t1.REPAIR_APPLY_ID,t1.DEPARTMENT_CODE,t4.NAME_CN AS DEPART_NAME_CN,\
				t1.TYPE,t1.SELF_FLAG,t1.REPAIR_LOCATION,t5.COMPANY_KEY,t5.NAME AS COMPANY_NAME_CN,\
				UNIX_TIMESTAMP(t1.APPLY_DATE) AS APPLY_DATE,UNIX_TIMESTAMP(t1.SCHEDULED_START_DATE) AS SCHEDULED_START_DATE,UNIX_TIMESTAMP(t1.SCHEDULED_END_DATE) AS SCHEDULED_END_DATE,UNIX_TIMESTAMP(t1.ACTUAL_START_DATE) AS ACTUAL_START_DATE,UNIX_TIMESTAMP(t1.ACTUAL_END_DATE) AS ACTUAL_END_DATE,\
				t1.REPAIR_ENGINEERING_ID,t6.STATUS AS ENG_STATUS,UNIX_TIMESTAMP(t6.SCHEDULED_START_DATE) AS ENG_SCHEDULED_START_DATE,UNIX_TIMESTAMP(t6.APPLY_DATE) AS ENG_APPLY_DATE,\
				t2.OPERATOR AS STATUS_OPERATOR,t2.OP_DATE AS STATUS_OP_DATE,t2.WORKFLOW_TASK_ID,t2.WORKFLOW_STEP_ID,t2.STATUS AS WORKFLOW_STEP_STATUS\
				FROM blm_emop_etl.T50_EMOP_REPAIR_APPLIES t1\
				LEFT JOIN (SELECT * FROM blm_emop_etl.T50_EMOP_PEPAIR_APPLY_STATUS ORDER BY OP_DATE DESC LIMIT 1) t2 ON t1.REPAIR_APPLY_ID=t2.REPAIR_APPLY_ID \
				LEFT JOIN blm_emop_etl.T41_EMOP_SHIP t3 ON t1.SHIP_ID=t3.SHIPID\
				LEFT JOIN blm_emop_etl.T50_EMOP_DEPARTMENT_CODE t4 ON t1.DEPARTMENT_CODE=t4.DEPARTMENT_CODE\
				LEFT JOIN blm_emop_etl.t41_company t5 ON t1.COMPANY_KEY=t5.COMPANY_KEY\
				LEFT JOIN blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING t6 ON t1.REPAIR_ENGINEERING_ID=t6.REPAIR_ENGINEERING_ID \
				%s %s %s %s %s %s ",shipsSql.str().c_str(),typeSql.str().c_str(),departSql.str().c_str(),taskSql.str().c_str(),stepSql.str().c_str(),statusSql.str().c_str()); 
 
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);
	string repairApplyId="";
	string shipId="";
	string shipName="";
	string departId="";
	string departName="";
	string companyKey="";
	string companyName="";
	int type=0;
	int selfFlag=0;
	string repairLoc="";
	int applyDate=0;
	int sStartDate=0;
	int sEndDate=0;
	int aStartDate=0;
	int aEndDate=0; 
	string enId="";//工程单id
	int enStatus=0;
	int enSStartDate=0;
	int enApplyDate=0;
	string taskId="";
	string stepId="";
	int stepStatus=0;
	string lastUser="";
	int lastDate=0;
	out<<"{seq:\""<<strSeq<<"\",info:[";
	int idx=0;
	while(psql->NextRow()){
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(REPAIR_APPLY_ID,repairApplyId);
		READMYSQL_STRING(SHIP_ID,shipId);
		READMYSQL_STRING(SHIP_NAME_CN,shipName);
		READMYSQL_STRING(DEPARTMENT_CODE,departId); 
		READMYSQL_STRING(DEPART_NAME_CN,departName);
		READMYSQL_STRING(COMPANY_KEY,companyKey);
		READMYSQL_STRING(COMPANY_NAME_CN,companyName);
		READMYSQL_INT(TYPE,type,0);
		READMYSQL_INT(SELF_FLAG,selfFlag,0);
		READMYSQL_STRING(REPAIR_LOCATION,repairLoc);
		READMYSQL_INT(APPLY_DATE,applyDate,0);
		READMYSQL_INT(SCHEDULED_START_DATE,sStartDate,0);
		READMYSQL_INT(SCHEDULED_END_DATE,sEndDate,0);
		READMYSQL_INT(ACTUAL_START_DATE,aEndDate,0);
		READMYSQL_INT(ACTUAL_END_DATE,aEndDate,0);
		READMYSQL_STRING(REPAIR_ENGINEERING_ID,enId);
		READMYSQL_INT(ENG_STATUS,enStatus,0);
		READMYSQL_INT(ENG_SCHEDULED_START_DATE,enSStartDate,0);
		READMYSQL_INT(ENG_APPLY_DATE,enApplyDate,0);
		READMYSQL_STRING(WORKFLOW_TASK_ID,taskId); 
		READMYSQL_STRING(WORKFLOW_STEP_ID,stepId); 
		READMYSQL_INT(WORKFLOW_STEP_STATUS,stepStatus,0);
		READMYSQL_STRING(STATUS_OPERATOR,lastUser);  
		READMYSQL_INT(STATUS_OP_DATE,lastDate,0);
		out<<"{applyid:\""<<repairApplyId<<"\",shipid:\""<<shipId<<"\",shipn:\""<<shipName;
		out<<"\",deparid:\""<<departId<<"\",deparn:\""<<departName<<"\",type:"<<type<<",proc:\""<<taskId;
		out<<"\",link:\""<<stepId<<"\",st:\""<<stepStatus<<"\",self:"<<selfFlag<<",comid:\""<<companyKey<<"\",comna:\""<<companyName;
		out<<"\", adate:"<<applyDate<<",bdate:"<<sStartDate<<",fdate:"<<sEndDate<<",loc:\""<<repairLoc;
		out<<"\",proid:\""<<enId<<"\",rbdate:"<<aEndDate<<",rfdate:"<<aEndDate<<",maded:"<<enApplyDate<<",pland:"<<enSStartDate;
		out<<",pstate:"<<enStatus<<",ldate:"<<lastDate<<",lname:\""<<lastUser<<"\"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}

//修理单详细信息
//{seq:"",applyid:""}
//{seq:"",item:{iid:"",ina:"",icon:"",info:[{cwbt:"",na:"",wcs:[{id:"",level:"",owner:""},{}]},{}]},}
int eMOPRepairSvc::GetRepairApplyDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::GetRepairApplyDetail]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	string strApplyid = root.getv("applyid", "");    
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//修理项信息
	sprintf(sql,"SELECT t1.SHIP_ID,t2.REPAIR_ITEM_CODE,t3.NAME_CN,t3.CONTENTS \
				FROM blm_emop_etl.T50_EMOP_REPAIR_APPLIES t1 \
				LEFT JOIN blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS t2 ON t1.REPAIR_APPLY_ID=t2.REPAIR_APPLY_ID \
				LEFT JOIN blm_emop_etl.T50_EMOP_REPAIR_ITEMS t3 ON t2.REPAIR_ITEM_CODE=t3.REPAIR_ITEM_CODE \
				WHERE t1.REPAIR_APPLY_ID='%s'",strApplyid.c_str()); 
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	string shipId="";
	string itemCode="";
	string itemName_cn="";
	string itemContents="";
	if(psql->NextRow()){
		READMYSQL_STRING(SHIP_ID,shipId);
		READMYSQL_STRING(REPAIR_ITEM_CODE,itemCode);
		READMYSQL_STRING(NAME_CN,itemName_cn);
		READMYSQL_STRING_JSON(CONTENTS,itemContents); 
	}
	//修理项的设备卡信息
	out<<"{seq:\""<<strSeq<<"\",item:{iid:\""<<itemCode<<"\",ina:\""<<itemName_cn<<"\",icon:\""<<itemContents<<"\",info:[";
	sprintf(sql,"SELECT t1.EQUIP_CWBT,t2.NAME_CN FROM blm_emop_etl.T50_EMOP_REPAIR_ITEMS_SHIPS_WORCARD t1 \
				LEFT JOIN blm_emop_etl.T50_CBWT_SHIP_EQUIP_CARD t2 ON t1.EQUIP_CWBT=t2.CWBT \
				WHERE  t1.SHIP_ID='%s' AND t1.REPAIR_ITEM_CODE='%s' GROUP BY t1.EQUIP_CWBT",shipId.c_str(),itemCode.c_str()); 
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

	string cwbt="";
	string equipName="";
	string workCards="";
	int idx=0;
	while(psql->NextRow()){  
		READMYSQL_STRING(EQUIP_CWBT,cwbt); 
		READMYSQL_STRING(CONTENTS,equipName); 
		if(idx>0)
			out<<",";
		idx++;
		getRepairEquipWorkCard(shipId,itemCode,cwbt,workCards);
		out<<"{cwbt:\""<<cwbt<<"\",na:\""<<equipName<<"\",wcs:["<<workCards<<"]}";
	} 
	out<<"]}}";

	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//{seq:"",distype:1, applyid:"",shipid:"",deparid:"", type:2,self:0, comid:"",comna:"", adate:,bdate:,fdate:,loc:"", rbdate:,rfdate:,itemid:"" }
//{eid:0,seq:"",applyid:""}
int eMOPRepairSvc::AddOrUpdateRepairApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPRepairSvc::AddOrUpdateRepairApply]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", ""); 
	int reqtype = root.getv("distype", 0);   
	string strApplyid = root.getv("applyid", "");
	string strShipid = root.getv("shipid", "");
	string strDeparid = root.getv("deparid", "");
	int type = root.getv("type", 0);
	int self = root.getv("self", 0); //自修标志
	string strCompanyId = root.getv("comid", "");
	string strCompanyName = root.getv("comna", "");
	int adate = root.getv("adate", 0);
	int bdate = root.getv("bdate", 0);
	int fdate = root.getv("fdate", 0);
	int rbdate = root.getv("rbdate", 0);
	int rfdate= root.getv("rfdate", 0);
	string strLoc = root.getv("loc", "");
	string strItemCode = root.getv("itemid", "");

	string stradate = TimeToTimestampStr(adate);
	string strbdate = TimeToTimestampStr(bdate);
	string strfdate = TimeToTimestampStr(fdate);
	string strrbdate = TimeToTimestampStr(rbdate);
	string strrfdate= TimeToTimestampStr(rfdate);


	checkNewCompany(strCompanyId,strCompanyName);//公司处理,不存在添加新公司

	MySql* psql = CREATE_MYSQL;
	char sql[2048] = "";   
	if(reqtype==0){
		//生成id
		time_t t = time(NULL);
		tm* aTm = localtime(&t); 
		sprintf(sql,"SELECT MAX(KEY_NUM) AS REPAIR_APPLY_NUM FROM (SELECT SUBSTRING(REPAIR_APPLY_ID,11)+0 AS KEY_NUM  \
					FROM blm_emop_etl.T50_EMOP_REPAIR_APPLIES WHERE REPAIR_APPLY_ID LIKE 'RP%04d%02d%02d%%') t",aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday); 
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

		int nextNum=0;
		if(psql->NextRow())
			READMYSQL_INT(REPAIR_APPLY_NUM,nextNum,0);
		nextNum++;
		char newKey[20];
		sprintf(newKey, "RP%04d%02d%02d%09d", aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday,nextNum);
		strApplyid = newKey;

		sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_REPAIR_APPLIES(REPAIR_APPLY_ID,SHIP_ID,DEPARTMENT_CODE,COMPANY_KEY,TYPE,SELF_FLAG,\
					APPLY_DATE,SCHEDULED_START_DATE,SCHEDULED_END_DATE,ACTUAL_START_DATE,ACTUAL_END_DATE,REPAIR_LOCATION)\
					VALUES('%s','%s','%s','%s',%d,%d,'%s','%s','%s','%s','%s','%s');",newKey,strShipid.c_str(),strDeparid.c_str(),strCompanyId.c_str(),type,self,
					stradate.c_str(),strbdate.c_str(),strfdate.c_str(),strrbdate.c_str(),strrfdate.c_str(),strLoc.c_str()); 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

		if(!strItemCode.empty()){
			//修理单修理项id
			sprintf(sql,"SELECT MAX(REPAIR_APPLY_ITEM_ID+0) AS MAX_NUM FROM blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS"); 
			CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  

			nextNum=0;
			if(psql->NextRow())
				READMYSQL_INT(MAX_NUM,nextNum,0);
			nextNum++;

			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS(REPAIR_APPLY_ITEM_ID,REPAIR_APPLY_ID,REPAIR_ITEM_CODE)\
						VALUES('%09d','%s','%s');",nextNum,newKey,strItemCode.c_str()); 
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
	}else if(reqtype==1){
		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_REPAIR_APPLIES SET SHIP_ID='%s',DEPARTMENT_CODE='%s',COMPANY_KEY='%s',TYPE=%d,SELF_FLAG=%d,\
					APPLY_DATE='%s',SCHEDULED_START_DATE='%s',SCHEDULED_END_DATE='%s',ACTUAL_START_DATE='%s',ACTUAL_END_DATE='%s',REPAIR_LOCATION='%s' \
					WHERE REPAIR_APPLY_ID='%s'",strShipid.c_str(),strDeparid.c_str(),strCompanyId.c_str(),type,self,
					stradate.c_str(),strbdate.c_str(),strfdate.c_str(),strrbdate.c_str(),strrfdate.c_str(),strLoc.c_str(),strApplyid.c_str()); 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 

		sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS WHERE REPAIR_APPLY_ID='%s'",strApplyid.c_str()); 
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  		
		if(!strItemCode.empty()){
			//修理单修理项id
			sprintf(sql,"SELECT MAX(REPAIR_APPLY_ITEM_ID+0) AS MAX_NUM FROM blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS"); 
			CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
			int nextNum=0; 
			if(psql->NextRow())
				READMYSQL_INT(MAX_NUM,nextNum,0);
			nextNum++;
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS(REPAIR_APPLY_ITEM_ID,REPAIR_APPLY_ID,REPAIR_ITEM_CODE)\
						VALUES('%09d','%s','%s');",nextNum,strApplyid.c_str(),strItemCode.c_str()); 
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
	}
	out<<"{eid:0,seq:\""<<strSeq<<"\",applyid:\""<<strApplyid<<"\"}";
	return 0;
}
//工程单的修理单集合
int eMOPRepairSvc::getRepairApplies(std::string &proid,std::string &outStr)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	sprintf(sql,"SELECT t1.REPAIR_APPLY_ID,t2.REPAIR_ITEM_CODE,t1.STATUS,t3.NAME_CN AS REPAIRITEM_NAME,\
				UNIX_TIMESTAMP(t1.APPLY_DATE) AS APPLY_DATE,UNIX_TIMESTAMP(t1.ACTUAL_START_DATE) AS ACTUAL_START_DATE,UNIX_TIMESTAMP(t1.ACTUAL_END_DATE) AS ACTUAL_END_DATE \
				FROM blm_emop_etl.T50_EMOP_REPAIR_APPLIES t1 \
				LEFT JOIN blm_emop_etl.T50_EMOP_REPAIR_APPLY_ITEMS t2 ON t1.REPAIR_APPLY_ID = t2.REPAIR_APPLY_ID \
				LEFT JOIN blm_emop_etl.T50_EMOP_REPAIR_ITEMS t3 ON t2.REPAIR_ITEM_CODE=t3.REPAIR_ITEM_CODE \
				WHERE t1.REPAIR_ENGINEERING_ID='%s'",proid.c_str());
	std::stringstream out;
	out<<"";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int idx=0;
	string appid="";
	string itemId="";
	string itemName="";
	int adate=0;
	int bdate=0;
	int fdate=0;
	int st=0; 
	out<<"[";
	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(REPAIR_APPLY_ID,appid);
		READMYSQL_STRING(REPAIR_ITEM_CODE,itemId);
		READMYSQL_STRING(REPAIRITEM_NAME,itemName);
		READMYSQL_INT(APPLY_DATE,adate,0);
		READMYSQL_INT(ACTUAL_START_DATE,bdate,0);
		READMYSQL_INT(ACTUAL_END_DATE,fdate,0);
		READMYSQL_INT(STATUS,st,0);
		out<<"{applyid:\""<<appid<<"\",itemid:\""<<itemId<<"\",itemn:\""<<itemName;
		out<<"\",adate:"<<adate<<",bdate:"<<bdate<<",fdate:"<<fdate<<",st:"<<st<<"}";
	}
	out<<"]";
	outStr = out.str();
	RELEASE_MYSQL_RETURN(psql, 0);
}


//工程单其他说明项
int eMOPRepairSvc::getOtherDrections(std::string &proid,std::string &outStr){

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	sprintf(sql,"SELECT ENG_DIRCTIONS_ID FROM blm_emop_etl.T50_EMOP_REPAIR_ENG_DIRECTIONS WHERE REPAIR_ENGINEERING_ID='%s'",proid.c_str());
	std::stringstream out;
	out<<"";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int idx=0;
	string code="";
	while(psql->NextRow())
	{
		if(idx>0)
			out<<"|";
		idx++;

		READMYSQL_STRING(ENG_DIRCTIONS_ID,code);
		out<<code;
	}
	outStr = out.str();
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;
}
//厂修内容
int eMOPRepairSvc::getFactoryRepairContent(std::string &id,std::string &outStr){
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶id
	std::stringstream shipsSql; 
	sprintf(sql,"SELECT t3.SURVEY_NAME FROM  blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCH_SURVEY t1 \
				LEFT JOIN  blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY t2 ON t1.ID=t2.ID \
				LEFT JOIN blm_emop_etl.T50_EMOP_SC_SURVEY_CODE t3 ON t2.SURVEY_CODE=t3.SURVEY_CODE \
				WHERE t1.DOCKED_REPAIR_ID='%s'",id.c_str());
	std::stringstream out;
	out<<"";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int idx=0;
	string name="";
	while(psql->NextRow())
	{
		if(idx>0)
			out<<" ";
		idx++;

		READMYSQL_STRING(SURVEY_NAME,name);
		out<<name;
	}
	outStr = out.str();
	RELEASE_MYSQL_RETURN(psql, 0);
}


//厂修内容
int eMOPRepairSvc::getFactoryRepairContent2(std::string &id,std::string &outStr){
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶id
	std::stringstream shipsSql; 
	sprintf(sql,"SELECT t1.ID,t3.SURVEY_NAME FROM  blm_emop_etl.T50_EMOP_DOCKED_REPAIR_SCH_SURVEY t1 \
				LEFT JOIN  blm_emop_etl.T50_EMOP_SC_SHIP_SURVEY t2 ON t1.ID=t2.ID \
				LEFT JOIN blm_emop_etl.T50_EMOP_SC_SURVEY_CODE t3 ON t2.SURVEY_CODE=t3.SURVEY_CODE \
				WHERE t1.DOCKED_REPAIR_ID='%s'",id.c_str());
	std::stringstream out;
	out<<"";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int idx=0;
	string surid="";
	string name="";
	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;

		READMYSQL_STRING(ID,surid);
		READMYSQL_STRING(SURVEY_NAME,name);
		out<<"{id:\""<<surid<<"\",na:\""<<name<<"\"}";
	}
	outStr = out.str();
	RELEASE_MYSQL_RETURN(psql, 0);
}


int eMOPRepairSvc::getFactoryEngineer(std::string &id,std::string &outStr){
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";   
	//处理船舶id
	std::stringstream shipsSql; 
	sprintf(sql,"SELECT REPAIR_ENGINEERING_ID,STATUS,UNIX_TIMESTAMP(SCHEDULED_START_DATE) AS SCHEDULED__START_DATE,UNIX_TIMESTAMP(SCHEDULED_END_DATE) AS SCHEDULED_START_DATE,UNIX_TIMESTAMP(ACTUAL_START_DATE) AS ACTUAL_START_DATE,UNIX_TIMESTAMP(ACTUAL_END_DATE) AS ACTUAL_START_DATE FROM blm_emop_etl.T50_EMOP_REPAIR_ENGINEERING WHERE DOCKED_REPAIR_ID='%s'",id.c_str());
	std::stringstream out;
	out<<"";
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int idx=0;
	string pid="";
	int STATUS=0;
	int sStartDate=0;
	int sEndDate=0;
	int aStartDate=0;
	int aEndDate=0;

	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;

		READMYSQL_STRING(REPAIR_ENGINEERING_ID,pid);
		READMYSQL_INT(STATUS,STATUS,0);
		READMYSQL_INT(SCHEDULED__START_DATE,sStartDate,0);
		READMYSQL_INT(SCHEDULED_END_DATE,sEndDate,0);
		READMYSQL_INT(ACTUAL_START_DATE,aStartDate,0);
		READMYSQL_INT(ACTUAL_END_DATE,aStartDate,0);

		out<<"{proid:\""<<pid<<"\",state:\""<<STATUS<<"\",psday:"<<sStartDate<<",pfday:"<<sEndDate<<",asday:"<<aStartDate<<",afday:"<<aStartDate<<"}";
	}
	outStr = out.str();
	RELEASE_MYSQL_RETURN(psql, 0);
}

bool eMOPRepairSvc::checkNewCompany(std::string &cid,std::string &cname){

	bool haveCompany=false;
	if(cid.empty() && cname.empty())
		return haveCompany;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";    
	//判断公司是否存在,不存在在需要插入一个
	sprintf(sql,"SELECT COUNT(*) AS COMPANY_COUNT FROM blm_emop_etl.T41_COMPANY WHERE COMPANY_KEY='%s'",cid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
	int countCompanyKey=0; 
	if(psql->NextRow())
		READMYSQL_INT(COMPANY_COUNT,countCompanyKey,0);


	if(countCompanyKey<=0)
	{
		int nextNum=0;
		time_t t = time(NULL);
		tm* aTm = localtime(&t); 
		sprintf(sql,"SELECT MAX(KEY_NUM) AS COMPANY_NUM FROM (SELECT SUBSTRING(COMPANY_KEY,10)+0 AS KEY_NUM  \
					FROM blm_emop_etl.T41_COMPANY WHERE COMPANY_KEY LIKE 'C%04d%02d%02d%%') t",aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday); 
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);  
		if(psql->NextRow())
			READMYSQL_INT(COMPANY_NUM,nextNum,0);
		nextNum++;

		char newCompanyKey[40];
		sprintf(newCompanyKey, "C%04d%02d%02d%07d", aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday,nextNum);
		sprintf(sql,"INSERT INTO blm_emop_etl.T41_COMPANY(COMPANY_KEY,NAME) VALUES('%s','%s');",newCompanyKey,cname.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  
		cid = newCompanyKey;
		haveCompany = true;
	}
	RELEASE_MYSQL_RETURN(psql, 0);

	return haveCompany;
}




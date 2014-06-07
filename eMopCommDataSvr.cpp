#include "eMopCommDataSvr.h"
#include "blmcom_head.h"
#include "eMOPMaintenanceSvc.h"
#include "MessageService.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include "NotifyService.h"
#include <iostream>
#include <fstream>
#include <sstream>



eMOPCommDataSvr::eMOPCommDataSvr():m_lastPartTimeStr(""),m_bpartLoading(false),m_lastMaterialTimeStr(""),m_bmaterialLoading(false)
{

}

eMOPCommDataSvr::~eMOPCommDataSvr()
{

}

void eMOPCommDataSvr::Init()
{
	_refreshParts();
    _refreshEuqips();
	//_refreshMaterials();
	InitMaterial();
	int interval = 300;//每5分钟更新一次
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
 
	if(timerId <= 0)
		return;
	return ;
}
int eMOPCommDataSvr::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{

	_refreshParts();
	_refreshMaterials();

	return 0;
}


SparePartsInfo *eMOPCommDataSvr::GetPartItemById(string partid)
{
	if (m_mapAllParts.find(partid)!=m_mapAllParts.end())
	{
		return  &m_mapAllParts[partid];
	}

	return NULL;
}

void eMOPCommDataSvr::AddNewPartItem(SparePartsInfo &partItem)
{
       m_mapAllParts[partItem.m_partid]=partItem;

}
void eMOPCommDataSvr::DeletePartItem(string partid)
{
	if (m_mapAllParts.find(partid)!=m_mapAllParts.end())
	{
		  m_mapAllParts.erase(partid);
	}
}


EquipBaseInfo *eMOPCommDataSvr::GetEquipItemById(string equipid)
{
	if (m_mapAllEquips.find(equipid)!=m_mapAllEquips.end())
	{
		return  &m_mapAllEquips[equipid];
	}

	return NULL;
}

void eMOPCommDataSvr::AddNewEquipItem(EquipBaseInfo &equipItem)
{
	m_mapAllEquips[equipItem.m_equipid]=equipItem;

}
void eMOPCommDataSvr::DeleteEquipItem(string equipid)
{

	if (m_mapAllEquips.find(equipid)!=m_mapAllEquips.end())
	{
		 m_mapAllEquips.erase(equipid);
	}
}


MaterialBaseInfo *eMOPCommDataSvr::GetMaterialItemById(string materialId)
{
	if (m_mapAllMaterials.find(materialId)!=m_mapAllMaterials.end())
	{
         return &m_mapAllMaterials[materialId];
	}
   
	return NULL;
}

void eMOPCommDataSvr::AddMaterialItem(MaterialBaseInfo &materialItem)
{
     m_mapAllMaterials[materialItem.m_materialId]=materialItem;

}
void eMOPCommDataSvr::DeleteMaterialItem(string materialId)
{
	if (m_mapAllMaterials.find(materialId)!=m_mapAllMaterials.end())
	{
		m_mapAllMaterials.erase(materialId);
	}

}


bool eMOPCommDataSvr::_refreshParts()
{ 
	if(m_bpartLoading)
	{
		return false;
	}
	m_bpartLoading = true;

	MySql *psql = CREATE_MYSQL;
	char sql[512];
	char whereSql[100]="";
	if(!m_lastPartTimeStr.empty())
		sprintf(whereSql,"where last_upd_dt>='%s'",m_lastPartTimeStr.c_str());
	sprintf(sql,"select PART_MODEL_ID,MODEL,COMPANY_KEY,NAME_CN,NAME_EN,CHART_NO,STANDARD_NO,DESC_C,DESC_E,\
		REMARKS,HIGH_REQUIRED,LOW_REQUIRED,UNIT from blm_emop_etl.T50_EMOP_PARTS_MODELS %s",whereSql);
	
	psql->Query(sql);

	int rowcount = psql->GetRowCount();
	if(rowcount > 0)
	{
		ACE_Write_Guard<ACE_RW_Thread_Mutex> lock(m_partsLock);

		while(psql->NextRow())
		{
			SparePartsInfo partItem;

			 READMYSQL_STRING(PART_MODEL_ID, partItem.m_partid)
			 READMYSQL_STRING(MODEL, partItem.m_model)
			 READMYSQL_STRING(COMPANY_KEY, partItem.m_makefactory)
			 READMYSQL_STRING(NAME_CN, partItem.m_name_cn)
			 READMYSQL_STRING(NAME_EN, partItem.m_name_en)

			 READMYSQL_STRING(CHART_NO, partItem.m_charno)
			 READMYSQL_STRING(STANDARD_NO, partItem.m_standardno)
			 READMYSQL_STRING(DESC_C, partItem.m_desc_cn)
			 READMYSQL_STRING(DESC_E, partItem.m_desc_en)
			 READMYSQL_STRING(REMARKS, partItem.m_remarks)
			 READMYSQL_STRING(UNIT, partItem.m_unit)
			 
			 READMYSQL_INT(HIGH_REQUIRED, partItem.m_highreq,0)
			 READMYSQL_INT(LOW_REQUIRED, partItem.m_lowreq,0)

			partItem.m_name_cn=StrReplace(partItem.m_name_cn,"\"","\\\"");
            partItem.m_name_en=StrReplace(partItem.m_name_en,"\"","\\\"");
			partItem.m_desc_cn=StrReplace(partItem.m_desc_cn,"\"","\\\"");
			partItem.m_desc_en=StrReplace(partItem.m_desc_en,"\"","\\\"");
            partItem.m_remarks=StrReplace(partItem.m_remarks,"\"","\\\"");
			

			m_mapAllParts[partItem.m_partid]=partItem;
		}
	}
    m_lastPartTimeStr = GetCurrentTmStr();
	
	m_bpartLoading = false;

	//SYSTEM_LOG("[CAisShip::RefreshShip] end, refresh:%d ============================== ", m_ShipIdMap.size());
 
	RELEASE_MYSQL_RETURN(psql, true);
}


bool eMOPCommDataSvr::_refreshEuqips()
{ 
	MySql *psql = CREATE_MYSQL;
	char sql[512]="SELECT EQUIP_MODEL_ID,MODE,COMPANY_KEY,EQUIP_TYPE_CODE,COMPANY_NAME,`DESC` AS DTXT FROM blm_emop_etl.T50_EMOP_EQUIP_MODEL";

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int rowcount = psql->GetRowCount();
	if(rowcount > 0)
	{

		while(psql->NextRow())
		{
			EquipBaseInfo equipItem;

			READMYSQL_STRING(EQUIP_MODEL_ID, equipItem.m_equipid)
			READMYSQL_STRING(MODE, equipItem.m_model)
			READMYSQL_STRING(COMPANY_KEY, equipItem.m_cmpykey)
			READMYSQL_STRING(EQUIP_TYPE_CODE, equipItem.m_typecode)
			READMYSQL_STRING(COMPANY_NAME, equipItem.m_cmpyname)

			READMYSQL_STRING(DTXT, equipItem.m_desc)


			equipItem.m_model=StrReplace(equipItem.m_model,"\"","\\\"");
			equipItem.m_cmpyname=StrReplace(equipItem.m_cmpyname,"\"","\\\"");
			equipItem.m_desc=StrReplace(equipItem.m_desc,"\"","\\\"");
				
		    m_mapAllEquips[equipItem.m_equipid]=equipItem;
		}
	}
 
	RELEASE_MYSQL_RETURN(psql, true);
}

int eMOPCommDataSvr::InitMaterialPics()
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
 
	sprintf (sql, "SELECT pick_id FROM blm_emop_etl.t50_emop_picture WHERE obj_type='1'");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	string index = "";
	string mid = "";
	while (psql->NextRow())
	{
		READMYSQL_STRING(pick_id, index);
		READMYSQL_STRING(obj_id,mid);
		 MaterialBaseInfo *pMbase = GetMaterialItemById(mid);
		 if(pMbase!=NULL)
			 pMbase->picIds.push_back(index);
		  
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}

int eMOPCommDataSvr::GetPics(MaterialBaseInfo &mbase)
{
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
 
	sprintf (sql, "SELECT pick_id FROM blm_emop_etl.t50_emop_picture WHERE obj_type='1' AND obj_id='%s'",mbase.m_materialId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char index[100] = ""; 
	while (psql->NextRow())
	{
		READMYSQL_STR1(pick_id, index,psql);
		mbase.picIds.push_back(index);
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}

int eMOPCommDataSvr::InitMaterial()
{ 
	if(m_bmaterialLoading)
	{
		return false;
	}
	m_bmaterialLoading = true;

	MySql *psql = CREATE_MYSQL;
	char sql[1024]; 

	sprintf(sql,"SELECT T1.MATERIALS_ID,T1.MATERIALS_CAT_ID,T1.MATERIALS_CODE,T1.NAME_CN,T1.NAME_EN,T1.SPEC_CN,T1.SPEC_EN, \
				T1.LIFE_TIME,T1.APPLY_PERIOD,T1.UNIT_CN,T1.UNIT_EN,T1.STORE_FLAG,T1.REMARK_CN,T1.REMARK_EN, \
				T2.KEY1,T2.KEY2,T2.KEY3,t2.KEY3_DESC AS CNAME_CN,t2.KEY3_DESC_EN AS CNAME_EN,t3.CODE  as CAT_CODE \
				FROM blm_emop_etl.T50_EMOP_MATERIALS T1 \
				LEFT JOIN blm_emop_etl.T50_EMOP_MATERIALS_CATEGORY T2 ON T1.CATEGORY=T2.CATEGORY  \
				LEFT JOIN blm_emop_etl.T50_EMOP_MATERIALS_CAT1 t3 ON T3.KEY1=T2.KEY1 \
				order by T1.MATERIALS_CODE DESC");

	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	while(psql->NextRow())
	{
		MaterialBaseInfo materialItem;

		READMYSQL_STRING(MATERIALS_ID, materialItem.m_materialId);
		READMYSQL_STRING(CATEGORY, materialItem.m_category) ;
		READMYSQL_STRING(KEY1, materialItem.m_key1);
		READMYSQL_STRING(KEY2, materialItem.m_key2);
		READMYSQL_STRING(KEY3, materialItem.m_key3);
		READMYSQL_STRING(CNAME_CN, materialItem.m_catname_cn);
		READMYSQL_STRING(CNAME_EN, materialItem.m_catname_en);


		READMYSQL_STRING(MATERIALS_CODE, materialItem.m_catCode);
		READMYSQL_STRING(NAME_CN, materialItem.m_name_cn);
		READMYSQL_STRING(NAME_EN, materialItem.m_name_en);

		READMYSQL_STRING(SPEC_CN, materialItem.m_spec_cn);
		READMYSQL_STRING(SPEC_EN, materialItem.m_spec_en);
		READMYSQL_STRING(LIFE_TIME, materialItem.m_lifeTime);
		READMYSQL_STRING(APPLY_PERIOD, materialItem.m_applyPeriod);
		READMYSQL_INT(STORE_FLAG, materialItem.m_storeFlag,-1);
		READMYSQL_STRING(REMARK_CN, materialItem.m_remark_cn);
		READMYSQL_STRING(REMARK_EN, materialItem.m_remark_en);
		READMYSQL_STRING(UNIT_CN, materialItem.m_unit_cn);
		READMYSQL_STRING(UNIT_EN, materialItem.m_unit_en);
		READMYSQL_STRING(CAT_CODE, materialItem.m_catCode1);
		 
		materialItem.m_name_cn=StrReplace(materialItem.m_name_cn,"\"","\\\"");
		materialItem.m_name_en=StrReplace(materialItem.m_name_en,"\"","\\\"");
		//StrReplace(equipItem.m_model,"\"","\\\"");
		m_mapAllMaterials[materialItem.m_materialId]=materialItem;
	}
 
	InitMaterialPics(); 

	m_lastMaterialTimeStr = GetCurrentTmStr();

	m_bmaterialLoading = false;
 
	RELEASE_MYSQL_RETURN(psql, true);
}

bool eMOPCommDataSvr::_refreshMaterials()
{ 
	if(m_bmaterialLoading)
	{
		return false;
	}
	m_bmaterialLoading = true;

	MySql *psql = CREATE_MYSQL;
	char sql[1024];
	char whereSql[100]="";
	if(!m_lastMaterialTimeStr.empty())
		sprintf(whereSql,"WHERE T1.OP_DATE>='%s'",m_lastMaterialTimeStr.c_str());

	sprintf(sql,"SELECT T1.MATERIALS_ID,T1.MATERIALS_CAT_ID,T1.MATERIALS_CODE,T1.NAME_CN,T1.NAME_EN,T1.SPEC_CN,T1.SPEC_EN, \
				T1.LIFE_TIME,T1.APPLY_PERIOD,T1.UNIT_CN,T1.UNIT_EN,T1.STORE_FLAG,T1.REMARK_CN,T1.REMARK_EN, \
				T2.KEY1,T2.KEY2,T2.KEY3,t2.KEY3_DESC AS CNAME_CN,t2.KEY3_DESC_EN AS CNAME_EN,t3.CODE  as CAT_CODE \
				FROM blm_emop_etl.T50_EMOP_MATERIALS T1 \
				LEFT JOIN blm_emop_etl.T50_EMOP_MATERIALS_CATEGORY T2 ON T1.CATEGORY=T2.CATEGORY  \
				LEFT JOIN blm_emop_etl.T50_EMOP_MATERIALS_CAT1 t3 ON T3.KEY1=T2.KEY1 \
				%s order by T1.MATERIALS_CODE DESC",whereSql);

	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	while(psql->NextRow())
	{
		MaterialBaseInfo materialItem;

		READMYSQL_STRING(MATERIALS_ID, materialItem.m_materialId);
		READMYSQL_STRING(CATEGORY, materialItem.m_category) ;
		READMYSQL_STRING(KEY1, materialItem.m_key1);
		READMYSQL_STRING(KEY2, materialItem.m_key2);
		READMYSQL_STRING(KEY3, materialItem.m_key3);
		READMYSQL_STRING(CNAME_CN, materialItem.m_catname_cn);
		READMYSQL_STRING(CNAME_EN, materialItem.m_catname_en);


		READMYSQL_STRING(MATERIALS_CODE, materialItem.m_catCode);
		READMYSQL_STRING(NAME_CN, materialItem.m_name_cn);
		READMYSQL_STRING(NAME_EN, materialItem.m_name_en);

		READMYSQL_STRING(SPEC_CN, materialItem.m_spec_cn);
		READMYSQL_STRING(SPEC_EN, materialItem.m_spec_en);
		READMYSQL_STRING(LIFE_TIME, materialItem.m_lifeTime);
		READMYSQL_STRING(APPLY_PERIOD, materialItem.m_applyPeriod);
		READMYSQL_INT(STORE_FLAG, materialItem.m_storeFlag,-1);
		READMYSQL_STRING(REMARK_CN, materialItem.m_remark_cn);
		READMYSQL_STRING(REMARK_EN, materialItem.m_remark_en);
		READMYSQL_STRING(UNIT_CN, materialItem.m_unit_cn);
		READMYSQL_STRING(UNIT_EN, materialItem.m_unit_en);
		READMYSQL_STRING(CAT_CODE, materialItem.m_catCode1);
		GetPics(materialItem);;

		materialItem.m_name_cn=StrReplace(materialItem.m_name_cn,"\"","\\\"");
		materialItem.m_name_en=StrReplace(materialItem.m_name_en,"\"","\\\"");
		//StrReplace(equipItem.m_model,"\"","\\\"");
		m_mapAllMaterials[materialItem.m_materialId]=materialItem;
	}
	m_lastMaterialTimeStr = GetCurrentTmStr();

	m_bmaterialLoading = false;
 
	RELEASE_MYSQL_RETURN(psql, true);
}

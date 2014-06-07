#include "blmcom_head.h"
#include "eMOPMaterialsSvc.h"
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
#include "SequenceManager.h"
#include "eMopCommDataSvr.h"
#include "eMOPEquipSvc.h"
using namespace std;
                                            

IMPLEMENT_SERVICE_MAP(eMOPMaterialsSvc)

eMOPMaterialsSvc::eMOPMaterialsSvc()
{

}

eMOPMaterialsSvc::~eMOPMaterialsSvc()
{

}

bool eMOPMaterialsSvc::Start()
{
	if(!g_MessageService::instance()->RegisterCmd(MID_EMOP_MATERIALS, this))
		return false;

	int interval = 60;
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
	if(timerId <= 0)
		return false;

	SERVICE_MAP(SID_EMOP_MATERIALS_GETCATS,eMOPMaterialsSvc,getMaterialsCatInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_NEWCATS,eMOPMaterialsSvc,NewMaterialsCatInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_UDCATS,eMOPMaterialsSvc,UpdateMaterialsCatInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_DELCATS,eMOPMaterialsSvc,DelMaterialsCatInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_GET,eMOPMaterialsSvc,getCatMaterialsInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_NEW,eMOPMaterialsSvc,NewMaterialsInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_UPDATE,eMOPMaterialsSvc,UpdateMaterialsInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_DELETE,eMOPMaterialsSvc,DelMaterialsInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_SEARCH,eMOPMaterialsSvc,SearchMaterialsInfo);

	SERVICE_MAP(SID_EMOP_MATERIALS_GETSHIPCATS,eMOPMaterialsSvc,getShipMCatInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_SHIPGET,eMOPMaterialsSvc,getShipMaterialsInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_SHIPNEW,eMOPMaterialsSvc,NewShipMaterialsInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_SHIPUPDATE,eMOPMaterialsSvc,UpdateShipMaterialsInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_SHIPDELETE,eMOPMaterialsSvc,DelShipMaterialsInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_SHIPSEARCH,eMOPMaterialsSvc,SearchShipMaterialsInfo);
	//SERVICE_MAP(SID_INSERT_EQUIMENT_MEMORY,eMOPMaterialsSvc,insertEquimentMemory);
	SERVICE_MAP(SID_EMOP_BASEDATA_GETL,eMOPMaterialsSvc,GetBaseData_l);
	SERVICE_MAP(SID_EMOP_BASEDATA_GETR,eMOPMaterialsSvc,GetBaseData_r);
	SERVICE_MAP(SID_EMOP_BASEDATA_NEW,eMOPMaterialsSvc,NewBaseData);
	SERVICE_MAP(SID_EMOP_BASEDATA_DEL,eMOPMaterialsSvc,DelBaseData);

	SERVICE_MAP(0xf5,eMOPMaterialsSvc,GetStorageSubNodeList);
	SERVICE_MAP(0xf6,eMOPMaterialsSvc,GetStorageTreeList);
	SERVICE_MAP(0xf7,eMOPMaterialsSvc,NewModfiyStorageNode);
	SERVICE_MAP(0xf8,eMOPMaterialsSvc,DeleteStorageNode);
	SERVICE_MAP(0xf9,eMOPMaterialsSvc,RemarkManager);
	SERVICE_MAP(0xfa,eMOPMaterialsSvc,AddAttach);
	SERVICE_MAP(0xfb,eMOPMaterialsSvc,DeleteAttach);
	SERVICE_MAP(0xfc,eMOPMaterialsSvc,GetAttach);

	//SERVICE_MAP(SID_DEAL_USERAPPLY,eMOPMaterialsSvc,DealWithUserApplication);
	SERVICE_MAP(0x13,eMOPMaterialsSvc,GetAllMterialsTmp);
	SERVICE_MAP(0x14,eMOPMaterialsSvc,GetMterialsTmpInfo);
	SERVICE_MAP(0x15,eMOPMaterialsSvc,NewMterialsTmpName);
	SERVICE_MAP(0x16,eMOPMaterialsSvc,ModifyMterialsTmpName);
	SERVICE_MAP(0x17,eMOPMaterialsSvc,DelMterialsTmp);
	SERVICE_MAP(0x18,eMOPMaterialsSvc,AddMterialsToTmp);
	SERVICE_MAP(0x19,eMOPMaterialsSvc,ModifyMterialsOnTmp);
	SERVICE_MAP(0x1a,eMOPMaterialsSvc,DelMterialsOnTmp);
	//SERVICE_MAP(SID_GET_EQUIPMENT_PICT,eMOPMaterialsSvc,getEqPict);
	//SERVICE_MAP(SID_DEL_EQUIPMENT_PICT,eMOPMaterialsSvc,delEqPict);
	//material
	SERVICE_MAP(SID_EMOP_MATERIALS_APPLIST,eMOPMaterialsSvc,getAppList);
	SERVICE_MAP(SID_EMOP_MATERIALS_APPMAININFO,eMOPMaterialsSvc,getAppMainInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_APPSUBINFO,eMOPMaterialsSvc,getAppSubInfo);
	SERVICE_MAP(SID_EMOP_MATERIALS_APPSTATUS,eMOPMaterialsSvc,getAPPStatus);
	SERVICE_MAP(SID_EMOP_MATERIALS_INSERTAPPMAININFO,eMOPMaterialsSvc,insertAppMainIfon);
	SERVICE_MAP(SID_EMOP_MATERIALS_ADDSTOREMATERIAL,eMOPMaterialsSvc,insertAppMaterialFromStore);
	SERVICE_MAP(SID_EMOP_MATERIALS_GETMATERIALDETIAL,eMOPMaterialsSvc,getAppMaterialDetial);
	SERVICE_MAP(SID_EMOP_MATERIALS_INSERTMATERIALDETIAL,eMOPMaterialsSvc,insertAppMaterialDetial);
	SERVICE_MAP(SID_EMOP_MATERIALS_INSERTAPPSTATUS,eMOPMaterialsSvc,insertAPPStatus);
	SERVICE_MAP(SID_EMOP_MATERIALS_INSERTAPPMATERIALNUM,eMOPMaterialsSvc,insertAppMaterialNum);
	SERVICE_MAP(SID_EMOP_MATERIALS_INSERTINQUIRYSTATUS,eMOPMaterialsSvc,insertInquiryStatus);
	SERVICE_MAP(SID_EMOP_MATERIALS_PROVIDERLIST,eMOPMaterialsSvc,getProviderList);
	SERVICE_MAP(SID_EMOP_MATERIALS_APPLYCONFIRM,eMOPMaterialsSvc,applyConfirm);
	SERVICE_MAP(SID_EMOP_MATERIALS_GENPRICEINQUIRY,eMOPMaterialsSvc,generatePriceInquiry);
	SERVICE_MAP(SID_EMOP_MATERIALS_PRICEINQUIRYLIST,eMOPMaterialsSvc,PriceInquiryList);
	SERVICE_MAP(SID_EMOP_MATERIALS_PRICEINQUIRYMAIN,eMOPMaterialsSvc,PriceInquiryMain);
	SERVICE_MAP(SID_EMOP_MATERIALS_PRICEINQUIRYSUB,eMOPMaterialsSvc,PriceInquirySub);
	SERVICE_MAP(SID_EMOP_MATERIALS_GENPRICEINQUIRYQOUTED,eMOPMaterialsSvc,generatePriceInquiryQuoted);
	SERVICE_MAP(SID_EMOP_MATERIALS_SENDINQUIRYQOUTEDMATERIAL,eMOPMaterialsSvc,sendPriceInquiryQuotedMaterial);
	SERVICE_MAP(SID_EMOP_MATERIALS_PRICEINQUIRYPRO,eMOPMaterialsSvc,getPriceInquiryProcess);
	SERVICE_MAP(SID_EMOP_MATERIALS_INQUIRYQOUTEDHISTORY,eMOPMaterialsSvc,getPriceInquiryQuotedHistory);

	SERVICE_MAP(SID_EMOP_MATERIALS_REQINQUIRYBYPURCHASE,eMOPMaterialsSvc,reqInquiryByPurchase);
	SERVICE_MAP(SID_EMOP_MATERIALS_REQMATERIALBYINQUIRY,eMOPMaterialsSvc,reqMaterialByInquiry);
	SERVICE_MAP(SID_EMOP_MATERIALS_REQMATERIALBYPURCHASE,eMOPMaterialsSvc,reqMaterialByPurchase);
	SERVICE_MAP(SID_EMOP_MATERIALS_REQINQUIRYBYMATERIAL,eMOPMaterialsSvc,reqInquiryByMaterial);
	SERVICE_MAP(SID_EMOP_MATERIALS_REQORDERDATA,eMOPMaterialsSvc,reqOrderData);
	SERVICE_MAP(SID_EMOP_MATERIALS_GENPURCHASEORDER,eMOPMaterialsSvc,generatePurchaseOrders);
	SERVICE_MAP(SID_EMOP_MATERIALS_GENPURCHASEORDERDETAIL,eMOPMaterialsSvc,generatePurchaseOrderDetail);
	SERVICE_MAP(SID_EMOP_MATERIALS_APPLISTAFTERSHIPSIGN,eMOPMaterialsSvc,getAppListAfterShipSign);
	SERVICE_MAP(SID_EMOP_MATERIALS_APPMAINAFTERSHIPSIGN,eMOPMaterialsSvc,getAppMainInfoAfterShipSign);
	SERVICE_MAP(SID_EMOP_MATERIALS_APPSUBAFTERSHIPSIGN,eMOPMaterialsSvc,getAppSubInfoAfterShipSign);
	SERVICE_MAP(SID_EMOP_MATERIALS_APPSTATUSAFTERSHIPSIGN,eMOPMaterialsSvc,getAppStatusAfterShipSign);

	SERVICE_MAP(SID_EMOP_MATERIALS_ORDERLIST,eMOPMaterialsSvc,getOrderList);
	SERVICE_MAP(SID_EMOP_MATERIALS_ORDERMAIN,eMOPMaterialsSvc,getOrderMain);
	SERVICE_MAP(SID_EMOP_MATERIALS_ORDERSUB,eMOPMaterialsSvc,getOrderSub);
	SERVICE_MAP(SID_EMOP_MATERIALS_ORDERPROCESS,eMOPMaterialsSvc,getOrderPro);
	SERVICE_MAP(SID_EMOP_MATERIALS_ORDERCONFIRM,eMOPMaterialsSvc,sendOrderConfirm);
	SERVICE_MAP(SID_EMOP_MATERIALS_ORDERMATERIALNUM,eMOPMaterialsSvc,sendOrderMaterialNum);
	SERVICE_MAP(SID_EMOP_MATERIALS_CHANGEORDERSTATUS,eMOPMaterialsSvc,changeOrderStatus);
	SERVICE_MAP(SID_EMOP_MATERIALS_CHANGEAPPSTATUS,eMOPMaterialsSvc,changeAppStatus);

	SERVICE_MAP(SID_EMOP_MATERIALS_INSERTAPPSTATUSFORCOMMENT,eMOPMaterialsSvc,insertAPPStatusForComment);


	SERVICE_MAP(0x50,eMOPMaterialsSvc,GetInstockOrderList);
	SERVICE_MAP(0x51,eMOPMaterialsSvc,GetInstockOrderMaterialList);
	SERVICE_MAP(0x52,eMOPMaterialsSvc,DeleteInstockOrderMaterialItem);
	SERVICE_MAP(0x53,eMOPMaterialsSvc,ConformMaterialInstockOrder);
	SERVICE_MAP(0x54,eMOPMaterialsSvc,GetPurchaseOrderList);
	SERVICE_MAP(0x55,eMOPMaterialsSvc,GetPurchaseOrderMaterialList);
	SERVICE_MAP(0x56,eMOPMaterialsSvc,SetMaterialInstockOrder);

	SERVICE_MAP(0x57,eMOPMaterialsSvc,GetOutstockOrderList);
	SERVICE_MAP(0x58,eMOPMaterialsSvc,GetOutstockOrderMaterialList);
	SERVICE_MAP(0x59,eMOPMaterialsSvc,SetMaterialOutstockOrder);
	SERVICE_MAP(0x5a,eMOPMaterialsSvc,DeleteOutstockOrderMaterialItem);
	SERVICE_MAP(0x5b,eMOPMaterialsSvc,ConformMaterialOutstockOrder);
	SERVICE_MAP(0x5c,eMOPMaterialsSvc,GetMaterialItemInStockList);

	SERVICE_MAP(0x60,eMOPMaterialsSvc,GetMaterialInventoryCounting);
	SERVICE_MAP(0x61,eMOPMaterialsSvc,SetMaterialInventoryCount);

	SERVICE_MAP(0x10,eMOPMaterialsSvc,GetMlOrderHis_Ship);
	SERVICE_MAP(0x11,eMOPMaterialsSvc,GetMlStoreHis_Ship);
	SERVICE_MAP(0x12,eMOPMaterialsSvc,GetMlStockDetail_Ship);
	SERVICE_MAP(0x1d,eMOPMaterialsSvc,GetMlOutstockStat_Ship);
	SERVICE_MAP(0x1e,eMOPMaterialsSvc,GetMlIOstockStat_Ship);
	SERVICE_MAP(0x1f,eMOPMaterialsSvc,GetMlInstockStat_Ship);


	DEBUG_LOG("[eMOPMaterialsSvc::Start] OK......................................");
	return true;
}
int eMOPMaterialsSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{

	return 0;
}


//¿âÎ»¹ÜÀí ÇëÇóÊ÷½ÚµãµÄ×Ó½ÚµãÊý¾Ý 0xf5
int eMOPMaterialsSvc::GetStorageSubNodeList(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetStorageSubNodeList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string shipId=root.getv("shipid", "");
	string nodeId=root.getv("id", "");
	int level=root.getv("type", -1);
	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";

	if (nodeId.empty()) //ÇëÇó¼×°å²ã
	{
		sprintf(sql,"select LOCATION_ID,NAME from blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
					where shipid='%s' and room is NULL",shipId.c_str());
	}
	else
	{

		string andStr="";

		if (level==1)
		{
			andStr="deck='%s' and room is not null and shelf is null";

		}
		else if (level==2)
		{
			andStr="room='%s' and shelf is not null and layer is null";
		}
		else if (level==3)
		{
			andStr="shelf='%s' and layer is not null and box is null";
		}
		else if (level==4)
		{
			andStr="layer='%s' and box is not null";
		}
		else
		{
			andStr="box='%s'";
		}

		string sqlStr="select LOCATION_ID,NAME from blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION where shipid='%s' and "+andStr;
		sprintf(sql,sqlStr.c_str(),shipId.c_str(),nodeId.c_str());
	}


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	out <<"{eid:0,type:"<<level<<",seq:\""<<strSeq<<"\",info:[";	

	char id[64]="";
	char name[128]="";


	int cnt = 0;
	while (psql->NextRow())
	{

		READMYSQL_STR(LOCATION_ID, id)
			READMYSQL_STR(NAME, name)

			if (cnt++)
			{
				out << ",";
			}
			out << "{id:\"" << id <<"\",label:\"" << name<<"\"}";

	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//¿âÎ»¹ÜÀí ¸ù¾ÝÄ³³éÌëºÅ½Úµã»ñÈ¡Õû¸öÊ÷½á¹¹ 0xf6
int eMOPMaterialsSvc::GetStorageTreeList(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetStorageTreeList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string shipId=root.getv("shipid", "");
	string nodeId=root.getv("locatecode", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"select LOCATION_ID,DECK,ROOM,SHELF,LAYER,BOX,NAME \
				FROM blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
				WHERE SHIPID='%s' and LOCATION_ID='%s'",shipId.c_str(),nodeId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	vector<StorageItem> m_vecDecks;
	vector<StorageItem> m_vecRooms;
	vector<StorageItem> m_vecShelfs;
	vector<StorageItem> m_vecLayers;
	vector<StorageItem> m_vecBoxs;
	int m_idxs[5]={0,0,0,0,0};

	if (psql->NextRow())
	{
		char deckid[64]="";
		char roomid[64]="";
		char shelfid[64]="";
		char layerid[64]="";
		char boxid[64]="";

		READMYSQL_STR(DECK, deckid)
			READMYSQL_STR(ROOM, roomid)
			READMYSQL_STR(SHELF, shelfid)
			READMYSQL_STR(LAYER, layerid)
			READMYSQL_STR(BOX, boxid)


			//¼×°å
			char sql1[512]="";

		sprintf(sql1,"select LOCATION_ID,NAME FROM blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
					 WHERE SHIPID='%s' and room is NULL",shipId.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

		int index=0;
		while(psql->NextRow())
		{
			StorageItem item;
			item.level=0;
			READMYSQL_STRING(LOCATION_ID, item.id)
				READMYSQL_STRING(NAME, item.name)
				m_vecDecks.push_back(item);
			if (item.id==string(deckid))
			{
				m_idxs[0]=index;
			}
			index++;
		}

		//·¿¼äROOM

		char sql2[512]="";

		sprintf(sql2,"select LOCATION_ID,NAME FROM blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
					 WHERE SHIPID='%s' and deck='%s' and room is not null and shelf is null",shipId.c_str(),deckid);

		CHECK_MYSQL_STATUS(psql->Query(sql2), 3);
		index=0;

		while(psql->NextRow())
		{
			StorageItem item;
			item.level=1;
			READMYSQL_STRING(LOCATION_ID, item.id)
				READMYSQL_STRING(NAME, item.name)
				m_vecRooms.push_back(item);
			if (item.id==string(roomid))
			{
				m_idxs[1]=index;
			}
			index++;
		}

		//»ú¹ñ¼Ü

		char sql3[512]="";

		sprintf(sql3," select LOCATION_ID,NAME FROM blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
					 WHERE SHIPID='%s' and room='%s' and shelf is not null and layer is null ",shipId.c_str(),roomid);

		CHECK_MYSQL_STATUS(psql->Query(sql3), 3);
		index=0;

		while(psql->NextRow())
		{
			StorageItem item;
			item.level=2;
			READMYSQL_STRING(LOCATION_ID, item.id)
				READMYSQL_STRING(NAME, item.name)
				m_vecShelfs.push_back(item);
			if (item.id==string(shelfid))
			{
				m_idxs[2]=index;
			}
			index++;
		}


		//»ú¹ñ²ã
		char sql4[512]="";
		sprintf(sql4," select LOCATION_ID,NAME FROM blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
					 WHERE SHIPID='%s' and shelf='%s' and layer is not null and box is null ",shipId.c_str(),shelfid);

		CHECK_MYSQL_STATUS(psql->Query(sql4), 3);
		index=0;

		while(psql->NextRow())
		{
			StorageItem item;
			item.level=3;
			READMYSQL_STRING(LOCATION_ID, item.id)
				READMYSQL_STRING(NAME, item.name)
				m_vecLayers.push_back(item);
			if (item.id==string(layerid))
			{
				m_idxs[3]=index;
			}
			index++;
		}

		//»ú¹ñºÐ
		char sql5[512]="";
		sprintf(sql5," select LOCATION_ID,NAME FROM blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
					 WHERE SHIPID='%s' and layer='%s' and box is not null ",shipId.c_str(),layerid);

		CHECK_MYSQL_STATUS(psql->Query(sql5), 3);
		index=0;

		while(psql->NextRow())
		{
			StorageItem item;
			item.level=4;
			READMYSQL_STRING(LOCATION_ID, item.id)
				READMYSQL_STRING(NAME, item.name)
				m_vecBoxs.push_back(item);

			if (item.id==string(boxid))
			{
				m_idxs[4]=index;
			}
			index++;
		}

		out <<"{eid:0,seq:\""<<strSeq<<"\",level:[";

		for (int i=0;i<m_vecDecks.size();i++)
		{
			if (i>0)
		 {
			 out<<",";
		 }
			out<<"{id:\"" << m_vecDecks[i].id << "\",label:\"" << m_vecDecks[i].name<<"\"}";
		}
		out<<"],room:[";

		for (int i=0;i<m_vecRooms.size();i++)
		{
			if (i>0)
		 {
			 out<<",";
		 }
			out<<"{id:\"" << m_vecRooms[i].id << "\",label:\"" << m_vecRooms[i].name<<"\"}";
		}
		out<<"],shelf:[";
		for (int i=0;i<m_vecShelfs.size();i++)
		{
			if (i>0)
		 {
			 out<<",";
		 }
			out<<"{id:\"" << m_vecShelfs[i].id << "\",label:\"" << m_vecShelfs[i].name<<"\"}";
		}  
		out<<"],floor:[";
		for (int i=0;i<m_vecLayers.size();i++)
		{
			if (i>0)
		 {
			 out<<",";
		 }
			out<<"{id:\"" << m_vecLayers[i].id << "\",label:\"" << m_vecLayers[i].name<<"\"}";
		}  
		out<<"],drawer:[";
		for (int i=0;i<m_vecBoxs.size();i++)
		{
			if (i>0)
		 {
			 out<<",";
		 }
			out<<"{id:\"" << m_vecBoxs[i].id << "\",label:\"" << m_vecBoxs[i].name<<"\"}";
		}
		out<<"],selected:[";

		for (int i=0;i<5;i++)
		{
			if (i>0)
		 {
			 out<<",";
		 }
			out<<m_idxs[i];
		}
		out<<"]}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}
//¿âÎ»¹ÜÀí Ìí¼Ó¡¢ÐÞ¸Ä¿âÎ»½Úµã 0xf7
int eMOPMaterialsSvc::NewModfiyStorageNode(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::NewModfiyStorageNode]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string shipId=root.getv("shipid", "");
	string nameStr=root.getv("name", "");
	string nodeId=root.getv("id", "");
	int type=root.getv("type", -1);
	string parenId=root.getv("pid", "");

	MySql* psql = CREATE_MYSQL;

	if (type==1)//ÐÂÔö
	{

		char sql[512]="";
		string sequence="";	

		if (parenId.empty())
		{
			GET_EMOP_SEQUENCE_STRING(DK,sequence)	
				sequence="DK"+sequence;
			sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION (LOCATION_ID,SHIPID,DECK,NAME) \
						VALUES('%s','%s','%s','%s')",sequence.c_str(),shipId.c_str(),sequence.c_str(),nameStr.c_str());
		}
		else 
		{

			char sqlsel[512]="";

			sprintf(sqlsel,"select LOCATION_ID,DECK,ROOM,SHELF,LAYER,BOX,NAME \
						   FROM blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
						   WHERE SHIPID='%s' and LOCATION_ID='%s' ",shipId.c_str(),parenId.c_str());

			CHECK_MYSQL_STATUS(psql->Query(sqlsel), 3);

			if (psql->NextRow())
			{

				char deckid[64]="";
				char roomid[64]="";
				char shelfid[64]="";
				char layerid[64]="";
				char boxid[64]="";

				READMYSQL_STR(DECK, deckid)
					READMYSQL_STR(ROOM, roomid)
					READMYSQL_STR(SHELF, shelfid)
					READMYSQL_STR(LAYER, layerid)
					READMYSQL_STR(BOX, boxid)

					if (strlen(roomid)==0)
					{
						GET_EMOP_SEQUENCE_STRING(RM,sequence)
							sequence="RM"+sequence;
						sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION (LOCATION_ID,SHIPID,DECK,ROOM,NAME) \
									VALUES('%s','%s','%s','%s','%s')",sequence.c_str(),shipId.c_str(),deckid,sequence.c_str(),nameStr.c_str());
					}
					else if (strlen(shelfid)==0)
					{
						GET_EMOP_SEQUENCE_STRING(SF,sequence)	
							sequence="SF"+sequence;
						sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION (LOCATION_ID,SHIPID,DECK,ROOM,SHELF,NAME) \
									VALUES('%s','%s','%s','%s','%s','%s')",sequence.c_str(),shipId.c_str(),deckid,roomid,sequence.c_str(),nameStr.c_str());
					}
					else if (strlen(layerid)==0)
					{
						GET_EMOP_SEQUENCE_STRING(LR,sequence)
							sequence="LR"+sequence;
						sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION (LOCATION_ID,SHIPID,DECK,ROOM,SHELF,LAYER,NAME) \
									VALUES('%s','%s','%s','%s','%s','%s','%s')",sequence.c_str(),shipId.c_str(),deckid,roomid,shelfid,sequence.c_str(),nameStr.c_str());
					}
					else //if (strlen(boxid)==0)
					{
						GET_EMOP_SEQUENCE_STRING(BX,sequence)
							sequence="BX"+sequence;

						sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION (LOCATION_ID,SHIPID,DECK,ROOM,SHELF,LAYER,BOX,NAME) \
									VALUES('%s','%s','%s','%s','%s','%s','%s','%s')",sequence.c_str(),shipId.c_str(),deckid,roomid,shelfid,layerid,sequence.c_str(),nameStr.c_str());
					}
			}

		}

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		out<<"{eid:0,type:"<<type<<",seq:\""<<strSeq<<"\",id:\""<<sequence<<"\"}";

	}
	else if (type==2)//ÐÞ¸Ä
	{
		char sql[512]="";

		sprintf(sql,"update blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
					set name='%s' where SHIPID='%s' and LOCATION_ID='%s'",nameStr.c_str(),shipId.c_str(),nodeId.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		out<<"{eid:0,type:"<<type<<",seq:\""<<strSeq<<"\",id:\""<<nodeId<<"\"}";
	}


	RELEASE_MYSQL_RETURN(psql, 0);

}
//¿âÎ»¹ÜÀí É¾³ý¿âÎ»½Úµã 0xf8
int eMOPMaterialsSvc::DeleteStorageNode(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DeleteStorageNode]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string shipId=root.getv("shipid", "");
	string nodeId=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sqlsel[512]="";

	sprintf(sqlsel,"select LOCATION_ID,DECK,ROOM,SHELF,LAYER,BOX,NAME \
				   FROM blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION \
				   WHERE SHIPID='%s' and LOCATION_ID='%s' ",shipId.c_str(),nodeId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sqlsel), 3);

	if (psql->NextRow())
	{

		char deckid[64]="";
		char roomid[64]="";
		char shelfid[64]="";
		char layerid[64]="";
		char boxid[64]="";

		READMYSQL_STR(DECK, deckid)
			READMYSQL_STR(ROOM, roomid)
			READMYSQL_STR(SHELF, shelfid)
			READMYSQL_STR(LAYER, layerid)
			READMYSQL_STR(BOX, boxid)

			char sql[512]="";

		if (nodeId==string(deckid))
		{
			sprintf(sql,"delete from blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION where SHIPID='%s' and DECK='%s' ",shipId.c_str(),nodeId.c_str());
		}
		else if (nodeId==string(roomid))
		{
			sprintf(sql,"delete from blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION where SHIPID='%s' and ROOM='%s' ",shipId.c_str(),nodeId.c_str());
		}
		else if (nodeId==string(shelfid))
		{
			sprintf(sql,"delete from blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION where SHIPID='%s' and SHELF='%s' ",shipId.c_str(),nodeId.c_str());
		}
		else if (nodeId==string(layerid))
		{
			sprintf(sql,"delete from blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION where SHIPID='%s' and LAYER='%s' ",shipId.c_str(),nodeId.c_str());
		}
		else
		{
			sprintf(sql,"delete from blm_emop_etl.T50_EMOP_SHIP_INVENTORY_LOCATION where SHIPID='%s' and LOCATION_ID='%s' ",shipId.c_str(),nodeId.c_str());
		}


		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		out<<"{eid:0,seq:\""<<strSeq<<"\",id:\""<<nodeId<<"\"}";

	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

//附件管理******* 
int eMOPMaterialsSvc:: AddAttach(const char* pUid, const char* jsonString, std::stringstream& out)
{ 
	  //请求接口 {seq:"",type:1, pid:"",annex:[{name:"附件名称",file:"文件服务器唯一标识"},{},{}....]}  
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::AddAttach]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL; 
	char sql[1024]=""; 

	string strSeq = root.getv("seq", "");
	string strPid = root.getv("pid", "");
	int opertype= root.getv("type", 0); 
	Json *child1=root.get("annex");  

	string datestr=GetCurrentTmStr();
	sprintf(sql,"SELECT MAX(ATTA_NO) FROM blm_emop_etl.t41_emop_attachment WHERE \
				   OBJ_ID='%s' AND OBJ_TYPE=%d",strPid.c_str(),opertype);

	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int max_no=0;
	if(psql->NextRow())
		READMYSQL_INT(MAX(ATTA_NO), max_no,0); 

	int iSize=child1->size();
	string strname;
	string strfile;
	string ids="";
	char attid[1024]=""; 
	for(int i=0;i<iSize;i++)
	{
		Json* para = child1->get(i);
		strname=para->getv("name","");
		strfile=para->getv("file","");  

		++max_no;
		sprintf(attid, "%d-%s-%d",opertype,strPid.c_str(),max_no);

		if(i>0)
			ids +="|";

		ids+=attid;
		sprintf(sql,"INSERT  blm_emop_etl.t41_emop_attachment(ATTA_ID,OBJ_ID,OBJ_TYPE,ATTA_NO,ATTA_NAME,ATTA_DATE,ATTA_OPERATOR,REL_ATTA_ID)\
					VALUES('%s','%s',%d,%d,'%s','%s','%s','%s')",
					attid,strPid.c_str(),opertype,max_no,strname.c_str(),datestr.c_str(),pUid,strfile.c_str());
        DEBUG_LOG(sql);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}	 

	out<<"{seq:\""<<strSeq<<"\",eid:0,type:"<<opertype<<",pid:\""<<strPid<<"\",axids:\""<<ids<<"\"}"; 
	// 返回接口 {seq:"",eid:0,type:, pid:"",axids:"id1|id2|id3|...."}
	RELEASE_MYSQL_RETURN(psql, 0);
}
int eMOPMaterialsSvc:: DeleteAttach(const char* pUid, const char* jsonString, std::stringstream& out)
{
	/* 请求接口 {seq:"",type:1, pid:"", axid:""};  
             返回接口 {seq:"",eid:0,type:1}*/
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DeleteAttach]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	int opertype= root.getv("type", 0); 
	string strPid = root.getv("pid", "");
	string axid = root.getv("axid");

	MySql* psql = CREATE_MYSQL; 
	char sql[1024]="";  
	sprintf(sql,"DELETE FROM  blm_emop_etl.t41_emop_attachment WHERE ATTA_ID='%s'",axid.c_str()); 
	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);  

	out<<"{seq:\""<<strSeq<<"\",eid:0,type:"<<opertype<<"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
int eMOPMaterialsSvc:: GetAttach(const char* pUid, const char* jsonString, std::stringstream& out)
{
	//请求接口{seq:"",type:1,pid:""}
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetAttach]bad format:", jsonString, 1); 

	string strSeq = root.getv("seq", "");
	int opertype= root.getv("type", 0); 
	string strPid = root.getv("pid", "");

	MySql* psql = CREATE_MYSQL; 
	char sql[1024]="";  
	 
	sprintf(sql,"SELECT ATTA_ID,ATTA_NAME,REL_ATTA_ID FROM blm_emop_etl.t41_emop_attachment WHERE OBJ_ID='%s' AND OBJ_TYPE=%d",
		strPid.c_str(),opertype); 
	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	out<<"{seq:\""<<strSeq<<"\",eid:0,type:"<<opertype<<",pid:\""<<strPid<<"\", annex:[";
	int cnt=0;
	char attid[100] = "";
	char attaname[1024] = "";
	char fileId[100] = "";
	while (psql->NextRow())
	{
		READMYSQL_STR(ATTA_ID, attid);
		READMYSQL_STR(ATTA_NAME, attaname);
		READMYSQL_STR(REL_ATTA_ID, fileId);
		
		if (cnt++)
			out << ","; 
		 
		//{axid:"",name:"" ,file:""}
		out<<"{axid:\""<<attid<<"\",name:\""<<attaname<<"\",file:\""<<fileId<<"\"}";
	}

	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int eMOPMaterialsSvc::RemarkManager(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAppList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	int opertype= root.getv("optype", 0);
	int type= root.getv("type", 0);
	string remarkid=root.getv("remarkid", "");
	string objid=root.getv("objid", "");
	string theme=root.getv("theme", "");
	string uname=root.getv("uname", "");
	string content=root.getv("content", "");


	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";
	string datestr=GetCurrentTmStr();

	if (opertype==1)//添加
	{

		sprintf(sql,"INSERT INTO blm_emop_etl.T50_EMOP_BLM_EMOP_REMARK(RMK_ID,OBJ_ID,OBJ_TYPE,RMK_USER,\
					RMK_DT,RMK_SUBJECT,RMK_CONTENT)VALUES('%s','%s',%d,'%s','%s','%s','%s')",
					remarkid.c_str(),objid.c_str(),type,uname.c_str(),datestr.c_str(),theme.c_str(),content.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		out<<"{optype:1,seq:\""<<strSeq<<"\",remarkid:\""<<remarkid<<"\"}";

	}
	else if (opertype==2)//删除
	{
		sprintf(sql,"DELETE FROM blm_emop_etl.T50_EMOP_BLM_EMOP_REMARK WHERE RMK_ID='%s'",remarkid.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		out<<"{optype:2,seq:\""<<strSeq<<"\",remarkid:\""<<remarkid<<"\"}";
	}
	else if (opertype==3)//修改
	{

		sprintf(sql,"UPDATE blm_emop_etl.T50_EMOP_BLM_EMOP_REMARK SET RMK_USER='%s',RMK_DT='%s',RMK_SUBJECT='%s',RMK_CONTENT='%s' \
					WHERE RMK_ID='%s'",uname.c_str(),datestr.c_str(),theme.c_str(),content.c_str(),remarkid.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		out<<"{optype:3,seq:\""<<strSeq<<"\",remarkid:\""<<remarkid<<"\"}";
	}
	else if (opertype==4)
	{

		sprintf(sql, "select RMK_ID,RMK_USER,UNIX_TIMESTAMP(RMK_DT) AS RMK_DT,RMK_SUBJECT,RMK_CONTENT from blm_emop_etl.T50_EMOP_BLM_EMOP_REMARK WHERE OBJ_ID='%s' AND OBJ_TYPE=%d",objid.c_str(),type);

		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		out<<"{optype:4,seq:\""<<strSeq<<"\",data:[";


		char id[80] = "";
		char user[256]="";
		string subject="";
		string content="";
		long rmkdt=0;

		int cnt = 0;

		while (psql->NextRow())
		{
			READMYSQL_STR(RMK_ID, id)
				READMYSQL_STR(RMK_USER, user)
				READMYSQL_INT(RMK_DT, rmkdt,0)
				READMYSQL_STRING(RMK_SUBJECT, subject)
				READMYSQL_STRING(RMK_CONTENT, content)

				if (cnt++)
					out << ",";
			out << "{id:\"" << id << "\",time:" << rmkdt << ",theme:\"" << subject <<"\",uname:\"" << user;
			out	<<"\",content:\"" << content<<"\"}" ;
		}

		out << "]}";

	}


	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60,SID: 0x0020 { {seq:0,uid:"",shipId:0,depart:"",taskid:"",stepid:""}}
//{eid:0 seq: 0, content:[{shipId:??123??,shipName:???????,AppDate:??20131015??,Appid:??X13121??,status:0}, { shipName:???????,AppDate:??20131015??,Appid:??X13121??,status:0]}
int eMOPMaterialsSvc::getAppList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAppList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string shipId = root.getv("shipId", "");
	string uid = root.getv("uid", "");
	string strdepart = root.getv("depart", "");
	string taskid = root.getv("taskid", "");
	string stepid = root.getv("stepid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,content:";

	MySql* psql = CREATE_MYSQL;
	vector<AppInfo> vecAppList;

	char sql[1024] = "";
	if(shipId=="" && strdepart=="")
		sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_applies t1;");
	else
		sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_applies t1 WHERE t1.DEPARTMENT_CODE='%s' AND t1.SHIP_ID='%s' ;",strdepart.c_str(), shipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	// char Appid[100] = "";
	// char AppDate[100] = "";
	char status[100] = "";
	// char depart[100] = "";
	// char ship[100] = "";

	// int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		AppInfo appInfoItem;
		READMYSQL_STRING(MATERIALS_APPLY_ID, appInfoItem.Appid);
		READMYSQL_STRING(OP_DATE, appInfoItem.AppDate);
		READMYSQL_STRING(SHIP_ID, appInfoItem.ship);
		READMYSQL_STRING(DEPARTMENT_CODE, appInfoItem.depart);
		vecAppList.push_back(appInfoItem);
	}
	// out << "]}";
	// string strTmp=out.str();
	//	FILE *pf;
	/*pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);*/

	for (int i=0;i<vecAppList.size();i++){
		AppInfo appInfoItem=vecAppList[i];

		char sql1[512]="";
		sprintf (sql1, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_status t1 WHERE t1.MATERIALS_APPLY_ID='%s' AND t1.WORKFLOW_TASK_ID='%s' AND t1.WORKFLOW_STEP_ID='%s'  ORDER BY OP_DATE DESC LIMIT 1;",appInfoItem.Appid.c_str(), taskid.c_str(), stepid.c_str());
		psql->Query(sql1);
		if (psql->NextRow())
		{
			if (i>0)
			{
				out << ",";
			}
			READMYSQL_STR(STATUS, status);
			out << "{Appid:\"" << appInfoItem.Appid << "\",AppDate:\"" << appInfoItem.AppDate <<"\",status:\"" << status <<"\",shipName:\"" << appInfoItem.ship <<"\",department:\"" << appInfoItem.depart <<"\"}";
		}
	}


	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);

}



//Mid:0x60,SID: 0x0040 {{seq:\"%s\",uid:\"%s\",shipId:\"%s\",depart:\"%s\",taskid:\"%s\",stepid:\"%s\"}}
//{eid:0 seq: 0, content:[{shipId:??123??,shipName:???????,AppDate:??20131015??,Appid:??X13121??,status:0}, { shipName:???????,AppDate:??20131015??,Appid:??X13121??,status:0]}
int eMOPMaterialsSvc::getAppListAfterShipSign(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAppList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string shipId = root.getv("shipId", "");
	string uid = root.getv("uid", "");
	string strdepart = root.getv("depart", "");
	string taskid = root.getv("taskid", "");
	string stepid = root.getv("stepid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,content:";

	char sql[1024] = "";
	MySql* psql = CREATE_MYSQL;
	vector<AppInfo> vecAppList;

	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_applies t1 WHERE t1.DEPARTMENT_CODE='%s' AND t1.SHIP_ID='%s' ;",strdepart.c_str(), shipId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char status[100] = "";
	out << "[";
	while (psql->NextRow())
	{
		AppInfo appInfoItem;
		READMYSQL_STRING(MATERIALS_APPLY_ID, appInfoItem.Appid);
		READMYSQL_STRING(OP_DATE, appInfoItem.AppDate);
		READMYSQL_STRING(SHIP_ID, appInfoItem.ship);
		READMYSQL_STRING(DEPARTMENT_CODE, appInfoItem.depart);
		vecAppList.push_back(appInfoItem);
	}
	for (int i=0;i<vecAppList.size();i++){
		AppInfo appInfoItem=vecAppList[i];

		char sql1[512]="";
		sprintf (sql1, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_status t1 WHERE t1.MATERIALS_APPLY_ID='%s' AND t1.WORKFLOW_TASK_ID='%s' AND t1.WORKFLOW_STEP_ID='%s'  ORDER BY OP_DATE DESC LIMIT 1;",appInfoItem.Appid.c_str(), taskid.c_str(), stepid.c_str());
		psql->Query(sql1);
		if (psql->NextRow())
		{
			if (i>0)
			{
				out << ",";
			}
			READMYSQL_STR(STATUS, status);
			out << "{Appid:\"" << appInfoItem.Appid << "\",AppDate:\"" << appInfoItem.AppDate <<"\",status:\"" << status <<"\",shipName:\"" << appInfoItem.ship <<"\",department:\"" << appInfoItem.depart <<"\"}";
		}
	}

	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}

//Mid:0x60,SID: 0x0021 {seq: 0, Appid: "X123"}
//{ seq: 0, content:{shipName:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â°ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚? Appid:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂX13121ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?status:1,department:0,Appkind:1,internalKind:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?3ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?port:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â§ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?date:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?0131023ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?reason:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â§ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â³ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¯ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â·ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¸ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â}}
int eMOPMaterialsSvc::getAppMainInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAppList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strAppid = root.getv("Appid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,content:{";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_applies WHERE t50_emop_materials_applies.MATERIALS_APPLY_ID='%s'",strAppid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char shipName[100] = "";
	// char status[100] = "";
	char department[100] = "";
	char Appkind[100] = "";
	char internalKind[100] = "";
	char port[100] = "";
	char date[100] = "";
	char reason[100] = "";

	while (psql->NextRow())
	{
		READMYSQL_STR(SHIP_ID, shipName);
		READMYSQL_STR(DEPARTMENT_CODE, department);
		READMYSQL_STR(TYPE, Appkind);
		READMYSQL_STR(INNER_CAT, internalKind);
		READMYSQL_STR(PORT_CODE_ID, port);
		READMYSQL_STR(SUPPLY_DATE, date);
		READMYSQL_STR(APPLY_REASON, reason);
		out << "shipName:\"" << shipName << "\",department:\"" << department <<"\",Appkind:\"" << Appkind <<"\",internalKind:\"" << internalKind <<"\",port:\"" << port <<"\",date:\"" << date <<"\",reason:\"" << reason <<"\"";
	}
	out << "}}";
	RELEASE_MYSQL_RETURN(psql, 0);

}



//Mid:0x60,SID: 0x0041 {seq: 0, Appid: "X123"}
//{ seq: 0, content:{shipName:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â°ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚? Appid:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂX13121ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?status:1,department:0,Appkind:1,internalKind:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?3ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?port:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â§ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?date:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?0131023ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?reason:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â§ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â³ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¯ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â·ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¸ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â}}
int eMOPMaterialsSvc::getAppMainInfoAfterShipSign(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAppList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strAppid = root.getv("Appid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,content:{";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_applies WHERE t50_emop_materials_applies.MATERIALS_APPLY_ID='%s'",strAppid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char shipName[100] = "";
	// char status[100] = "";
	char department[100] = "";
	char Appkind[100] = "";
	char internalKind[100] = "";
	char port[100] = "";
	char date[100] = "";
	char reason[100] = "";

	while (psql->NextRow())
	{
		READMYSQL_STR(SHIP_ID, shipName);
		READMYSQL_STR(DEPARTMENT_CODE, department);
		READMYSQL_STR(TYPE, Appkind);
		READMYSQL_STR(INNER_CAT, internalKind);
		READMYSQL_STR(PORT_CODE_ID, port);
		READMYSQL_STR(SUPPLY_DATE, date);
		READMYSQL_STR(APPLY_REASON, reason);
		out << "shipName:\"" << shipName << "\",department:\"" << department <<"\",Appkind:\"" << Appkind <<"\",internalKind:\"" << internalKind <<"\",port:\"" << port <<"\",date:\"" << date <<"\",reason:\"" << reason <<"\"";
	}
	out << "}}";
	RELEASE_MYSQL_RETURN(psql, 0);

}


//Mid:0x60,SID: 0x0022 {seq: 0, Appid: "X123"}
int eMOPMaterialsSvc::getAppSubInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAppSubInfo]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strAppid = root.getv("Appid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_details JOIN blm_emop_etl.t50_emop_materials WHERE t50_emop_materials_apply_details.MATERIALSS_APPLY_ID='%s' AND t50_emop_materials.MATERIALS_ID =t50_emop_materials_apply_details.MATERIALS_ID ;",strAppid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char appID[100] = "";
	char chnName[100] = "";
	char norm[100] = "";
	int appnum;
	char EngName[100] = "";

	char store[100] = "";
	char unit[100] = "";
	char buyKind[100] = "";
	char materialKind[100] = "";
	char materialNo[100] = "";
	char providerNo[100] = "";
	char dispatchNum[100] = "";
	char location[100] = "";
	char remark[100] = "";


	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALSS_APPLY_ID, appID);
		READMYSQL_STR(MATERIALS_ID, materialNo);
		READMYSQL_STR(MATERIALS_CAT_ID, materialKind);
		READMYSQL_STR(NAME_CN, chnName);
		READMYSQL_STR(NAME_EN, EngName);
		READMYSQL_STR(QUANTITY, store);
		READMYSQL_STR(TYPE, buyKind);
		READMYSQL_STR(STORE_FLAG, location);
		READMYSQL_STR(UNIT_CN, unit);
		READMYSQL_STR(SPEC_CN, norm);
		READMYSQL_STR(REMARK_CN, remark);
		READMYSQL_INT(QUATITY, appnum,0);


		if (cnt++)
			out << ",";
		out << "{appID:\"" << appID << "\",materialNo:\"" << materialNo <<"\",materialKind:\"" << materialKind <<"\",chnName:\"" << chnName <<"\",EngName:\"" << EngName <<"\",location:\"" << location <<"\",norm:\"" << norm <<"\",unit:\"" << unit <<"\",remark:\"" << remark <<"\",appnum:" << appnum <<",store:\"" << store <<"\",buyKind:\"" << buyKind <<"\"}";
	}
	out << "]}";
	string strTmp=out.str();
	RELEASE_MYSQL_RETURN(psql, 0);

}


//Mid:0x60,SID: 0x0042 {seq: 0, Appid: "X123"}
int eMOPMaterialsSvc::getAppSubInfoAfterShipSign(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAppSubInfo]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strAppid = root.getv("Appid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_details JOIN blm_emop_etl.t50_emop_materials WHERE t50_emop_materials_apply_details.MATERIALSS_APPLY_ID='%s' AND t50_emop_materials.MATERIALS_ID =t50_emop_materials_apply_details.MATERIALS_ID ;",strAppid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char appID[100] = "";
	char chnName[100] = "";
	char norm[100] = "";
	int appnum;
	char EngName[100] = "";

	char store[100] = "";
	char unit[100] = "";
	char buyKind[100] = "";
	char materialKind[100] = "";
	char materialNo[100] = "";
	char providerNo[100] = "";
	char dispatchNum[100] = "";
	char location[100] = "";
	char remark[100] = "";


	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALSS_APPLY_ID, appID);
		READMYSQL_STR(MATERIALS_ID, materialNo);
		READMYSQL_STR(MATERIALS_CAT_ID, materialKind);
		READMYSQL_STR(NAME_CN, chnName);
		READMYSQL_STR(NAME_EN, EngName);
		READMYSQL_STR(QUANTITY, store);
		READMYSQL_STR(TYPE, buyKind);
		READMYSQL_STR(STORE_FLAG, location);
		READMYSQL_STR(UNIT_CN, unit);
		READMYSQL_STR(SPEC_CN, norm);
		READMYSQL_STR(REMARK_CN, remark);
		READMYSQL_INT(QUATITY, appnum,0);


		if (cnt++)
			out << ",";
		out << "{appID:\"" << appID << "\",materialNo:\"" << materialNo <<"\",materialKind:\"" << materialKind <<"\",chnName:\"" << chnName <<"\",EngName:\"" << EngName <<"\",location:\"" << location <<"\",norm:\"" << norm <<"\",unit:\"" << unit <<"\",remark:\"" << remark <<"\",appnum:" << appnum <<",store:\"" << store <<"\",buyKind:\"" << buyKind <<"\"}";
	}
	out << "]}";
	string strTmp=out.str();
	RELEASE_MYSQL_RETURN(psql, 0);

}



//0x0023
//{seq:"0",Appid:"%s",mid:",,,",num:1 }
int eMOPMaterialsSvc::insertAppMaterialFromStore(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::insertAppMaterialFromStore]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strAppid = root.getv("Appid", "");
	string strmid= root.getv("mid", "");
	int num= root.getv("num", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"replace into blm_emop_etl.t50_emop_materials_apply_details(MATERIALSS_APPLY_ID, MATERIALS_ID, QUATITY) \
		values ('%s','%s',%d)",strAppid.c_str(),strmid.c_str(),num);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//0x0028
//{seq:"0",Appid:"%s",mid:",,,",num:1 }
int eMOPMaterialsSvc::insertAppMaterialNum(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::insertAppMaterialNum]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strAppid = root.getv("Appid", "");
	string strmid= root.getv("mid", "");
	int num= root.getv("num", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"replace into blm_emop_etl.t50_emop_materials_apply_details(MATERIALSS_APPLY_ID, MATERIALS_ID, QUATITY) \
		values ('%s','%s',%d)",strAppid.c_str(),strmid.c_str(),num);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//Mid:0x61 SID: 0x0026 { seq: 0, Appid: "X123" }
//{ seq: 0, process:[{nm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¼ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¸ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€?,ac:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â§ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â³ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¯Ã?,tm:123232,rm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€?,state:0},{nm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ?,ac:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â°ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤Ã?,tm:123232,rm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€?,state:0},{nm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â§ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ?,ac:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¯ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â»ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã?,tm:123232,rm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€?,state:0}]}}
int eMOPMaterialsSvc::getAPPStatus(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAPPStatus]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strAppid = root.getv("Appid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\","<<"appid:\""<<strAppid.c_str()<<"\",eid:0,process:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_status t1 WHERE \
				  t1.MATERIALS_APPLY_ID='%s' AND t1.WORKFLOW_TASK_ID='31' ",strAppid.c_str());



	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	char nm[100] = "";
	char ac[100] = "";
	char tm[100] = "";
	char rm[100] = "";
	char state[100] = "";
	char stepid[100] = "";
	char taskid[100] = "";

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(OPERATOR, nm);
		// READMYSQL_STR(NAME, ac);
		READMYSQL_STR(WORKFLOW_STEP_ID, stepid);
		READMYSQL_STR(OP_DATE, tm);
		READMYSQL_STR(REMARK_CN, rm);
		READMYSQL_STR(STATUS, state);
		READMYSQL_STR(WORKFLOW_TASK_ID, taskid);
		if (cnt++)
			out << ",";
		out << "{nm:\"" << nm << "\",ac:\"" << ac <<"\",tm:\"" << tm <<"\",rm:\"" << rm <<"\",stepid:\"" << stepid <<"\",taskid:\"" << taskid <<"\",state:\"" << state <<"\"}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x61 SID: 0x0043 { seq: 0, Appid: "X123" }
//{ seq: 0, process:[{nm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¼ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¸ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€?,ac:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â§ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â³ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¯Ã?,tm:123232,rm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€?,state:0},{nm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ?,ac:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â°ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤Ã?,tm:123232,rm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€?,state:0},{nm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â§ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ?,ac:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¯ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â»ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã?,tm:123232,rm:"ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€?,state:0}]}}
int eMOPMaterialsSvc::getAppStatusAfterShipSign(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAppStatusAfterShipSign]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strAppid = root.getv("Appid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,process:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_status t2 WHERE t2.MATERIALS_APPLY_ID='%s' AND ( (t2.WORKFLOW_TASK_ID='31' AND t2.WORKFLOW_STEP_ID='3') OR t2.WORKFLOW_TASK_ID='32') ",strAppid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	char nm[100] = "";
	char ac[100] = "";
	char tm[100] = "";
	char rm[100] = "";
	char state[100] = "";
	char stepid[100] = "";
	char taskid[100] = "";

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(OPERATOR, nm);
		// READMYSQL_STR(NAME, ac);
		READMYSQL_STR(WORKFLOW_STEP_ID, stepid);
		READMYSQL_STR(OP_DATE, tm);
		READMYSQL_STR(REMARK_CN, rm);
		READMYSQL_STR(STATUS, state);
		READMYSQL_STR(WORKFLOW_TASK_ID, taskid);
		if (cnt++)
			out << ",";
		out << "{nm:\"" << nm << "\",ac:\"" << ac <<"\",tm:\"" << tm <<"\",rm:\"" << rm <<"\",stepid:\"" << stepid <<"\",taskid:\"" << taskid <<"\",state:\"" << state <<"\"}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x61 SID: 0x0039 {seq:\"%s\",uid:\"%s\",shipid:\"%s\",depart:\"%s\",status:0}
//{seq:"s1",shipid:"sss",data:[{aid:"s000001",time:"123212232",status:0},{aid:"s000002",time:"123212312",state:"0"}]}'
int eMOPMaterialsSvc::getOrderList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getOrderList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string uid = root.getv("uid", "");
	string strshipid = root.getv("shipid", "");
	string strdepart = root.getv("depart", "");
	int statusint = root.getv("status", 0);


	out<<"{seq:\""<<strSeq.c_str()<<"\",shipid:\""<<strshipid.c_str()<<"\",data:";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT *,t1.STATUS as ST FROM blm_emop_etl.t50_emop_materials_purchase_orders t1,blm_emop_etl.t50_emop_materials_inqueried_quoted t2,blm_emop_etl.t50_emop_materials_apply_inquery t3,blm_emop_etl.t50_emop_materials_applies t4 WHERE t1.MATERIALS_APPLY_INQUERIED_QUOTED_ID = t2.MATERIALS_INQUERIED_QUOTED_ID AND t2.MATERIALS_APPLY_INQUERY_ID = t3.MATERIALS_APPLY_INQUERY_ID AND t3.MATERIALS_APPLY_ID = t4.MATERIALS_APPLY_ID AND t1.STATUS=%d",statusint);
	// FILE *pf;
	// string tem = sql;
	// pf=fopen("/tmp/json1.log","wb");
	// fwrite(tem.c_str(),sizeof(char),tem.length(),pf);
	// fclose(pf);


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char aid[100] = "";
	char shipid[100] = "";
	char depart[100] = "";
	char date[100] = "";
	int status;
	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_PURCHASE_ORDER_ID, aid);
		READMYSQL_STR(SHIP_ID, shipid);
		READMYSQL_STR(DEPARTMENT_CODE, depart);
		READMYSQL_STR(OP_DATE, date);
		READMYSQL_INT(ST, status,0);
		// string stroid = aid;
		// char sql2[1024] = "";
		// sprintf (sql2, "SELECT OP_DATE FROM blm_emop_etl.t50_emop_materials_purchase_order_status t1 WHERE t1.MATERIALS_PURCHASE_ORDER_ID='%s' AND t1.STATUS=0",stroid.c_str());
		// CHECK_MYSQL_STATUS(psql->Query(sql2), 3);
		// READMYSQL_STR(OP_DATE, tm);
		// READMYSQL_STR(REMARK_CN, rm);
		// READMYSQL_STR(STATUS, state);
		// READMYSQL_STR(WORKFLOW_TASK_ID, taskid);
		if (cnt++)
			out << ",";
		out << "{aid:\"" << aid << "\",shipid:\"" << shipid << "\",depart:\"" << depart << "\",date:\"" << date << "\",status:" << status << "}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x61 SID: 0x003A {seq:\"%s\",oid:\"%s\"}
//{seq:"detail0000",data:{id:"shipid123",sid:"123456789ssss",ship:"s001",part:"20",state:"待审核",pid:"1234567s001",eid:"1234567s002",cid:"123456s003",provideShipDate:12343333,confirmDate:20121112,provider:"上海商贸",location:"二炮",currency:"人民币",freight:1000,other:2000,total:3000,serviceFee:200,toRMB:2000}
int eMOPMaterialsSvc::getOrderMain(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getOrderMain]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string oid = root.getv("oid", "");

	char sid[100] = "";//订购单号
	char pid[100] = "";//采购单号
	char eid[100] = "";//询价单号
	char cid[100] = "";//确认单号
	float toRMB;
	float totalFee;
	float otherFee;
	float serviceFee;
	float transportFee;
	int currency;
	char provider[100] = "";
	char purchaseReason[100] = "";
	char departmentName[100] = "";
	char purchaseKind[100] = "";
	char providePort[100] = "";
	char provideDate[100] = "";
	char internalKind[100] = "";
	char shipName[100] = "";
	char provideShipDate[100] = "";
	char confirmDate[100] = "";
	char confirmNo[100] = "";
	char provideLocation[100] = "";

	out<<"{seq:\""<<strSeq.c_str()<<"\",data:";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT *,t2.TOTAL_PRICE AS TP FROM blm_emop_etl.t50_emop_materials_purchase_orders t1,blm_emop_etl.t50_emop_materials_inqueried_quoted t2,blm_emop_etl.t50_emop_materials_apply_inquery t3,blm_emop_etl.t50_emop_materials_applies t4 WHERE t1.MATERIALS_APPLY_INQUERIED_QUOTED_ID = t2.MATERIALS_INQUERIED_QUOTED_ID AND t2.MATERIALS_APPLY_INQUERY_ID = t3.MATERIALS_APPLY_INQUERY_ID AND t3.MATERIALS_APPLY_ID = t4.MATERIALS_APPLY_ID AND t1.MATERIALS_PURCHASE_ORDER_ID='%s'",oid.c_str());


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_PURCHASE_ORDER_ID, sid);
		READMYSQL_STR(MATERIALS_APPLY_ID, pid);
		READMYSQL_STR(MATERIALS_APPLY_INQUERY_ID, eid);
		READMYSQL_STR(QUOTE_CODE, cid);

		READMYSQL_DOUBLE(TOTAL_RMB, toRMB,0);
		READMYSQL_DOUBLE(TP, totalFee,0);
		READMYSQL_DOUBLE(OTHER_EXPENSE, otherFee,0);
		READMYSQL_DOUBLE(SERVICE_EXPENSE, serviceFee,0);
		READMYSQL_DOUBLE(FREIGHT, transportFee,0);
		READMYSQL_INT(CURRENCY, currency,1);
		READMYSQL_STR(DELIVERY_DATE, provideShipDate);
		READMYSQL_STR(DELIVERY_LOCATION, provideLocation);
		READMYSQL_STR(QUOTE_DATE, confirmDate);

		READMYSQL_STR(APPLY_REASON, purchaseReason);
		READMYSQL_STR(INNER_CAT, internalKind);
		READMYSQL_STR(SHIP_ID, shipName);
		READMYSQL_STR(DEPARTMENT_CODE, departmentName);

		READMYSQL_STR(SUPPLY_DATE, provideDate);
		READMYSQL_STR(PORT_CODE_ID, providePort);
		READMYSQL_STR(COMPANY_KEY, provider);
		// READMYSQL_STR(WORKFLOW_TASK_ID, taskid);
		// if (cnt++)
		// 	out << ",";
		out << "{sid:\"" << sid << "\",pid:\"" << pid << "\",eid:\"" << eid << "\",cid:\"" << cid << "\",toRMB:" << toRMB << ",currency:" << currency << ",totalFee:" << totalFee << ",otherFee:" << otherFee << ",serviceFee:" << serviceFee << ",transportFee:" << transportFee << ",provideShipDate:\"" << provideShipDate << "\",provideLocation:\"" << provideLocation << "\",confirmDate:\"" << confirmDate << "\",shipName:\"" << shipName << "\",departmentName:\"" << departmentName << "\",provideDate:\"" << provideDate << "\",providePort:\"" << providePort << "\",provider:\"" << provider << "\"}";
	}
	out << "}";
	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x61 SID: 0x0044 {seq:\"%s\",oid:\"%s\"}
//,material:[{purchaseKind:"2-1232",materialNo:"0x123",materialKind:"亚麻布类",providerNo:"0x213",materialName:"电源1",num:10,unit:"个",norm:100,pr:0.9,store:10,location:10,rm:"备注"}]
int eMOPMaterialsSvc::getOrderSub(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getOrderSub]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string oid = root.getv("oid", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",material:";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_purchase_order_details t1,blm_emop_etl.t50_emop_materials t2 WHERE t1.MATERIALS_PURCHASE_ORDER_ID='%s' AND t1.MATERIALS_ID = t2.MATERIALS_ID ",oid.c_str());


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char purchaseKind[100] = "";
	char materialNo[100] = "";
	char materialKind[100] = "";
	char materialName[100] = "";
	char unit[100] = "";
	char norm[100] = "";
	char rm[100] = "";
	float pr;
	int num;
	int store;



	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		// READMYSQL_STR(MATERIALS_PURCHASE_ORDER_ID, purchaseKind);
		READMYSQL_STR(MATERIALS_ID, materialNo);
		READMYSQL_STR(MATERIALS_CAT_ID, materialKind);
		READMYSQL_STR(NAME_CN, materialName);
		READMYSQL_STR(UNIT_CN, unit);
		// READMYSQL_STR(REMARK_CN, rm);
		READMYSQL_STR(SPEC_CN, norm);
		READMYSQL_INT(QUATITY, num,0);
		READMYSQL_DOUBLE(UNIT_PRICE, pr,0.0);
		// READMYSQL_STR(WORKFLOW_TASK_ID, taskid);
		if (cnt++)
			out << ",";
		out << "{materialNo:\"" << materialNo << "\",materialKind:\"" << materialKind << "\",materialName:\"" << materialName << "\",unit:\"" << unit << "\",rm:\"" << rm << "\",norm:\"" << norm << "\",pr:" << pr << ",num:" << num << "}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//Mid:0x60,SID: 0x0045{ seq:????,inquiryNo,appId,oid} 
//{ seq: 0, process:[{nm:"Ã¥Â¼Â Ã¤Â¸â€?,ac:"Ã§â€Â³Ã¨Â¯Â?,tm:123232,rm:"Ã¥ÂÅ’Ã¦â€žÂ?,state:0},{nm:"Ã¦ÂÅ½Ã¥â€ºâ€?,ac:"Ã¦â€°Â¹Ã¥Â¤Â?,tm:123232,rm:"Ã¥ÂÅ’Ã¦â€žÂ?,state:0},{nm:"Ã§Å½â€¹Ã¤Âºâ€?,ac:"Ã¨Â¯Â¢Ã¤Â»Â·",tm:123232,rm:"Ã¥ÂÅ’Ã¦â€žÂ?,state:0}]}
int eMOPMaterialsSvc::getOrderPro(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getOrderPro]bad format:", jsonString, 1);
	string strTmp;
	int iscontinue = 1;
	string strSeq = root.getv("seq", "");
	string inquiryNo = root.getv("inquiryNo", "");
	string oid = root.getv("oid", "");
	string appId = root.getv("appId", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,process:";
	out << "[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char nm[100] = "";
	char ac[100] = "";
	char tm[100] = "";
	char rm[100] = "";

	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_status WHERE t50_emop_materials_apply_status.MATERIALS_APPLY_ID = '%s' AND t50_emop_materials_apply_status.WORKFLOW_TASK_ID='31' AND t50_emop_materials_apply_status.WORKFLOW_STEP_ID='3'",appId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	strTmp=sql;


	int cnt = 0;
	if(psql->NextRow()){
		READMYSQL_STR(OPERATOR, nm);
		READMYSQL_STR(OP_DATE, tm);
		READMYSQL_STR(REMARK_CN, rm);
	}

	out << "{nm:\"" << nm << "\",ac:\"" << ac << "\",tm:\"" << tm << "\",rm:\"" << rm << "\"}";
	// char sql1[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_status t1 WHERE t1.MATERIALS_APPLY_ID = '%s' AND t1.WORKFLOW_TASK_ID='32'",appId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow()){
		READMYSQL_STR(OPERATOR, nm);
		READMYSQL_STR(OP_DATE, tm);
		READMYSQL_STR(REMARK_CN, rm);
		out << ",{nm:\"" << nm << "\",ac:\"" << ac << "\",tm:\"" << tm << "\",rm:\"" << rm << "\"}";
	}
	else{
		out << "]}";
		iscontinue = 0;
	}
	// char sql2[1024] = "";
	// char nm[100] = "";
	// char ac[100] = "";
	// char tm[100] = "";
	// char rm[100] = "";
	if(iscontinue)
	{	
		sprintf (sql, "SELECT USER_ID, OP_DATE FROM blm_emop_etl.t50_emop_materials_apply_inquery WHERE t50_emop_materials_apply_inquery.MATERIALS_APPLY_INQUERY_ID = '%s' ",inquiryNo.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->NextRow()){
			READMYSQL_STR(USER_ID, nm);
			READMYSQL_STR(OP_DATE, tm);
			out << ",{nm:\"" << nm << "\",ac:\"" << ac << "\",tm:\"" << tm << "\",rm:\"" << rm << "\"}";
		}

	}

	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_purchase_order_status t1 WHERE t1.MATERIALS_PURCHASE_ORDER_ID = '%s' ",oid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow()){
		READMYSQL_STR(OPERATOR, nm);
		READMYSQL_STR(OP_DATE, tm);
		READMYSQL_STR(REMARK_CN, rm);
		out << ",{nm:\"" << nm << "\",ac:\"" << ac << "\",tm:\"" << tm << "\",rm:\"" << rm << "\"}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}



//0x003B
//{seq:"seq",oderID:"WPS000102210000Q10052D10009",date:"2013-12-16",uid:"blm123",totalFee:,taskid:"34",stepid:"1"}
//{ seq: 0, process:[{nm:"Ã¥Â¼Â Ã¤Â¸â€?,ac:"Ã§â€Â³Ã¨Â¯Â?,tm:123232,rm:"Ã¥ÂÅ’Ã¦â€žÂ?,state:0},{nm:"Ã¦ÂÅ½Ã¥â€ºâ€?,ac:"Ã¦â€°Â¹Ã¥Â¤Â?,tm:123232,rm:"Ã¥ÂÅ’Ã¦â€žÂ?,state:0},{nm:"Ã§Å½â€¹Ã¤Âºâ€?,ac:"Ã¨Â¯Â¢Ã¤Â»Â·",tm:123232,rm:"Ã¥ÂÅ’Ã¦â€žÂ?,state:0}]}
int eMOPMaterialsSvc::sendOrderConfirm(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::sendOrderConfirm]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string oderID = root.getv("oderID", "");
	string date = root.getv("date", "");
	string strrm = root.getv("remark", "");
	string taskid = root.getv("taskid", "");
	string stepid = root.getv("stepid", "");
	int state = root.getv("state", 1);
	string uid= root.getv("uid", "");
	float totalFee = root.getv("totalFee", 0.0);


	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "INSERT INTO blm_emop_etl.t50_emop_materials_purchase_order_status(MATERIALS_PURCHASE_ORDER_ID, STATUS, OP_DATE,REMARK_CN,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID)\
				  VALUES('%s',%d,'%s','%s','%s','%s','%s')",oderID.c_str(),state,date.c_str(),strrm.c_str(),uid.c_str(),taskid.c_str(),stepid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	sprintf (sql, "UPDATE blm_emop_etl.t50_emop_materials_purchase_orders SET TOTAL_PRICE=%f,STATUS=1 WHERE MATERIALS_PURCHASE_ORDER_ID='%s'\
				  ",totalFee,oderID.c_str());
	string strTmp=sql;

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	RELEASE_MYSQL_RETURN(psql, 0);
}




//0x0047
//{seq:"seq",oderID:"WPS000102210000Q10052D10009",date:"2013-12-16",uid:"blm123",totalFee:,taskid:"34",stepid:"1"}
//
int eMOPMaterialsSvc::changeOrderStatus(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::changeOrderStatus]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string oderID = root.getv("oderID", "");
	string date = root.getv("date", "");
	string strrm = root.getv("strrm", "");
	string taskid = root.getv("taskid", "");
	string stepid = root.getv("stepid", "");
	int state = root.getv("state", 1);
	string uid= root.getv("uid", "");



	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "INSERT INTO blm_emop_etl.t50_emop_materials_purchase_order_status(MATERIALS_PURCHASE_ORDER_ID, STATUS, OP_DATE,REMARK_CN,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID)\
				  VALUES('%s',%d,'%s','%s','%s','%s','%s')",oderID.c_str(),state,date.c_str(),strrm.c_str(),uid.c_str(),taskid.c_str(),stepid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);



	RELEASE_MYSQL_RETURN(psql, 0);
}




//0x0048
//{seq:"seq",oderID:"WPS000102210000Q10052D10009",date:"2013-12-16",uid:"blm123",totalFee:,taskid:"34",stepid:"1"}
//
int eMOPMaterialsSvc::changeAppStatus(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::changeAppStatus]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string appID = root.getv("appid", "");
	string date = root.getv("date", "");
	string strrm = root.getv("rm", "");
	string taskid = root.getv("taskid", "");
	string stepid = root.getv("stepid", "");
	int state = root.getv("state", 1);
	string uid= root.getv("uid", "");



	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "INSERT INTO blm_emop_etl.t50_emop_materials_apply_status(MATERIALS_APPLY_ID, STATUS, OP_DATE,REMARK_CN,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID)\
				  VALUES('%s',%d,'%s','%s','%s','%s','%s')",appID.c_str(),state,date.c_str(),strrm.c_str(),uid.c_str(),taskid.c_str(),stepid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);



	RELEASE_MYSQL_RETURN(psql, 0);
}

//0x003C
//{seq:"seq",oderID:"WPS000102210000Q10052D10009",materialNo:"M0000000000000000013",num:38}
//
int eMOPMaterialsSvc::sendOrderMaterialNum(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::sendOrderMaterialNum]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string oderID = root.getv("oderID", "");
	string materialNo = root.getv("materialNo", "");
	int num = root.getv("num", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "UPDATE blm_emop_etl.t50_emop_materials_purchase_order_details SET QUATITY='%d' WHERE MATERIALS_PURCHASE_ORDER_ID='%s' AND MATERIALS_ID='%s'\
				  ",num,oderID.c_str(),materialNo.c_str());
	string strTmp=sql;

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}


//0x0029
//{seq:\"%s\",appid:\"%s\",state:\"%s\",nm:\"%s\",ac:\"%s\",tm:\"%s\",rm:\"%s\",stepid:"",taskid:""}
int eMOPMaterialsSvc::insertAPPStatus(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::insertAPPStatus]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strappid = root.getv("appid", "");
	int state = root.getv("state", 0);
	string strnm = root.getv("nm", "");
	string strac= root.getv("ac", "");
	string strtm= root.getv("tm", "");
	string strrm= root.getv("rm", "");
	string strstepid= root.getv("stepid", "");
	string strtaskid= root.getv("taskid", "");
	out<< "{seq:\""<<strSeq.c_str()<<"\","<<"appid:\""<<strappid.c_str()<<"\",eid:0,process:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	//sprintf (sql, "DELEtE FROM blm_emop_etl.t50_emop_materials_apply_status WHERE t50_emop_materials_apply_status.MATERIALS_APPLY_ID='%s' AND t50_emop_materials_apply_status.WORKFLOW_STEP_ID='%s' ",strappid.c_str(),stepid.c_str());
	//CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	sprintf (sql, "INSERT INTO blm_emop_etl.t50_emop_materials_apply_status(MATERIALS_APPLY_ID, STATUS, OP_DATE,REMARK_CN,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID)\
				  VALUES('%s',%d,'%s','%s','%s','%s','%s')",strappid.c_str(),state,strtm.c_str(),strrm.c_str(),strnm.c_str(),strtaskid.c_str(),strstepid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_status t2 WHERE t2.MATERIALS_APPLY_ID='%s' AND ( (t2.WORKFLOW_TASK_ID='31' AND t2.WORKFLOW_STEP_ID='3') OR t2.WORKFLOW_TASK_ID='32') ",strappid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	char nm[100] = "";
	char ac[100] = "";
	char tm[100] = "";
	char rm[100] = "";
	int status;
	char stepid[100] = "";
	char taskid[100] = "";

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(OPERATOR, nm);
		// READMYSQL_STR(NAME, ac);
		READMYSQL_STR(WORKFLOW_STEP_ID, stepid);
		READMYSQL_STR(OP_DATE, tm);
		READMYSQL_STR(REMARK_CN, rm);
		READMYSQL_INT(STATUS, status,0);
		READMYSQL_STR(WORKFLOW_TASK_ID, taskid);
		if (cnt++)
			out << ",";
		out << "{nm:\"" << nm << "\",ac:\"" << ac <<"\",tm:\"" << tm <<"\",rm:\"" << rm <<"\",stepid:\"" << stepid <<"\",taskid:\"" << taskid <<"\",status:\"" << status <<"\"}";
	}
	out << "]}";


	RELEASE_MYSQL_RETURN(psql, 0);
}


//0x0046
//{seq:\"%s\",appid:\"%s\",state:\"%s\",nm:\"%s\",ac:\"%s\",tm:\"%s\",rm:\"%s\",stepid:"",taskid:""}
int eMOPMaterialsSvc::insertAPPStatusForComment(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::insertAPPStatusForComment]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strappid = root.getv("appid", "");
	int state = root.getv("state", 0);
	string strnm = root.getv("nm", "");
	string strac= root.getv("ac", "");
	string strtm= root.getv("tm", "");
	string strrm= root.getv("rm", "");
	string strstepid= root.getv("stepid", "");
	string strtaskid= root.getv("taskid", "");
	out<< "{seq:\""<<strSeq.c_str()<<"\","<<"appid:\""<<strappid.c_str()<<"\",eid:0,process:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	//sprintf (sql, "DELEtE FROM blm_emop_etl.t50_emop_materials_apply_status WHERE t50_emop_materials_apply_status.MATERIALS_APPLY_ID='%s' AND t50_emop_materials_apply_status.WORKFLOW_STEP_ID='%s' ",strappid.c_str(),stepid.c_str());
	//CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	sprintf (sql, "INSERT INTO blm_emop_etl.t50_emop_materials_apply_status(MATERIALS_APPLY_ID, STATUS, OP_DATE,REMARK_CN,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID)\
				  VALUES('%s',%d,'%s','%s','%s','%s','%s')",strappid.c_str(),state,strtm.c_str(),strrm.c_str(),strnm.c_str(),strtaskid.c_str(),strstepid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_status t2 WHERE t2.MATERIALS_APPLY_ID='%s' AND ( (t2.WORKFLOW_TASK_ID='31' AND t2.WORKFLOW_STEP_ID='3') OR t2.WORKFLOW_TASK_ID='32') ",strappid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	char nm[100] = "";
	char ac[100] = "";
	char tm[100] = "";
	char rm[100] = "";
	int status;
	char stepid[100] = "";
	char taskid[100] = "";

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(OPERATOR, nm);
		// READMYSQL_STR(NAME, ac);
		READMYSQL_STR(WORKFLOW_STEP_ID, stepid);
		READMYSQL_STR(OP_DATE, tm);
		READMYSQL_STR(REMARK_CN, rm);
		READMYSQL_INT(STATUS, status,0);
		READMYSQL_STR(WORKFLOW_TASK_ID, taskid);
		if (cnt++)
			out << ",";
		out << "{nm:\"" << nm << "\",ac:\"" << ac <<"\",tm:\"" << tm <<"\",rm:\"" << rm <<"\",stepid:\"" << stepid <<"\",taskid:\"" << taskid <<"\",status:\"" << status <<"\"}";
	}
	out << "]}";


	RELEASE_MYSQL_RETURN(psql, 0);
}


//0x002A
//{seq:\"%s\",appid:\"%s\",state:\"%s\",nm:\"%s\",ac:\"%s\",tm:\"%s\",rm:\"%s\",stepid:"",taskid:""}
int eMOPMaterialsSvc::insertInquiryStatus(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::insertInquiryStatus]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strappid = root.getv("appid", "");
	int state = root.getv("state", 0);
	string strnm = root.getv("nm", "");
	string strac= root.getv("ac", "");
	string strtm= root.getv("tm", "");
	string strrm= root.getv("rm", "");
	string stepid= root.getv("stepid", "");
	string taskid= root.getv("taskid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "DELEtE FROM blm_emop_etl.t50_emop_materials_apply_status WHERE t50_emop_materials_apply_status.MATERIALS_APPLY_ID='%s' AND t50_emop_materials_apply_status.WORKFLOW_STEP_ID='%s' ",strappid.c_str(),stepid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	sprintf (sql, "INSERT INTO blm_emop_etl.t50_emop_materials_apply_status(MATERIALS_APPLY_ID, STATUS, OP_DATE,REMARK_CN,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID)\
				  VALUES('%s',%d,'%s','%s','%s','%s','%s')",strappid.c_str(),state,strtm.c_str(),strrm.c_str(),strnm.c_str(),taskid.c_str(),stepid.c_str());
	// 
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60 SID: 0x0027
//	{seq:\"%s\", appid:\"%s\",shipid:\"%s\",userid:\"%s\",opdate:\"%s\",depart:\"%s\",Appkind:\"%s\",prodate:\"%s\",internalKind:\"%s\",port:\"%s\",reason:\"%s\",state:\"%s\",nm:\"%s\",ac:\"%s\",tm:\"%s\",rm:\"%s\",stepid:\"%s\",taskid:\"%s\",materials:[{mid:\"%s\",num:%d,new:%d},{}]}

int eMOPMaterialsSvc::insertAppMainIfon(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::insertAppMainIfon]bad format:", jsonString, 1);
	//string appid = root.getv("appid", "");
	string depart = root.getv("depart", "");
	string shipid = root.getv("shipid", "");
	string userid = root.getv("userid", "");
	string Appkind = root.getv("Appkind", "");
	string opdate = root.getv("opdate", "");
	string prodate = root.getv("prodate", "");
	string internalKind= root.getv("internalKind", "");
	string port= root.getv("port", "");
	string reason= root.getv("reason", "");
	string strSeq= root.getv("seq", "");

	string strnm = root.getv("nm", "");
	string strac= root.getv("ac", "");
	string strtm= root.getv("tm", "");
	string strrm= root.getv("rm", "");
	string strstepid= root.getv("stepid", "");
	string strtaskid= root.getv("taskid", "");
	int state = root.getv("state", 0);

	string seqno="";
	string appid="";
	GET_EMOP_SEQUENCE_STRING(WP,seqno);
	appid = appid+"WP"+"S"+shipid+seqno;

	out<< "{seq:\""<<strSeq.c_str()<<"\","<<"appid:\""<<appid.c_str()<<"\",eid:0,process:";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//insert app info
	sprintf(sql,\
		"insert into blm_emop_etl.t50_emop_materials_applies(MATERIALS_APPLY_ID, DEPARTMENT_CODE, SHIP_ID, PORT_CODE_ID ,USER_ID,OP_DATE,SUPPLY_DATE,TYPE,INNER_CAT,APPLY_REASON) \
		values ('%s','%s','%s','%s','%s','%s','%s',%d,'%s','%s')",appid.c_str(),depart.c_str(),shipid.c_str(),port.c_str(),userid.c_str(),opdate.c_str(),prodate.c_str(),atoi(Appkind.c_str()),internalKind.c_str(),reason.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	//insert status
	sprintf (sql, "INSERT INTO blm_emop_etl.t50_emop_materials_apply_status(MATERIALS_APPLY_ID, STATUS, OP_DATE,REMARK_CN,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID)\
				  VALUES('%s',%d,'%s','%s','%s','%s','%s')",appid.c_str(),state,strtm.c_str(),strrm.c_str(),strnm.c_str(),strtaskid.c_str(),strstepid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);


	Json* materials = root.get("materials");

	if (materials)
	{
		for (int i=0;i<materials->size();i++)
		{
			Json* Item=materials->get(i);
			if (!Item)
			{
				continue;
			}

			int num=Item->getv("num",0);
			int isnew=Item->getv("new",0);
			string materialid = "";
			if(isnew==1){
				string seqno="";
				GET_EMOP_SEQUENCE_STRING(M,seqno);
				materialid = "M"+seqno;
			}
			else{
				materialid=Item->getv("mid","");
			}




			char sql1[512]="";
			sprintf(sql1,\
				"replace into blm_emop_etl.t50_emop_materials_apply_details(MATERIALSS_APPLY_ID, MATERIALS_ID, QUATITY) \
				values ('%s','%s',%d)",appid.c_str(),materialid.c_str(),num);

			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);

		}
	}


	char nm[100] = "";
	char ac[100] = "";
	char tm[100] = "";
	char rm[100] = "";
	int status;
	char stepid[100] = "";
	char taskid[100] = "";

	int cnt = 0;
	out << "[";
	// return new status
	sprintf (sql, "SELECT *,UNIX_TIMESTAMP(OP_DATE) as OP_DATE2 FROM blm_emop_etl.t50_emop_materials_apply_status t1 WHERE \
				  t1.MATERIALS_APPLY_ID='%s' AND t1.WORKFLOW_TASK_ID='31' ",appid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	FILE *pf;
	string tem = sql;
	pf=fopen("/tmp/json88.log","wb");
	fwrite(tem.c_str(),sizeof(char),tem.length(),pf);
	fclose(pf);
	while (psql->NextRow())
	{
		READMYSQL_STR(OPERATOR, nm);
		// READMYSQL_STR(NAME, ac);
		READMYSQL_STR(WORKFLOW_STEP_ID, stepid);
		READMYSQL_STR(OP_DATE2, tm);
		READMYSQL_STR(REMARK_CN, rm);
		READMYSQL_INT(STATUS, status,0);
		READMYSQL_STR(WORKFLOW_TASK_ID, taskid);
		if (cnt++)
			out << ",";
		out << "{nm:\"" << nm << "\",ac:\"" << ac <<"\",tm:\"" << tm <<"\",rm:\"" << rm <<"\",stepid:\"" << stepid <<"\",taskid:\"" << taskid <<"\",status:\"" << status <<"\"}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//{ seq: 0, materialId: "X123" }
int eMOPMaterialsSvc::getAppMaterialDetial(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getAppMaterialDetial]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strmaterialId= root.getv("materialId", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials WHERE t50_emop_materials.MATERIALS_ID='%s'",strmaterialId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char chnName[100] = "";
	char EngName[100] = "";
	char materialKind[100] = "";
	char buyKind[100] = "";
	char norm[100] = "";
	char cycle[100] = "";
	char unit[100] = "";
	char usage[100] = "";
	char location[100] = "";
	char remark[100] = "";

	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_CAT_ID, materialKind);
		READMYSQL_STR(MATERIALS_CODE, buyKind);
		READMYSQL_STR(NAME_CN, chnName);
		READMYSQL_STR(NAME_EN, EngName);
		READMYSQL_STR(SPEC_CN, norm);
		READMYSQL_STR(LIFE_TIME, usage);
		READMYSQL_STR(APPLY_PERIOD, cycle);
		READMYSQL_STR(UNIT_CN, unit);
		READMYSQL_STR(STORE_FLAG, location);
		READMYSQL_STR(REMARK_CN, remark);
		out << "{materialKind:\"" << materialKind << "\",buyKind:\"" << buyKind <<"\",chnName:\"" << chnName <<"\",EngName:\"" << EngName <<"\",norm:\"" << norm <<"\",usage:\"" << usage <<"\",cycle:\"" << cycle <<"\",unit:\"" << unit <<"\",location:\"" << location <<"\",remark:\"" << remark <<"\"}";
	}
	out << "}";
	string strTmp=out.str();
	RELEASE_MYSQL_RETURN(psql, 0);
}

//Mid:0x60 SID: 0x0025
//{seq:\"%s\",materialid:\"%s\,appid:"",num:10,chnName:\"%s\",cycle:2,EngName:\"%s\",isNew:\"%d\",kind:\"%s\",""norm:\"%s\",unit:\"%s\",usage:1.5,store_mark:\"%s\"}
int eMOPMaterialsSvc::insertAppMaterialDetial(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::insertAppMainIfon]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");
	// string materialid = root.getv("materialid", "");
	string appid = root.getv("appid", "");
	string kind = root.getv("kind", "");
	string EngName = root.getv("EngName", "");
	string chnName = root.getv("chnName", "");
	float cycle = root.getv("cycle", 0.0);
	int num = root.getv("num", 0);
	string norm = root.getv("norm", "");
	string date = root.getv("date", "");
	string unit = root.getv("unit", "");
	float usage= root.getv("usage", 0.0);
	int store_mark= root.getv("store_mark", 0);
	string remark = root.getv("remark", "");

	string materialid = "";
	string seqno="";
	GET_EMOP_SEQUENCE_STRING(M,seqno);
	materialid = "M"+seqno;
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf(sql,\
		"insert into blm_emop_etl.t50_emop_materials(MATERIALS_ID, MATERIALS_CAT_ID, NAME_CN ,NAME_EN,SPEC_CN,LIFE_TIME,APPLY_PERIOD,UNIT_CN,STORE_FLAG,OP_DATE,REMARK_CN) \
		values ('%s','%s','%s','%s','%s',%f,%f,'%s',%d,'%s','%s')",materialid.c_str(),kind.c_str(),chnName.c_str(),EngName.c_str(),norm.c_str(),usage,cycle,unit.c_str(),store_mark,date.c_str(),remark.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	// sprintf(sql,\
	// 	"replace into blm_emop_etl.t50_emop_materials_apply_details(MATERIALSS_APPLY_ID, MATERIALS_ID, QUATITY) \
	// 	values ('%s','%s','%d')",appid.c_str(),materialid.c_str(),num);
	// CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\",mid:\"" << materialid <<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60,SID: 0x002B{ seq:????} 
//{seq:\"%s\",appId:\"%s\", materialNo:\"%s\",dispatchNum:\"%s\"}
int eMOPMaterialsSvc::applyConfirm(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::applyConfirm]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string appId = root.getv("appId", "");
	string materialNo = root.getv("materialNo", "");
	int dispatchNum = root.getv("dispatchNum", 0);
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf(sql,\
		"replace into blm_emop_etl.t50_emop_materials_apply_status_details(MATERIALSS_APPLY_ID, MATERIALS_ID, QUANTIRY,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID) \
		values ('%s','%s','%d','32','1')",appId.c_str(),materialNo.c_str(),dispatchNum);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//Mid:0x60,SID: 0x002C{ seq:????} 
//{seq:"1",uid:"blm123",appId:"WPS000102210000",proNo:"C201310120000001",date:"2013-12-14",materials:[{materialNo:"M0000000000000000013",num:38},{materialNo:"M0000000000000000018",num:28}]}
int eMOPMaterialsSvc::generatePriceInquiry(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::generatePriceInquiry]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string appId = root.getv("appId", "");
	//string inquiryQuotedId = root.getv("inquiryQuotedId", "");
	string uid = root.getv("uid", "");
	string date = root.getv("date", "");
	string taskid = root.getv("taskid", "");
	string stepid = root.getv("stepid", "");
	int status = root.getv("status", 0);
	// string materialNo = root.getv("materialNo", "");
	string proNo = root.getv("proNo", "");
	// int num = root.getv("num", 0);
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	string inquiryQuotedId="";
	string inquiryId="";
	string seqInquiryId="";
	GET_EMOP_SEQUENCE_STRING(INQ,seqInquiryId);
	inquiryId = appId+"Q"+seqInquiryId;

	string seqInquiryQuotedId="";
	inquiryQuotedId = inquiryId;

	sprintf(sql,\
		"replace into blm_emop_etl.t50_emop_materials_apply_inquery(MATERIALS_APPLY_INQUERY_ID, MATERIALS_APPLY_ID, USER_ID,OP_DATE,COMPANY_KEY) \
		values ('%s','%s','%s','%s','%s')",inquiryId.c_str(),appId.c_str(),uid.c_str(),date.c_str(),proNo.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	// sprintf(sql,\
	// 	"REPLACE into blm_emop_etl.t50_emop_materials_apply_status(MATERIALS_APPLY_ID, STATUS, OP_DATE, WORKFLOW_TASK_ID, WORKFLOW_STEP_ID, OPERATOR) \
	// 	values ('%s',%d,'%s','%s','%s','%s')",appId.c_str(),status,date.c_str(),taskid.c_str(),stepid,c_str(),uid.c_str());
	// CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int quoted_stauts = 0;
	sprintf(sql,\
		"replace into blm_emop_etl.t50_emop_materials_inqueried_quoted(MATERIALS_INQUERIED_QUOTED_ID,MATERIALS_APPLY_INQUERY_ID,STATUS) \
		values ('%s','%s',%d)",inquiryId.c_str(),inquiryQuotedId.c_str(),quoted_stauts);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	Json* materials = root.get("materials");

	if (materials)
	{
		for (int i=0;i<materials->size();i++)
		{
			Json* materialItem=materials->get(i);
			if (!materialItem)
			{
				continue;
			}


			string materialNo=materialItem->getv("materialNo","");
			int num=materialItem->getv("num",0);

			char sql1[512]="";
			sprintf(sql1,\
				"replace into blm_emop_etl.t50_emop_materials_apply_inqueried_details(MATERIALS_APPLY_INQUERY_ID, MATERIALS_ID, QUTITY) \
				values ('%s','%s',%d)",inquiryId.c_str(),materialNo.c_str(),num);
			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);

			sprintf(sql1,\
				"replace into blm_emop_etl.t50_emop_materials_inquery_quoted_details(MATERIALS_INQUERIED_QUOTED_ID,MATERIALS_ID,QUATITY) \
				values ('%s','%s',%d)",inquiryQuotedId.c_str(),materialNo.c_str(),num);
			FILE *pf;
			string strTmp = sql1;
			pf=fopen("/tmp/json1.log","wb");
			fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
			fclose(pf);
			CHECK_MYSQL_STATUS(psql->Query(sql1), 3);
		}
	}


	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60,SID: 0x002E{ seq:????} 
//{seq:\"%s\",uid:\"%s\",shipId:\"%s\",depart:\"%s\"}
//{seq:0,eid:0,content:[{appid:"",shipId:"S0001",shipName:"ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â€šÂ¬Ã…â€œÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â°ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â½ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â©ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚?,departmentId:"D0001",departmentName:"ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¨ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â®ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã¢â‚¬Â¦ÃƒÂ¢Ã¢â€šÂ¬Ã…â€œÃƒÆ’Ã¢â‚¬Å¡Ãƒ?,inquiryNo:"I0001",date:"2013-10-23",status:0}]}
int eMOPMaterialsSvc::PriceInquiryList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::PriceInquiryList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string uid = root.getv("uid", "");
	string shipIdstr = root.getv("shipId", "");
	string depart = root.getv("depart", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT *,UNIX_TIMESTAMP(OP_DATE) as OP_DATE2 FROM blm_emop_etl.t50_emop_materials_apply_inquery t1,blm_emop_etl.t50_emop_materials_applies t2,blm_emop_etl.t50_emop_materials_inqueried_quoted t3 WHERE t2.MATERIALS_APPLY_ID = t1.MATERIALS_APPLY_ID AND t1.MATERIALS_APPLY_INQUERY_ID = t3.MATERIALS_APPLY_INQUERY_ID AND t2.SHIP_ID='%s' AND t2.DEPARTMENT_CODE='%s' ",shipIdstr.c_str(),depart.c_str());

	// FILE *pf;
	// string strTmp = sql;
	// pf=fopen("/tmp/json1.log","wb");
	// fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	// fclose(pf);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char appid[100] = "";
	char inquiryQuotedNo[100] = "";
	char inquiryNo[100] = "";
	char date[100] = "";
	char shipId[100] = "";
	char shipName[100] = "";
	char departmentId[100] = "";
	char departmentName[100] = "";
	int status = 0;
	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_APPLY_INQUERY_ID, inquiryNo);
		READMYSQL_STR(MATERIALS_INQUERIED_QUOTED_ID, inquiryQuotedNo);
		READMYSQL_STR(OP_DATE2, date);
		READMYSQL_STR(SHIP_ID, shipId);
		READMYSQL_STR(SHIP_ID, shipName);
		READMYSQL_STR(DEPARTMENT_CODE, departmentId);
		READMYSQL_STR(MATERIALS_APPLY_ID,appid);
		READMYSQL_INT(STATUS, status,0);

		if (cnt++)
			out << ",";
		out << "{appid:\"" << appid << "\",inquiryNo:\"" << inquiryNo << "\",inquiryQuotedNo:\"" << inquiryQuotedNo << "\",date:\"" << date << "\",shipId:\"" << shipId << "\",shipName:\"" << shipName << "\",departmentId:\"" << departmentId << "\",departmentName:\"" << departmentName << "\",status:" << status << "}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x60,SID: 0x002F{ seq:????} 
//{seq:\"%s\",inquiryNo:\"%s\"}
//{seq:0,eid:0,content:{shipName:"ÃƒÆ’Ã‚Â¦ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“Ãƒâ€šÃ‚Â°ÃƒÆ’Ã‚Â¥Ãƒâ€¦Ã‚Â½Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã‚Â©ÃƒÂ¢Ã¢â€šÂ?,departmentName:"ÃƒÆ’Ã‚Â¨Ãƒâ€šÃ‚Â½Ãƒâ€šÃ‚Â®ÃƒÆ’Ã‚Â¦Ãƒâ€¦Ã¢â‚¬Å“Ãƒâ€šÃ?,purchaseKind:"ÃƒÆ’Ã‚Â¥Ãƒâ€šÃ‚ÂÃƒâ€¦Ã‚Â ÃƒÆ’Ã‚Â¥Ãƒâ€šÃ‚Â¹Ãƒâ€šÃ‚Â?,status:0,internalKind:"ÃƒÆ’Ã‚Â¤Ãƒâ€šÃ‚ÂºÃƒâ€¦Ã‚Â¡ÃƒÆ’Ã‚Â©Ãƒâ€šÃ‚ÂºÃƒâ€šÃ‚Â»ÃƒÆ’Ã‚Â¥Ãƒâ€šÃ‚Â¸Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â§Ãƒâ€šÃ‚Â±Ãƒâ€šÃ?,providePort:"ÃƒÆ’Ã‚Â¥Ãƒâ€šÃ‚Â¤Ãƒâ€šÃ‚Â§ÃƒÆ’Ã‚Â¨Ãƒâ€šÃ‚Â¿Ãƒâ€¦Ã‚Â?,provideDate:"2013-10-22",purchaseReason:"ÃƒÆ’Ã‚Â¤Ãƒâ€šÃ‚Â¸Ãƒâ€šÃ‚ÂÃƒÆ’Ã‚Â§Ãƒâ€¦Ã‚Â¸Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã‚Â©Ãƒâ€šÃ‚Â?,purchaseNo:"P0001",provider:"ÃƒÆ’Ã‚Â¤Ãƒâ€šÃ‚Â¸Ãƒâ€¦Ã‚Â ÃƒÆ’Ã‚Â¦Ãƒâ€šÃ‚ÂµÃƒâ€šÃ‚Â·ÃƒÆ’Ã‚Â¥Ãƒâ€¹Ã¢â‚¬Â Ãƒâ€šÃ‚Â¶ÃƒÆ’Ã‚Â¨Ãƒâ€šÃ‚Â¡Ãƒâ€šÃ‚Â£ÃƒÆ’Ã‚Â¥Ãƒâ€¦Ã?,confirmNo:"P0001",confirmDate:"2013-11-01",provideShipDate:"2013-10-22",provideLocation:"ÃƒÆ’Ã‚Â¨Ãƒâ€šÃ‚Â½Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã‚Â¥Ãƒâ€šÃ‚ÂºÃƒÂ¢Ã¢â€?,currency:"ÃƒÆ’Ã‚Â¤Ãƒâ€šÃ‚ÂºÃƒâ€šÃ‚ÂºÃƒÆ’Ã‚Â¦Ãƒâ€šÃ‚Â°ÃƒÂ¢Ã¢â€šÂ¬Ã‹Å“ÃƒÆ’Ã‚Â¥Ã?,transportFee:13,serviceFee:13,otherFee:13,totalFee:23,toRMB:13}
int eMOPMaterialsSvc::PriceInquiryMain(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::PriceInquiryList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string inquiryNo = root.getv("inquiryNo", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_inqueried_quoted t1,blm_emop_etl.T50_EMOP_MATERIALS_APPLY_INQUERY t2,blm_emop_etl.T50_EMOP_MATERIALS_APPLIES t3 WHERE t1.MATERIALS_APPLY_INQUERY_ID = '%s' AND t1.MATERIALS_APPLY_INQUERY_ID = t2.MATERIALS_APPLY_INQUERY_ID AND t2.MATERIALS_APPLY_ID = t3.MATERIALS_APPLY_ID",inquiryNo.c_str());


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	// FILE *pf;
	// string strTmp = sql;
	// pf=fopen("/tmp/json1.log","wb");
	// fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	// fclose(pf);

	float toRMB;
	float totalFee;
	float otherFee;
	float serviceFee;
	float transportFee;
	int currency;
	char appid[100] = "";
	char provider[100] = "";
	char purchaseReason[100] = "";
	char departmentName[100] = "";
	char purchaseKind[100] = "";
	char providePort[100] = "";
	char provideDate[100] = "";
	char internalKind[100] = "";
	char shipName[100] = "";
	char provideShipDate[100] = "";
	char confirmDate[100] = "";
	char confirmNo[100] = "";
	char provideLocation[100] = "";
	int status = 0;
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_APPLY_ID, appid);
		READMYSQL_DOUBLE(TOTAL_RMB, toRMB,0);
		READMYSQL_DOUBLE(TOTAL_PRICE, totalFee,0);
		READMYSQL_DOUBLE(OTHER_EXPENSE, otherFee,0);
		READMYSQL_DOUBLE(SERVICE_EXPENSE, serviceFee,0);
		READMYSQL_DOUBLE(FREIGHT, transportFee,0);
		READMYSQL_STR(DELIVERY_DATE, provideShipDate);
		READMYSQL_STR(DELIVERY_LOCATION, provideLocation);
		READMYSQL_STR(QUOTE_DATE, confirmDate);
		READMYSQL_STR(QUOTE_CODE, confirmNo);

		READMYSQL_STR(APPLY_REASON, purchaseReason);
		READMYSQL_STR(INNER_CAT, internalKind);
		READMYSQL_STR(SHIP_ID, shipName);
		READMYSQL_STR(DEPARTMENT_CODE, departmentName);

		READMYSQL_STR(SUPPLY_DATE, provideDate);
		READMYSQL_STR(PORT_CODE_ID, providePort);
		READMYSQL_STR(COMPANY_KEY, provider);
		READMYSQL_INT(STATUS, status,0);
		READMYSQL_INT(CURRENCY, currency,1);
		// READMYSQL_STR(DEPARTMENT_CODE, confirmNo);
		// if (cnt++)
		// 	out << ",";
		out << "{toRMB:" << toRMB << ",currency:" << currency << ",totalFee:" << totalFee << ",otherFee:" << otherFee << ",serviceFee:" << serviceFee << ",transportFee:" << transportFee << ",provideShipDate:\"" << provideShipDate << "\",provideLocation:\"" << provideLocation << "\",confirmDate:\"" << confirmDate << "\",confirmNo:\"" << confirmNo << "\",\
																																																																																							   purchaseReason:\"" << purchaseReason << "\",internalKind:\"" << internalKind << "\",shipName:\"" << shipName << "\",departmentName:\"" << departmentName << "\",provideDate:\"" << provideDate << "\",providePort:\"" << providePort << "\",provider:\"" << provider << "\",status:" << status << ",appid:\"" << appid << "\"}";
	}
	out << "}";

	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x60,SID: 0x0030{ seq:????} 
//seq:\"%s\",inquiryNo:\"%s\"
//{seq:0,eid:0,content:[{materialKind:"ÃƒÆ’Ã‚Â¦Ãƒâ€šÃ‚Â²Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã‚Â¦Ãƒâ€šÃ‚Â¼ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¥Ãƒâ€šÃ‚Â·Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã‚Â¥ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦Ãƒâ€?,materialNo:"M0001",materialName_CN:"ÃƒÆ’Ã‚Â§Ãƒâ€šÃ‚ÂºÃƒâ€šÃ‚Â¯ÃƒÆ’Ã‚Â§Ãƒâ€šÃ‚Â¾Ãƒâ€¦Ã‚Â ÃƒÆ’Ã‚Â¦Ãƒâ€šÃ‚Â¯ÃƒÂ¢Ã¢â€šÂ¬Ã‚ÂºÃƒÆ’Ã‚Â¦Ãƒâ€šÃ‚Â¯Ã?,materialName_EN:"maotan",norm:"ÃƒÆ’Ã‚Â¥Ãƒâ€šÃ‚Â?400mm ÃƒÆ’Ã‚Â§ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢Ãƒâ€šÃ‚Â½ÃƒÆ’Ã‚Â¨ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â°Ãƒâ€?,inquiryNum:10, unit:"ÃƒÆ’Ã‚Â¤Ãƒâ€šÃ‚Â?, unitPrice:10,discountRate:0.8,discountTotalPrice:8,shipStoreNum:20,location:"ÃƒÆ’Ã‚Â¨Ãƒâ€šÃ‚Â½Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã‚Â¥Ãƒâ€šÃ‚ÂºÃƒÂ¢Ã¢â€?,}]}
int eMOPMaterialsSvc::PriceInquirySub(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::PriceInquirySub]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string inquiryNo = root.getv("inquiryNo", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",eid:0,content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_inquery_quoted_details t1, blm_emop_etl.T50_EMOP_MATERIALS t2 WHERE t1.MATERIALS_INQUERIED_QUOTED_ID='%s' AND t1.MATERIALS_ID = t2.MATERIALS_ID" ,inquiryNo.c_str());


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	FILE *pf;
	string strTmp = sql;
	pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);

	char materialNo[100] = "";
	float unitPrice;
	float discountRate;
	float discountTotalPrice;
	int inquiryNum;
	char materialKind[100] = "";
	char materialName_CN[100] = "";
	char materialName_EN[100] = "";
	char norm[100] = "";
	char unit[100] = "";
	// char departmentName[100] = "";
	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_ID, materialNo);
		READMYSQL_DOUBLE(UNIT_PRICE, unitPrice,0);
		READMYSQL_DOUBLE(DISCOUNTS, discountRate,0);
		READMYSQL_DOUBLE(TOTAL_PRICE_DISCOUNTED, discountTotalPrice,0);
		READMYSQL_INT(QUATITY, inquiryNum,0);

		READMYSQL_STR(NAME_CN, materialName_CN);
		READMYSQL_STR(NAME_EN, materialName_EN);
		READMYSQL_STR(SPEC_CN, norm);
		READMYSQL_STR(UNIT_CN, unit);
		READMYSQL_STR(MATERIALS_CAT_ID, materialKind);
		//READMYSQL_STR(DEPARTMENT_CODE, departmentName);

		if (cnt++)
			out << ",";
		out << "{materialNo:\"" << materialNo << "\",unitPrice:" << unitPrice << ",discountRate:" << discountRate << ",discountTotalPrice:" << discountTotalPrice << ",inquiryNum:" << inquiryNum << ",\
																																																	 materialName_CN:\"" << materialName_CN << "\",materialName_EN:\"" << materialName_EN << "\",norm:\"" << norm << "\",unit:\"" << unit << "\",materialKind:\"" << materialKind << "\"}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x60,SID: 0x0031
//{seq:0,inquiryQuotedNo:"",inquiryNo:"1",cid:"123456s003",provideShipDate:"20121122",confirmDate:"20121112",locate:"sbczl",currency:0,freight:1000,other:2000,total:3000,serviceFee:200,toRMB:2000}
int eMOPMaterialsSvc::generatePriceInquiryQuoted(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::generatePriceInquiry]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string inquiryNo = root.getv("inquiryNo", "");
	string inquiryQuotedNo = root.getv("inquiryQuotedNo", "");
	string cid = root.getv("cid", "");
	string provideShipDate = root.getv("provideShipDate", "");
	string confirmDate = root.getv("confirmDate", "");
	string locate = root.getv("locate", "");
	string status = root.getv("status", "");
	int currency = root.getv("currency", 0);

	float freight = root.getv("freight", 0.0);
	float other = root.getv("other", 0.0);
	float total = root.getv("total", 0.0);
	float serviceFee = root.getv("serviceFee", 0.0);
	float toRMB = root.getv("toRMB", 0.0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf(sql,\
		"replace into blm_emop_etl.t50_emop_materials_inqueried_quoted(MATERIALS_INQUERIED_QUOTED_ID,MATERIALS_APPLY_INQUERY_ID, QUOTE_DATE, QUOTE_CODE,CURRENCY,FREIGHT,SERVICE_EXPENSE,OTHER_EXPENSE,TOTAL_PRICE,TOTAL_RMB,DELIVERY_DATE,DELIVERY_LOCATION,STATUS) \
		values ('%s','%s','%s','%s',%d,%f,%f,%f,%f,%f,'%s','%s','%s')",inquiryQuotedNo.c_str(),inquiryNo.c_str(),confirmDate.c_str(),cid.c_str(),currency,freight,serviceFee,other,total,toRMB,provideShipDate.c_str(),locate.c_str(),status.c_str());

	FILE *pf;
	string tem = sql;
	pf=fopen("/tmp/json8.log","wb");
	fwrite(tem.c_str(),sizeof(char),tem.length(),pf);
	fclose(pf);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60,SID: 0x002d
//{seq:0,inquiryQuotedNo:"",no:"",num:,pr:, dis:discountTotalPrice:}
int eMOPMaterialsSvc::sendPriceInquiryQuotedMaterial(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::sendPriceInquiryQuotedMaterial]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string inquiryQuotedNo = root.getv("inquiryQuotedNo", "");
	string no = root.getv("no", "");

	float pr = root.getv("pr", 0.0);
	float dis = root.getv("dis", 0.0);
	int num = root.getv("num", 0);
	float discountTotalPrice = root.getv("discountTotalPrice", 0.0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf(sql,\
		"replace into blm_emop_etl.t50_emop_materials_inquery_quoted_details(MATERIALS_INQUERIED_QUOTED_ID,MATERIALS_ID, UNIT_PRICE, DISCOUNTS,TOTAL_PRICE_DISCOUNTED,QUATITY) \
		values ('%s','%s',%f,%f,%f,%d)",inquiryQuotedNo.c_str(),no.c_str(),pr,dis,discountTotalPrice,num);

	FILE *pf;
	string tem = sql;
	pf=fopen("/tmp/json2.log","wb");
	fwrite(tem.c_str(),sizeof(char),tem.length(),pf);
	fclose(pf);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60,SID: 0x003E{ seq:????,inquiryNo,appId} 
//{ seq: 0, process:[{nm:"Ã¥Â¼Â Ã¤Â¸â€?,ac:"Ã§â€Â³Ã¨Â¯Â?,tm:123232,rm:"Ã¥ÂÅ’Ã¦â€žÂ?,state:0},{nm:"Ã¦ÂÅ½Ã¥â€ºâ€?,ac:"Ã¦â€°Â¹Ã¥Â¤Â?,tm:123232,rm:"Ã¥ÂÅ’Ã¦â€žÂ?,state:0},{nm:"Ã§Å½â€¹Ã¤Âºâ€?,ac:"Ã¨Â¯Â¢Ã¤Â»Â·",tm:123232,rm:"Ã¥ÂÅ’Ã¦â€žÂ?,state:0}]}
int eMOPMaterialsSvc::getPriceInquiryProcess(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getPriceInquiryProcess]bad format:", jsonString, 1);
	string strTmp;
	int iscontinue = 1;
	string strSeq = root.getv("seq", "");
	string inquiryNo = root.getv("inquiryNo", "");
	string appId = root.getv("appId", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",process:";
	out << "[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	char nm[100] = "";
	char ac[100] = "";
	char tm[100] = "";
	char rm[100] = "";

	sprintf (sql, "SELECT *,UNIX_TIMESTAMP(OP_DATE) as OP_DATE2 FROM blm_emop_etl.t50_emop_materials_apply_status WHERE t50_emop_materials_apply_status.MATERIALS_APPLY_ID = '%s' AND t50_emop_materials_apply_status.WORKFLOW_TASK_ID='31' AND t50_emop_materials_apply_status.WORKFLOW_STEP_ID='3'",appId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	strTmp=sql;
	FILE *pf;
	pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);	

	int cnt = 0;
	if(psql->NextRow()){
		READMYSQL_STR(OPERATOR, nm);
		READMYSQL_STR(OP_DATE2, tm);
		READMYSQL_STR(REMARK_CN, rm);
	}

	out << "{nm:\"" << nm << "\",ac:\"" << ac << "\",tm:\"" << tm << "\",rm:\"" << rm << "\"}";
	// char sql1[1024] = "";
	sprintf (sql, "SELECT *,UNIX_TIMESTAMP(OP_DATE) as OP_DATE2 FROM blm_emop_etl.t50_emop_materials_apply_status t1 WHERE t1.MATERIALS_APPLY_ID = '%s' AND t1.WORKFLOW_TASK_ID='32'",appId.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow()){
		READMYSQL_STR(OPERATOR, nm);
		READMYSQL_STR(OP_DATE2, tm);
		READMYSQL_STR(REMARK_CN, rm);
		out << ",{nm:\"" << nm << "\",ac:\"" << ac << "\",tm:\"" << tm << "\",rm:\"" << rm << "\"}";
	}
	else{
		out << "]}";
		iscontinue = 0;
	}
	// char sql2[1024] = "";
	// char nm[100] = "";
	// char ac[100] = "";
	// char tm[100] = "";
	// char rm[100] = "";
	if(iscontinue)
	{	
		sprintf (sql, "SELECT USER_ID, ,UNIX_TIMESTAMP(OP_DATE) as OP_DATE2 FROM blm_emop_etl.t50_emop_materials_apply_inquery WHERE t50_emop_materials_apply_inquery.MATERIALS_APPLY_INQUERY_ID = '%s' ",inquiryNo.c_str());
		strTmp=sql;
		//FILE *pf;
		pf=fopen("/tmp/json2.log","wb");
		fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
		fclose(pf);	
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->NextRow()){
			READMYSQL_STR(USER_ID, nm);
			READMYSQL_STR(OP_DATE2, tm);
			out << ",{nm:\"" << nm << "\",ac:\"" << ac << "\",tm:\"" << tm << "\",rm:\"" << rm << "\"}";
		}
		out << "]}";
	}



	// out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x60,SID: 0x0032{seq:\"%s\",materialNo:\"%s\"}
//{seq:0,eid:0,content:[{provider:"ä¸Šæµ·åˆ¶è¡£åŽ?,inquiryNo:"I0001",date:"2012-10-09",inquiryNum:10,unitPrice:20,currency:"äººæ°‘å¸?}]}'
int eMOPMaterialsSvc::getPriceInquiryQuotedHistory(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getPriceInquiryQuotedHistory]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string materialNo = root.getv("materialNo", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_inquery_quoted_details t1, blm_emop_etl.t50_emop_materials_inqueried_quoted t2,blm_emop_etl.t50_emop_materials_apply_inquery t3 WHERE t1.MATERIALS_ID='%s' AND t1.MATERIALS_INQUERIED_QUOTED_ID=t2.MATERIALS_INQUERIED_QUOTED_ID AND t2.MATERIALS_APPLY_INQUERY_ID = t3.MATERIALS_APPLY_INQUERY_ID",materialNo.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	string strTmp=sql;
	FILE *pf;
	pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);
	char inquiryNum[100] = "";
	char unitPrice[100] = "";
	char currency[100] = "";
	char provider[100] = "";
	char inquiryNo[100] = "";
	char date[100] = "";
	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(QUATITY, inquiryNum);
		READMYSQL_STR(UNIT_PRICE, unitPrice);
		READMYSQL_STR(MATERIALS_INQUERIED_QUOTED_ID, inquiryNo);
		READMYSQL_STR(COMPANY_KEY, provider);
		READMYSQL_STR(CURRENCY, currency);
		READMYSQL_STR(QUOTE_DATE, date);

		if (cnt++)
			out << ",";
		out << "{inquiryNum:\"" << inquiryNum << "\",unitPrice:\"" << unitPrice << "\",inquiryNo:\"" << inquiryNo << "\",provider:\"" << provider << "\",currency:\"" << currency << "\",date:\"" << date << "\"}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//Mid:0x60,SID: 0x0033{seq:\"%s\",purchaseNo:\"%s\"}
//{seq:0,eid:0,content:[{inquiryNo:"I0001",provider:"上海制造商",totalFee:20, currency:"人民币",date:"2013-10-10"}]}
int eMOPMaterialsSvc::reqInquiryByPurchase(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::reqInquiryByPurchase]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string purchaseNo = root.getv("purchaseNo", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT *,UNIX_TIMESTAMP(OP_DATE) as OP_DATE2 FROM blm_emop_etl.t50_emop_materials_apply_inquery t1, blm_emop_etl.t50_emop_materials_inqueried_quoted t2 WHERE t1.MATERIALS_APPLY_ID = '%s' AND t1.MATERIALS_APPLY_INQUERY_ID = t2.MATERIALS_APPLY_INQUERY_ID",purchaseNo.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char inquiryNo[100] = "";
	char provider[100] = "";
	char date[100] = "";
	float totalFee;
	int currency;

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_APPLY_INQUERY_ID, inquiryNo);
		READMYSQL_STR(COMPANY_KEY, provider);
		READMYSQL_STR(OP_DATE2, date);
		READMYSQL_DOUBLE(TOTAL_PRICE,totalFee,0);
		READMYSQL_INT(CURRENCY,currency,0);

		if (cnt++)
			out << ",";
		out << "{inquiryNo:\"" << inquiryNo << "\",provider:\"" << provider << "\",date:\"" << date << "\",totalFee:" << totalFee << ",currency:" << currency << "}";
	}
	out << "]}";
	string strTmp=out.str();
	//	FILE *pf;
	/*pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);*/
	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60,SID: 0x0034{seq:\"%s\",inquiryNo:\"%s\"}
//{seq:0,eid:0,content:[{materialNo:"M0001",materialName_CN:"100%纯羊毛",unitPrice:"10",inquiryNum:2,totalFee:20}]}
int eMOPMaterialsSvc::reqMaterialByInquiry(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::reqMaterialByInquiry]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string inquiryNo = root.getv("inquiryNo", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_inquery_quoted_details t1, blm_emop_etl.t50_emop_materials t2 WHERE t1.MATERIALS_ID = t2.MATERIALS_ID AND t1.MATERIALS_INQUERIED_QUOTED_ID ='%s';",inquiryNo.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char materialNo[100] = "";
	char materialName_CN[100] = "";
	float unitPrice;
	int inquiryNum;
	float totalFee;

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_ID, materialNo);
		READMYSQL_STR(NAME_CN, materialName_CN);
		READMYSQL_INT(QUATITY,inquiryNum,0);
		READMYSQL_DOUBLE(UNIT_PRICE,unitPrice,0);
		READMYSQL_DOUBLE(TOTAL_PRICE,totalFee,0);

		if (cnt++)
			out << ",";
		out << "{materialNo:\"" << materialNo << "\",materialName_CN:\"" << materialName_CN << "\",inquiryNum:" << inquiryNum << ",unitPrice:" << unitPrice << "}";
	}
	out << "]}";
	string strTmp=out.str();
	//	FILE *pf;
	/*pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);*/
	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60,SID: 0x0035{seq:\"%s\",purchaseNo:\"%s\"}
//{seq:0,eid:0,content:[{materialNo:"M0001",materialName_CN:"100%纯羊毛",inquiryNum:2}]}
int eMOPMaterialsSvc::reqMaterialByPurchase(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::reqMaterialByPurchase]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string purchaseNo = root.getv("purchaseNo", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_apply_details t1, blm_emop_etl.t50_emop_materials t2 WHERE t1.MATERIALSS_APPLY_ID = '%s' AND t1.MATERIALS_ID = t2.MATERIALS_ID",purchaseNo.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char materialNo[100] = "";
	char materialName_CN[100] = "";
	int inquiryNum;

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_ID, materialNo);
		READMYSQL_STR(NAME_CN, materialName_CN);
		READMYSQL_INT(QUATITY,inquiryNum,0);

		if (cnt++)
			out << ",";
		out << "{materialNo:\"" << materialNo << "\",materialName_CN:\"" << materialName_CN << "\",inquiryNum:" << inquiryNum << "}";
	}
	out << "]}";
	string strTmp=out.str();
	//	FILE *pf;
	/*pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);*/
	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x60,SID: 0x0036{seq:\"%s\",materialNo:\"%s\"}
//{seq:0,eid:0,content:[{inquiryNo:"I0001",provider:"上海制造商",totalFee:20, currency:"人民币",unitPrice:"10"}]}
int eMOPMaterialsSvc::reqInquiryByMaterial(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::reqInquiryByMaterial]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string materialNo = root.getv("materialNo", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_inquery_quoted_details t1 WHERE t1.MATERIALS_ID = '%s' ",materialNo.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char inquiryNo[100] = "";
	char provider[100] = "";
	int currency;
	float totalFee;
	float unitPrice;

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_INQUERIED_QUOTED_ID, inquiryNo);
		READMYSQL_DOUBLE(TOTAL_PRICE, totalFee,0);
		READMYSQL_DOUBLE(UNIT_PRICE,unitPrice,0);

		if (cnt++)
			out << ",";
		out << "{inquiryNo:\"" << inquiryNo << "\",totalFee:" << totalFee << ",unitPrice:" << unitPrice << "}";
	}
	out << "]}";
	string strTmp=out.str();
	//	FILE *pf;
	/*pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);*/
	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x60,SID: 0x0037{seq:\"%s\",materialNo:\"%s\"}
//{seq:0,eid:0,content:[{orderNo:"O0001",unitPrice:10,orderNum:20,provider:"上海制造商",shipName:"新厦门",date:"2010-08-011"}]}
int eMOPMaterialsSvc::reqOrderData(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::reqOrderData]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string materialNo = root.getv("materialNo", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT * FROM blm_emop_etl.t50_emop_materials_purchase_order_details t1, blm_emop_etl.t50_emop_materials_purchase_orders t2 WHERE t1.MATERIALS_ID = '%s' AND t1.MATERIALS_PURCHASE_ORDER_ID = t2.MATERIALS_PURCHASE_ORDER_ID",materialNo.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char orderNo[100] = "";
	char provider[100] = "";
	char shipName[100] = "";
	char date[100] = "";
	int orderNum;
	float unitPrice;

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_PURCHASE_ORDER_ID, orderNo);
		// READMYSQL_STR(MATERIALS_INQUERIED_QUOTED_ID, provider);
		// READMYSQL_STR(MATERIALS_INQUERIED_QUOTED_ID, shipName);
		READMYSQL_INT(QUATITY, orderNum,0);
		READMYSQL_DOUBLE(UNIT_PRICE,unitPrice,0);

		if (cnt++)
			out << ",";
		out << "{orderNo:\"" << orderNo << "\",orderNum:" << orderNum << ",unitPrice:" << unitPrice << "}";
	}
	out << "]}";
	string strTmp=out.str();
	//	FILE *pf;
	/*pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);*/
	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60,SID: 0x0038
//{seq:0,date:"",taskid:"34",stepid:"1",state:0,inquiryQuotedNo:"",uid:"123456s003",provideShipDate:"20121122",locate:"sbczl",currency:0,freight:1000,other:2000,total:3000,serviceFee:200,toRMB:2000}
int eMOPMaterialsSvc::generatePurchaseOrders(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::generatePurchaseOrders]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string inquiryQuotedNo = root.getv("inquiryQuotedNo", "");
	string uid = root.getv("uid", "");
	string provideShipDate = root.getv("provideShipDate", "");
	string locate = root.getv("locate", "");
	int status = root.getv("status", 0);
	int currency = root.getv("currency", 0);

	string date = root.getv("date", "");
	string strrm = root.getv("strrm", "");
	string taskid = root.getv("taskid", "");
	string stepid = root.getv("stepid", "");
	int state = root.getv("state", 0);

	float freight = root.getv("freight", 0.0);
	float other = root.getv("other", 0.0);
	float total = root.getv("total", 0.0);
	float serviceFee = root.getv("serviceFee", 0.0);
	float toRMB = root.getv("toRMB", 0.0);


	string purchaseOrderNo="";
	string oderSeq="";
	GET_EMOP_SEQUENCE_STRING(ORD,oderSeq);
	purchaseOrderNo=inquiryQuotedNo+"D"+oderSeq;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf(sql,\
		"replace into blm_emop_etl.t50_emop_materials_purchase_orders(MATERIALS_PURCHASE_ORDER_ID,MATERIALS_APPLY_INQUERIED_QUOTED_ID, OPERATOR,CURRENCY,FREIGHT,SERVICE_EXPENSE,OTHER_EXPENSE,TOTAL_PRICE,DELIVERY_DATE,DELIVERY_LOCATION,STATUS) \
		values ('%s','%s','%s',%d,%f,%f,%f,%f,'%s','%s','%d')",purchaseOrderNo.c_str(),inquiryQuotedNo.c_str(),uid.c_str(),currency,freight,serviceFee,other,total,provideShipDate.c_str(),locate.c_str(),status);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	sprintf (sql, "INSERT INTO blm_emop_etl.t50_emop_materials_purchase_order_status(MATERIALS_PURCHASE_ORDER_ID, STATUS, OP_DATE,REMARK_CN,OPERATOR,WORKFLOW_TASK_ID,WORKFLOW_STEP_ID)\
				  VALUES('%s',%d,'%s','%s','%s','%s','%s')",purchaseOrderNo.c_str(),state,date.c_str(),strrm.c_str(),uid.c_str(),taskid.c_str(),stepid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\","<<"purchaseOrderNo:\""<<purchaseOrderNo.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}




//Mid:0x60,SID: 0x003F
//{seq:0,purchaseOrderNo:"",inquiryQuotedNo:"",materialNo:"",num:,unitPrice:,totalPrice:,discount:,totalPriceDiscount:}
int eMOPMaterialsSvc::generatePurchaseOrderDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::generatePurchaseOrderDetail]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string inquiryQuotedNo = root.getv("inquiryQuotedNo", "");
	string materialNo = root.getv("materialNo", "");
	string purchaseOrderNo = root.getv("purchaseOrderNo", "");
	int num = root.getv("num", 0);

	float unitPrice = root.getv("unitPrice", 0.0);
	float totalPrice = root.getv("totalPrice", 0.0);
	float discount = root.getv("discount", 0.0);
	float totalPriceDiscount = root.getv("totalPriceDiscount", 0.0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf(sql,\
		"replace into blm_emop_etl.t50_emop_materials_purchase_order_details(MATERIALS_PURCHASE_ORDER_ID,MATERIALS_ID, MATERIALS_INQUERIED_QUOTED_ID,QUATITY,UNIT_PRICE,TOTAL_PRICE,DISCOUNTS,TOTAL_PRICE_DISCOUNTED) \
		values ('%s','%s','%s',%d,%f,%f,%f,%f)",purchaseOrderNo.c_str(),materialNo.c_str(),inquiryQuotedNo.c_str(),num,unitPrice,totalPrice,discount,totalPriceDiscount);

	FILE *pf;
	string tem = sql;
	pf=fopen("/tmp/json11.log","wb");
	fwrite(tem.c_str(),sizeof(char),tem.length(),pf);
	fclose(pf);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<< "{eid:0,"<<"seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}



//Mid:0x60,SID: 0x003D{ seq:????} 
//{ seq: 0, content:[{proName:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¸ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂµÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â·ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ?ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬?proNo:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Âzxca34ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â}ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¯ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¼ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢{proName:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¸ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂµÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â·ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¤ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¥ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ?ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬?proNo:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Âzxca34ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â}] }
int eMOPMaterialsSvc::getProviderList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getProviderList]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT COMPANY_KEY, NAME FROM blm_emop_etl.t41_company");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char proNo[100] = "";
	char proName[100] = "";
	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(COMPANY_KEY, proNo);
		READMYSQL_STR(NAME, proName);

		if (cnt++)
			out << ",";
		out << "{proNo:\"" << proNo << "\",proName:\"" << proName << "\"}";
	}
	out << "]}";
	string strTmp=out.str();
	//	FILE *pf;
	/*pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);*/
	RELEASE_MYSQL_RETURN(psql, 0);
}


//Mid:0x60,SID: 0x0001{ seq:????} 
//{eid:0, seq:????,info:[{id:???? code:????,nc:"",ne:"",innerCate:"" },{}]}
int eMOPMaterialsSvc::getMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getMaterialsCatInfo]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strKey1 = root.getv("key1", "");
	string strKey2= root.getv("key2", "");
	
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
 
	if(strKey1.empty()){ //跟类别
		sprintf (sql, "SELECT t1.KEY1,t1.KEY1_DESC,t1.KEY1_DESC_EN,t1.INNER_CAT,t1.CODE,COUNT(t2.KEY2) AS childCount FROM blm_emop_etl.t50_emop_materials_cat1 t1 \
					  LEFT JOIN blm_emop_etl.t50_emop_materials_cat2 t2 ON t2.KEY1=t1.KEY1 GROUP BY t1.KEY1;");
	
		CHECK_MYSQL_STATUS(psql->Query(sql), 3); 

		string key="";
		string name_cn="";
		string name_en="";
		string inner_cat="";
		string code=""; 
		int childCount=0;

		out<<"{seq:\""<<strSeq<<"\",data:["; 
		int cnt=0;
		while (psql->NextRow())
		{
			READMYSQL_STRING(KEY1, key); 
			READMYSQL_STRING(KEY1_DESC, name_cn);
			READMYSQL_STRING(KEY1_DESC_EN, name_en); 
			READMYSQL_STRING(INNER_CAT, inner_cat);
			READMYSQL_STRING(CODE, code);
			READMYSQL_INT(childCount, childCount,0);

			if (cnt>0)
				out << ",";

			++cnt; 
           //只有三级分类有category，只有一级分类有 innerCate 和 code
			out << "{category:\"\",key:\""<<key<<"\",nc:\"" << name_cn << "\",ne:\"" << name_en <<"\",innerCate:\""<<inner_cat<<"\",code:\""<<code<<"\",ch:"<<childCount<<"}";
		}
		out << "]}";  
	}else if(!strKey1.empty() && strKey2.empty()){
		sprintf (sql, "SELECT t1.KEY2,t1.KEY2_DESC,t1.KEY2_DESC_EN,COUNT(t2.KEY3) as childCount FROM blm_emop_etl.t50_emop_materials_cat2 t1 \
					  LEFT JOIN blm_emop_etl.t50_emop_materials_category t2 ON t1.KEY1=t2.KEY1 AND t1.KEY2=t2.KEY2 \
					  WHERE t1.KEY1='%s' GROUP BY t1.KEY2;",strKey1.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3); 

		string key="";
		string name_cn="";
		string name_en=""; 
		int childCount=0;

		out<<"{seq:\""<<strSeq<<"\",data:["; 
		int cnt=0;
		while (psql->NextRow())
		{
			READMYSQL_STRING(KEY2, key); 
			READMYSQL_STRING(KEY2_DESC, name_cn);
			READMYSQL_STRING(KEY2_DESC_EN, name_en);  
			READMYSQL_INT(childCount, childCount,0);

			if (cnt>0)
				out << ",";
			++cnt; 
    //只有三级分类有category，只有一级分类有 innerCate 和 code
			out << "{category:\"\",key:\""<<key<<"\",nc:\"" << name_cn << "\",ne:\"" << name_en <<"\",innerCate:\"\",code:\"\",ch:"<<childCount<<"}";
		}
		out << "]}";  
	}  else if(!strKey1.empty() && !strKey2.empty()){
		sprintf (sql, "SELECT CATEGORY,KEY3,KEY3_DESC,KEY3_DESC_EN FROM blm_emop_etl.t50_emop_materials_category \
					  WHERE KEY1='%s' AND KEY2='%s';",strKey1.c_str(),strKey2.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3); 

		string category="";
		string key="";
		string name_cn="";
		string name_en="";  

		out<<"{seq:\""<<strSeq<<"\",data:["; 
		int cnt=0;
		while (psql->NextRow())
		{
			READMYSQL_STRING(CATEGORY, category); 
			READMYSQL_STRING(KEY3, key); 
			READMYSQL_STRING(KEY3_DESC, name_cn);
			READMYSQL_STRING(KEY3_DESC_EN, name_en);   

			if (cnt>0)
				out << ",";
			++cnt; 
            //只有三级分类有category，只有一级分类有 innerCate 和 code
			out << "{category:\""<<category<<"\",key:\""<<key<<"\",nc:\"" << name_cn << "\",ne:\"" << name_en <<"\",innerCate:\"\",code:\"\",ch:0}";
		}
		out << "]}";  
	}else{
		out<<"{seq:\""<<strSeq<<"\",data:[]}"; 
	}  
	

	RELEASE_MYSQL_RETURN(psql, 0);
}
//请求新key值type 0 1 2 对应1级类别，2级类别，3级类别
int eMOPMaterialsSvc::GetNewCagegoryKey(int type,string &newKeyStr)
{
	string sqlstr="";
	if(type==0){
		sqlstr="SELECT MAX(key1)+1 AS newkey FROM blm_emop_etl.t50_emop_materials_cat1"; 
	}else if(type==1){
		sqlstr="SELECT MAX(key2)+1 AS newkey  FROM blm_emop_etl.t50_emop_materials_cat2"; 
	}else if(type==2){
		sqlstr="SELECT MAX(key3)+1 AS newkey FROM blm_emop_etl.t50_emop_materials_category"; 
	}
	MySql* psql = CREATE_MYSQL; 
	CHECK_MYSQL_STATUS(psql->Query(sqlstr.c_str())>=0, 3);

	int maxNum=0;
	if(psql->NextRow())
		READMYSQL_INT(newkey,maxNum,0);
	char maxNumChar[20]="";
	sprintf(maxNumChar,"%03d",maxNum);
		newKeyStr = maxNumChar;
	RELEASE_MYSQL_RETURN(psql, 0);
}

//Mid:0x60  SID: 0x0002
//{seq:””,key1:”001”, key2:””,nc:"",ne:””,innerCate:"",code:"00&12" }
//{seq:””,key:””,category:""}
int eMOPMaterialsSvc::NewMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::NewMaterialsCatInfo]bad format:", jsonString, 1); 
	string strCode = root.getv("code", "");
	string strNc = root.getv("nc", "");
	string strNe = root.getv("ne", "");
	string strKey1= root.getv("key1", "");
	string strKey2= root.getv("key2", "");
	string strKey3= root.getv("key3", ""); 
	string strInnerCate= root.getv("innerCate", "");
	string strSeq= root.getv("seq", "");
	
	char sql[1024] = "";
	MySql* psql = CREATE_MYSQL; 

	char category[50]=""; //三级时，有值
	string newKey="";
	if(strKey1.empty()) 
	{//新增类别1 
		GetNewCagegoryKey(0,newKey); 
		sprintf(sql,"INSERT INTO blm_emop_etl.t50_emop_materials_cat1(KEY1,KEY1_DESC,KEY1_DESC_EN,INNER_CAT,CODE) VALUES('%s','%s','%s','%s','%s');",
			newKey.c_str(),strNc.c_str(),strNe.c_str(),strInnerCate.c_str(),strCode.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	} 
	else if(!strKey1.empty() && strKey2.empty()) 
	{//新增类别2 
		GetNewCagegoryKey(1,newKey); 
		sprintf(sql,"INSERT INTO blm_emop_etl.t50_emop_materials_cat2(KEY1,KEY2,KEY2_DESC,KEY2_DESC_EN) VALUES('%s','%s','%s','%s');",
			strKey1.c_str(),newKey.c_str(),strNc.c_str(),strNe.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else if(!strKey1.empty() && !strKey2.empty() && strKey3.empty()) 
	{//新增类别3 
		GetNewCagegoryKey(2,newKey); 
		sprintf(category,"%s%s%s",strKey1.c_str(),strKey2.c_str(),newKey.c_str());
		sprintf(sql,"INSERT INTO blm_emop_etl.t50_emop_materials_category(CATEGORY,KEY1,KEY2,KEY3,KEY3_DESC,KEY3_DESC_EN) VALUES('%s','%s','%s','%s','%s','%s');",
			category,strKey1.c_str(),strKey2.c_str(),newKey.c_str(),strNc.c_str(),strNe.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	out<<"{seq:\""<<strSeq<<"\",key:\""<<newKey<<"\",category:\""<<category<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//Mid:0x60  SID: 0x0003
//{cid:????,code:????, nc:"",ne:"", innerCate:"",seq:???? }
//{eid:0,seq:????,etype:1,id:????}
int eMOPMaterialsSvc::UpdateMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::UpdateMaterialsCatInfo]bad format:", jsonString, 1);
	string strCode = root.getv("code", "");
	string strNc = root.getv("nc", "");
	string strNe = root.getv("ne", "");
	string strKey1= root.getv("key1", "");
	string strKey2= root.getv("key2", "");
	string strKey3= root.getv("key3", ""); 
	string strInnerCate= root.getv("innerCate", "");
	string strSeq= root.getv("seq", "");
	
	char sql[1024] = "";
	MySql* psql = CREATE_MYSQL;  
 
	if(!strKey3.empty() && !strKey2.empty() && !strKey1.empty()) 
	{//类别3  
		sprintf(sql,"UPDATE blm_emop_etl.t50_emop_materials_category SET KEY3_DESC='%s',KEY3_DESC_EN='%s' WHERE KEY1='%s' AND KEY2='%s' AND key3='%s'",
			strNc.c_str(),strNe.c_str(),strKey1.c_str(),strKey2.c_str(),strKey3.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	} 
	else if(strKey3.empty() && !strKey2.empty() && !strKey1.empty()) 
	{//类别2 
		sprintf(sql,"UPDATE blm_emop_etl.t50_emop_materials_cat2 SET KEY2_DESC='%s',KEY2_DESC_EN='%s' WHERE KEY1='%s' AND KEY2='%s'",
			strNc.c_str(),strNe.c_str(),strKey1.c_str(),strKey2.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else if(strKey3.empty() && strKey2.empty() && !strKey1.empty()) 
	{//类别1 
		sprintf(sql,"UPDATE blm_emop_etl.t50_emop_materials_cat1 SET KEY1_DESC='%s',KEY1_DESC_EN='%s',INNER_CAT='%s',CODE='%s' WHERE KEY1='%s'",
			strKey1.c_str(),strNc.c_str(),strNe.c_str(),strInnerCate.c_str(),strCode.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

// Mid:0x60 SID: 0x0004
// { cid:????,seq:???? }
// {eid:0,seq:????,etype:1}
int eMOPMaterialsSvc::DelMaterialsCatInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DelMaterialsCatInfo]bad format:", jsonString, 1);
	string strKey1= root.getv("key1", "");
	string strKey2= root.getv("key2", "");
	string strKey3= root.getv("key3", "");  
	string strSeq= root.getv("seq", "");
	
	int etype=0;
	char sql[1024] = "";
	MySql* psql = CREATE_MYSQL;  
 
	if(!strKey3.empty() && !strKey2.empty() && !strKey1.empty()) 
	{//类别3  
		sprintf(sql,"SELECT COUNT(*) FROM blm_emop_etl.t50_emop_materials_category  t1 \
					INNER JOIN blm_emop_etl.T50_EMOP_MATERIALS t2 ON t1.CATEGORY=t2.CATEGORY \
					WHERE t1.KEY1='%s' AND t1.KEY2='%s' AND t1.KEY3='%s'",
					strKey1.c_str(),strKey2.c_str(),strKey3.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);
		int countUsed=0;
		if(psql->NextRow()){
			READMYSQL_INT(countNum, countUsed,0);
		}  
		if(countUsed==0){
			etype=0;
			sprintf(sql,"DELETE FROM blm_emop_etl.t50_emop_materials_category WHERE KEY1='%s' AND KEY2='%s' and KEY3='%s' ",
				strKey1.c_str(),strKey2.c_str(),strKey3.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
		else
		{
			etype=1;
		}
	} 
	else if(strKey3.empty() && !strKey2.empty() && !strKey1.empty()) 
	{//类别2 
		sprintf(sql,"SELECT COUNT(*) as countNum FROM blm_emop_etl.t50_emop_materials_category WHERE where KEY1='%s' and KEY2='%s' ",
				strKey1.c_str(),strKey2.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);

		int countUsed=0;
		if(psql->NextRow()){
			READMYSQL_INT(countNum, countUsed,0);
		}  
		if(countUsed==0){
			etype=0;
			sprintf(sql,"delete from blm_emop_etl.t50_emop_materials_cat2 where KEY1='%s' and KEY2='%s' ",
				strKey1.c_str(),strKey2.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
		else
		{
			etype=1;
		}
	}
	else if(strKey3.empty() && strKey2.empty() && !strKey1.empty()) 
	{//类别1 
		sprintf(sql,"SELECT COUNT(*) as countNum FROM blm_emop_etl.t50_emop_materials_cat2 WHERE KEY1='%s' ",
			strKey1.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0, 3);

		int countUsed=0;
		if(psql->NextRow()){
			READMYSQL_INT(countNum, countUsed,0);
		}  
		if(countUsed==0){
			etype=0;
			sprintf(sql,"delete from blm_emop_etl.t50_emop_materials_cat1 where  KEY1='%s' ",
				strKey1.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3); 
		}
		else
		{
			etype=1;
		}
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0,etype:"<<etype<<"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//void transchar(char *des,char *src)
//{
//	while(*src++!='\0')
//	{
//		if(*src!='\"')
//			*des++=*src;
//		else
//			*des++="\\\"";
//	}
//}
//Mid:0x60  SID: 0x0005
//{ cid:????, seq:}
//{eid:0,seq:????,cid:????,info:[{id:"", code:????,nameCN:"",nameEN:"",spec:"",unit:????,flag:????,remark:????.lifetime:????,applytime:???? },{}]}
int eMOPMaterialsSvc::getCatMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{ 
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getMaterialsInfo]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");
	string strkey1 = root.getv("key1", "");
	string strkey2 = root.getv("key2", "");
	string strkey3 = root.getv("key3", "");
	string lang = root.getv("lan", "");

	MySql* psql = CREATE_MYSQL;

	stringstream appOs;
	appOs<<"";
	if(!strkey1.empty()){
		appOs<<" WHERE t1.KEY1='"<<strkey1<<"' ";
		if(!strkey2.empty()){
			appOs<<" and t1.key2='"<<strkey2<<"' ";
			if(!strkey3.empty()){
				appOs<<" and t1.key3='"<<strkey3<<"' ";
			}
		}
	} 
	char sql[1024] = ""; 
	sprintf (sql, "SELECT t1.CATEGORY,t1.KEY1,t1.KEY2,t1.KEY3,t2.MATERIALS_ID,t2.MATERIALS_CODE,t2.NAME_CN,t2.NAME_EN,t2.SPEC_CN,t2.SPEC_EN,t2.UNIT_CN,t2.UNIT_EN,t2.STORE_FLAG, \
				  t2.REMARK_CN,t2.REMARK_EN,t2.LIFE_TIME,t2.APPLY_PERIOD \
				  FROM blm_emop_etl.t50_emop_materials t2 \
				  LEFT JOIN blm_emop_etl.t50_emop_materials_category t1  ON t1.CATEGORY = t2.CATEGORY \
				  %s ORDER BY t1.KEY1,t1.KEY2,t1.KEY3,t2.MATERIALS_CODE ASC;",
				  appOs.str().c_str());

	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char category[100] = "";
	char key1[100] = "";
	char key2[100] = "";
	char key3[100] = "";
	char mid[100] = "";
	char catcode[100] = "";
	string name="";
	string spec="";
	string remark; 
	short flag=0; 
	float fLifeTime=0.00001;
	float fAPeriod=0.00001;
	char unit[100]="";
	int cnt = 0; 
	string strMid,strPics;
	
	out<<"{eid:0,seq:\""<<strSeq.c_str()<<"\",info:[";
	while (psql->NextRow())
	{
		READMYSQL_STR(CATEGORY, category);
		READMYSQL_STR(KEY1, key1);
		READMYSQL_STR(key2, key2);
		READMYSQL_STR(key3, key3); 
		READMYSQL_STR(MATERIALS_ID, mid);
		READMYSQL_STR(MATERIALS_CODE, catcode);
		READMYSQL_DOUBLE(LIFE_TIME, fLifeTime, 0.0);
		READMYSQL_DOUBLE(APPLY_PERIODFROM, fAPeriod, 0.0);
		READMYSQL_INT(STORE_FLAG, flag, -1); 
		
		if(lang=="en_US"){ 
			READMYSQL_STRING_JSON(NAME_EN, name); 
			READMYSQL_STRING_JSON(SPEC_EN, spec);
			READMYSQL_STRING_JSON(REMARK_EN, remark);
			READMYSQL_STR(UNIT_EN, unit); 
		}else{
			READMYSQL_STRING_JSON(NAME_CN, name); 
			READMYSQL_STRING_JSON(SPEC_CN, spec);
			READMYSQL_STRING_JSON(REMARK_CN, remark);
			READMYSQL_STR(UNIT_CN, unit);  
		}
		 
		strMid=mid;
		GetPics(strPics,strMid);
		if (cnt++)
			out << ",";
		out << "{cate:\""<<category<<"\",key1:\""<<key1<<"\",key2:\""<<key2<<"\",key3:\""<<key3<<"\",id:\"" << mid << "\",code:\"" << catcode << "\",name:\"" << name.c_str() << "\",spec:\"" << spec<< "\",unit:\""<<unit<< "\",flag:\"" << flag << "\",remark:\"" << remark << "\",lt:\"" << fLifeTime << "\",at:\"" << fAPeriod<< "\",pics:[" << strPics << "]}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
int eMOPMaterialsSvc::GetPics(string &o_strPics,string &i_strMid)
{
	MySql* psql1 = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "SELECT pick_id FROM blm_emop_etl.t50_emop_picture WHERE obj_type='1' AND obj_id='%s'",i_strMid.c_str());
	if(!(psql1->Query(sql)))
	{RELEASE_MYSQL_RETURN(psql1, 3);}
	char index[100] = "";
	int cnt = 0;
	o_strPics="";	
	while (psql1->NextRow())
	{
		READMYSQL_STR1(pick_id, index,psql1);

		if (cnt++)
			o_strPics += ",";
		o_strPics+="\"";
		o_strPics +=index;
		o_strPics+="\"";
	}
	RELEASE_MYSQL_RETURN(psql1, 0);
}
//Mid:0x60 SID: 0x0006
//{code:"",nc:"",ne:"",spec:"",unit:????,flag:????,remark:????.lt:????,at:????,cate:????, seq:????}
//{eid:0,seq:????,id:????etype:1}
int eMOPMaterialsSvc::NewMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::NewMaterialsCatInfo]bad format:", jsonString, 1);
	string strCode = root.getv("code", "");
	string strName = root.getv("name", ""); 
	string strSpec = root.getv("spec", "");
	string strUnit = root.getv("unit", "");
	string strFlag = root.getv("flag", "");
	string strRemark = root.getv("remark", "");
	int liftTime = root.getv("lt", 0);
	int applytime = root.getv("at", 0);
	string strCate = root.getv("cate", "");
	string strSeq= root.getv("seq", "");
	string lang= root.getv("lan", "");


	string sequence="";
	GET_EMOP_SEQUENCE_STRING(M,sequence)
		string strMatId="M"+CurLocalDateSeq()+sequence;
	string curdateStr=GetCurrentTmStr();

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	// cat_code 改为

		string name;
		string spec;
		string remark;
		string unit;
		
		MaterialBaseInfo materialInfo; 
		materialInfo.m_materialId=strMatId;
		materialInfo.m_category=strCate;
		materialInfo.m_catCode=strCode;
		materialInfo.m_storeFlag=atoi(strFlag.c_str());
		materialInfo.m_lifeTime=IntToStr(liftTime);
		materialInfo.m_applyPeriod=IntToStr(applytime);
		if(lang=="en_US"){ 
			name="NAME_EN";
			spec="SPEC_EN";
			remark="REMARK_EN"; 
			unit="UNIT_EN";  
			materialInfo.m_name_en=StrReplace(strName,"\"","\\\"");
			materialInfo.m_unit_en=strUnit;
			materialInfo.m_spec_en=strSpec; 
			materialInfo.m_remark_en=strRemark;
		}else{
			name="NAME_CN";
			spec="SPEC_CN";
			remark="REMARK_CN"; 
			unit="UNIT_CN";
			materialInfo.m_name_cn=StrReplace(strName,"\"","\\\"");
			materialInfo.m_unit_cn=strUnit;
			materialInfo.m_spec_cn=strSpec; 
			materialInfo.m_remark_cn=strRemark;
		}

	sprintf(sql,\
		"insert into blm_emop_etl.T50_EMOP_MATERIALS \
		(MATERIALS_ID,CATEGORY,MATERIALS_CODE, %s ,%s,%s,%s,STORE_FLAG,LIFE_TIME,APPLY_PERIOD,OP_DATE) \
		values ('%s','%s','%s','%s','%s','%s','%s','%s',%d,%d,'%s')", 
		name.c_str(),spec.c_str(),remark.c_str(),unit.c_str(),
		strMatId.c_str(),strCate.c_str(),strCode.c_str(),
		name.c_str(),strSpec.c_str(),strRemark.c_str(),strUnit.c_str(),
		strFlag.c_str(),liftTime,applytime,curdateStr.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
 
	g_eMOPCommDataSvr::instance()->AddMaterialItem(materialInfo);


	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",id:\""<<strMatId.c_str()<<"\","<<"etype:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//Mid:0x60 SID: 0x0007
//{id:"",code:????,nc:"",ne:"",spec:"",unit:????,flag:????,remark:????.lt:????,at:????,seq:????}
//{eid:0,seq:????,etype:1}
int eMOPMaterialsSvc::UpdateMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::UpdateMaterialsCatInfo]bad format:", jsonString, 1);
	string strId = root.getv("id", "");
	string strCode = root.getv("code", "");
	string strName = root.getv("name", ""); 
	string strSpec = root.getv("spec", "");
	string strUnit = root.getv("unit", "");
	string strFlag = root.getv("flag", "");
	string strRemark = root.getv("remark", "");
	int liftTime = root.getv("lt", 0);
	int applytime = root.getv("at", 0); 
	string strSeq= root.getv("seq", "");
	string lang= root.getv("lan", "");
	
	string name;
	string spec;
	string remark;
	string unit;
	if(lang=="en_US"){ 
		name="NAME_EN";
		spec="SPEC_EN";
		remark="REMARK_EN"; 
		unit="UNIT_EN";   
	}else{
		name="NAME_CN";
		spec="SPEC_CN";
		remark="REMARK_CN"; 
		unit="UNIT_CN"; 
	}

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	string timestr=GetCurrentTmStr();
	sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_MATERIALS SET MATERIALS_CODE = '%s', \
				  %s = '%s', %s = '%s',%s='%s',%s='%s',\
				  STORE_FLAG='%s',LIFE_TIME=%d,APPLY_PERIOD=%d,OP_DATE='%s' WHERE MATERIALS_ID = '%s'",
				  strCode.c_str(),
				  name.c_str(),strName.c_str(),
				  spec.c_str(),strSpec.c_str(),
				  remark.c_str(),strRemark.c_str(),
				  unit.c_str(),strUnit.c_str(),
				  strFlag.c_str(),liftTime,applytime,timestr.c_str(),strId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	MaterialBaseInfo *pMaterialInfo=g_eMOPCommDataSvr::instance()->GetMaterialItemById(strId); 
	if (!pMaterialInfo)
	{  
		pMaterialInfo->m_catCode=strCode;
		pMaterialInfo->m_storeFlag=atoi(strFlag.c_str());
		pMaterialInfo->m_lifeTime=IntToStr(liftTime);
		pMaterialInfo->m_applyPeriod=IntToStr(applytime);
		if(lang=="en_US"){  
			pMaterialInfo->m_name_en=StrReplace(strName,"\"","\\\"");
			pMaterialInfo->m_unit_en=strUnit;
			pMaterialInfo->m_spec_en=strSpec; 
			pMaterialInfo->m_remark_en=strRemark;
		}else{ 
			pMaterialInfo->m_name_cn=StrReplace(strName,"\"","\\\"");
			pMaterialInfo->m_unit_cn=strUnit;
			pMaterialInfo->m_spec_cn=strSpec; 
			pMaterialInfo->m_remark_cn=strRemark;
		} 
	}

	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\","<<"etype:0}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
//Mid:0x60  SID: 0x0008
//{id:"",seq:???? }
//{eid:0,seq:????}
int eMOPMaterialsSvc::DelMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DelMaterialsInfo]bad format:", jsonString, 1);
	string strId = root.getv("id", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//char gmt0now[20];
	//GmtNow(gmt0now);
	sprintf (sql, "DELETE FROM blm_emop_etl.T50_EMOP_MATERIALS WHERE MATERIALS_ID = '%s'", strId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	g_eMOPCommDataSvr::instance()->DeleteMaterialItem(strId);

	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
//Mid:0x60  SID: 0x0009
//{ keyword:????,seq:???? }
//{seq:????,info:[{id:"",code:????,nc:"",ne:"",spec:"",unit:????,flag:????,remark:????.lt:????,at:????,cate:????,cateName:????},{}]}
int eMOPMaterialsSvc::SearchMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::SearchMaterialsInfo]bad format:", jsonString, 1);
	string strKeyword = root.getv("keyword", "");
	string strSeq = root.getv("seq", ""); 
	string lang = root.getv("lan", "");

	if(strKeyword.empty()){
		out<<"{seq:\""<<strSeq.c_str()<<"\",info:[]}";
		return 0;
	}

	map<string,MaterialBaseInfo> &mapMaterials=g_eMOPCommDataSvr::instance()->GetAllMaterials();

	out<<"{seq:\""<<strSeq.c_str()<<"\",info:[";
 
	char keyUpper[128]="";  
	strcpy(keyUpper,strKeyword.c_str());
	StrUpr(keyUpper); 
	wstring wstrKey = CodeConverter::Utf8ToUnicode(strKeyword.c_str());
	int idx=0;
	map<string,MaterialBaseInfo>::iterator it=mapMaterials.begin();  
	for (;it!=mapMaterials.end();it++)
	{
		MaterialBaseInfo materialItem=it->second;
		char NameUper[128]="";   
		strcpy(NameUper,materialItem.m_name_en.c_str()); 
		StrUpr(NameUper); 
		wstring wstrName = CodeConverter::Utf8ToUnicode(materialItem.m_name_cn.c_str());
		 
		if (wstrName.find(wstrKey)!=wstring::npos||strstr(NameUper,keyUpper)!=NULL||strstr(materialItem.m_catCode.c_str(),strKeyword.c_str())!=NULL)
		{
			if (idx>0)
			{
				out<<",";
			}

			string name="";
			string sepc="";
			string remark="";
			string unit="";
			if(lang=="en_US"){ 
				name = materialItem.m_name_en;
				sepc = materialItem.m_spec_en;
				remark = materialItem.m_remark_en;
				unit = materialItem.m_unit_en;
			}else{
				name = materialItem.m_name_cn;
				sepc = materialItem.m_spec_cn;
				remark = materialItem.m_remark_cn;
				unit = materialItem.m_unit_cn;
			}

			string strPics="";	 
			for (vector<string>::iterator picIt=materialItem.picIds.begin();picIt!=materialItem.picIds.end();picIt++)
			{ 
				if (picIt!=materialItem.picIds.begin())
					strPics += ",";
				strPics+="\"";
				strPics +=(*picIt);
				strPics+="\"";
			}
			out << "{cate:\""<<materialItem.m_category<<"\",key1:\""<<materialItem.m_key1<<"\",key2:\""<<materialItem.m_key2<<"\",key3:\""<<materialItem.m_key3;
			out<<"\",id:\"" << materialItem.m_materialId << "\",code:\"" << materialItem.m_catCode << "\",name:\"" << name<< "\",spec:\"" << sepc<< "\",unit:\""<<unit<< "\",flag:\"" << materialItem.m_storeFlag << "\",remark:\"" << remark << "\",lt:\"" << materialItem.m_lifeTime << "\",at:\"" << materialItem.m_applyPeriod<< "\",pics:[" << strPics << "]}";
	
			idx++;
		}
	}

	out << "]}";

	return 0;

}
//Mid:0x60  SID: 0x000a
//{shipid:\"%s\",departid:\"%s\",seq:????}
//{seq:????,info:[{id:"",code:????,nc:"",ne:"",innerCate:"" },{}]}
int eMOPMaterialsSvc::getShipMCatInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getShipMCatInfo]bad format:", jsonString, 1);
	string strShipID=root.getv("shipid", "");
	string strDptID=root.getv("departid", "");
	string strSeq = root.getv("seq", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",info:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	// 
	sprintf (sql, "SELECT distinct t1.MATERIALS_CAT_ID,t2.CAT_CODE,t2.NAME_CN,t2.NAME_EN ,t2.COMPANY_CAT_ID FROM blm_emop_etl.T50_EMOP_MATERIALS_SHIP t1 left join \
				  blm_emop_etl.T50_EMOP_MATERIALS_CATS t2 on t1.MATERIALS_CAT_ID=t2.MATERIALS_CAT_ID \
				  WHERE t1.SHIPID = '%s' and t1.DEPARTMENT_CODE= '%s' order by t2.CAT_CODE",strShipID.c_str(),strDptID.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	/*vector <string> vecCatsID;
	char cid[100] = "";
	string strCatsid;
	while (psql->NextRow())
	{
	READMYSQL_STR(MATERIALS_CAT_ID, cid);
	strCatsid=cid;
	vecCatsID.push_back(strCatsid);
	}*/
	//RELEASE_MYSQL_NORETURN(psql, 0);	
	char cid[100] = "";
	char code[100] = "";
	char nc[128]="";
	char ne[128]="";
	char innerCate[128] = "";
	int cnt = 0;
	out << "[";

	//int iSize=vecCatsID.size();
	//for(int i=0;i<iSize;i++)
	while(psql->NextRow())
	{
		/*sprintf (sql, "SELECT CAT_CODE, NAME_CN, NAME_EN ,COMPANY_CAT_ID FROM blm_emop_etl.T50_EMOP_MATERIALS_CATS WHERE MATERIALS_CAT_ID = '%s'",vecCatsID.at(i).c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->NextRow())
		{*/
		READMYSQL_STR(MATERIALS_CAT_ID, cid);
		READMYSQL_STR(CAT_CODE, code);
		READMYSQL_STR(NAME_CN, nc);
		READMYSQL_STR(NAME_EN, ne);
		if (cnt++)
			out << ",";
		out << "{id:\"" << /*vecCatsID.at(i).c_str()*/ cid<< "\",code:\"" << code << "\",nc:\"" << nc << "\",ne:\"" << ne <<"\",innerCate:\"" << innerCate<< "\"}";
		//}
	}	
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//Mid:0x60  SID: 0x000b
//{shipid:””,cid:””, departid:””, seq:}
//{seq:””,info:[{id:"",code:””,nc:"",ne:"",spec:"",unit:””,flag:””,remark:””.lt:””,at:””,materialno:””,storeNum:””,inNum:””,outNum:””,locate:””,lowNum:””,highNum:”” },{}]}
int eMOPMaterialsSvc::getShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
 	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getMaterialsInfo]bad format:", jsonString, 1);
	string strSeq = root.getv("seq", "");
	string strkey1 = root.getv("key1", "");
	string strkey2 = root.getv("key2", "");
	string strkey3 = root.getv("key3", "");
	string strshipid = root.getv("shipid", "");
	string strdepartid = root.getv("departid", "");
	string lang = root.getv("lan", "");

	MySql* psql = CREATE_MYSQL;

	stringstream appOs;
	appOs<<"";
	if(!strkey1.empty()){
		appOs<<" AND t_cat.KEY1='"<<strkey1<<"' ";
		if(!strkey2.empty()){
			appOs<<" and t_cat.key2='"<<strkey2<<"' ";
			if(!strkey3.empty()){
				appOs<<" and t_cat.key3='"<<strkey3<<"' ";
			}
		}
	} 
	char sql[2048] = ""; 
	sprintf (sql, "SELECT t_cat.CATEGORY,t_cat.KEY1,t_cat.KEY2,t_cat.KEY3, \
				  t_m.MATERIALS_ID,t_m.MATERIALS_CODE,t_m.NAME_CN,t_m.NAME_EN,t_m.SPEC_CN,t_m.SPEC_EN,t_m.UNIT_CN,t_m.UNIT_EN,t_m.STORE_FLAG,t_m.REMARK_CN,t_m.REMARK_EN,t_m.LIFE_TIME,t_m.APPLY_PERIOD, \
				  t_ship.LOCATION_ID,t_ship.QUANTITY,t_ship.LOW_QUANTITY,t_ship.HIGH_QUANTITY \
				  FROM \
				  blm_emop_etl.T50_EMOP_MATERIALS_SHIP t_ship \
				  INNER JOIN blm_emop_etl.T50_EMOP_MATERIALS t_m ON t_ship.MATERIALS_ID = t_m.MATERIALS_ID \
				  INNER JOIN blm_emop_etl.t50_emop_materials_category t_cat  ON t_cat.CATEGORY = t_m.CATEGORY \
				  WHERE t_ship.SHIPID='%s' AND t_ship.DEPARTMENT_CODE='%s' \
				  %s  ORDER BY t_cat.KEY1,t_cat.KEY2,t_cat.KEY3,t_m.MATERIALS_CODE ASC;",
				  strshipid.c_str() ,strdepartid.c_str(),appOs.str().c_str());

	DEBUG_LOG(sql);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char category[100] = "";
	char key1[100] = "";
	char key2[100] = "";
	char key3[100] = "";
	char mid[100] = "";
	char catcode[100] = "";
	string name="";
	string spec="";
	string remark; 
	short flag=0; 
	float fLifeTime=0.00001;
	float fAPeriod=0.00001;
	char unit[100]="";
	int cnt = 0; 
	char locate[200]="";
	int highQuantity=0;
	int lowQuantity=0;
	int quantity=0;
	string strMid,strPics;
	
	out<<"{eid:0,seq:\""<<strSeq.c_str()<<"\",info:[";
	while (psql->NextRow())
	{
		READMYSQL_STR(CATEGORY, category);
		READMYSQL_STR(KEY1, key1);
		READMYSQL_STR(key2, key2);
		READMYSQL_STR(key3, key3); 
		READMYSQL_STR(MATERIALS_ID, mid);
		READMYSQL_STR(MATERIALS_CODE, catcode);
		READMYSQL_DOUBLE(LIFE_TIME, fLifeTime, 0.0);
		READMYSQL_DOUBLE(APPLY_PERIODFROM, fAPeriod, 0.0);
		READMYSQL_INT(STORE_FLAG, flag, -1); 
		READMYSQL_STR(LOCATION_ID, locate);
		READMYSQL_INT(HIGH_QUANTITY, highQuantity,0); 
		READMYSQL_INT(LOW_QUANTITY, lowQuantity,0); 
		READMYSQL_INT(QUANTITY, quantity,0);
		
		if(lang=="en_US"){ 
			READMYSQL_STRING_JSON(NAME_EN, name); 
			READMYSQL_STRING_JSON(SPEC_EN, spec);
			READMYSQL_STRING_JSON(REMARK_EN, remark);
			READMYSQL_STR(UNIT_EN, unit); 
		}else{
			READMYSQL_STRING_JSON(NAME_CN, name); 
			READMYSQL_STRING_JSON(SPEC_CN, spec);
			READMYSQL_STRING_JSON(REMARK_CN, remark);
			READMYSQL_STR(UNIT_CN, unit);  
		}
		 
		strMid=mid;
		GetPics(strPics,strMid);
		if (cnt++)
			out << ",";
		out << "{cate:\""<<category<<"\",key1:\""<<key1<<"\",key2:\""<<key2<<"\",key3:\""<<key3<<"\",id:\"" << mid << "\",code:\"" << catcode << "\",name:\"" << name.c_str() << "\",spec:\"" << spec<< "\",unit:\""<<unit<< "\",flag:\"" << flag << "\",remark:\"" << remark << "\",lt:\"" << fLifeTime << "\",at:\"" << fAPeriod<< "\",locate:\""<<locate<<"\",storeNum:"<<quantity<<",lowNum:"<<lowQuantity<<",highNum:"<<highQuantity<<",pics:[" << strPics << "]}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//Mid:0x60  SID: 0x000c
//{shipId:????,departId:????,cate:????,mid:"1|2|3.." }
//{eid:0,seq:????,ids:?? 1|2|3..??}
int eMOPMaterialsSvc::NewShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::NewShipMaterialsInfo]bad format:", jsonString, 1);
	//   string strCode = root.getv("code", "");
	//string strNc = root.getv("nc", "");
	//string strNe = root.getv("ne", "");
	//string strSpec = root.getv("spec", "");
	//string strFlag = root.getv("flag", "");
	//string strRemark = root.getv("remark", "");
	//string strLt = root.getv("lt", "");
	//string strAt = root.getv("at", "");
	//string strCate = root.getv("cate", "");
	string strSeq= root.getv("seq", "");
	string strShipid = root.getv("shipId", "");
	string strDptid = root.getv("departId", "");
	string strCate = root.getv("cate", "");
	string strMids = root.getv("mid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	Tokens svrList;	
	if(strMids.find("|") != string::npos)
	{					 
		svrList = StrSplit(strMids, "|");
		for(int i=0; i<(int)svrList.size(); i++)
		{
			if(svrList[i].empty())
				continue;
			sprintf(sql, 
				"select *from blm_emop_etl.T50_EMOP_MATERIALS_SHIP WHERE SHIPID='%s' AND MATERIALS_ID='%s'",strShipid.c_str(),svrList[i].c_str());	
			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
			if(psql->GetRowCount()>0)
				continue;
			sprintf(sql,\
				"insert into blm_emop_etl.T50_EMOP_MATERIALS_SHIP(MATERIALS_ID,SHIPID,DEPARTMENT_CODE,Materials_Cat_ID) values ('%s','%s','%s','%s')",\
				svrList[i].c_str(),strShipid.c_str(),strDptid.c_str(),strCate.c_str());	  			
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		}
	} else
	{
		sprintf(sql, 
			"select *from blm_emop_etl.T50_EMOP_MATERIALS_SHIP WHERE SHIPID='%s' AND MATERIALS_ID='%s'",strShipid.c_str(),strMids.c_str());	
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->GetRowCount()<=0)
		{
			sprintf(sql,\
				"insert into blm_emop_etl.T50_EMOP_MATERIALS_SHIP(MATERIALS_ID,SHIPID,DEPARTMENT_CODE,Materials_Cat_ID) values ('%s','%s','%s','%s')",\
				strMids.c_str(),strShipid.c_str(),strDptid.c_str(),strCate.c_str());	  
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		}
	}

	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",ids:\""<<strMids.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
//Mid:0x60  SID: 0x000d 
//{ id:"", locate:??????seq:"",shipid:””,departid:””}
//{eid:0}
int eMOPMaterialsSvc::UpdateShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::UpdateShipMaterialsInfo]bad format:", jsonString, 1);
	string strId = root.getv("id", "");
	//   string strCode = root.getv("code", "");
	//string strNc = root.getv("nc", "");
	//string strNe = root.getv("ne", "");
	//string strSpec = root.getv("spec", "");
	//string strFlag = root.getv("flag", "");
	//string strRemark = root.getv("remark", "");
	//string strLt = root.getv("lt", "");
	//string strAt = root.getv("at", "");
	string strLocate = root.getv("locate", "");
	string strShipid = root.getv("shipid", "");
	string strDptid = root.getv("departid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char gmt0now[20];
	GmtNow(gmt0now);
	//	sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_MATERIALS_SHIP SET CAT_CODE = '%s', NAME_CN = '%s', NAME_EN = '%s',SPEC_CN='%s',STORE_FLAG='%s',REMARK_CN='%s',LIFE_TIME='%s',LIFE_TIME='%s',APPLY_PERIODFROM='%s',OP_DATE='%s' WHERE MATERIALS_ID = '%s'",\
	strCode.c_str(), strNc.c_str(), strNe.c_str(),strSpec.c_str(),strFlag.c_str(),strRemark.c_str(),strLt.c_str(),strAt.c_str(),gmt0now);
	sprintf(sql,\
		"update blm_emop_etl.T50_EMOP_MATERIALS_SHIP set LOCATION_ID='%s' where SHIPID='%s' and DEPARTMENT_CODE='%s' and MATERIALS_ID='%s'",\
		strLocate.c_str(),strShipid.c_str(),strDptid.c_str(),strId.c_str());
	FILE *pf;
	pf=fopen("/tmp/600d.log","wb");
	fwrite(sql,sizeof(char),strlen(sql),pf);
	fclose(pf);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\","<<"etype:0}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
//Mid:0x60  SID: 0x000e 
//{shipid:”%s”,departed:”%s”,id:"", seq}
//{eid:0,seq:????} 
int eMOPMaterialsSvc::DelShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DelShipMaterialsInfo]bad format:", jsonString, 1);
	string strId = root.getv("id", "");
	string strSeq= root.getv("seq", "");
	string strShipid = root.getv("shipid", "");
	string strDptid = root.getv("departed", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//char gmt0now[20];
	//GmtNow(gmt0now);
	sprintf (sql, "DELETE FROM blm_emop_etl.T50_EMOP_MATERIALS_SHIP WHERE MATERIALS_ID = '%s' and SHIPID='%s' and DEPARTMENT_CODE='%s'", \
		strId.c_str(),strShipid.c_str(),strDptid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
//Mid:0x60  SID: 0x000f

int eMOPMaterialsSvc::SearchShipMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{ 
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::SearchMaterialsInfo]bad format:", jsonString, 1);
	string strKeyword = root.getv("keyword", "");
	string strSeq = root.getv("seq", ""); 
	string lang = root.getv("lan", "");
    string strShipId = root.getv("shipid", ""); 
 

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf(sql, 
		"SELECT t_ship.MATERIALS_ID,t_ship.LOCATION_ID,t_ship.QUANTITY,t_ship.LOW_QUANTITY,t_ship.HIGH_QUANTITY,\
		t_depart.NAME_CN as DEPART_NAME_CN,t_depart.NAME_EN as DEPART_NAME_EN\
		FROM blm_emop_etl.T50_EMOP_MATERIALS_SHIP t_ship  \
		LEFT JOIN blm_emop_etl.T50_EMOP_DEPARTMENT_CODE t_depart ON t_ship.DEPARTMENT_CODE=t_depart.DEPARTMENT_CODE \
		WHERE t_ship.SHIPID='&s' ORDER BY t_ship.DEPARTMENT_CODE",strShipId.c_str());	


	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char mid[64]=""; 
 	string departName=""; 
	char locate[200]="";
	int highQuantity=0;
	int lowQuantity=0;
	int quantity=0;
	string strMid,strPics; 

	out<<"{seq:\""<<strSeq.c_str()<<"\",info:[";
	int idx=0;
	char keyUpper[128]="";  
	strcpy(keyUpper,strKeyword.c_str());
	StrUpr(keyUpper); 
	wstring wstrKey = CodeConverter::Utf8ToUnicode(strKeyword.c_str());

	while (psql->NextRow())
	{ 
		READMYSQL_STR(MATERIALS_ID, mid); 
		READMYSQL_STR(LOCATION_ID, locate);
		READMYSQL_INT(HIGH_QUANTITY, highQuantity,0); 
		READMYSQL_INT(LOW_QUANTITY, lowQuantity,0); 
		READMYSQL_INT(QUANTITY, quantity,0);
		
		if(lang=="en_US"){  
			READMYSQL_STRING(DEPART_NAME_EN, departName);
		}else{ 
			READMYSQL_STRING(DEPART_NAME_CN, departName);
		}
		 
		//		READMYSQL_STR(CAT_CODE, catcode);

		MaterialBaseInfo *pMaterialInfo=g_eMOPCommDataSvr::instance()->GetMaterialItemById(string(mid));

		if (!pMaterialInfo)
		{
			continue;
		}
 
		char NameUper[128]="";    
		strcpy(NameUper,pMaterialInfo->m_name_en.c_str()); 
		StrUpr(NameUper);

		wstring wstrName = CodeConverter::Utf8ToUnicode(pMaterialInfo->m_name_cn.c_str());  
		//搜索匹配
		if (wstrName.find(wstrKey)!=wstring::npos||strstr(NameUper,keyUpper)!=NULL||strstr(pMaterialInfo->m_catCode.c_str(),strKeyword.c_str())!=NULL)
		{
			if (idx>0)
			{
				out<<",";
			} 

			string name="";
			string sepc="";
			string remark="";
			string unit="";
			if(lang=="en_US"){ 
				name = pMaterialInfo->m_catname_en;
				sepc = pMaterialInfo->m_spec_en;
				remark = pMaterialInfo->m_remark_en;
				unit = pMaterialInfo->m_unit_en;
			}else{
				name = pMaterialInfo->m_catname_cn;
				sepc = pMaterialInfo->m_spec_cn;
				remark = pMaterialInfo->m_remark_cn;
				unit = pMaterialInfo->m_unit_cn;
			}

			out << "{cate:\""<<pMaterialInfo->m_category<<"\",key1:\""<<pMaterialInfo->m_key1<<"\",key2:\""<<pMaterialInfo->m_key2<<"\",key3:\""<<pMaterialInfo->m_key3;
			out<<"\",id:\"" << pMaterialInfo->m_materialId << "\",code:\"" << pMaterialInfo->m_catCode << "\",name:\"" << name<< "\",spec:\"" << sepc<< "\",unit:\""<<unit;
			out<< "\",flag:\"" << pMaterialInfo->m_storeFlag << "\",remark:\"" << remark << "\",lt:\"" << pMaterialInfo->m_lifeTime << "\",at:\"" << pMaterialInfo->m_applyPeriod;
			out<<"\",locate:\""<<locate<<"\",storeNum:"<<quantity<<",lowNum:"<<lowQuantity<<",highNum:"<<highQuantity<<",depart:\""<<departName<<"\",pics:[" << strPics << "]}";
			idx++;
		}
	}

	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);	

}
//MID:0x60  SID:0xf1
//{uid:??blm??,seq:??55??}
//{seq:55,content:[{id:????,name:????????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã??},{id:????,name:????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã?ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã??},{id:????,name:????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦ÃƒÂ¢Ã¢â€šÂ¬Ã…â€œÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡Ã?},{ id:????,name:?????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã???ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â±},{ id:????,name:????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â±ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦ÃƒÂ¢Ã¢â€šÂ¬Ã…â€œÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡Ã???}] }
//ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã???
//Id ??id
//Name ?????
// ???????????id????????????????
int eMOPMaterialsSvc::GetBaseData_l(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetBaseData_l]bad format:", jsonString, 1);
	string strUid = root.getv("uid", "");
	string strSeq = root.getv("seq", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT KEY1, DESC_CN FROM blm_emop_etl.t91_code WHERE CLASS_CODE = '1'");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char szID[100]="";
	char szCDesc[100] = "";
	//	int iIsvalid;
	int cnt = 0;
	out << "[";
	string strId,strDesc;
	while (psql->NextRow())
	{
		//		READMYSQL_INT(CLASS_CODE, iCCode, -1);
		READMYSQL_STR(KEY1, szID);
		READMYSQL_STR(DESC_CN, szCDesc);
		strId=szID;
		strDesc=szCDesc;
		trimstr(strId);
		trimstr(strDesc);
		//		if(!iIsvalid)
		//			continue;
		if (cnt++)
			out << ",";
		out << "{id:\"" << strId.c_str() << "\",name:\"" << strDesc.c_str() << "\"}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 2.2	??????????
//MID:0x60  SID:0xf2
//{uid:??blm??, seq:??66?? ,cid:????}
//{seq:66, cid:????,content:[{num:??11??,name:?????,remark:??USA??},{num:??12??,name:??ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦ÃƒÂ¢Ã¢â€šÂ¬Ã…â€œÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡Ã??,remark:??CHN??}]}
// ??ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã?
int eMOPMaterialsSvc::GetBaseData_r(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetBaseData_r]bad format:", jsonString, 1);
	string strUid = root.getv("uid", "");
	string strSeq = root.getv("seq", "");
	string strCid = root.getv("cid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",cid:\""<<strCid.c_str()<<"\",content:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT KEY1, DESC_CN ,REMARK FROM blm_emop_etl.t91_code WHERE KEY2 = '%s'",strCid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char szNum[100]="";
	char szCDesc[100] = "";
	char szRemark[1024] = "";
	//	int iIsvalid;
	int cnt = 0;
	out << "[";
	string strKey1,strDesc;
	while (psql->NextRow())
	{
		//		READMYSQL_INT(CLASS_CODE, iCCode, -1);
		READMYSQL_STR(KEY1, szNum);
		READMYSQL_STR(DESC_CN, szCDesc);
		READMYSQL_STR(REMARK, szRemark);
		//		if(!iIsvalid)
		//			continue;
		strKey1=szNum;
		strDesc=szCDesc;
		trimstr(strKey1);
		trimstr(strDesc);
		if (cnt++)
			out << ",";
		out << "{num:\"" << strKey1.c_str() << "\",name:\"" << strDesc.c_str() <<"\",remark:\"" << szRemark<< "\"}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 2.3	????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
//MID:0x60  SID:0xf3
//{uid:??blm??,seq:??77??,cid:????,num:??11??, name:?????,remark:??CHN??, type: 0}
//{eid:0, etype:????,seq:??77??,id:????,num:??11??,type:0}
int eMOPMaterialsSvc::NewBaseData(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::NewBaseData]bad format:", jsonString, 1);
	string strUid = root.getv("uid", "");
	string strSeq = root.getv("seq", "");
	string strCid = root.getv("cid", "");
	string strNum = root.getv("num", "");
	string strName = root.getv("name", "");
	string strRemark = root.getv("remark", "");
	int iType = root.getv("type", -1);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	if(iType ==0)// ???
	{
		sprintf(sql,\
			"insert into blm_emop_etl.t91_code(KEY2,KEY1, DESC_CN, REMARK) \
			values ('%s','%s','%s','%s')",\
			strCid.c_str(),strNum.c_str(),strName.c_str(),strRemark.c_str());
	}
	else // ???
	{
		sprintf (sql, "UPDATE blm_emop_etl.t91_code SET DESC_CN = '%s',REMARK='%s' WHERE KEY2 = '%s' and KEY1 = '%s'",\
			strName.c_str(),strRemark.c_str(),strCid.c_str(), strNum.c_str());
	}
	trimstr(strCid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:\"0\",etype:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",id:\""<<strCid.c_str()<<"\",num:\""<<strNum.c_str()<<"\",type:"<<iType<<"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 2.5	ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â°ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
//MID:0x60  SID:0xf4
//{uid:??blm??,seq:??88??,cid:????,num:??11??}
//cid ??id
//num ???
//{eid:0 ,etype:????, seq:??77??,cid:????,num:???? }
int eMOPMaterialsSvc::DelBaseData(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DelBaseData]bad format:", jsonString, 1);
	string strUid = root.getv("uid", "");
	string strSeq = root.getv("seq", "");
	string strCid = root.getv("cid", "");
	string strNum = root.getv("num", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "DELETE FROM blm_emop_etl.t91_code WHERE KEY2 = '%s' and KEY1 = '%s'",strCid.c_str(),strNum.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);	

	out<< "{eid:\"0\",etype:\"0\","<<"seq:\""<<strSeq.c_str()<<"\",cid:\""<<strCid.c_str()<<"\",num:\""<<strNum.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
// 6.19	????????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã?
//Mid:0x60 SID: 0x0013
//{ shipid:??1,2,3????,seq:???? }
//?????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
//{ seq:????,info:[{shipid:"",template:[{tid:,tn:????},{}]},{}]}
int eMOPMaterialsSvc::GetAllMterialsTmp(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetAllMterialsTmp]bad format:", jsonString, 1);

	string strShipID = root.getv("shipid", "");
	string strSeq = root.getv("seq", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",info:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	Tokens svrList;	
	char tid[100] = "";
	char tname[100] = "";	 

	if(strShipID.find(",") != string::npos)
	{					 
		svrList = StrSplit(strShipID, ",");
		//		out << "[";
		for(int i=0; i<(int)svrList.size(); i++)
		{
			if(svrList[i].empty())
				continue;			
			out<<"{shipid:\""<<svrList[i].c_str()<<"\",template:[";
			sprintf (sql, "SELECT MATERIALS_APPLY_TEMPLATE_ID, NAME FROM blm_emop_etl.t50_emop_materials_apply_template where SHIPID='%s' ",svrList[i].c_str());
			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
			int cnt = 0;
			while (psql->NextRow())
			{
				READMYSQL_STR(MATERIALS_APPLY_TEMPLATE_ID, tid);
				READMYSQL_STR(NAME, tname);
				if (cnt++)
					out << ",";
				out << "{tid:\"" << tid << "\",tn:\"" << tname<< "\"}";
			}
			out << "]}";
			if (i<((int)svrList.size()-1))
				out << ",";
		}
		//		out << "]";
	}
	else 
	{
		out<<"{shipid:\""<<strShipID.c_str()<<"\",template:[";
		sprintf (sql, "SELECT MATERIALS_APPLY_TEMPLATE_ID, NAME FROM blm_emop_etl.t50_emop_materials_apply_template where SHIPID='%s' ",strShipID.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		int cnt = 0;
		while (psql->NextRow())
		{
			READMYSQL_STR(MATERIALS_APPLY_TEMPLATE_ID, tid);
			READMYSQL_STR(NAME, tname);
			if (cnt++)
				out << ",";
			out << "{tid:\"" << tid << "\",tn:\"" << tname<< "\"}";
		}
		out << "]}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
// 6.20	??????????
//Mid:0x60 SID: 0x0014
//{ templateid:????,seq:???? }
//?????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
//{seq:,info:[{mid:"", mt:"",mc:"",nc:"",rnum:,spec:"", unit:"",remark:"" },{}]}
int eMOPMaterialsSvc::GetMterialsTmpInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetMterialsTmpInfo]bad format:", jsonString, 1);

	string strTmpID = root.getv("templateid", "");
	string strSeq = root.getv("seq", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",info:";

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "select t1.APPLY_QUANTITY,t2.* from blm_emop_etl.t50_emop_materials_template_details t1 left join blm_emop_etl.T50_EMOP_MATERIALS t2 on t1.MATERIALS_ID=t2.MATERIALS_ID where t1.MATERIALS_APPLY_TEMPLATE_ID= '%s'",strTmpID.c_str());
	FILE *pf;
	pf=fopen("/tmp/6014.log","wb");
	fwrite(sql,sizeof(char),strlen(sql),pf);
	fclose(pf);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char mid[100] = "";
	char mt[100] = "";
	char mc[100] = "";
	char nc[100] = "";
	int rnum = 0; //
	char spec[100] = "";
	char unit[100] = "";
	char remark[100] = "";

	int cnt = 0;
	out << "[";
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_ID, mid);
		READMYSQL_STR(MATERIALS_CAT_ID, mt);// 
		READMYSQL_STR(MATERIALS_CODE, mc);
		READMYSQL_STR(NAME_CN, nc);
		READMYSQL_STR(SPEC_CN, spec);
		//		READMYSQL_STR(NAME, unit);// ??ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã??ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã????????????
		READMYSQL_STR(REMARK_CN, remark);
		READMYSQL_INT(APPLY_QUANTITY,rnum ,0);
		//mid:"", mt:"",mc:"",nc:"",rnum:,spec:"", unit:"",remark:""
		if (cnt++)
			out << ",";
		out << "{mid:\"" << mid << "\",mt:\"" << mt << "\",mc:\"" << mc << "\",nc:\"" <<nc<<"\",rnum:" << rnum<< ",spec:\"" << spec <<"\",unit:\"0\""  <<",remark:\"" << remark <<"\"}";
	}
	out << "]}";
	string strTmp=out.str();

	RELEASE_MYSQL_RETURN(psql, 0);
}
// 6.21	?????????????
//Mid:0x60 SID: 0x0015
//{ shipid:????,tn:????,seq:???? }
//?????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
//{eid:0,seq:,tid:???? }
int eMOPMaterialsSvc::NewMterialsTmpName(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::NewMterialsTmpName]bad format:", jsonString, 1);
	string strShipid = root.getv("shipid", "");
	string strSeq = root.getv("seq", "");
	string strTn = root.getv("tn", "");

	// ???mpid
	string strTID="TID";
	char szTmp[32];
	long ltime =time(0);
	sprintf(szTmp,"%d",ltime);
	strTID+=szTmp;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	sprintf(sql,\
		"insert into blm_emop_etl.t50_emop_materials_apply_template(MATERIALS_APPLY_TEMPLATE_ID,NAME, SHIPID) values ('%s','%s','%s')",\
		strTID.c_str(),strTn.c_str(),strShipid.c_str() );	  
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:\"0"<<"\",seq:\""<<strSeq.c_str()<<"\",tid:\""<<strTID.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 6.22	?????????????
//Mid:0x60 SID: 0x0016
//{ tid:????,tn:????,seq:???? }
//tid:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã??d
//tn:ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?????
//?????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
//{eid:0,seq:, }
int eMOPMaterialsSvc::ModifyMterialsTmpName(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::ModifyMterialsTmpName]bad format:", jsonString, 1);
	string strTid = root.getv("tid", "");
	string strSeq = root.getv("seq", "");
	string strTn = root.getv("tn", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "UPDATE blm_emop_etl.t50_emop_materials_apply_template SET NAME = '%s' WHERE MATERIALS_APPLY_TEMPLATE_ID = '%s'",\
		strTn.c_str(),strTid.c_str() );

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:\"0"<<"\",seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 6.23	??ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â°ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã???????????
//Mid:0x60 SID: 0x0017
//{ tid:????,seq:???? }
//?????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
//{eid:0,seq:, }
int eMOPMaterialsSvc::DelMterialsTmp(const char* pUid, const char* jsonString, std::stringstream& out)
{ // 
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DelMterialsTmp]bad format:", jsonString, 1);
	string strId = root.getv("tid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_materials_apply_template WHERE MATERIALS_APPLY_TEMPLATE_ID = '%s'", strId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 6.24	??????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã?ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã??
//Mid:0x60 SID: 0x0018
//{tid:????,mids:??1|2|3???? ,seq:???? }
//{eid:0,seq:????, mids:?? 1|2|3??}
int eMOPMaterialsSvc::AddMterialsToTmp(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::AddMterialsToTmp]bad format:", jsonString, 1);

	string strTid = root.getv("tid", "");
	string strMids = root.getv("mids", "");
	string strSeq = root.getv("seq", "");
	//	out<<"{seq:\""<<strSeq.c_str()<<"\",info:";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = ""; 
	Tokens svrList;	
	if(strMids.find("|") != string::npos)
	{					 
		svrList = StrSplit(strMids, "|");
		for(int i=0; i<(int)svrList.size(); i++)
		{
			if(svrList[i].empty())
				continue;		
			sprintf(sql,\
				"insert into blm_emop_etl.t50_emop_materials_template_details(MATERIALS_APPLY_TEMPLATE_ID,MATERIALS_ID) values ('%s','%s')",\
				strTid.c_str(),svrList[i].c_str());	  			
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		}
	}
	else
	{
		sprintf(sql,\
			"insert into blm_emop_etl.t50_emop_materials_template_details(MATERIALS_APPLY_TEMPLATE_ID,MATERIALS_ID) values ('%s','%s')",\
			strTid.c_str(),strMids.c_str());	  			
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	out<< "{eid:\"0"<<"\",seq:\""<<strSeq.c_str()<<"\",mids:\""<<strMids.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 6.25	??????????
//Mid:0x60 SID: 0x0019
//{tmid:???? ,rnum: ,seq:???? }
//?????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
//{eid:0,seq:, }
int eMOPMaterialsSvc::ModifyMterialsOnTmp(const char* pUid, const char* jsonString, std::stringstream& out)
{// ????????????????????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã???????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã???ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã??ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::ModifyMterialsOnTmp]bad format:", jsonString, 1);
	string strTMId = root.getv("tmid", "");
	int iRnum = root.getv("rnum", -1);
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "Update blm_emop_etl.t50_emop_materials_template_details Set APPLY_QUANTITY='%d' WHERE MATERIALS_ID = '%s'", iRnum,strTMId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
// 6.26	??ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â°ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã??????????????
//Mid:0x60 SID: 0x001a
//{tid????,tmid:????seq:???? }
//?????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
//{eid:0,seq:, }
int eMOPMaterialsSvc::DelMterialsOnTmp(const char* pUid, const char* jsonString, std::stringstream& out)
{	
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DelMterialsOnTmp]bad format:", jsonString, 1);
	string strTId = root.getv("tid", "");
	string strTMId = root.getv("tmid", "");
	string strSeq= root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	sprintf (sql, "DELETE FROM blm_emop_etl.t50_emop_materials_template_details WHERE MATERIALS_APPLY_TEMPLATE_ID = '%s' and MATERIALS_ID='%s'", strTId.c_str(),strTMId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	out<< "{eid:\"0\","<<"seq:\""<<strSeq.c_str()<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//Mid:0x60 SID: 0x001c
//{seq:????, mid:????}
//?????ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¿ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ã?
// {seq:????id:"",code:????,nameCN:"",nameEN:"",spec:"",unit:????,flag:????,remark:????.lifetime:????,applytime:????,cid:????,ccode:????,cne:????,cnc:???? }
int eMOPMaterialsSvc::getMaterialsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::getMaterialsInfo]bad format:", jsonString, 1);
	string strMid = root.getv("mid", "");
	string strSeq = root.getv("seq", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, \
		"SELECT MATERIALS_CAT_ID,MATERIALS_CODE, NAME_CN, NAME_EN ,SPEC_CN,STORE_FLAG,REMARK_CN,LIFE_TIME,APPLY_PERIOD FROM blm_emop_etl.T50_EMOP_MATERIALS WHERE MATERIALS_ID = '%s'",\
		strMid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char cid[100] = "";
	char catcode[100] = "";
	char nc[128]="";
	char ne[128]="";
	char spec[1024]="";
	short flag=0;
	char remark[1024] = "";
	float fLifeTime=0.00001;
	float fAPeriod=0.00001;
	int cnt = 0;
	out<<"{seq:\""<<strSeq.c_str()<<"\"";
	if(psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_CAT_ID, cid);
		READMYSQL_STR(MATERIALS_CODE, catcode);
		READMYSQL_STR(NAME_CN, nc);
		READMYSQL_STR(NAME_EN, ne);
		READMYSQL_STR(SPEC_CN, spec);
		READMYSQL_STR(REMARK_CN, remark);
		READMYSQL_DOUBLE(LIFE_TIME, fLifeTime, 0.0);
		READMYSQL_DOUBLE(APPLY_PERIODFROM, fAPeriod, 0.0);
		READMYSQL_INT(STORE_FLAG, flag, -1);
		// {seq:????id:"",code:????,nameCN:"",nameEN:"",spec:"",unit:????,flag:????,remark:????.lifetime:????,applytime:????,cid:????,ccode:????,cne:????,cnc:???? }
		out << "{id:\"" << strMid.c_str() << "\",code:\"" << catcode << "\",nameCN:\"" << nc << "\",nameEN:\"" << ne <<"\",spec:\"" << spec<< "\",unit:\""<< "\",flag:\"" << flag << "\",remark:\"" << remark << "\",lifetime:\"" << fLifeTime << "\",applytime:\"" << fAPeriod << "\"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}



//获取物料入库单列表 0x50
int eMOPMaterialsSvc::GetInstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetPartsInventoryCounting]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strShipid=root.getv("shipid", "");
	string strUid=root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql, "select MATERIALS_INSTOCK_ID,OP_DATE,DEPARTMENT_ID,INSTOCK_LOCTION,TYPE,REVIEW_STATUS,STOCK_STATUS,OPERATOR,MATERIALS_PURCHASE_ORDER_ID,REMARK_CN \
				 from blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK WHERE SHIPID='%s'",strShipid.c_str());

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

		READMYSQL_STR(MATERIALS_INSTOCK_ID, orderid)
			READMYSQL_STR(OP_DATE, orderdt)
			READMYSQL_STR(DEPARTMENT_ID, departid)
			READMYSQL_STR(INSTOCK_LOCTION, instocklocation)
			READMYSQL_INT(TYPE,type,0)
			READMYSQL_INT(REVIEW_STATUS,reviewstatus,0)
			READMYSQL_INT(STOCK_STATUS,stockstatus,0)
			READMYSQL_STR(OPERATOR,operatorid)
			READMYSQL_STRING(REMARK_CN,remark)
			READMYSQL_STR(MATERIALS_PURCHASE_ORDER_ID,purchaseid)

			if (idx++)
			{
				out<<",";
			}

			out << "{id:\"" << orderid << "\",date:\"" << orderdt <<"\",depart:\"" << departid <<"\",stateC:\"" << stockstatus << "\",stateB:\"";
			out	<< reviewstatus<<"\",orderid:\"" << purchaseid<<"\",type:\"" << type<<"\",locate:\""<<instocklocation<<"\",remark:\"" << remark<<"\"}";

	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//获取某入库单下所有的物料列表 0x51
int eMOPMaterialsSvc::GetInstockOrderMaterialList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetInstockOrderMaterialList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strOrderId=root.getv("id", "");
	string strShipId=root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";

	sprintf(sql,"select T1.MATERIALS_INSTOCK_ID,T1.MATERIALS_ID,T1.QUATITY,T1.REMARK_CN,T1.INSTOCK_REST,T2.QUANTITY AS INSTOCK,T2.LOCATION_ID \
				from blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK_DETAILS T1,blm_emop_etl.T50_EMOP_MATERIALS_SHIP T2 \
				WHERE T1.MATERIALS_INSTOCK_ID='%s' AND T2.SHIPID='%s' AND T1.MATERIALS_ID=T2.MATERIALS_ID",strOrderId.c_str(),strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char materialid[64]="";
	int quantity=0;//入库数量
	int instocknum=0;//库存量
	string remark="";
	char locationid[64]="";


	string price="";

	out <<"{eid:\"0\",seq:\""<<strSeq<<"\",id:\""<<strOrderId<<"\",info:[";	

	int idx=0;

	while(psql->NextRow())
	{

		READMYSQL_STR(MATERIALS_ID, materialid)
			READMYSQL_INT(QUATITY,quantity,0)
			READMYSQL_INT(INSTOCK,instocknum,0)
			READMYSQL_STRING(REMARK_CN, remark)
			READMYSQL_STR(LOCATION_ID,locationid)


			MaterialBaseInfo *pMaterialInfo=g_eMOPCommDataSvr::instance()->GetMaterialItemById(string(materialid));

		string unitStr="";
		string specStr="";
		string materialName="";
		string materialCatName="";

		if (pMaterialInfo)
		{
			unitStr=pMaterialInfo->m_unit_cn;
			specStr=pMaterialInfo->m_spec_cn;
			materialName=pMaterialInfo->m_name_cn.empty()?pMaterialInfo->m_name_en:pMaterialInfo->m_name_cn;
			materialCatName=pMaterialInfo->m_catname_cn.empty()?pMaterialInfo->m_catname_en:pMaterialInfo->m_catname_cn;
		}

		if (idx++)
		{
			out<<",";
		}

		out << "{mcate:\"" << materialCatName << "\",mid:\"" << materialid <<"\",mname:\"" << materialName<<"\",spec:\"" << specStr <<"\",innum:\"" << quantity << "\",storenum:\"";
		out	<< instocknum <<"\",unit:\"" << unitStr<<"\",price:\""<<price<<"\",loc:\"" << locationid<<"\",rem:\"" <<remark<<"\"}";

	}

	out <<"]}";


	RELEASE_MYSQL_RETURN(psql, 0);

}
//删除物料入库单中某物料 0x52
int eMOPMaterialsSvc::DeleteInstockOrderMaterialItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DeleteInstockOrderMaterialItem]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strOrderId=root.getv("id", "");
	string strPartId=root.getv("mid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512]="";

	sprintf (sql, "delete from blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK_DETAILS where MATERIALS_INSTOCK_ID='%s' and MATERIALS_ID='%s'",strOrderId.c_str(),strPartId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;
}
//物料入库单部门长确认 0x53
int eMOPMaterialsSvc::ConformMaterialInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaterialsSvc::ConformMaterialInstockOrder]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string Id=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512]="";

	sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK SET REVIEW_STATUS='1' WHERE MATERIALS_INSTOCK_ID='%s'",Id.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//获取物料订购单列表（物料待入库） 0x54
int eMOPMaterialsSvc::GetPurchaseOrderList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetPurchaseOrderList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strShipid=root.getv("shipid", "");
	string strUid=root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql, "select T1.MATERIALS_PURCHASE_ORDER_ID,T1.DELIVERY_DATE AS ORDER_DATE,T1.REMARK_CN,T1.DEPARTMENT_CODE,T2.STOCK_STATUS \
				 from blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDERS T1 LEFT JOIN blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK T2 \
				 ON T1.SHIPID=T2.SHIPID AND T1.MATERIALS_PURCHASE_ORDER_ID=T2.MATERIALS_PURCHASE_ORDER_ID WHERE T1.SHIPID='%s'",strShipid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	map<string,PuchaseOrderItem> mapOrders;


	while (psql->NextRow())
	{
		string stockstatus="";

		PuchaseOrderItem orderItem;
		READMYSQL_STRING(MATERIALS_PURCHASE_ORDER_ID, orderItem.orderid)
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
//获取某订购单下的物料信息列表 0x55
int eMOPMaterialsSvc::GetPurchaseOrderMaterialList(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetPurchaseOrderPartList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strOrderId=root.getv("id", "");
	string strShipId=root.getv("shipId", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";

	sprintf(sql,"select T1.MATERIALS_PURCHASE_ORDER_ID,T1.MATERIALS_ID,T1.QUATITY,T1.UNIT_PRICE,T1.REMARK_CN,T2.QUANTITY AS INSTOCK,T2.LOCATION_ID \
				from blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDER_DETAILS T1 LEFT JOIN blm_emop_etl.T50_EMOP_MATERIALS_SHIP T2 \
				ON T1.MATERIALS_ID=T2.MATERIALS_ID WHERE T1.MATERIALS_PURCHASE_ORDER_ID='%s'",strOrderId.c_str());//,strShipId.c_str()

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char materialid[64]="";
	int quantity=0;//入库数量
	int instocknum=0;//
	string remark="";
	char locationid[64]="";

	string price="";

	out <<"{eid:\"0\",seq:\""<<strSeq<<"\",id:\""<<strOrderId<<"\",info:[";	

	int idx=0;

	while(psql->NextRow())
	{

		READMYSQL_STR(MATERIALS_PURCHASE_ORDER_ID, materialid)
			READMYSQL_INT(QUATITY,quantity,0)
			//READMYSQL_INT(INSTOCK,instocknum,0)
			READMYSQL_STRING(REMARK_CN, remark)
			READMYSQL_STR(LOCATION_ID,locationid)
			READMYSQL_STRING(UNIT_PRICE,price)


			MaterialBaseInfo *pMaterialInfo=g_eMOPCommDataSvr::instance()->GetMaterialItemById(string(materialid));

		string unitStr="";
		string specStr="";
		string materialName="";

		if (pMaterialInfo)
		{
			unitStr=pMaterialInfo->m_unit_cn;
			specStr=pMaterialInfo->m_spec_cn;
			materialName=pMaterialInfo->m_name_cn.empty()?pMaterialInfo->m_name_en:pMaterialInfo->m_name_cn;
		}

		if (idx++)
		{
			out<<",";
		}

		out << "{mid:\"" << materialid<<"\",name:\"" << materialName<<"\",spec:\"" << specStr <<"\",ordernum:\"" << quantity;
		out	<<"\",unit:\"" << unitStr<<"\",price:\""<<price<<"\",rem:\"" <<remark<<"\"}";

	}

	out <<"]}";


	RELEASE_MYSQL_RETURN(psql, 0);
}

//提交物料入库单0x56
int eMOPMaterialsSvc::SetMaterialInstockOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::SetMaterialInstockOrder]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strShipid=root.getv("shipId", "");
	string strDepartid=root.getv("deparId", "");
	string strLocation=root.getv("loca", "");//入库地点

	int type=1;
	int state=root.getv("state", 0);//入库状态（0：部分入库 1：全部入库）
	string purchaseid=root.getv("purchaseid", "");//订购单ID

	string sequence="";
	GET_EMOP_SEQUENCE_STRING(MR,sequence)
		string instockid="MRS"+strShipid+sequence;

	string date=CurLocalDateSeq();

	MySql* psql = CREATE_MYSQL;

	char sql[512]="";
	sprintf (sql, "INSERT INTO blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK(MATERIALS_INSTOCK_ID,OP_DATE,SHIPID,DEPARTMENT_ID,\
				  INSTOCK_LOCTION,TYPE,MATERIALS_PURCHASE_ORDER_ID,REVIEW_STATUS,STOCK_STATUS,OPERATOR,REMARK_CN) \
				  VALUES('%s','%s','%s','%s','%s',%d,'%s',%d,%d,'%s','%s')",instockid.c_str(),date.c_str(),strShipid.c_str(),
				  strDepartid.c_str(),strLocation.c_str(),type,purchaseid.c_str(),0,state,pUid,"");
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


	Json* infos = root.get("info");

	if (infos)
	{
		for (int i=0;i<infos->size();i++)
		{
			Json* materialItem=infos->get(i);
			if (!materialItem)
			{
				continue;
			}

			string materialid=materialItem->getv("mid","");
			int number=atoi(materialItem->getv("num",""));

			char sql1[512]="";
			sprintf (sql1, "INSERT INTO blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK_DETAILS(MATERIALS_INSTOCK_ID,MATERIALS_ID,QUATITY,REMARK_CN) \
						   VALUES('%s','%s',%d,'%s')",instockid.c_str(),materialid.c_str(),number,"");
			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);
		}
	}

	out <<"{eid:0,seq:\""<<strSeq<<"\",id:\""<<instockid<<"\"}";	


	RELEASE_MYSQL_RETURN(psql, 0);

}


//获取物料出库单列表 0x57 
int eMOPMaterialsSvc::GetOutstockOrderList(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetOutstockOrderList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strShipid=root.getv("shipid", "");
	string strUid=root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	sprintf(sql, "select MATERIALS_OUTSTOCK_ID,OP_DATE,DEPARTMENT_ID,OUTSTOCK_STATUS,REVIEW_STATUS,STOCK_STATUS,OPERATOR,REMARK_CN \
				 from blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK WHERE SHIPID='%s'",strShipid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char orderid[64]="";//出库单ID
	char orderdt[20]="";//出库单日期
	char departid[32]="";//部门ID
	int status=0;//出库类型 0：领用，1：保养，2：报废，3：丢失，4盘点
	int reviewstatus=0;//0: 未审核 1;已审核（部门长已确认）
	int stockstatus=0;//0：部分出库 1：全部出库
	char operatorid[64]="";
	string remark="";

	int idx=0;

	out <<"{seq:\""<<strSeq<<"\",who:\"\",info:[";	

	while (psql->NextRow())
	{

		READMYSQL_STR(MATERIALS_OUTSTOCK_ID, orderid)
			READMYSQL_STR(OP_DATE, orderdt)
			READMYSQL_STR(DEPARTMENT_ID, departid)
			READMYSQL_INT(OUTSTOCK_STATUS,status,0)
			READMYSQL_INT(REVIEW_STATUS,reviewstatus,0)
			READMYSQL_INT(STOCK_STATUS,stockstatus,0)
			READMYSQL_STR(OPERATOR,operatorid)
			READMYSQL_STRING(REMARK_CN,remark)

			if (idx++)
			{
				out<<",";
			}

			out << "{id:\"" << orderid << "\",date:\"" << orderdt <<"\",depart:\"" << departid <<"\",state:\"" << reviewstatus << "\",outtype:\"";
			out	<< status<<"\",remark:\"" << remark<<"\"}";

	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//获取某出库单下所有的物料列表 0x58
int eMOPMaterialsSvc::GetOutstockOrderMaterialList(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetOutstockOrderPartList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strOrderId=root.getv("id", "");
	string strShipId=root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";

	sprintf(sql,"select T1.MATERIALS_ID,T1.MATERIALS_INSTOCK_ID,T1.QUATITY,T1.REMARK_CN,T2.QUANTITY AS INSTOCK,T2.LOCATION_ID \
				from blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK_DETAILS T1,blm_emop_etl.T50_EMOP_MATERIALS_SHIP T2 \
				WHERE T1.MATERIALS_OUTSTOCK_ID='%s' AND T2.SHIPID='%s' AND T1.MATERIALS_ID=T2.MATERIALS_ID",strOrderId.c_str(),strShipId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char materialid[64]="";
	int quantity=0;//出库数量
	int instocknum=0;//库存量
	string remark="";
	char locationid[64]="";
	char instockorderid[64]="";

	string price="";

	out <<"{eid:\"0\",seq:\""<<strSeq<<"\",id:\""<<strOrderId<<"\",info:[";	

	int idx=0;

	while(psql->NextRow())
	{

		READMYSQL_STR(MATERIALS_ID, materialid)
			READMYSQL_INT(QUATITY,quantity,0)
			READMYSQL_INT(INSTOCK,instocknum,0)
			READMYSQL_STRING(REMARK_CN, remark)
			READMYSQL_STR(LOCATION_ID,locationid)
			READMYSQL_STR(MATERIALS_INSTOCK_ID,instockorderid)


			MaterialBaseInfo *pMaterialInfo=g_eMOPCommDataSvr::instance()->GetMaterialItemById(string(materialid));

		string unitStr="";
		string specStr="";
		string materialName="";
		string materialCatName="";

		if (pMaterialInfo)
		{
			unitStr=pMaterialInfo->m_unit_cn;
			specStr=pMaterialInfo->m_spec_cn;
			materialName=pMaterialInfo->m_name_cn.empty()?pMaterialInfo->m_name_en:pMaterialInfo->m_name_cn;
			materialCatName=pMaterialInfo->m_catname_cn.empty()?pMaterialInfo->m_catname_en:pMaterialInfo->m_catname_cn;
		}

		if (idx++)
		{
			out<<",";
		}

		out << "{mcate:\"" << materialCatName << "\",mid:\"" << materialid <<"\",mname:\"" << materialName<<"\",spec:\"" << specStr <<"\",outnum:\"" << quantity << "\",storenum:\"";
		out	<< instocknum <<"\",unit:\"" << unitStr<<"\",orderid:\""<<instockorderid<<"\",locateid:\"" << locationid<<"\",remark:\"" <<remark<<"\"}";


	}

	out <<"]}";


	RELEASE_MYSQL_RETURN(psql, 0);


}
//提交物料出库单 0x59
int eMOPMaterialsSvc::SetMaterialOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaterialsSvc::SetMaterialOutstockOrder]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string strShipid=root.getv("shipId", "");
	string strDepartid=root.getv("deparId", "");
	int type=root.getv("type", 0);//出库类型 0：领用，1：保养，2：报废，3：丢失，4盘点）

	int stockstatus=0;//0：部分出库 1：全部出库 

	string sequence="";
	GET_EMOP_SEQUENCE_STRING(MC,sequence)
		string outstockid="MCS"+strShipid+sequence;

	string date=CurLocalDateSeq();

	MySql* psql = CREATE_MYSQL;


	char sql[512]="";

	sprintf (sql, "INSERT INTO blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK(MATERIALS_OUTSTOCK_ID,OP_DATE,SHIPID,DEPARTMENT_ID,\
				  OUTSTOCK_STATUS,REVIEW_STATUS,STOCK_STATUS,OPERATOR,REMARK_CN) \
				  VALUES('%s','%s','%s','%s',%d,%d,%d,'%s','%s')",outstockid.c_str(),date.c_str(),strShipid.c_str(),
				  strDepartid.c_str(),type,0,stockstatus,pUid,"");
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	Json* materials = root.get("info");

	if (materials)
	{
		for (int i=0;i<materials->size();i++)
		{
			Json* materialItem=materials->get(i);

			if (!materialItem)
			{
				continue;
			}
			string materialid=materialItem->getv("mid","");
			int number=atoi(materialItem->getv("num",""));
			string instockorder=materialItem->getv("orderid","");

			char sql1[512]="";
			sprintf (sql1, "INSERT INTO blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK_DETAILS \
						   (MATERIALS_OUTSTOCK_ID,MATERIALS_ID,MATERIALS_INSTOCK_ID,QUATITY,REMARK_CN) \
						   VALUES('%s','%s','%s',%d,'%s')",outstockid.c_str(),materialid.c_str(),instockorder.c_str(),number,"");
			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);
		}
	}

	out <<"{seq:\""<<strSeq<<"\",id:\""<<outstockid<<"\",eid:0}";	

	RELEASE_MYSQL_RETURN(psql, 0);


}
//删除物料出库单中某备件0x5a
int eMOPMaterialsSvc::DeleteOutstockOrderMaterialItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::DeleteOutstockOrderMaterialItem]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string strSeq=root.getv("seq", "");
	string strOrderId=root.getv("id", "");
	string strMaterialId=root.getv("mid", "");

	char sql[512]="";

	sprintf (sql, "delete from blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK_DETAILS where MATERIALS_OUTSTOCK_ID='%s' and MATERIALS_ID='%s'",strOrderId.c_str(),strMaterialId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";


	RELEASE_MYSQL_RETURN(psql, 0);

}
//物料出库单部门长确认0x5b
int eMOPMaterialsSvc::ConformMaterialOutstockOrder(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::ConformMaterialOutstockOrder]bad format:", jsonString, 1);

	string strSeq=root.getv("seq", "");
	string Id=root.getv("id", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512]="";

	sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK SET REVIEW_STATUS='1' WHERE MATERIALS_OUTSTOCK_ID='%s'",Id.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out<< "{eid:0,seq:\""<<strSeq.c_str()<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}



//某船舶某部门物料所有的入库单列表0x5c
int eMOPMaterialsSvc::GetMaterialItemInStockList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetMaterialItemInStockList]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipid", "");
	string strDepartId=root.getv("departid", "");
	string strMaterialId=root.getv("mid", "");


	char sql[1024] = "";
	sprintf (sql, "SELECT T1.MATERIALS_INSTOCK_ID,T1.TYPE,T1.OP_DATE,T2.QUATITY,T2.INSTOCK_REST FROM blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK T1,blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK_DETAILS T2 \
				  WHERE T1.SHIPID='%s' AND T1.DEPARTMENT_ID='%s' AND T1.MATERIALS_INSTOCK_ID=T2.MATERIALS_INSTOCK_ID \
				  AND T2.MATERIALS_ID='%s'", strShipId.c_str(),strDepartId.c_str(),strMaterialId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char instockorderid[64]="";
	int type=1;
	char opdate[20]="";
	int instocknum=0;
	int remainnum=0;

	out <<"{eid:0,seq:\""<<strSeq<<"\",info:[";	

	int idx=0;
	while(psql->NextRow())
	{
		long lastdt=0;

		READMYSQL_STR(MATERIALS_INSTOCK_ID, instockorderid)
			READMYSQL_INT(TYPE,type,0)
			READMYSQL_STR(OP_DATE, opdate)
			READMYSQL_INT(QUATITY,instocknum,0)
			READMYSQL_INT(INSTOCK_REST,remainnum,0)

			if (idx++)
			{ 
				out<<",";
			}

			out << "{inseq:\"" << instockorderid << "\",intype:\"" << type<<"\",indate:\"" << opdate;
			out	<<"\",innum:\"" << instocknum<<"\",leftnum:\"" << remainnum<<"\"}";

	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//请求物料盘点信息 0x60
int eMOPMaterialsSvc::GetMaterialInventoryCounting(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPMaterialsSvc::GetMaterialInventoryCounting]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipId", "");
	string strDepartId=root.getv("departid", "");
	string strCatId=root.getv("cid", "");


	char sql[1024] = "";
	sprintf (sql, "SELECT MATERIALS_ID,LOCATION_ID,LAST_QUANTITY,UNIX_TIMESTAMP(LAST_QUANTITY_DT) AS LAST_DT,QUANTITY \
				  FROM blm_emop_etl.T50_EMOP_MATERIALS_SHIP WHERE SHIPID='%s' AND DEPARTMENT_CODE='%s' AND MATERIALS_CAT_ID='%s'",
				  strShipId.c_str(),strDepartId.c_str(),strCatId.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	string materialid="";
	char locationid[64]="";
	long laststockdt=0;
	int laststock=0;


	out <<"{seq:\""<<strSeq<<"\",info:[";	


	int idx=0;
	while(psql->NextRow())
	{
		long lastdt=0;

		READMYSQL_STRING(MATERIALS_ID, materialid)
			READMYSQL_INT(LAST_DT,lastdt,0)
			READMYSQL_STR(LOCATION_ID, locationid)
			READMYSQL_INT(LAST_QUANTITY,laststock,0)

			if (lastdt>laststockdt)
			{
				laststockdt=lastdt;
			}


			string name="";
			string catcode="";
			string spec="";
			string unit="";
			int stockflag=0;
			string remark="";
			MaterialBaseInfo *pMaterialInfo=g_eMOPCommDataSvr::instance()->GetMaterialItemById(materialid);

			if (pMaterialInfo)
			{
				catcode=pMaterialInfo->m_catCode;
				unit=pMaterialInfo->m_unit_cn;
				spec=pMaterialInfo->m_spec_cn;
				remark=pMaterialInfo->m_remark_cn;
				stockflag=pMaterialInfo->m_storeFlag;
				name=pMaterialInfo->m_name_cn.empty()?pMaterialInfo->m_name_en:pMaterialInfo->m_name_cn;
			}


			if (idx++)
			{ 
				out<<",";
			}


			out << "{mid:\"" << materialid << "\",mname:\"" << name<<"\",mcode:\"" << catcode<<"\",flag:\"" << stockflag;
			out	<<"\",lastC:\"" << laststock<<"\",spec:\"" << spec<<"\",unit:\"" << unit<<"\",departid:\"" << strDepartId<<"\",rm:\"" << remark<<"\"}";

	}

	out<<"],date:"<<laststockdt<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//设置物料盘点 0x61
int eMOPMaterialsSvc::SetMaterialInventoryCount(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[eMOPMaterialsSvc::SetMaterialInventoryCount]bad format:", jsonString, 1);

	string strSeq = root.getv("seq", "");
	string strShipId=root.getv("shipId", "");
	string strDepartId=root.getv("departid", "");
	int type=root.getv("type", -1);
	string mIds=root.getv("mid", "");
	string checks=root.getv("checkS", "");


	Tokens mIdTokens=StrSplit(mIds,"|");
	Tokens checkTokens=StrSplit(checks,"|");

	if (mIdTokens.size()!=checkTokens.size())
	{
		out <<"{seq:\""<<strSeq<<"\",type:\""<<type<<"\",check:\"0\"}";	
		return 0;
	}

	MySql* psql = CREATE_MYSQL;

	string date=CurLocalDate();


	for (int i=0;i<mIdTokens.size();i++)
	{
		char sql[512]="";

		sprintf (sql, "UPDATE blm_emop_etl.T50_EMOP_MATERIALS_SHIP SET LAST_QUANTITY='%s',LAST_QUANTITY_DT='%s' \
					  where SHIPID='%s' AND DEPARTMENT_CODE='%s' AND MATERIALS_ID='%s'",
					  checkTokens[i].c_str(),date.c_str(),strShipId.c_str(),strDepartId.c_str(),mIdTokens[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}


	//需要补充盘点出库入库流水

	out <<"{seq:\""<<strSeq<<"\",type:\""<<type<<"\",check:\"1\"}";	

	RELEASE_MYSQL_RETURN(psql, 0);

}
//Mid:0x60 SID: 0x0010
//{id:””,seq:”” }
//{eid:0,info:[{aid:"",ad:"",as:"",storenum:"",applynum:””,unit:””,price:, currency:””, applicant:””,auditnum:,approvalnum:””,lc:”” },{}]}
int eMOPMaterialsSvc::GetMlOrderHis_Ship(const char* pUid, const char* jsonString, std::stringstream& out)
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
	//aon:申请单号  T50_EMOP_MATERIALS_APPLIES
	//adt:申请日期   T50_EMOP_MATERIALS_APPLIES
	//ast:申请状态  //
	//sto: 库存量   T50_EMOP_MATERIALS_SHIP
	//aq:申请数量  T50_EMOP_MATERIALS_APPLY_DETAILS 
	//unit:单位     T50_EMOP_MATERIALS
	//price:单价  询价或订购才有
	//curr:币种 询价表 or 订购表
	//per:申请人   T50_EMOP_MATERIALS_APPLIES
	//aun: 审核数量
	//acn: 验收数量  //入库表
	//slc:库位代码   //T50_EMOP_MATERIALS_SHIP

	sprintf (sql, \
		"select t1.QUATITY as appquantity,t2.MATERIALS_APPLY_ID,UNIX_TIMESTAMP(t2.OP_DATE) as t2apptm,t2.USER_ID as appuser,t3.QUANTITY as INSTOCK,t3.LOCATION_ID \
		,t5.CURRENCY,t7.UNIT_PRICE,t7.QUATITY as odquantity,t8.QUATITY as inquantity,t9.NAME AS STOCKNM,t10.UNIT_CN from \
		blm_emop_etl.T50_EMOP_MATERIALS_APPLY_DETAILS t1 left join \
		blm_emop_etl.T50_EMOP_MATERIALS_APPLIES t2 on t1.MATERIALSS_APPLY_ID=t2.MATERIALS_APPLY_ID left join \
		blm_emop_etl.T50_EMOP_MATERIALS_SHIP t3 on t1.MATERIALS_ID=t3.MATERIALS_ID and t2.SHIP_ID=t3.SHIPID left join \
		blm_emop_etl.T50_EMOP_MATERIALS_APPLY_INQUERY t4 on	t1.MATERIALSS_APPLY_ID=t4.MATERIALS_APPLY_ID left join \
		blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDERS t5 on t4.MATERIALS_APPLY_INQUERY_ID=t5.MATERIALS_APPLY_INQUERIED_QUOTED_ID left join \
		blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK t6 on t5.MATERIALS_PURCHASE_ORDER_ID=t6.MATERIALS_PURCHASE_ORDER_ID left join \
		blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDER_DETAILS t7 on t5.MATERIALS_PURCHASE_ORDER_ID=t7.MATERIALS_PURCHASE_ORDER_ID and t1.MATERIALS_ID=t7.MATERIALS_ID left join \
		blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK_DETAILS t8 on t6.MATERIALS_INSTOCK_ID=t8.MATERIALS_INSTOCK_ID and t1.MATERIALS_ID=t8.MATERIALS_ID LEFT JOIN \
		blm_emop_etl.t50_emop_ship_inventory_location t9 ON t3.LOCATION_ID=t9.LOCATION_ID left join\
		blm_emop_etl.T50_EMOP_MATERIALS t10 ON t1.MATERIALS_ID=t10.MATERIALS_ID \
		where t1.MATERIALS_ID='%s' and t2.SHIP_ID='%s' and t2.OP_DATE between FROM_UNIXTIME('%d') and FROM_UNIXTIME('%d') ",\
		strID.c_str(),strShipId.c_str(),iStartTime,iEndTime);	
	FILE *pf;
	pf=fopen("/tmp/SQL_6010_1.log","wb");
	fwrite(sql,sizeof(char),strlen(sql),pf);
	fclose(pf);
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
		READMYSQL_STR(UNIT_CN, unit);
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
// Mid:0x60 SID: 0x0011
//{ id:””,seq:”” }
//{eid:0,info:[{id:"",inseq:"",intime:"",innum:"",staff:””,intype:””,curstorenum:””,inprice:””,incurrency:””,outseq:””,outtime:””,staff:””,outtype:”” },{}]}
int eMOPMaterialsSvc::GetMlStoreHis_Ship(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetMlStoreHis_Ship]bad format:", jsonString, 1);
	string strID=root.getv("id", "");//物料ID
	int iStartTime=root.getv("startTime", -1);
	int iEndTime=root.getv("endTime", -1);
	string strSeq = root.getv("seq", "");
	string strShipId = root.getv("shipId", "");
	string strDptid = root.getv("dptid", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",info:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.MATERIALS_INSTOCK_ID,t1.QUATITY,t1.INSTOCK_REST,UNIX_TIMESTAMP(t2.OP_DATE) AS OPDATE,t2.OPERATOR,t2.STOCK_STATUS,t3.UNIT_PRICE,t4.CURRENCY FROM\
				  blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK_DETAILS t1 left join\
				  blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK t2 on t1.MATERIALS_INSTOCK_ID=t2.MATERIALS_INSTOCK_ID left join \
				  blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDER_DETAILS t3 on t2.MATERIALS_PURCHASE_ORDER_ID=t3.MATERIALS_PURCHASE_ORDER_ID and t1.MATERIALS_ID=t3.MATERIALS_ID left join\
				  blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDERS t4 on t2.MATERIALS_PURCHASE_ORDER_ID=t4.MATERIALS_PURCHASE_ORDER_ID\
				  where t1.MATERIALS_ID='%s' and t2.SHIPID='%s' and t2.DEPARTMENT_ID='%s' and t2.OP_DATE between FROM_UNIXTIME('%d') and FROM_UNIXTIME('%d') ORDER BY OP_DATE",strID.c_str(),strShipId.c_str(),strDptid.c_str(),iStartTime,iEndTime);
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
		READMYSQL_DOUBLE(MATERIALS_CAT_ID,ffPrice ,0.0);
		if (cnt++)
			out << ",";
		//{ooi,id:"",inseq:"",intime:"",innum:"",staff:””,intype:””,curstorenum:””,inprice:””,incurrency:”” }
		out<<"{ooi:1"<<",on:\""<<on<<"\",dt:"<<iDt<<",ope:\""<<ope<<"\",type:"<<iType<<",quan:"<<iQuan<<",cbs:"<<iCbs<<",price:"<<ffPrice<<",curr:"<<iCurr<<"}";
	}
	//出库历史
	sprintf (sql, "SELECT t1.MATERIALS_OUTSTOCK_ID,t1.QUATITY,UNIX_TIMESTAMP(t2.OP_DATE) AS OPDATE,t2.OPERATOR,t2.STOCK_STATUS FROM\
				  blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK_DETAILS t1 left join\
				  blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK t2 on t1.MATERIALS_OUTSTOCK_ID=t2.MATERIALS_OUTSTOCK_ID \
				  where t1.MATERIALS_ID='%s'and t2.SHIPID='%s' and t2.DEPARTMENT_ID='%s' and t2.OP_DATE between FROM_UNIXTIME('%d') and FROM_UNIXTIME('%d') ORDER BY OP_DATE",strID.c_str(),strShipId.c_str(),strDptid.c_str(),iStartTime,iEndTime);

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
		//		READMYSQL_DOUBLE(MATERIALS_CAT_ID,ffPrice ,0.0);
		out << ",";
		//{ooi: ,on:" ",dt:" ",ope:" ",type:" ",quan: ,cbs:" ",price:,curr:" "}
		out<<"{ooi:0"<<",on:\""<<on<<"\",dt:"<<iDt<<",ope:\""<<ope<<"\",type:"<<iType<<",quan:"<<iQuan<<",cbs:"<<0<<",price:"<<0<<",curr:"<<0<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
// Mid:0x60 SID: 0x0012
//{id:””,seq:””, ,shipid:””,dptid:””}
//{seq:"",id:””,info:[{id:"",inseq:"",intype:””,intime:"",innum:"",leftnum:””, inprice:””,currency:”” ,seq:},{}]}
int eMOPMaterialsSvc::GetMlStockDetail_Ship(const char* pUid, const char* jsonString, std::stringstream& out)
{ 
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetMlStockDetail_Ship]bad format:", jsonString, 1);

	string strMLid=root.getv("id", "");//物料ID;
	string strSeq = root.getv("seq", "");
	string strShipId = root.getv("shipid", "");
	string strDptid = root.getv("dptid", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",id:\""<<strMLid.c_str()<<"\",info:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql,\
		"SELECT t1.MATERIALS_INSTOCK_ID,t1.QUATITY,t1.INSTOCK_REST,UNIX_TIMESTAMP(t2.OP_DATE) AS OPDATE,t3.UNIT_PRICE,t4.CURRENCY FROM\
		blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK_DETAILS t1 left join\
		blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK t2 on t1.MATERIALS_INSTOCK_ID=t2.MATERIALS_INSTOCK_ID left join \
		blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDER_DETAILS t3 on t2.MATERIALS_PURCHASE_ORDER_ID=t3.MATERIALS_PURCHASE_ORDER_ID and t1.MATERIALS_ID=t3.MATERIALS_ID left join\
		blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDERS t4 on t2.MATERIALS_PURCHASE_ORDER_ID=t4.MATERIALS_PURCHASE_ORDER_ID\
		where t1.MATERIALS_ID='%s' and t1.REST>0 and t2.SHIPID='%s' and t2.DEPARTMENT_ID='%s' ",strMLid.c_str(),strShipId.c_str(),strDptid.c_str());
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
		READMYSQL_STR(MATERIALS_INSTOCK_ID, on);
		//		READMYSQL_STR(OPERATOR, ope);
		READMYSQL_INT(QUATITY,iQuan ,0);
		READMYSQL_INT(INSTOCK_REST,iReq ,0);
		READMYSQL_INT(OPDATE, iDt,0);
		READMYSQL_INT(STOCK_STATUS,iType ,0);
		READMYSQL_INT(CURRENCY,iCurr ,0);
		READMYSQL_DOUBLE(UNIT_PRICE,ffPrice ,0.0);
		if (cnt++)
			out << ",";
		//{inseq:"",intype:””,intime:"",innum:"",leftnum:””, inprice:””,currency:”” }
		out<<"{inseq:\""<<on<<"\",intime:"<<iDt<<",intype:"<<iType<<",innum:"<<iQuan<<",leftnum:"<<iReq<<",inprice:"<<ffPrice<<",currency:"<<iCurr<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
// Mid:0x60 SID: 0x001d
//{ id:"",shipid:””,departid:””,seq:””, stime:23, etime:23  }
//{eid:0,seq:,info:[{id:"",mid:"",mc:””,nc:"",ne:"",spec:””, unit:””,outnum:,price:,currency:””,money,outdate,depart, operator:””, auditor：””,outseq:”” stime:,etime: },{}]}
int eMOPMaterialsSvc::GetMlOutstockStat_Ship(const char* pUid, const char* jsonString, std::stringstream& out)
{ 
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetMlOutstockStat_Ship]bad format:", jsonString, 1);
	string strMlid=root.getv("id", "");//物料ID
	int iStartTime=root.getv("stime", -1);
	int iEndTime=root.getv("etime", -1);
	string strSeq = root.getv("seq", "");
	string strShipId = root.getv("shipid", "");
	string strDptid = root.getv("departid", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",info:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char mid[100]="";//物料类别(名称)---T50_EMOP_MATERIALS t7
	char mc[100]="";//	物料编码（）t5
	char nc[100]="";//中文名称
	char ne[100]="";//英文名称
	char spec[100]="";//规格
	char unit[100]="";//单位
	int iOutnum=0;//出库数量 ---T50_EMOP_MATERIALS_OUTSTOCK_DETAILS
	double ffPrice=0;//单价---T50_EMOP_MATERIALS_PURCHASE_ORDER_DETAILS
	int iCurrency=0;//币种---T50_EMOP_MATERIALS_PURCHASE_ORDERS
	double ffMoney=0;//金额 
	int iOutdate=0;//出库日期---T50_EMOP_MATERIALS_OUTSTOCK
	char depart[100]="";//部门 t6
	char optor[100]="";//经办人 t2
	char auditor[100]="";//审核人 t2 
	char outseq[100]="";//出库单号 t1

	//出库统计
	sprintf (sql, "SELECT t0.MATERIALS_OUTSTOCK_ID,t0.QUATITY,UNIX_TIMESTAMP(t1.OP_DATE) AS OPDATE,t1.OPERATOR,t1.CHECKER,t3.UNIT_PRICE,t3.TOTAL_PRICE_DISCOUNTED,t4.CURRENCY, \
				  t7.NAME_CN AS MCATNM_CN,t7.NAME_EN AS mid,t5.MATERIALS_CODE,t5.NAME_CN AS MLNM_CN,t5.NAME_EN AS MLNM_EN,t5.SPEC_CN,t5.UNIT_CN,\
				  t6.NAME_CN as dptname FROM\
				  blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK_DETAILS t0 left join\
				  blm_emop_etl.T50_EMOP_MATERIALS_OUTSTOCK t1 on t0.MATERIALS_OUTSTOCK_ID=t1.MATERIALS_OUTSTOCK_ID left join \
				  blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK t2 on t0.MATERIALS_INSTOCK_ID=t2.MATERIALS_INSTOCK_ID left join \
				  blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDER_DETAILS t3 on t2.MATERIALS_PURCHASE_ORDER_ID=t3.MATERIALS_PURCHASE_ORDER_ID and t0.MATERIALS_ID=t3.MATERIALS_ID left join\
				  blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDERS t4 on t2.MATERIALS_PURCHASE_ORDER_ID=t4.MATERIALS_PURCHASE_ORDER_ID left join\
				  blm_emop_etl.T50_EMOP_MATERIALS t5 on t0.MATERIALS_ID=t5.MATERIALS_ID left join \
				  blm_emop_etl.t50_emop_department_code t6 on t1.DEPARTMENT_ID=t6.DEPARTMENT_CODE left join\
				  blm_emop_etl.T50_EMOP_MATERIALS_CATS t7 on t5.MATERIALS_CAT_ID=t7.MATERIALS_CAT_ID\
				  where t1.SHIPID='%s' and t1.DEPARTMENT_ID='%s' and t1.OP_DATE between FROM_UNIXTIME('%d') and FROM_UNIXTIME('%d') ORDER BY OPDATE",\
				  strShipId.c_str(),strDptid.c_str(),iStartTime,iEndTime);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int cnt=0;
	while (psql->NextRow())
	{
		READMYSQL_STR(mid, mid);
		READMYSQL_STR(CAT_CODE, mc);
		READMYSQL_STR(MLNM_CN, nc);
		READMYSQL_STR(MLNM_EN, ne);
		READMYSQL_STR(SPEC_CN, spec);
		READMYSQL_STR(UNIT_CN, unit);
		READMYSQL_INT(QUATITY,iOutnum ,0);
		READMYSQL_DOUBLE(UNIT_PRICE,ffPrice ,0);
		READMYSQL_INT(CURRENCY,iCurrency ,0);
		READMYSQL_INT(TOTAL_PRICE_DISCOUNTED,ffMoney ,0);
		READMYSQL_INT(OPDATE,iOutdate ,0);
		READMYSQL_STR(dptname, depart);
		READMYSQL_STR(OPERATOR, optor);
		READMYSQL_STR(CHECKER, auditor);
		READMYSQL_STR(MATERIALS_OUTSTOCK_ID, outseq);
		if(cnt++)
			out << ",";
		//{id:0,seq:,info:[{mid:"",mc:””,nc:"",ne:"",spec:””, unit:””,outnum:,price:,currency:””,money,outdate,depart, operator:””, auditor：””,outseq:”” stime:,etime: },{}]}
		out<<"{mid:\""<<mid<<"\",mc:\""<<mc<<"\",nc:\""<<nc<<"\",ne:\""<<ne<<"\",spec:\""<<spec<<"\",unit:\""<<unit<<"\",outnum:"<<iOutnum<<",price:"<<ffPrice\
			<<",currency:"<<iCurrency<<",money:"<<ffMoney<<",outdate:"<<iOutdate<<",depart:\""<<depart<<"\",operator:\""<<optor<<"\",auditor:\""<<auditor\
			<<"\",outseq:\""<<outseq<<"\"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
// Mid:0x60 SID: 0x001e
//{ shipid:””,departid:””,seq:””, stime:23, etime:23 }
//{eid:0,seq:,info:[{id:"",mid:"",mc:””,nc:"",ne:"",spec:””, unit:””,lb:, receive:,:””, consume：,balance:,stime:,etime: },{}]}
int eMOPMaterialsSvc::GetMlIOstockStat_Ship(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetMlIOstockStat_Ship]bad format:", jsonString, 1);
	int iStartTime=root.getv("stime", -1);
	int iEndTime=root.getv("etime", -1);
	string strSeq = root.getv("seq", "");
	string strShipId = root.getv("shipid", "");
	string strDptid = root.getv("departid", "");
	out<<"{seq:\""<<strSeq.c_str()<<"\",info:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	// 条件导入入库数据
	sprintf (sql, "SELECT t1.MATERIALS_ID,t1.LAST_QUANTITY,t1.QUANTITY,UNIX_TIMESTAMP(t1.LAST_QUANTITY_DT) AS INVENDATE,t2.MATERIALS_CODE,t2.NAME_CN AS MLNAME_CN,\
				  t2.NAME_EN AS MLNAME_EN,t2.SPEC_CN,t2.UNIT_CN,t3.NAME_CN AS CATNAME_CN,t3.NAME_EN AS CATNAME_EN FROM\
				  blm_emop_etl.T50_EMOP_MATERIALS_SHIP t1 left join \
				  blm_emop_etl.T50_EMOP_MATERIALS t2 on t1.MATERIALS_ID=t2.MATERIALS_ID left join \
				  blm_emop_etl.T50_EMOP_MATERIALS_CATS t3 on t1.MATERIALS_CAT_ID=t2.MATERIALS_CAT_ID \
				  where t1.SHIPID='%s' and t1.DEPARTMENT_CODE='%s' and t1.LAST_QUANTITY_DT between FROM_UNIXTIME('%d') and FROM_UNIXTIME('%d') ORDER BY INVENDATE",\
				  strShipId.c_str(),strDptid.c_str(),iStartTime,iEndTime);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	char id[100]="";
	char mid[100]="";//物料类别   ----T50_EMOP_MATERIALS_CATS类别名称
	char mc[100]="";//物料编码  ----T50_EMOP_MATERIALS
	char nc[100]="";//中文名称
	char ne[100]="";//英文名称
	char spec[100]="";//规格
	char unit[100]="";//单位
	int iLb=0;//上次结存-----T50_EMOP_MATERIALS_SHIP
	int iReceive=0;//接收------时间段内出入库和
	int iConsume=0;//消耗
	int iBalance=0;//结存----当前库存T50_EMOP_MATERIALS_SHIP

	int cnt=0;
	while (psql->NextRow())
	{
		READMYSQL_STR(MATERIALS_ID, id);
		READMYSQL_STR(CATNAME_CN, mid);
		READMYSQL_STR(MLNAME_CN, nc);
		READMYSQL_STR(MLNAME_EN, ne);
		READMYSQL_STR(SPEC_CN, spec);
		READMYSQL_STR(UNIT_CN, unit);
		READMYSQL_INT(LAST_QUANTITY,iLb ,0);
		READMYSQL_INT(QUANTITY,iBalance ,0);
		if(cnt++)
			out << ",";
		//{id:"",mid:"",mc:””,nc:"",ne:"",spec:””, unit:””,lb:, receive:,:””, consume：,balance:,stime:,etime: }
		out<<"{id:\""<<id<<"\",mid:\""<<mid<<"\",mc:\""<<mc<<"\",nc:\""<<nc<<"\",ne:\""<<ne<<"\",spec:\""<<spec<<"\",unit:\""<<unit<<"\",lb:"<<iLb<<",receive:"<<0\
			<<",consume:"<<0<<",balance:"<<iBalance<<"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//MID:0x0016  SID: 0x0001
//{ shipid:””,departid:””,seq:”” ,stime:,etime:,}
//{eid:0,seq:,info:[{id:"",mid:"",mc:””,nc:"",ne:"",spec:””, unit:””,innum:,price:,currency:””,money,indate,depart, operator:””, auditor：””,inseq:”” },{}]}
int eMOPMaterialsSvc::GetMlInstockStat_Ship(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[eMOPEquipSvc::GetMlInstockStat_Ship]bad format:", jsonString, 1);
	//	string strMlid=root.getv("id", "");//物料ID
	int iStartTime=root.getv("stime", -1);
	int iEndTime=root.getv("etime", -1);
	string strSeq = root.getv("seq", "");
	string strShipId = root.getv("shipid", "");
	string strDptid = root.getv("departid", "");

	out<<"{seq:\""<<strSeq.c_str()<<"\",info:[";
	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char mid[100]="";//物料类别(名称)---T50_EMOP_MATERIALS t7
	char mc[100]="";//	物料编码（）t5
	char nc[100]="";//中文名称
	char ne[100]="";//英文名称
	char spec[100]="";//规格
	char unit[100]="";//单位
	int iOutnum=0;//入库数量 ---T50_EMOP_MATERIALS_OUTSTOCK_DETAILS
	double ffPrice=0;//单价---T50_EMOP_MATERIALS_PURCHASE_ORDER_DETAILS
	int iCurrency=0;//币种---T50_EMOP_MATERIALS_PURCHASE_ORDERS
	double ffMoney=0;//金额 
	int iOutdate=0;//入库日期---T50_EMOP_MATERIALS_OUTSTOCK
	char depart[100]="";//部门 t6
	char optor[100]="";//经办人 t2
	char auditor[100]="";//审核人 t2 
	char inseq[100]="";//入库单号 t1

	//入库统计
	sprintf (sql, "SELECT t1.MATERIALS_INSTOCK_ID,t1.QUATITY,UNIX_TIMESTAMP(t2.OP_DATE) AS OPDATE,t2.OPERATOR,t2.CHECKER,t3.UNIT_PRICE,t3.TOTAL_PRICE_DISCOUNTED,t4.CURRENCY, \
				  t7.NAME_CN AS MCATNM_CN,t7.NAME_EN AS mid,t5.MATERIALS_CODE,t5.NAME_CN AS MLNM_CN,t5.NAME_EN AS MLNM_EN,t5.SPEC_CN,t5.UNIT_CN,\
				  t6.NAME_CN as dptname FROM\
				  blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK_DETAILS t1 left join\
				  blm_emop_etl.T50_EMOP_MATERIALS_INSTOCK t2 on t1.MATERIALS_INSTOCK_ID=t2.MATERIALS_INSTOCK_ID left join \
				  blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDER_DETAILS t3 on t2.MATERIALS_PURCHASE_ORDER_ID=t3.MATERIALS_PURCHASE_ORDER_ID and t1.MATERIALS_ID=t3.MATERIALS_ID left join\
				  blm_emop_etl.T50_EMOP_MATERIALS_PURCHASE_ORDERS t4 on t2.MATERIALS_PURCHASE_ORDER_ID=t4.MATERIALS_PURCHASE_ORDER_ID left join \
				  blm_emop_etl.T50_EMOP_MATERIALS t5 on t1.MATERIALS_ID=t5.MATERIALS_ID left join \
				  blm_emop_etl.t50_emop_department_code t6 on t2.DEPARTMENT_ID=t6.DEPARTMENT_CODE left join \
				  blm_emop_etl.T50_EMOP_MATERIALS_CATS t7 on t5.MATERIALS_CAT_ID=t7.MATERIALS_CAT_ID \
				  where t2.SHIPID='%s' and t2.DEPARTMENT_ID='%s' and t2.OP_DATE between FROM_UNIXTIME('%d') and FROM_UNIXTIME('%d') ORDER BY OPDATE",\
				  strShipId.c_str(),strDptid.c_str(),iStartTime,iEndTime);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int cnt=0;
	while (psql->NextRow())
	{
		READMYSQL_STR(mid, mid);
		READMYSQL_STR(CAT_CODE, mc);
		READMYSQL_STR(MLNM_CN, nc);
		READMYSQL_STR(MLNM_EN, ne);
		READMYSQL_STR(SPEC_CN, spec);
		READMYSQL_STR(UNIT_CN, unit);
		READMYSQL_INT(QUATITY,iOutnum ,0);
		READMYSQL_DOUBLE(UNIT_PRICE,ffPrice ,0);
		READMYSQL_INT(CURRENCY,iCurrency ,0);
		READMYSQL_INT(TOTAL_PRICE_DISCOUNTED,ffMoney ,0);
		READMYSQL_INT(OPDATE,iOutdate ,0);
		READMYSQL_STR(dptname, depart);
		READMYSQL_STR(OPERATOR, optor);
		READMYSQL_STR(CHECKER, auditor);
		READMYSQL_STR(MATERIALS_INSTOCK_ID, inseq);
		if(cnt++)
			out << ",";
		//{id:0,seq:,info:[{mid:"",mc:””,nc:"",ne:"",spec:””, unit:””,outnum:,price:,currency:””,money,outdate,depart, operator:””, auditor：””,outseq:”” stime:,etime: },{}]}
		out<<"{mid:\""<<mid<<"\",mc:\""<<mc<<"\",nc:\""<<nc<<"\",ne:\""<<ne<<"\",spec:\""<<spec<<"\",unit:\""<<unit<<"\",outnum:"<<iOutnum<<",price:"<<ffPrice\
			<<",currency:"<<iCurrency<<",money:"<<ffMoney<<",outdate:"<<iOutdate<<",depart:\""<<depart<<"\",operator:\""<<optor<<"\",auditor:\""<<auditor\
			<<"\",inseq:\""<<inseq<<"\"}";
	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


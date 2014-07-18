#include "ShipAis.h"
#include "ObjectPool.h"
#include "Util.h"
#include "Pinyin4j.h"
#include "LogMgr.h"
#include <functional>
#include <algorithm>

bool is_digitOrChar(char ch)
{
	if((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))
		return true;
	return false;
}

void delete_non_digitAndChar(char buff[], bool toLower=true, bool keepSpace=false)
{
	int len = strlen(buff);
	if(!len) return;

	std::stringstream ss;
	for(int i=0; i<len; i++)
		if(is_digitOrChar(buff[i]) || (keepSpace && buff[i] == ' '))
			ss << buff[i];

	std::string s = ss.str();
	if(keepSpace && !s.empty())
	{
		s.erase(0,s.find_first_not_of(" ")); 
		s.erase(s.find_last_not_of(" ")+1);
	}

	strcpy(buff, s.c_str());
	if(toLower)
		strlwr(buff);
}

void ShipData::clean()
{
	delete_non_digitAndChar(callsign, false);
	strcpy(trimcs, callsign);
	strlwr(trimcs);

	delete_non_digitAndChar(name, false, true);
	strcpy(trimname, name);
	delete_non_digitAndChar(trimname);
}

void AisRealtime::clean()
{
	delete_non_digitAndChar(callsign, false);
	strcpy(trimcs, callsign);
	strlwr(trimcs);

	delete_non_digitAndChar(name, false, true);
	strcpy(trimname, name);
	delete_non_digitAndChar(trimname);
}

void sort_AisList(AISLIST& aisList, const char* text)
{
	int len = strlen(text);

	AISLIST tmp1, tmp2;
	tmp1.reserve(aisList.size());
	tmp2.reserve(aisList.size());
	for(int i=0; i<(int)aisList.size(); i++)
	{
		if(strncmp(aisList[i].trimname, text, len) == 0)
			tmp1.push_back(aisList[i]);
		else
			tmp2.push_back(aisList[i]);
	}
	std::sort(tmp1.begin(), tmp1.end());
	std::sort(tmp2.begin(), tmp2.end());
	aisList.clear();
	aisList.insert(aisList.end(), tmp1.begin(), tmp1.end());
	aisList.insert(aisList.end(), tmp2.begin(), tmp2.end());
}


void sort_ShipList(SHIPLIST& shipList, const char* text)
{
	int len = strlen(text);

	SHIPLIST tmp1, tmp2;
	tmp1.reserve(shipList.size());
	tmp2.reserve(shipList.size());
	for(int i=0; i<(int)shipList.size(); i++)
	{
		if(strncmp(shipList[i].trimname, text, len) == 0)
			tmp1.push_back(shipList[i]);
		else
			tmp2.push_back(shipList[i]);
	}
	std::sort(tmp1.begin(), tmp1.end());
	std::sort(tmp2.begin(), tmp2.end());
	shipList.clear();
	shipList.insert(shipList.end(), tmp1.begin(), tmp1.end());
	shipList.insert(shipList.end(), tmp2.begin(), tmp2.end());
}

void CAisShip::readShipData(MySql *psql, ShipData &sd)
{
	READMYSQL_STR(shipid, sd.shipid)
	READMYSQL_STR(imo, sd.imo)
	READMYSQL_STR(name, sd.name)

	/*strcpy(sd.trimname,sd.name);
	delete_non_digitAndChar(sd.trimname);*/

	READMYSQL_STR(callsign, sd.callsign)
	/*strcpy(sd.trimcs,sd.callsign);
	delete_non_digitAndChar(sd.trimcs);*/

	READMYSQL_FLOAT(speed, sd.speed, 0.00);
	READMYSQL_INT(built, sd.built, 0);
	READMYSQL_STR(COUNTRY_CODE, sd.country);
	READMYSQL_STR(shiptype_key, sd.shipTypeKey);
	READMYSQL_64(mmsi, sd.mmsi, 0);
	READMYSQL_FLOAT(LOA, sd.length, 0);
	READMYSQL_FLOAT(beam, sd.beam, 0);
	READMYSQL_FLOAT(depth, sd.depth, 0);
	READMYSQL_FLOAT(draft, sd.draft, 0);
	READMYSQL_INT(dwt, sd.dwt, 0);
}


void CAisShip::loadCnssShip()
{

	MySql *psql = CREATE_MYSQL;
	char sql[256]="select * from boloomodb.t41_ship_cnss_doc";
	psql->Query(sql);
	
	char shipid[64];
	int rowcount = psql->GetRowCount();
	if(rowcount > 0)
	{
		while(psql->NextRow())
		{
            READMYSQL_STR(SHIPID,shipid)
			m_CnssShipSet.insert(string(shipid));

		}
	}

}

//装入船舶表中更新的数据
bool CAisShip::refreshShip()
{
	SYSTEM_LOG("[CAisShip::RefreshShip] begin ============================== ");
	if(bShipLoading)
	{
		SYSTEM_LOG("[CAisShip::RefreshShip] end, is loading ============================== ");
		return false;
	}
	bShipLoading = true;

	MySql *psql = CREATE_MYSQL;
	char sql[1024];
	sprintf(sql,"select t1.shipid,t1.imo,t1.name,t1.trimname,t1.mmsi,t1.callsign,t1.built,t1.speed,t1.country_code,t1.shiptype_key,t2.loa,t2.BEAMMOULDED AS beam,t2.depth,t2.draft,t3.dwt from t41_ship t1 left join t41_ship_dimension t2 ON t1.shipid = t2.shipid LEFT JOIN t41_ship_tonnage t3 ON t1.shipid = t3.shipid where t1.Last_upd_dt_long>=%ld", m_LastShipTime);
	psql->Query(sql);

	
	int rowcount = psql->GetRowCount();
	if(rowcount > 0)
	{
		ACE_Write_Guard<ACE_RW_Thread_Mutex> lock(m_shipLock);

		while(psql->NextRow())
		{
			ShipData sd;
			readShipData(psql, sd);
			sd.clean();
			

			/*SHIPMAP::iterator iter = m_ShipMap.find(sd.mmsi);
			if(iter == m_ShipMap.end())
				m_ShipMap.insert(SHIPMAP::value_type(sd.mmsi, sd));
			else
				iter->second = sd;*/
            m_ShipMap[sd.mmsi]=sd;
			m_ShipIdMap[string(sd.shipid)]=sd;

		}
	}
    m_LastShipTime = time(NULL);
	
	bShipLoading = false;

	SYSTEM_LOG("[CAisShip::RefreshShip] end, refresh:%d ============================== ", m_ShipIdMap.size());
	//DEBUG_LOG("[CAisShip::RefreshShip] end, refresh:%d ============================== ", rowcount);
	RELEASE_MYSQL_RETURN(psql, true);
}

void CAisShip::readAisData(MySql *psql, AisRealtime &ar)
{
	READMYSQL_64(mmsi, ar.mmsi, 0);
	READMYSQL_STR(name, ar.name);
	READMYSQL_STR(callsign, ar.callsign);
	READMYSQL_INT(srcid, ar.srcid, 0);
	READMYSQL_DOUBLE(longitude, ar.lon, 0.0);
	READMYSQL_DOUBLE(latitude, ar.lat, 90.0);
	READMYSQL_INT(nav_status, ar.nav_status, 0);
	READMYSQL_INT(sog, ar.sog, 0);
	READMYSQL_INT(true_head, ar.true_head, 0);
	READMYSQL_INT(cog, ar.cog, 0);
	READMYSQL_STR(dest, ar.dest);
	READMYSQL_INT(to_portid, ar.toPort, 0);
	READMYSQL_INT(eta, ar.eta, 0);
	READMYSQL_INT(draught, ar.draught, 0);
	READMYSQL_INT(width, ar.width, 0);
	READMYSQL_INT(length, ar.length, 0);
	READMYSQL_INT(ship_type, ar.ship_type, 90); //两位数有效

	if (ar.ship_type>=100||ar.ship_type<20)
	{
        ar.ship_type=90;
	}
	ar.ship_type=(int)(ar.ship_type/10);
	READMYSQL_64(imo, ar.imo, 0);
	READMYSQL_FLOAT(avgspeed, ar.avgspeed, 0.0);
	READMYSQL_INT(time, ar.time, 0);


}

bool CAisShip::refreshAis()
{
	SYSTEM_LOG("[CAisShip::RefreshAis] begin ========================== ");
	if(bAisLoading)
	{
		SYSTEM_LOG("[CAisShip::RefreshAis] end, is loading ========================== ");
		return false;
	}
	bAisLoading = true;

	// 只加载2013年以后的数据
	char sql[1024];
    MySql *psql = CREATE_MYSQL;
    sprintf(sql, "select t1.mmsi,t1.name, t1.callsign, t1.srcid,t1.longitude,t1.latitude,t1.nav_status,t1.sog,t1.true_head,t1.cog,t1.dest,t1.draught,t1.width,t1.length,t2.to_portid,t1.eta,t1.ship_type,t1.imo,t1.avgspeed,t1.time from aisdb.t41_ais_ship_realtime t1 left join aisdb.t41_ais_dest_map t2 on t1.dest_id = t2.dest_id where UNIX_TIMESTAMP(t1.last_upd_dt)>=%ld and t1.time >=1356969600 and t1.mmsi >= 100000000", m_LastAisTime);
	psql->Query(sql);

	ACE_Write_Guard<ACE_RW_Thread_Mutex> lock(m_aisLock);

	while(psql->NextRow())
	{
		AisRealtime ar;
		readAisData(psql, ar);
		delete_non_digitAndChar(ar.name,false,true);
        
		if (strlen(ar.name)==0||strlen(ar.callsign)==0||ar.imo==0)
		{
			if (m_ShipMap.find(ar.mmsi)!=m_ShipMap.end())
			{
				ShipData shipData=m_ShipMap[ar.mmsi];
               
				if ((strlen(ar.name)==0||IsNumeric(ar.name))&&strlen(shipData.name)>0)
				{
					strcpy(ar.name,shipData.name);
				}

                if (strlen(ar.callsign)==0&&strlen(shipData.callsign)>0)
                {
					strcpy(ar.callsign,shipData.callsign);
                }

				if (ar.imo==0&&atoi(shipData.imo)>0)
				{
					ar.imo=atoi(shipData.imo);
				}
			}
		}

        ar.clean();

        m_AisMap[ar.mmsi]=ar;
	}

	m_LastAisTime = time(NULL)-8*3600;

	bAisLoading = false;

	SYSTEM_LOG("[CAisShip::RefreshAis] end , refresh count:%d ========================== ", m_AisMap.size());
	RELEASE_MYSQL_RETURN(psql, true);
}

AisRealtime* CAisShip::FindAis(long mmsi)
{
	ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_aisLock);
    AISMAP::iterator it = m_AisMap.find(mmsi);
    if(it != m_AisMap.end())
        return &it->second;
    return NULL;
}

int CAisShip::GetAisState( long mmsi, time_t& lastTm )
{
	AisRealtime *pais = FindAis(mmsi);
	if(!pais)
		return 0;

	lastTm = pais->time;
	return pais->getState();
}

int CAisShip::Search_MmsiImo(AISLIST& aisList, const char* text)
{
	ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_aisLock);
	//aisList.reserve(2000);
	aisList.clear();

	AISMAP::iterator itBegin = m_AisMap.begin();
	AISMAP::iterator itEnd = m_AisMap.end();
	while(itBegin != itEnd)
	{
		if(itBegin->second.match_mmsiOrImo(text))
			aisList.push_back(itBegin->second);
		itBegin++;

		if (aisList.size()>=2000)
		{
			break;
		}
	}


    std::sort(aisList.begin(), aisList.end());
	//sort_AisList(aisList, text);

	return aisList.size();
}

int CAisShip::Search_MmsiImo(SHIPLIST& shipList, const char* text)
{
	ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_shipLock);
	shipList.reserve(2000);

	SHIPMAP::iterator itBegin = m_ShipMap.begin();
	SHIPMAP::iterator itEnd = m_ShipMap.end();
	while(itBegin != itEnd)
	{
		if(itBegin->second.match_mmsiOrImo(text))
			shipList.push_back(itBegin->second);
		itBegin++;
	}
	std::sort(shipList.begin(), shipList.end());

	return shipList.size();
}

int CAisShip::Search_NameCallsign(AISLIST& aisList, const char* text)
{
	char text2[1024];
	strcpy(text2, text);
	delete_non_digitAndChar(text2);

	ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_aisLock);
	aisList.reserve(2000);

	AISMAP::iterator itBegin = m_AisMap.begin();
	AISMAP::iterator itEnd = m_AisMap.end();
	while(itBegin != itEnd)
	{
		if(itBegin->second.match_nameOrCallsign(text2))
			aisList.push_back(itBegin->second);
		itBegin++;

		if (aisList.size()>=2000)
		{
			break;
		}
	}
	sort_AisList(aisList, text2);

	return aisList.size();
}

int CAisShip::Search_NameCallsign(SHIPLIST& shipList, const char* text)
{
	char text2[1024];
	strcpy(text2, text);
	delete_non_digitAndChar(text2);

	ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_shipLock);
	shipList.reserve(2000);

	SHIPMAP::iterator itBegin = m_ShipMap.begin();
	SHIPMAP::iterator itEnd = m_ShipMap.end();
	while(itBegin != itEnd)
	{
		if(itBegin->second.match_nameOrCallsign(text2))
			shipList.push_back(itBegin->second);
		itBegin++;
	}
	std::sort(shipList.begin(), shipList.end());

	return shipList.size();
}

int CAisShip::AdvSearchWithCondition(AISLIST &aisList,const char* text,ShipCondition *filter)
{  
	ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_aisLock);
	aisList.reserve(2000);

	AISMAP::iterator itBegin = m_AisMap.begin();
	AISMAP::iterator itEnd = m_AisMap.end();

	if (IsNumeric(string(text)))//判断是数字
	{
		if (strlen(text)==9)//MMSI
		{
			long mmsi=atoi(text);
			if (m_AisMap.find(mmsi)!=m_AisMap.end())
			{
				AisRealtime shipAis=m_AisMap[mmsi];

				if (filterShip(&shipAis,filter))
				{
					aisList.push_back(shipAis);  
				}

			}
		}
		else
		{
			while(itBegin != itEnd)
			{
				AisRealtime shipAis=itBegin->second;
				if(shipAis.match_mmsiOrImo(text)&&filterShip(&shipAis,filter))
					aisList.push_back(shipAis);
				itBegin++;
			}

		}

		if (!aisList.empty())
		{
			std::sort(aisList.begin(), aisList.end());
		}
	}
	else
	{
		while(itBegin != itEnd)
		{
			AisRealtime shipAis=itBegin->second;
			if(shipAis.match_nameOrCallsign(text)&&filterShip(&shipAis,filter))
				aisList.push_back(shipAis);
			itBegin++;
		}
        sort_AisList(aisList,text);
	}

	
	return aisList.size();


}


int CAisShip::AdvSearchWithCondition(SHIPLIST &shipList,const char* text,ShipCondition *filter,bool bonlycnss)
{
    ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_shipLock);
	//shipList.clear();
	shipList.reserve(2000);

	SHIPIDMAP::iterator itBegin = m_ShipIdMap.begin();
	SHIPIDMAP::iterator itEnd = m_ShipIdMap.end();
	if (IsNumeric(string(text)))//判断是数字
	{
		if (strlen(text)==9)//MMSI
		{
			long mmsi=atoi(text);

			if (m_ShipMap.find(mmsi)!=m_ShipMap.end())
			{
                  ShipData shipData=m_ShipMap[mmsi];

				  if (filterShip(&shipData,filter,bonlycnss))
				  {
                       shipList.push_back(shipData);  
				  }

			}
		}
		else
		{
			while(itBegin != itEnd)
			{
				ShipData shipData=itBegin->second;
				if(shipData.match_mmsiOrImo(text)&&filterShip(&shipData,filter,bonlycnss))
					shipList.push_back(shipData);
				itBegin++;
			}
          

		}

		if (!shipList.empty())
		{
			std::sort(shipList.begin(), shipList.end());
		}
		
	}
	else
	{
		while(itBegin != itEnd)
		{
			ShipData shipData=itBegin->second;
			if(shipData.match_nameOrCallsign(text)&&filterShip(&shipData,filter,bonlycnss))
				shipList.push_back(shipData);
			itBegin++;
		}

		sort_ShipList(shipList,text);

	}

	
	return shipList.size();

}

bool CAisShip::filterShip(ShipData *pShipdata,ShipCondition *filter,bool bonlycnss)
{

	if (!pShipdata)
	{
		return false;
	}


	if (bonlycnss&&m_CnssShipSet.find(string(pShipdata->shipid))==m_CnssShipSet.end())
	{
		return false;
	}
	

	if (filter==NULL||!filter->IsValid()) //不需要过滤
	{
		return true;
	}

	if (strlen(filter->country)==3)
	{
		if (strcmp(filter->country,pShipdata->country)!=0)
		{
			return false;
		}
	}

	if (filter->speed_l>0&&filter->speed_u>0)
	{
		if (!(pShipdata->speed>=filter->speed_l&&pShipdata->speed<=filter->speed_u))
		{
			return false;
		}
	}

	if (filter->built_l>0&&filter->built_u>0)
	{
		if (!(pShipdata->built>=filter->built_l&&pShipdata->built<=filter->built_u))
		{
			return false;
		}
	}



	if ((filter->type>=2&&filter->type<=9)||(filter->state>=1&&filter->state<=3))
	{

		AisRealtime* pShipAis=NULL;
		
		if (pShipdata->mmsi>0)
		{
			pShipAis=FindAis(pShipdata->mmsi);
		
		}

		if (filter->state>=1&&filter->state<=3)
		{
			if (filter->state==3)//无信号
			{
				if (pShipAis)
				{
					return false;
				}
			}
			else
			{
				if (!pShipAis||pShipAis->getState()!=filter->state)
				{
					return false;
				}
			}

		}

		if ((filter->type>=2&&filter->type<=9))
		{
            if (filter->type==9)
            {
				if (pShipAis&&pShipAis->ship_type!=filter->type)
				{
					return false;
				}
            }
			else
			{
			    if (!pShipAis||(pShipAis&&pShipAis->ship_type!=filter->type))
			    {
					return false;
			    }
			}	
			return false;
		}

	}

	return true;
}

bool CAisShip::filterShip(AisRealtime *pShipAis,ShipCondition *pfilter)
{

	if (!pShipAis)
	{
		return false;
	}

	if (pfilter==NULL||!pfilter->filter) //不需要过滤
	{
		return true;
	}


	if ((pfilter->type>=2&&pfilter->type<=9))
	{
		if (pShipAis->ship_type!=pfilter->type)
		{
			return false;
		}
	}

	if (pfilter->state>=1&&pfilter->state<=3)
	{
		if (pfilter->state!=pShipAis->getState())
		{
			return false;
		}
	}

	ShipData *pShipData=FindShip(pShipAis->mmsi);

	if (strlen(pfilter->country)==3)
	{
		if (!pShipData||(pShipData&&strcmp(pfilter->country,pShipData->country)!=0))
		{
			return false;
		}
	}

	if (pfilter->built_l>0&&pfilter->built_u>0)
	{
		if (!pShipData||(pShipData&&pfilter->built_l!=pShipData->built))
		{
			return false;
		}
	}

	if (pfilter->speedopt>=0&&pfilter->speed_l>0)
	{
		if (!pShipData)
		{
			return false;
		}
 
		if (pfilter->speedopt==0&&pShipData->speed<pfilter->speed_l) //>=
		{
			return false;
		}
		else if (pfilter->speedopt==1&&pShipData->speed!=pfilter->speed_l)  // ==
		{
			return false;
		}
		else if (pfilter->speedopt==2&&pShipData->speed>pfilter->speed_l)  // <=
		{
			return false;
		}

	}

	if (pfilter->dwtopt>=0&&pfilter->dwt>0)
	{

		if (!pShipData)
		{
			return false;
		}

		if (pfilter->dwtopt==0&&pShipData->dwt<pfilter->dwt) //>=
		{
			return false;
		}
		else if (pfilter->dwtopt==1&&pShipData->dwt!=pfilter->dwt)  // ==
		{
			return false;
		}
		else if (pfilter->dwtopt==2&&pShipData->dwt>pfilter->dwt)  // <=
		{
			return false;
		}

	}

	return true;
    
}


void CAisShip::GetRealAis(AISLIST& aisList, int seconds)
{
	ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_aisLock);
	aisList.reserve(m_AisMap.size());

	time_t tt = time(NULL);
	AISMAP::iterator itBegin = m_AisMap.begin();
	AISMAP::iterator itEnd = m_AisMap.end();
	while(itBegin != itEnd)
	{
		if(itBegin->second.time >= tt - seconds)
			aisList.push_back(itBegin->second);
		itBegin++;
	}
	std::sort(aisList.begin(), aisList.end());
}

ShipData* CAisShip::FindShip(long mmsi)
{
	ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_shipLock);
	SHIPMAP::iterator it = m_ShipMap.find(mmsi);
	if(it != m_ShipMap.end())
		return &it->second;
	return NULL;
}

ShipData* CAisShip::FindShip_imo(long imo)
{
	ACE_Read_Guard<ACE_RW_Thread_Mutex> al(m_shipLock);
	SHIPMAP::iterator it = m_ShipMap.begin();
	while(it != m_ShipMap.end())
	{
		if(atoi(it->second.imo) == imo)
			return &(it->second);
		it++;
	}

	return NULL;
}

CAisShip::CAisShip(void) : m_LastAisTime(0), m_LastShipTime(0), bAisLoading(false), bShipLoading(false)
{
	loadCnssShip();
}

CAisShip::~CAisShip(void)
{
}

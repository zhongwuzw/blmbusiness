#include "blmcom_head.h"
#include "ScheduleSvc.h"
#include "VesselSvc.h"
#include "PortSvc.h"
#include "MessageService.h"
#include "ObjectPool.h"
#include "kSQL.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
using namespace std;

IMPLEMENT_SERVICE_MAP(ScheduleSvc)

ScheduleSvc::ScheduleSvc() : m_bTrackConnected(false), m_bVoyConnected(false)
{
	m_lastLoadTime = (long)time(NULL) - 30*24*60*60; //一个月前的数据
}

ScheduleSvc::~ScheduleSvc()
{

}

bool ScheduleSvc::loadCarrier()
{
	SYSTEM_LOG("[ScheduleSvc::loadCarrier] begin ==============================");

    char sql[1024];
    sprintf(sql, "SELECT t1.CARRIER_SRV_ID, t1.SRV_NAME_CN, t1.SRV_NAME_EN, t1.LINE_NAME_CN, t1.LINE_NAME_EN, \
		t2.CARRIER_ID, concat(t2.NAME_ABBR_EN,'-',t2.NAME_ABBR_CN) as CARRIER_NAME \
		FROM T41_voy_schedule_serv t1 \
		LEFT JOIN T41_Carrier t2 ON t1.CARRIER_ID = t2.CARRIER_ID");

    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), false);

    while(psql->NextRow())
    {
        TCarrier carrier;
        READMYSQL_STR(carrier_srv_id, carrier.m_szCarrierSrvId);
        READMYSQL_STR(srv_name_cn, carrier.m_szCarrierSrvNameCn);
        READMYSQL_STR(srv_name_en, carrier.m_szCarrierSrvNameEn);
        READMYSQL_STR(line_name_cn, carrier.m_szLineNameCn);
        READMYSQL_STR(line_name_en, carrier.m_szLineNameEn);
        READMYSQL_STR(carrier_id, carrier.m_szCarrierId);
        READMYSQL_STR(carrier_name, carrier.m_szCarrierName);

        m_mapCarrier.insert(make_pair(carrier.m_szCarrierSrvId, carrier));
    }

	SYSTEM_LOG("[ScheduleSvc::loadCarrier] end, total:%d ==============================", (int)m_mapCarrier.size());
    RELEASE_MYSQL_RETURN(psql, true);
}

std::string SetToSqlRange(StrSet& aSet)
{
    std::stringstream out;
    StrSetIter iter;
    bool bFirst = true;
    out << "(";

    for(iter = aSet.begin(); iter != aSet.end(); iter++)
    {
        if(bFirst)
        {
            bFirst = false;
            out << "'" << *iter << "'";
        }
        else
        {
            out << ",'" << *iter << "'";
        }
    }
    if(bFirst)
        out << "''";

    out << ")";

    return out.str();
}

bool ScheduleSvc::loadSchedule()
{
	SYSTEM_LOG("[ScheduleSvc::loadSchedule] begin ==============================");

    char sql[200*1024];

    //ACE_Write_Guard<ACE_RW_Thread_Mutex> WGuard2(m_ThreadLock);

	SYSTEM_LOG("[ScheduleSvc::loadSchedule] debug 1 ==============================");

    //1、加载船期航次表
    sprintf(sql, "SELECT t1.VOYAGE_ID, t1.SRC_VOYAGENO, t1.CARRIER_ID, t1.CARRIER_SRV_ID, t1.SHIPID, t2.MMSI, t1.SRC_SHIPNAME, t1.PORTCOUNT, t1.DISTANCE, t1.DEPT_WEEKDAYS, t1.DEST_WEEKDAYS, t1.PLAN_DURATION, UNIX_TIMESTAMP(t1.DEPT_PTA) AS DeptPTA, UNIX_TIMESTAMP(t1.DEPT_PTD) AS DeptPTD, UNIX_TIMESTAMP(t1.DEST_PTA) AS DestPTA, t1.VOY_STATUS, t1.REMARK, UNIX_TIMESTAMP(t1.update_dt) AS UpdateTime \
				FROM t30_voyage_sch_info t1 LEFT JOIN t41_ship t2 ON t1.SHIPID = t2.SHIPID WHERE UNIX_TIMESTAMP(update_dt) > %ld", m_lastLoadTime);

    StrSet addSet;

    MySql* psql = CREATE_MYSQL;
    {
		SYSTEM_LOG("[ScheduleSvc::loadSchedule] debug 2 ==============================");

        CHECK_MYSQL_STATUS(psql->Query(sql), false);

		SYSTEM_LOG("[ScheduleSvc::loadSchedule] debug 3 ==============================");

        while(psql->NextRow())
        {
            long updateTime;
            TSchedule* pSche = new TSchedule;
            READMYSQL_STR(voyage_id, pSche->m_szVoyId);
            READMYSQL_STR(src_voyageno, pSche->m_szVoyName);
            READMYSQL_STR(carrier_id, pSche->m_szCarrierId);
            READMYSQL_STR(carrier_srv_id, pSche->m_szCarrierSrvId);
            READMYSQL_STR(shipid, pSche->m_szShipId);
            READMYSQL_INT(mmsi, pSche->m_nMmsi, 0);
            READMYSQL_STR(src_shipname, pSche->m_szShipName);
            READMYSQL_INT(portcount, pSche->m_nPortCount, 0);
            READMYSQL_DOUBLE(distance, pSche->m_dbDist, 0.0);
            READMYSQL_INT(dept_weekdays, pSche->m_nDeptWkd, 0);
            READMYSQL_INT(dest_weekdays, pSche->m_nDestWkd, 0);
            READMYSQL_DOUBLE(plan_duration, pSche->m_dbDuration, 0.0);
            READMYSQL_64(DeptPTA, pSche->m_lDeptPta, 0L);
            READMYSQL_64(DeptPTD, pSche->m_lDeptPtd, 0L);
            READMYSQL_64(DestPTA, pSche->m_lDestPta, 0L);
            READMYSQL_INT(voy_status, pSche->m_nStatus, 0);
            READMYSQL_STR(remark, pSche->m_szRmk);
            READMYSQL_64(UpdateTime, updateTime, 0L);
            if(updateTime > m_lastLoadTime)
                m_lastLoadTime = updateTime;

            if(pSche->m_lDeptPta == 0)
                pSche->m_lDeptPta = pSche->m_lDeptPtd;
            if(pSche->m_lDeptPtd == 0)
                pSche->m_lDeptPtd = pSche->m_lDeptPta;

            MapScheduleIter iter = m_mapSchedule.find(pSche->m_szVoyId);
            if(iter != m_mapSchedule.end())		// 更新
            {
				TSchedule* pOldSche = iter->second;

                //voyage_id->shedule删除
                m_mapSchedule.erase(iter);

                //carrier_srv_id->schedule删除
                MapCarrierScheduleRange range1 = m_mapCarrierSchedule.equal_range(pOldSche->m_szCarrierSrvId);
                for(MapCarrierScheduleIter iter1=range1.first; iter1!=range1.second; iter1++)
                {
                    if(iter1->second == pOldSche)
                    {
                        m_mapCarrierSchedule.erase(iter1);
                        break;
                    }
                }

                //ship_id->shedule删除
                MapShipScheduleRange range2 = m_mapShipSchedule.equal_range(pOldSche->m_szShipId);
                for(MapShipScheduleIter iter2=range2.first; iter2!=range2.second; iter2++)
                {
                    if(iter2->second == pOldSche)
                    {
                        m_mapShipSchedule.erase(iter2);
                        break;
                    }
                }

                //port_id->schedule删除
                VecScheduleSection& section = pOldSche->m_vecSection;
                for(int i=0; i<(int)section.size(); i++)
                {
                    int portId = section[i].m_nPortId;
                    MapPortScheduleRange range3 = m_mapPortSchedule.equal_range(portId);
                    for(MapPortScheduleIter iter3=range3.first; iter3!=range3.second; iter3++)
                    {
                        if(iter3->second == pOldSche)
                        {
                            m_mapPortSchedule.erase(iter3);
                            break;
                        }
                    }
                }

                delete pOldSche;
            }

			m_mapSchedule.insert(make_pair(pSche->m_szVoyId, pSche));
			m_mapCarrierSchedule.insert(make_pair(pSche->m_szCarrierSrvId, pSche));
			if(pSche->m_szShipId[0] != '\0')
				m_mapShipSchedule.insert(make_pair(pSche->m_szShipId, pSche));

			addSet.insert(pSche->m_szVoyId);
        }

		SYSTEM_LOG("[ScheduleSvc::loadSchedule] debug 4 ==============================");
    }

    char* pSQL = sql;
    if(addSet.size() > 10000)
    {
        pSQL = new char[addSet.size() * 20 + 1000];
        pSQL[0] = 0;
    }

    //2、加载航次分段表
    sprintf(pSQL, "SELECT VOYAGE_ID, SEQ, blm_port_name, PORT_ID, ISO3, UNIX_TIMESTAMP(PTA) AS PTA, UNIX_TIMESTAMP(ATA) AS ATA, UNIX_TIMESTAMP(PTD) AS PTD, UNIX_TIMESTAMP(ATD) AS ATD, DISTANCE, PLAN_DURATION \
				 FROM t30_voyage_sch_section WHERE VOYAGE_ID IN %s ORDER BY VOYAGE_ID, SEQ", SetToSqlRange(addSet).c_str());

    {
		SYSTEM_LOG("[ScheduleSvc::loadSchedule] debug 5 ==============================");

        if(!psql->Query(pSQL))
        {
            if(pSQL != sql)
                delete []pSQL;
            RELEASE_MYSQL_RETURN(psql, false);
        }

		SYSTEM_LOG("[ScheduleSvc::loadSchedule] debug 6 ==============================");

        while(psql->NextRow())
        {
            TScheduleSection section;
            READMYSQL_STR(voyage_id, section.m_szVoyId);
            READMYSQL_INT(seq, section.m_nSeq, 0);
            READMYSQL_STR(blm_port_name, section.m_szPortName);
            READMYSQL_INT(port_id, section.m_nPortId, 0);
            READMYSQL_STR(iso3, section.m_szPortISO3);
            READMYSQL_64(pta, section.m_lPta, 0L);
            READMYSQL_64(ata, section.m_lAta, 0L);
            READMYSQL_64(ptd, section.m_lPtd, 0L);
            READMYSQL_64(atd, section.m_lAtd, 0L);
            READMYSQL_DOUBLE(distance, section.m_dbDist, 0.0);
            READMYSQL_DOUBLE(plan_duration, section.m_dbDuration, 0.0);

            if(section.m_lPta == 0)
                section.m_lPta = section.m_lPtd;
            if(section.m_lPtd == 0)
                section.m_lPtd = section.m_lPta;

            MapScheduleIter iter = m_mapSchedule.find(section.m_szVoyId);
            if(iter == m_mapSchedule.end())
            {
                SYSTEM_LOG("[ScheduleSvc::loadSchedule] not exists voyage_id:%s.", section.m_szVoyId);
                continue;
            }
            section.pSche = iter->second;

            iter->second->m_vecSection.push_back(section);
            if(section.m_nPortId > 0)
            {
                iter->second->m_mapPortIdx.insert(make_pair(section.m_nPortId, section.m_nSeq));
                m_mapPortSchedule.insert(make_pair(section.m_nPortId, iter->second));
            }
        }

		SYSTEM_LOG("[ScheduleSvc::loadSchedule] debug 7 ==============================");
    }

    //3、加载船期中转表
    sprintf(pSQL, "SELECT VOYAGE_ID, NEXT_VOYAGE_ID, PORTID from t30_voyage_sch_transit WHERE VOYAGE_ID IN %s", SetToSqlRange(addSet).c_str());

    {
        if(!psql->Query(pSQL))
        {
            if(pSQL != sql)
                delete []pSQL;
            RELEASE_MYSQL_RETURN(psql, false);
        }

		SYSTEM_LOG("[ScheduleSvc::loadSchedule] debug 8 ==============================");

        while(psql->NextRow())
        {
            char voyageId[64];
            char next_voyageId[64];
            int portid;
            READMYSQL_STR(voyage_id, voyageId);
            READMYSQL_STR(next_voyage_id, next_voyageId);
            READMYSQL_INT(portid, portid, 0);

            MapScheduleIter iter1 = m_mapSchedule.find(voyageId);
            MapScheduleIter iter2 = m_mapSchedule.find(next_voyageId);
            if(iter1 == m_mapSchedule.end())
            {
                SYSTEM_LOG("[ScheduleSvc::loadSchedule] not exists voyage_id:%s.", voyageId);
                continue;
            }
            if(iter2 == m_mapSchedule.end())
            {
                SYSTEM_LOG("[ScheduleSvc::loadSchedule] not exists voyage_id:%s.", next_voyageId);
                continue;
            }

            iter1->second->m_vecTransit.push_back(iter2->second);
            iter1->second->m_vecTransitPortId.push_back(portid);
        }

		SYSTEM_LOG("[ScheduleSvc::loadSchedule] debug 9 ==============================");
    }

    if(pSQL != sql)
        delete []pSQL;

	SYSTEM_LOG("[ScheduleSvc::loadSchedule] end, mapSchedule:%d, mapPortSchedule:%d, carrierSchedule:%d, shipSchedule:%d ==============================", (int)m_mapSchedule.size(), (int)m_mapPortSchedule.size(), (int)m_mapCarrierSchedule.size(), (int)m_mapShipSchedule.size());
    RELEASE_MYSQL_RETURN(psql, true);
}

bool ScheduleSvc::checkConnect()
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_SockLock);

    ACE_Time_Value tv(0, 500*1000);

    if(!m_bTrackConnected)
    {
        ACE_INET_Addr& addrTrack = g_MainConfig::instance()->GetTrackAddr();
        if(m_Connector.connect(m_TrackPeer, addrTrack, &tv) == 0)
        {
            m_bTrackConnected = true;
        }
    }

    if(!m_bVoyConnected)
    {
        ACE_INET_Addr& addrVoy = g_MainConfig::instance()->GetVoyAddr();
        if(m_Connector.connect(m_VoyPeer, addrVoy, &tv) == 0)
        {
            m_bVoyConnected = true;
        }
    }

    return m_bTrackConnected && m_bVoyConnected;
}

bool ScheduleSvc::Start()
{
    if(!loadCarrier() || !loadSchedule())
        return false;

    uint32 interval = g_MainConfig::instance()->GetScheduleFreshTime();
    if(interval <= 0)
        interval = 60*60;

    //int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
    //if(timerId <= 0)
    //    return false;

    checkConnect();

    if(!g_MessageService::instance()->RegisterCmd(MID_SCHEDULE, this))
        return false;

    SERVICE_MAP(SID_SCHEDULE_SEARCH,ScheduleSvc,scheduleSearch);
    SERVICE_MAP(SID_CARRIER_COMPANY,ScheduleSvc,carrierCompany);
    SERVICE_MAP(SID_SCHEDULE_LINE_DETAIL,ScheduleSvc,scheduleLineDetail);
    SERVICE_MAP(SID_SCHEDULE_BY_VSL,ScheduleSvc,scheduleByVsl);
    SERVICE_MAP(SID_SCHEDULE_REALTIME,ScheduleSvc,scheduleRealtime);
    SERVICE_MAP(SID_FREIGHT_PORT,ScheduleSvc,freightPort);
    SERVICE_MAP(SID_FREIGHT_DETAIL,ScheduleSvc,freightDetail);
    SERVICE_MAP(SID_FREIGHT_MY,ScheduleSvc,freightMy);
    SERVICE_MAP(SID_FREIGHT_PUBLISH,ScheduleSvc,freightPublish);
    SERVICE_MAP(SID_FREIGHT_MY_CURR,ScheduleSvc,freightMyCurr);
    SERVICE_MAP(SID_FREIGHT_UPD_BASE,ScheduleSvc,freightUpdBase);
    SERVICE_MAP(SID_FREIGHT_UPD_PRICE,ScheduleSvc,freightUpdPrice);
    SERVICE_MAP(SID_FREIGHT_MY_MGR,ScheduleSvc,freightMyMgr);
    SERVICE_MAP(SID_FREIGHT_MGR,ScheduleSvc,freightMgr);
    SERVICE_MAP(SID_FREIGHT_CHG_CAT,ScheduleSvc,freightChgCat);
    SERVICE_MAP(SID_VOY_COUNT_FOR_VSL,ScheduleSvc,voyCountForVsl);
	
    DEBUG_LOG("[ScheduleSvc::Start] OK......................................");
    return true;
}

int ScheduleSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	SYSTEM_LOG("[ScheduleSvc::handle_timeout] begin ============================ ");
    //更新缓存
    loadSchedule();
	SYSTEM_LOG("[ScheduleSvc::handle_timeout] end ============================ ");
    return 0;
}

std::string TSchedule::toJson3()
{
    TCarrier* pCarrier = g_ScheduleSvc::instance()->GetCarrierBySvrId(m_szCarrierSrvId);

    char buff[1024];
    sprintf(buff, "{lineId:\"%s\",voyId:\"%s\",voynm:\"%s\",cid:\"%s\",cnm:\"%s\",ports:[", m_szCarrierSrvId, m_szVoyId, m_szVoyName, m_szCarrierId, (pCarrier?pCarrier->m_szCarrierName:""));

    std::stringstream out;
    out << buff;
    for(int i=0; i<(int)m_vecSection.size(); i++)
    {
        if(i==0)
            out << m_vecSection[i].toJson();
        else
            out << "," << m_vecSection[i].toJson();
    }
    out << "]}";

    return out.str();
}

TCarrier* ScheduleSvc::GetCarrierBySvrId(const std::string& svrId)
{
    MapCarrierIter iter = m_mapCarrier.find(svrId);
    if(iter == m_mapCarrier.end())
        return NULL;

    return &(iter->second);
}

//{loadport:"40440",destport:"40102"}
int ScheduleSvc::scheduleSearch(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::scheduleSearch]bad format:", jsonString, 1);

    int lp = atoi(root.getv("loadport", ""));
    int dp = atoi(root.getv("destport", ""));

    if(lp <= 0 || dp <= 0)
		return 1;

    std::map<TSchedule*, int> mapScheduleTPort;
    std::map<TSchedule*, int>::iterator iterMapScheduleTPort;

    //ACE_Read_Guard<ACE_RW_Thread_Mutex> WGuard(m_ThreadLock);

    VecSchedule schedules;
    MapPortScheduleRange range = m_mapPortSchedule.equal_range(lp);
    for(MapPortScheduleIter iter=range.first; iter!=range.second; iter++)
    {
        TSchedule* pSchedule = iter->second;
        IntIntMapIter iter1 = pSchedule->m_mapPortIdx.find(lp);

        int tportid = 0;
        TSchedule* pTran = pSchedule->GetHowToPort(dp, tportid);

        //到不了
        if(!pTran)
            continue;

        //中转
        if(tportid)
        {
            schedules.push_back(pSchedule);
            mapScheduleTPort.insert(std::map<TSchedule*, int>::value_type(pSchedule, tportid));
            continue;
        }

        //直达
        IntIntMapIter iter2 = pSchedule->m_mapPortIdx.find(dp);
        if(iter1->second < iter2->second)
            schedules.push_back(pSchedule);
    }

    time_t curT = time(NULL);
    tm* curTm = gmtime(&curT);
    int nCurMonth = curTm->tm_mon+1;

    std::map<std::string, VecSchedule> mapVecSchedule;
    std::map<std::string, VecSchedule>::iterator iterMapVecSchedule;

    for(int i=0; i<(int)schedules.size(); i++)
    {
        TScheduleSection* lpSec = schedules[i]->GetSection(lp);
        TScheduleSection* dpSec = schedules[i]->GetSection(dp);

        time_t scheT = lpSec->m_lPta;
        tm* scheTm = gmtime(&scheT);
		if(!scheTm)	//数据有问题
			continue;

        int nMonth = scheTm->tm_mon+1;

        //只取当前月份和下一个月份的数据
        if(nMonth!=nCurMonth && nMonth!=nCurMonth+1)
            continue;

        iterMapVecSchedule = mapVecSchedule.find(schedules[i]->m_szCarrierSrvId);
        if(iterMapVecSchedule == mapVecSchedule.end())
        {
            VecSchedule vec;
            vec.push_back(schedules[i]);
            mapVecSchedule.insert(std::make_pair(schedules[i]->m_szCarrierSrvId, vec));
        }
        else
        {
            iterMapVecSchedule->second.push_back(schedules[i]);
        }
    }

    //拼串返回
    out << "[";
    for(iterMapVecSchedule = mapVecSchedule.begin(); iterMapVecSchedule != mapVecSchedule.end(); iterMapVecSchedule++)
    {
        TCarrier* pCarrier = GetCarrierBySvrId(iterMapVecSchedule->first);
        VecSchedule& vecSchedule = iterMapVecSchedule->second;

        double transtime = 0.0;
        string ldts, pdts;
        int tportid = 0;
        char sportid[64] = {0};

        for(int i=0; i<(int)vecSchedule.size(); i++)
        {
            TScheduleSection* lpSec = vecSchedule[i]->GetSection(lp);
            TScheduleSection* dpSec = vecSchedule[i]->GetSection(dp);

            double _transtime = (dpSec->m_lPta - lpSec->m_lPta)/(24.0*3600);
            if(_transtime < 0)
            {
                continue;
            }

            transtime = _transtime;

            if(i==0)
            {
                ldts += TimeToLocalDate((time_t)lpSec->m_lPta);
                pdts += TimeToLocalDate((time_t)dpSec->m_lPta);

                iterMapScheduleTPort = mapScheduleTPort.find(vecSchedule[i]);
                if(iterMapScheduleTPort != mapScheduleTPort.end())
                    sprintf(sportid, "%d", iterMapScheduleTPort->second);
            }
            else
            {
                ldts += "," + TimeToLocalDate((time_t)lpSec->m_lPta);
                pdts += "," + TimeToLocalDate((time_t)dpSec->m_lPta);
            }
        }

        if(iterMapVecSchedule != mapVecSchedule.begin())
            out << ",";

        out << "{lineId:\"" << iterMapVecSchedule->first << "\",linenm:\"" << (pCarrier?pCarrier->m_szCarrierSrvNameEn:"") << "\",cid:\"" << (pCarrier?pCarrier->m_szCarrierId:"") << "\",cnm:\"" << (pCarrier?pCarrier->m_szCarrierName:"") << "\",transtime:\"" << transtime << "\",";
        out << "lpt:\"\",tpt:\"" << sportid << "\",dpt:\"\",ldt:\"" << ldts << "\",ddt:\"" << pdts << "\"}";
    }
    out << "]";

    return 0;
}

//{cid:"C100023"}
int ScheduleSvc::carrierCompany(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::carrierCompany]bad format:", jsonString, 1);

    std::string carrierID = root.getv("cid", "");
    if(carrierID.empty())
		return 1;

    char sql[1024];
    sprintf(sql, "SELECT name_en, name_cn from t41_carrier where carrier_id = '%s'", carrierID.c_str());

    MySql* psql = CREATE_MYSQL;
    if(!psql->Query(sql) || !psql->NextRow())
        RELEASE_MYSQL_RETURN(psql, 3);

    char name_cn[128];
    char name_en[128];
    READMYSQL_STR(name_en, name_en);
    READMYSQL_STR(name_cn, name_cn);

    sprintf(sql, "SELECT t1.company_key, t2.name, t2.address, t2.telno, t2.website \
				 FROM t41_carrier_company t1, t41_company t2 \
				 WHERE t1.COMPANY_KEY = t2.Company_Key AND t1.CARRIER_ID = '%s'", carrierID.c_str());
    if(!psql->Query(sql))
    {
        RELEASE_MYSQL_RETURN(psql, 3);
    }

    char id[64];
    char name[128];
    char address[128];
    char telno[128];
    char website[128];
    bool bFirst = true;

    out << "{nmen:\"" << name_cn << "\",nmcn:\"" << name_cn << "\",Cmpys:[";
    while(psql->NextRow())
    {
        READMYSQL_STR(company_key, id);
        READMYSQL_STR(name, name);
        READMYSQL_STR(address, address);
        READMYSQL_STR(telno, telno);
        READMYSQL_STR(website, website);
        READMYSQL_STR(name_cn, name_cn);
        READMYSQL_STR(name_en, name_en);

        if(bFirst)
        {
            bFirst = false;
        }
        else
        {
            out << ",";
        }
        out << "{Id:\"" << id << "\",nm:\"" << name << "\",addr:\"" << address << "\",tel:\"" << telno << "\",web:\"" << website << "\"}";
    }
    out << "]}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{lineid:"",loadptid:"40383",destptid:"40240",transit:0,month:3,dtflag:0,startdt:"132323445",enddt:"137323445"}
int ScheduleSvc::scheduleLineDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::scheduleLineDetail]bad format:", jsonString, 1);

    string lineid = root.getv("lineid", "");
    int month = root.getv("month", 0);
    string lpid = root.getv("loadptid", "");
    string dpid = root.getv("destptid", "");
    int dtFlag = root.getv("dtflag", 0);
    int sdt = atoi(root.getv("startdt", ""));
    int edt = atoi(root.getv("enddt", ""));

    if(lineid.empty() || lpid.empty() || dpid.empty())
		return 1;

    time_t curT = time(NULL);
    tm* curTm = gmtime(&curT);
    int nCurMonth = curTm->tm_mon+1;

    //ACE_Read_Guard<ACE_RW_Thread_Mutex> WGuard(m_ThreadLock);

    VecSchedule schedules;
    MapCarrierScheduleRange range = m_mapCarrierSchedule.equal_range(lineid);
    for(MapCarrierScheduleIter iter=range.first; iter!=range.second; iter++)
    {
        TSchedule* pSchedule = iter->second;

        int tportid = 0;
        TSchedule* pTran1 = pSchedule->GetHowToPort(atoi(lpid.c_str()), tportid);
        TSchedule* pTran2 = pSchedule->GetHowToPort(atoi(dpid.c_str()), tportid);

        if(!pTran1 || pTran1 != pSchedule || !pTran2)
            continue;

        if(pTran1 == pTran2)
        {
            int idx1 = pTran1->m_mapPortIdx[atoi(lpid.c_str())];
            int idx2 = pTran2->m_mapPortIdx[atoi(dpid.c_str())];
            if(idx1 >= idx2)
                continue;
        }

        if(sdt == 0 || edt == 0)	//按月份过滤
        {
            time_t scheT = pTran1->GetSection(atoi(lpid.c_str()))->m_lPta;
            tm* scheTm = gmtime(&scheT);
            int nMonth = scheTm->tm_mon+1;

            //只取当前月份和下一个月份的数据
            if(nMonth!=nCurMonth && nMonth!=nCurMonth+1)
                continue;
        }
        else						//按时间区间过滤
        {
            if(dtFlag == 0 && !(pTran1->m_lDeptPta >= sdt && pTran1->m_lDeptPta <= edt))
                continue;
            if(dtFlag == 1 && !(pTran1->m_lDestPta >= sdt && pTran1->m_lDestPta <= edt))
                continue;
        }

        schedules.push_back(pSchedule);
    }

    out << "[";
    for(int i=0; i<(int)schedules.size(); i++)
    {
        if(i==0)
            out << schedules[i]->toJson2(atoi(dpid.c_str()));
        else
            out << "," << schedules[i]->toJson2(atoi(dpid.c_str()));
    }
    out << "]";

    return 0;
}

int ScheduleSvc::scheduleByVsl(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::scheduleByVsl]bad format:", jsonString, 1);

    string shipid = root.getv("shipid", "");
    if(shipid.empty())
		return 1;

    //ACE_Read_Guard<ACE_RW_Thread_Mutex> WGuard(m_ThreadLock);

    VecSchedule schedules;
    MapShipScheduleRange range = m_mapShipSchedule.equal_range(shipid);
    for(MapShipScheduleIter iter=range.first; iter!=range.second; iter++)
    {
        schedules.push_back(iter->second);
    }

    out << "[";
    for(int i=0; i<(int)schedules.size(); i++)
    {
        if(i==0)
            out << schedules[i]->toJson3();
        else
            out << "," << schedules[i]->toJson3();
    }
    out << "]";

    return 0;
}

struct _Track
{
    int time;
    double lon, lat;
    double speed;
    int status;
};

typedef std::vector<_Track> VecTrack;
int getNearestPoint(VecTrack& trackList, double lon, double lat)
{
    if(trackList.empty())
        return -1;

    int iMin = 0;
    double dbMin = CalCircleDistance(lon, lat, trackList[0].lon, trackList[0].lat);
    for(int i=1; i<(int)trackList.size(); i++)
    {
        double dbDist = CalCircleDistance(lon, lat, trackList[i].lon, trackList[i].lat);
        if(dbDist < dbMin)
        {
            iMin = i;
            dbMin = dbDist;
        }
    }

    return iMin;
}
int getMooredPoint(VecTrack& trackList, int idx)
{
    if(trackList[idx].status == 5)
        return idx;

    int i=idx-1, j=idx+1, n=10;
    while(i > 0 && j < (int)trackList.size() && n > 0)
    {
        if(trackList[i].status == 5)
            return i;
        if(trackList[j].status == 5)
            return j;
        i--;
        j++;
        n--;
    }

    while(i > 0 && n > 0)
    {
        if(trackList[i].status == 5)
            return i;
        i--;
        n--;
    }

    while(j < (int)trackList.size() && n > 0)
    {
        if(trackList[j].status == 5)
            return j;
        j++;
        n--;
    }

    return -1;
}
int getStartPoint(VecTrack& trackList, int idx)
{
    for(int i=idx+1; i<(int)trackList.size(); i++)
        if(trackList[i].status == 0)
            return i;
    return -1;
}

//{voyId:"",shipid:""}
//{voyId:"", ports:[{portId:"40449",portnm:"Shanghai",seq:0,ata:"1331981519",atd:"1331981519",udis:"0",utime:"0"},{portId:"40440",portnm:"guangzhou",seq:1,ata:"1331991203",atd:"1331991203",udis:"32193",utime:"12389"}]
int ScheduleSvc::scheduleRealtime(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::scheduleRealtime]bad format:", jsonString, 1);

    string voyid = root.getv("voyId", "");
    string shipid = root.getv("shipid", "");
    if(voyid.empty())
		return 1;

	SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 1===============");

    if(!checkConnect())
    {
        SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] checkConnect failed.");
        return 2;
    }

	SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 2===============");

    //ACE_Read_Guard<ACE_RW_Thread_Mutex> WGuard(m_ThreadLock);

    MapScheduleIter iter = m_mapSchedule.find(voyid);
    if(iter == m_mapSchedule.end())
        return 2;

    TSchedule* pSche = iter->second;
    if(pSche->m_nMmsi == 0)
        return 2;

	SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 3===============");

    //请求实时信息
    double lon, lat, speed;
    int aisTime;
	AisRealtime* pAis = g_VesselSvc::instance()->aisShip.FindAis(pSche->m_nMmsi);
	if(!pAis)
	{
		SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] no realtime ais.");
		return 2;
	}

	SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 4===============");

	aisTime = pAis->time;
	lon = pAis->lon;
	lat = pAis->lat;
	speed = pAis->sog/10;
	if(speed == 0)
		speed = pAis->avgspeed/10;

    //信号不在时间区间范围内
    if(aisTime <= pSche->m_lDeptPta || aisTime >= pSche->m_lDestPta)
    {
        DEBUG_LOG("[ScheduleSvc::scheduleRealtime] ais time not in schedule's time.");
        return 2;
    }

    //根据时间判断落在哪两个港口之间
    int i, j;
    VecScheduleSection vecSection = pSche->m_vecSection;
    for(i=0; i<(int)vecSection.size()-1; i++)
    {
        TScheduleSection& begin = vecSection[i];
        TScheduleSection& end = vecSection[i+1];
        if(aisTime >= begin.m_lPtd && aisTime <= end.m_lPtd)
            break;
    }
    if(i == (int)vecSection.size()-1) //数据有问题
    {
        DEBUG_LOG("[ScheduleSvc::scheduleRealtime] data problem");
        return 2;
    }

    ACE_Time_Value tv(0, 3000*1000);
    char pReq[1024];
    char* pRes = NULL;
    string retJsonString;

	SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 5===============");

    //当前位置到剩余港口的计算
    //1. 当前位置到下一个港口的距离
    {
        ACE_Guard<ACE_Thread_Mutex> guard(m_SockLock);
        sprintf(pReq, "{nr_point:[%.2f,%.2f], dest_port:%d}", lon, lat, vecSection[i+1].m_nPortId);

		SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 6===============");

        if(!ReqPostServer(m_VoyPeer, 0x0502, pReq, tv, pRes))
        {
            m_VoyPeer.close();
            m_bVoyConnected = false;

            DEBUG_LOG("[ScheduleSvc::scheduleRealtime] request voyageserver failed.");
            return 2;
        }
        retJsonString = string(pRes+16);
        delete pRes;

		SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 7===============");
    }

    {
        JSON_PARSE_RETURN("[ScheduleSvc::scheduleRealtime]bad format:", retJsonString.c_str(), 2);
        int nAis2NowDist = (int)((time(NULL) - aisTime) / 3600 * speed * 1.852 * 1000); //米
        int nAis2PortDist = atoi(root.getv("distance", ""));
        if(nAis2NowDist >= nAis2PortDist)										 //实际上已经经过了这个港口,而且ATA/ATD是没法计算的,同时还要把港口往后推,找出当前时间实际在哪
        {
            vecSection[i+1].m_nCalLeftDist = 0;
            vecSection[i+1].m_nCalLeftTime = 0;
            vecSection[i+1].m_lCalAta = vecSection[i+1].m_lPta;
            vecSection[i+1].m_lCalAtd = vecSection[i+1].m_lPtd;
            nAis2NowDist -= nAis2PortDist;

            j = i+2;
            while(j < (int)vecSection.size())
            {
                if(nAis2NowDist >= vecSection[j].m_dbDist)
                {
                    vecSection[j].m_nCalLeftDist = 0;
                    vecSection[j].m_nCalLeftTime = 0;
                    vecSection[j].m_lCalAta = vecSection[j].m_lPta;
                    vecSection[j].m_lCalAtd = vecSection[j].m_lPtd;
                    nAis2NowDist -= (int)vecSection[j].m_dbDist;
                }
                else
                    break;
            }

            if(j < (int)vecSection.size())
            {
                vecSection[j].m_nCalLeftDist = (int)(vecSection[j].m_dbDist - nAis2NowDist);
                vecSection[j].m_nCalLeftTime = (int)((vecSection[j].m_nCalLeftDist / 1000 / 1.852 / speed) * 3600); //秒
                vecSection[j].m_lCalAta = (long)(time(NULL) + vecSection[j].m_nCalLeftTime);
                vecSection[j].m_lCalAtd = vecSection[j].m_lCalAta + (vecSection[j].m_lPtd - vecSection[j].m_lPta);

                for(i=j+1; i<(int)vecSection.size(); i++)
                {
                    vecSection[i].m_nCalLeftDist = (int)(vecSection[i].m_dbDist + vecSection[i-1].m_nCalLeftDist);
                    vecSection[i].m_nCalLeftTime = (int)((vecSection[i].m_nCalLeftDist /1000 / 1.852 / speed) * 3600);
                    vecSection[i].m_lCalAta = vecSection[i-1].m_lCalAtd + vecSection[i].m_nCalLeftTime;
                    vecSection[i].m_lCalAtd = vecSection[i].m_lCalAta + (vecSection[i].m_lPtd - vecSection[i].m_lPta);
                    vecSection[i].m_nCalLeftTime += (vecSection[i-1].m_lPtd - vecSection[i-1].m_lPta);
                }
            }
        }
        else
        {
            vecSection[i+1].m_nCalLeftDist = nAis2PortDist - nAis2NowDist;
            vecSection[i+1].m_nCalLeftTime = (int)((vecSection[i+1].m_nCalLeftDist / 1000 / 1.852 / speed) * 3600); //秒
            vecSection[i+1].m_lCalAta = (long)(time(NULL) + vecSection[i+1].m_nCalLeftTime);
            vecSection[i+1].m_lCalAtd = vecSection[i+1].m_lCalAta + (vecSection[i+1].m_lPtd - vecSection[i+1].m_lPta);

            for(j=i+2; j<(int)vecSection.size(); j++)
            {
                vecSection[j].m_nCalLeftDist = (int)(vecSection[j].m_dbDist + vecSection[j-1].m_nCalLeftDist);
                vecSection[j].m_nCalLeftTime = (int)((vecSection[j].m_nCalLeftDist /1000 / 1.852 / speed) * 3600);
                vecSection[j].m_lCalAta = vecSection[j-1].m_lCalAtd + vecSection[j].m_nCalLeftTime;
                vecSection[j].m_lCalAtd = vecSection[j].m_lCalAta + (vecSection[j].m_lPtd - vecSection[j].m_lPta);
                vecSection[j].m_nCalLeftTime += (vecSection[j-1].m_lPtd - vecSection[j-1].m_lPta);
            }
        }
    }

	SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 8===============");

    //航线第一个港口到当前港口的计算（请求轨迹信息，从trackserver获取)
    {
        ACE_Guard<ACE_Thread_Mutex> guard(m_SockLock);
        sprintf(pReq, "{uid:\"blmmain\",mmsi:\"%d\",segs:[{sdt:\"%d\",edt:\"%d\"}]}", pSche->m_nMmsi, pSche->m_lDeptPta, pSche->m_lDestPta);
        
		SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 9===============");

		if(!ReqPostServer(m_TrackPeer, 0x011d, pReq, tv, pRes))
        {
            m_TrackPeer.close();
            m_bTrackConnected = false;

            DEBUG_LOG("[ScheduleSvc::scheduleRealtime] request trackserver failed.");
            return 2;
        }
        retJsonString = string(pRes+16);
        delete pRes;

		SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 10===============");
    }

    {
        VecTrack trackList;
        JSON_PARSE_RETURN("[ScheduleSvc::scheduleRealtime]bad format:", retJsonString.c_str(), 2);
        if(root.size() >= 1)
        {
            Json* trackArray = root.get(0);
            for(j=0; j<trackArray->size(); j++)
            {
                string trackString = trackArray->getv(j, "");
                Tokens items = StrSplit(trackString, "|");
                if(items.size() != 13)
                    continue;

                _Track track;
                track.lon = atof(items[1].c_str());
                track.lat = atof(items[2].c_str());
                track.time = atoi(items[3].c_str());
                track.speed = atoi(items[5].c_str()) / 10.0;
                track.status = atoi(items[12].c_str());
                trackList.push_back(track);
            }
        }

		SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 11===============");

        for(j=0; j<=i; j++)
        {
            PortInfo* pPort = g_PortSvc::instance()->FindPortByID(vecSection[j].m_nPortId);
            if(!pPort)
            {
                //数据有问题
                SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] could not find port:%d.", vecSection[j].m_nPortId);
                return 2;
            }
            int iMin = getNearestPoint(trackList, pPort->x, pPort->y);
            if(iMin != -1)
            {
                vecSection[j].m_lCalAta = vecSection[j].m_lCalAtd = trackList[i].time;
                int iMoored = getMooredPoint(trackList, iMin);
                if(iMoored != -1)
                {
                    vecSection[j].m_lCalAta = trackList[iMoored].time;
                    int iStart = getStartPoint(trackList, iMoored);
                    if(iStart != -1)
                        vecSection[j].m_lCalAtd = trackList[iStart].time;
                }

            }
            else
            {
                vecSection[j].m_lCalAta = vecSection[j].m_lPta;
                vecSection[j].m_lCalAtd = vecSection[j].m_lPtd;
            }
            vecSection[j].m_nCalLeftDist = 0;
            vecSection[j].m_nCalLeftTime = 0;
        }

		SYSTEM_LOG("[ScheduleSvc::scheduleRealtime] 12===============");
    }

    out << "{voyId:\"" << pSche->m_szVoyId << "\",ports:";
    out << "[";
    for(int i=0; i<(int)vecSection.size(); i++)
    {
        out << (i==0?"":",") << vecSection[i].toJson2();
    }
    out << "]}";

    return 0;
}

//{loadport:"40440",destport:"40102",lineId:""}
//{pid:"f00001",lid:"HPSX",cid:"C100023",cnm:"Cosco",cmpyId:"",cmpynm:"",day:3,trm:30, boxtp:0,lpt:"shanghai",dpt:"new york",sdt:"2012-03-08",adt:"2012-04-10",fee:"300/500/0/0"}
int ScheduleSvc::freightPort(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::freightPort]bad format:", jsonString, 1);

    string lp = root.getv("loadport", "");
    string dp = root.getv("destport", "");
    string lineId = root.getv("lineId", "");

    if(lp.empty() || dp.empty())
		return 1;

    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    if(!lineId.empty())
        sprintf(sql, "SELECT t1.PRICE_ID, t1.CARRIER_SRV_ID, t1.CARRIER_ID, CONCAT(t3.NAME_ABBR_EN,'-',T3.NAME_ABBR_CN) as CARRIER_NAME, t1.WEEKDAYS, t1.PRICE, t1.DEPT_PORT, t1.DEST_PORT, t1.BOX_TYPE, t2.USER_ID, t2.COMPANY_KEY, t4.Name \
			FROM T30_price t1, T41_carrier t3, T30_myfreight t2 \
			LEFT JOIN t41_company t4 ON t2.COMPANY_KEY = t4.COMPANY_KEY \
			WHERE t1.DEPT_PORT = '%s' AND t1.DEST_PORT = '%s' AND t1.CARRIER_SRV_ID = '%s' AND \
			t2.SURVEY = '1' AND t2.TRASH_FLAG = '0' AND t2.PAUSE_FLAG = '0' AND \
			t1.FREIGHT_ID = t2.FREIGHT_ID AND \
			t1.CARRIER_ID = t3.CARRIER_ID", lp.c_str(), dp.c_str(), lineId.c_str());
    else
        sprintf(sql, "SELECT t1.PRICE_ID, t1.CARRIER_SRV_ID, t1.CARRIER_ID, CONCAT(t3.NAME_ABBR_EN,'-',T3.NAME_ABBR_CN) as CARRIER_NAME, t1.WEEKDAYS, t1.PRICE, t1.DEPT_PORT, t1.DEST_PORT, t1.BOX_TYPE, t2.USER_ID, t2.COMPANY_KEY, t4.Name \
			FROM T30_price t1, T41_carrier t3, T30_myfreight t2 \
			LEFT JOIN t41_company t4 ON t2.COMPANY_KEY = t4.COMPANY_KEY \
			WHERE t1.DEPT_PORT = '%s' AND t1.DEST_PORT = '%s' AND \
			t2.SURVEY = '1' AND t2.TRASH_FLAG = '0' AND t2.PAUSE_FLAG = '0' AND \
			t1.FREIGHT_ID = t2.FREIGHT_ID AND \
			t1.CARRIER_ID = t3.CARRIER_ID", lp.c_str(), dp.c_str());

    if(!psql->Query(sql))
    {
        RELEASE_MYSQL_RETURN(psql, 3);
    }

    time_t curT = time(NULL);
    tm* curTm = gmtime(&curT);
    int nCurMonth = curTm->tm_mon+1;

    bool bFirst = true;
    out << "[";
    while(psql->NextRow())
    {
        char price_id[64];
        char carrier_srv_id[64];
        char carrier_id[64];
        char carrier_name[64];
        int  day;
        int  boxtp;
        char price[64];
        char dept_port[64];
        char dest_port[64];
        char cmp_key[64];
        char cmp_name[64];

        READMYSQL_STR(price_id, price_id);
        READMYSQL_STR(carrier_srv_id, carrier_srv_id);
        READMYSQL_STR(carrier_id, carrier_id);
        READMYSQL_STR(carrier_name, carrier_name);
        READMYSQL_INT(weekdays, day, 0);
        READMYSQL_INT(box_type, boxtp, 0);
        READMYSQL_STR(price, price);
        READMYSQL_STR(dept_port, dept_port);
        READMYSQL_STR(dest_port, dest_port);
        READMYSQL_STR(company_key, cmp_key);
        READMYSQL_STR(name, cmp_name);

        //ACE_Read_Guard<ACE_RW_Thread_Mutex> WGuard(m_ThreadLock);

        VecSchedule vecSchedule;
        MapCarrierScheduleRange range = m_mapCarrierSchedule.equal_range(carrier_srv_id);
        for(MapCarrierScheduleIter iter=range.first; iter!=range.second; iter++)
        {
            TScheduleSection* lpSec = iter->second->GetSection(atoi(lp.c_str()));
            TScheduleSection* dpSec = iter->second->GetSection(atoi(dp.c_str()));
            if(!lpSec || !dpSec)
                continue;

            if(lpSec->pSche == dpSec->pSche)
            {
                if(lpSec->m_nSeq >= dpSec->m_nSeq)
                    continue;
            }

            time_t scheT = lpSec->m_lPta;
            tm* scheTm = gmtime(&scheT);
            int nMonth = scheTm->tm_mon+1;

            //只取当前月份和下一个月份的数据
            if(nMonth!=nCurMonth && nMonth!=nCurMonth+1)
                continue;

            vecSchedule.push_back(iter->second);
        }

        double transtime = 0.0;
        string sdts, edts;
        for(int i=0; i<(int)vecSchedule.size(); i++)
        {
            TScheduleSection* lpSec = vecSchedule[i]->GetSection(atoi(lp.c_str()));
            TScheduleSection* dpSec = vecSchedule[i]->GetSection(atoi(dp.c_str()));

            double _transtime = (dpSec->m_lPta - lpSec->m_lPta)/(24.0*3600);
            if(_transtime < 0)
                continue;

            transtime = _transtime;

            if(i==0)
            {
                sdts += TimeToLocalDate((time_t)lpSec->m_lPta);
                edts += TimeToLocalDate((time_t)dpSec->m_lPta);
            }
            else
            {
                sdts += "," + TimeToLocalDate((time_t)lpSec->m_lPta);
                edts += "," + TimeToLocalDate((time_t)dpSec->m_lPta);
            }
        }

        char buff[1024];
        sprintf(buff, "{pid:\"%s\",lid:\"%s\",cid:\"%s\",cnm:\"%s\",cmpyId:\"%s\",cmpynm:\"%s\",day:%d,trm:\"%f\",boxtp:%d,lpt:\"%s\",dpt:\"%s\",sdt:\"%s\",edt:\"%s\",fee:\"%s\"}",
                price_id, carrier_srv_id, carrier_id, carrier_name, cmp_key, cmp_name, day, transtime, boxtp, dept_port, dest_port, sdts.c_str(), edts.c_str(), price);
        if(bFirst)
        {
            out << buff;
            bFirst = false;
        }
        else
        {
            out << "," << buff;
        }
    }
    out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int ScheduleSvc::freightDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::freightDetail]bad format:", jsonString, 1);

    string pi = root.getv("priceId", "");
    if(pi.empty())
		return 1;
   
    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "SELECT t1.PRICE, UNIX_TIMESTAMP(t2.EXPIRE_DT) AS validdt, t2.PAY_DEMAND, t2.BL_DEMAND, t2.LIMIT_MT, t2.MIN_TEU, t2.REMARK, \
				 t1.CARRIER_SRV_ID, t1.DEPT_PORT, t1.DEST_PORT, t1.CARRIER_ID, t1.WEEKDAYS, t1.BOX_TYPE, CONCAT(t5.NAME_ABBR_EN,'-',t5.NAME_ABBR_CN) AS Carrier_Name, \
		t3.USER_ID, t3.NICKNAME, t3.TELNO, t3.MOBILE, t3.EMAIL, \
		t4.Company_Key, t4.Name AS CompName, t4.VERIFIED, t4.Address, t4.Detail, t4.TelNo AS CompTelNo \
		FROM t30_price t1, t00_user t3, t41_carrier t5, t30_myfreight t2 LEFT JOIN t41_company t4 ON t2.COMPANY_KEY = t4.Company_Key \
		WHERE t1.PRICE_ID = '%s' AND t1.FREIGHT_ID = t2.FREIGHT_ID AND t2.USER_ID = t3.USER_ID AND t1.CARRIER_ID = t5.CARRIER_ID", pi.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{
		out << "{";

		// freight
		char price[64];
		long validdt;
		int paytp, billtp;
		char limitMt[64];
		int minTeu;
		int weekdays;
		int boxtp;
		char rmk[1024];
		READMYSQL_STR(price, price);
		READMYSQL_64(validdt, validdt, 0L);
		READMYSQL_INT(pay_demand, paytp, 0);
		READMYSQL_INT(bl_demand, billtp, 0);
		READMYSQL_STR(limit_mt, limitMt);
		READMYSQL_INT(min_teu, minTeu, 0);
		READMYSQL_INT(weekdays, weekdays, 0);
		READMYSQL_INT(box_type, boxtp, 0);
		READMYSQL_STR(remark, rmk);
		out << "freight:{fare:\""
			<< price << "\",validdt:\""
			<< validdt << "\",paytp:\""
			<< paytp << "\",boxtp:\""
			<< boxtp << "\",bill:\""
			<< billtp << "\",limitwt:\""
			<< limitMt << "\",minnum:\""
			<< minTeu << "\",des:\""
			<< rmk << "\"},";

		// carrier
		char lineId[64];
		int deptPort, destPort;
		char carrierId[64];
		char carrierName[64];
		READMYSQL_STR(carrier_srv_id, lineId);
		READMYSQL_STR(carrier_id, carrierId);
		READMYSQL_STR(carrier_name, carrierName);
		READMYSQL_INT(dept_port, deptPort, 0);
		READMYSQL_INT(dest_port, destPort, 0);
		PortInfo* pDeptPort = g_PortSvc::instance()->FindPortByID(deptPort);
		PortInfo* pDestPort = g_PortSvc::instance()->FindPortByID(destPort);
		out << "voy:{lineId:\""
			<< lineId << "\",loadport:\""
			<< (pDeptPort?pDeptPort->portname.c_str():"") << "\",destport:\""
			<< (pDestPort?pDestPort->portname.c_str():"") << "\", carrierid:\""
			<< carrierId << "\",carrier:\""
			<< carrierName << "\",day:" << weekdays << ",tanstime:0},";

		// user
		char userId[64];
		char nickName[64];
		char telno[64];
		char mobile[64];
		char email[64];
		READMYSQL_STR(user_id, userId);
		READMYSQL_STR(nickName, nickName);
		READMYSQL_STR(telno, telno);
		READMYSQL_STR(mobile, mobile);
		READMYSQL_STR(email, email);
		out << "usrInfo:{usrId:\""
			<< userId << "\", usrnm:\""
			<< nickName << "\", online:0,telno:\""
			<< telno << "\",mobile:\""
			<< mobile << "\",email:\""
			<< email << "\"},";

		// company
		char companyKey[64];
		char companyName[64];
		int verified;
		char companyAddr[64];
		char companyDetail[1024];
		char companyTelno[64];
		READMYSQL_STR(company_key, companyKey);
		READMYSQL_STR(CompName, companyName);
		READMYSQL_INT(VERIFIED, verified, 0);
		READMYSQL_STR(address, companyAddr);
		READMYSQL_STR(detail, companyDetail);
		READMYSQL_STR(comptelno, companyTelno);
		out << "cmpyInfo:{cmpyId:\""
			<< companyKey << "\",cmpyName:\""
			<< companyName << "\",valid:"
			<< verified << ",addr:\""
			<< companyAddr << "\",cdes:\""
			<< companyDetail << "\",telno:\""
			<< companyTelno << "\"}";

		out << "}";
	}
	else
	{
		RELEASE_MYSQL_RETURN(psql, 2);
	}

    RELEASE_MYSQL_RETURN(psql, 0);
}

int ScheduleSvc::freightMy(const char* pUid, const char* jsonString, std::stringstream& out)
{
    string uid(pUid);

    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "SELECT categoryid,categoryname,UNIX_TIMESTAMP(create_dt) as createdt,trash_flag,v1.num FROM T30_myfreight_category t1 LEFT JOIN t41_remark_count v1 ON t1.user_id=v1.user_id AND t1.categoryid=v1.object_id AND v1.type=7 WHERE t1.user_id ='%s'", uid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	MapFreightCategory mapFreightCat;
	if(psql->GetRowCount()==0)
	{
		sprintf(sql, "insert into t30_myfreight_category(user_id, categoryid, categoryname, update_dt, create_dt, trash_flag) values ('%s', '00', 'my freight', CURRENT_TIMESTAMP(), CURRENT_TIMESTAMP(), 0)", uid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		TFreightCategory cat;
		sprintf(cat.m_szCatId, "00");
		sprintf(cat.m_szCatName, "my freight");
		cat.m_lCreate = (long)time(NULL);
		cat.m_nRmkNum = 0;
		cat.m_nTrash = 0;
		mapFreightCat.insert(make_pair(cat.m_szCatName, cat));
	}
	else
	{
		while(psql->NextRow())
		{
			TFreightCategory cat;
			READMYSQL_STR(categoryid, cat.m_szCatId);
			READMYSQL_STR(categoryname, cat.m_szCatName);
			READMYSQL_64(createdt, cat.m_lCreate, 0L);
			READMYSQL_INT(trash_flag, cat.m_nTrash, 0);
			READMYSQL_INT(num, cat.m_nRmkNum, 0);
			mapFreightCat.insert(make_pair(cat.m_szCatId, cat));
		}
	}
	sprintf(sql, "Select category_id,freight_id,freight_name,UNIX_TIMESTAMP(update_dt) as createdt, UNIX_TIMESTAMP(expire_dt) as validdt, trash_flag,pause_flag,survey,v1.num from t30_myfreight t1 left join t41_remark_count v1 on t1.user_id=v1.user_id and t1.freight_id=v1.object_id and v1.type=8 where t1.user_id ='%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	while(psql->NextRow())
	{
		TFreight freight;
		READMYSQL_STR(category_id, freight.m_szFreightCatID);
		READMYSQL_STR(freight_id, freight.m_szFreightID);
		READMYSQL_STR(freight_name, freight.m_szFreightName);
		READMYSQL_64(creatdt, freight.m_lCreateTime, 0L);
		READMYSQL_64(validdt, freight.m_lValidTime, 0L);
		READMYSQL_INT(trash_flag, freight.m_nTrash, 0);
		READMYSQL_INT(pause_flag, freight.m_nPause, 0);
		READMYSQL_INT(survey, freight.m_nFlag, 0);
		READMYSQL_INT(num, freight.m_nRmkNum, 0);
		MapFreightCatIter iter = mapFreightCat.find(freight.m_szFreightCatID);
		if(iter != mapFreightCat.end())
			iter->second.m_vecFreight.push_back(freight);
	}

	out << "[";

	MapFreightCatIter iter;
	for(iter=mapFreightCat.begin(); iter!=mapFreightCat.end(); iter++)
	{
		if(iter == mapFreightCat.begin())
			out << iter->second.toJson();
		else
			out << "," << iter->second.toJson();
	}

	out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int ScheduleSvc::freightPublish(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::freightPublish]bad format:", jsonString, 1);

    string uid(pUid);

    string fn = SqlReplace(root.getv("fn", ""));
    string catid = root.getv("catid", "");
    int validdt = root.getv("validdt", 0);
    int paytp = root.getv("paytp", 0);
    int billtp = root.getv("bill", 0);
    string lwt = root.getv("lwt", "");
    int minTeu = root.getv("min", 0);
    string rmk = root.getv("des", "");
    Json* prices = root["fare"];

    if(fn.empty() || catid.empty())
		return 1;
    
    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    //查询发布人所属公司
    char companyKey[20] = {'\0'};
    sprintf(sql, "select company_key from t41_comp_contacts where user_id = '%s'", pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    if(psql->NextRow())
    {
        READMYSQL_STR(company_key, companyKey);
    }

    sprintf(sql, "insert into t30_myfreight(FREIGHT_NAME, UPDATE_DT, USER_ID, CATEGORY_ID, COMPANY_KEY, EXPIRE_DT, LIMIT_MT, MIN_TEU, PAY_DEMAND, BL_DEMAND, REMARK, SURVEY, PAUSE_FLAG, TRASH_FLAG) \
				 values ('%s', CURRENT_TIMESTAMP(), '%s', '%s', '%s', FROM_UNIXTIME(%d), '%s', '%d', '%d', '%d', '%s', 1, 0, 0)",
            fn.c_str(), uid.c_str(), catid.c_str(), companyKey, validdt, lwt.c_str(), minTeu, paytp, billtp, rmk.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    int freightId = psql->GetInsertId();

    for(int i=0; i<(int)prices->size(); i++)
    {
        TPrice price;
        sprintf(price.m_szLineID, prices->get(i)->getv("li", ""));
        price.m_nDeptPort = atoi(prices->get(i)->getv("lptid", ""));
        price.m_nDestPort = atoi(prices->get(i)->getv("dptid", ""));
        sprintf(price.m_szDeptPortName, prices->get(i)->getv("lptnm", ""));
        sprintf(price.m_szDestPortName, prices->get(i)->getv("dptnm", ""));
        price.m_nBoxType = prices->get(i)->getv("boxtp", 0);
        price.m_nDay = prices->get(i)->getv("day", 0);
        sprintf(price.m_szPrice, prices->get(i)->getv("price", ""));
        sprintf(price.m_szCarrierID, prices->get(i)->getv("ci", ""));
        sprintf(price.m_szCarrierName, prices->get(i)->getv("cn", ""));

        sprintf(sql, "insert into t30_price(FREIGHT_ID, CARRIER_SRV_ID, CARRIER_ID, DEPT_PORT, DEPT_PORTNAME, DEST_PORT, DEST_PORTNAME, BOX_TYPE, WEEKDAYS, PRICE) values ('%d', '%s', '%s', '%d', '%s', '%d', '%s', '%d', '%d', '%s')",
                freightId, price.m_szLineID, price.m_szCarrierID, price.m_nDeptPort, price.m_szDeptPortName, price.m_nDestPort, price.m_szDestPortName, price.m_nBoxType, price.m_nDay, price.m_szPrice);
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }

    out << "{fi:\"" << freightId << "\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int ScheduleSvc::freightMyCurr(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::freightMyCurr]bad format:", jsonString, 1);

    string fi = root.getv("fi", "");
    if(fi.empty())
		return 1;

    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    TFreight freight;

    sprintf(sql, "SELECT freight_id, freight_name, category_id, UNIX_TIMESTAMP(expire_dt) AS validdt, PAY_DEMAND, BL_DEMAND, LIMIT_MT, MIN_TEU, REMARK  FROM t30_myfreight WHERE freight_id = '%s'", fi.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	
	if(psql->NextRow())
	{
		READMYSQL_STR(freight_id, freight.m_szFreightID);
		READMYSQL_STR(freight_name, freight.m_szFreightName);
		READMYSQL_STR(category_id, freight.m_szFreightCatID);
		READMYSQL_64(validdt, freight.m_lValidTime, 0L);
		READMYSQL_INT(pay_demand, freight.m_nPayType, 0);
		READMYSQL_INT(bl_demand, freight.m_nBillType, 0);
		READMYSQL_STR(limit_mt, freight.m_szLimitMt);
		READMYSQL_INT(min_teu, freight.m_nMinTeu, 0);
		READMYSQL_STR(remark, freight.m_szRemark);

		sprintf(sql, "SELECT t1.price_id, t1.CARRIER_SRV_ID, t1.dept_port, t1.dest_port, t1.dept_portname, t1.dest_portname, t1.box_type, t1.weekdays, t1.price, t1.carrier_id, concat(t2.name_abbr_en,'-',t2.name_abbr_cn) as carrier_name \
					 FROM  t30_price t1, t41_carrier t2 WHERE t1.FREIGHT_ID = '%s' AND t1.CARRIER_ID = t2.CARRIER_ID", fi.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		
		while(psql->NextRow())
		{
			TPrice price;
			READMYSQL_STR(price_id, price.m_szPriceID);
			READMYSQL_STR(carrier_srv_id, price.m_szLineID);
			READMYSQL_INT(dept_port, price.m_nDeptPort, 0);
			READMYSQL_INT(dest_port, price.m_nDestPort, 0);
			READMYSQL_STR(dept_portname, price.m_szDeptPortName);
			READMYSQL_STR(dest_portname, price.m_szDestPortName);
			READMYSQL_INT(box_type, price.m_nBoxType, 0);
			READMYSQL_INT(weekdays, price.m_nDay, 0);
			READMYSQL_STR(price, price.m_szPrice);
			READMYSQL_STR(carrier_id, price.m_szCarrierID);
			READMYSQL_STR(carrier_name, price.m_szCarrierName);
			freight.m_vecPrice.push_back(price);
		}
	}
	else
	{
		RELEASE_MYSQL_RETURN(psql, 2);
	}

    out << freight.toJson();
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{fi:"",fn:"",validdt:1232323,paytp:0,bill:0,limitwt:"", minnum:1,des:""}
int ScheduleSvc::freightUpdBase(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::freightUpdBase]bad format:", jsonString, 1);

    string fi = root.getv("fi", "");
    string fn = root.getv("fn", "");
    int    dt = root.getv("validdt", 0);
    int    pt = root.getv("paytp", 0);
    int    bt = root.getv("bill", 0);
    string lt = root.getv("limitwt", "");
    int    mm = root.getv("minnum", 0);
    string rmk = root.getv("des", "");

    if(fi.empty())
		return 1;

    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "UPDATE t30_myfreight SET freight_name = '%s', expire_dt = FROM_UNIXTIME('%d'), PAY_DEMAND = %d, BL_DEMAND = %d, LIMIT_MT = '%s', MIN_TEU = %d, REMARK = '%s' WHERE freight_id = '%s'",
            fn.c_str(), dt, pt, bt, lt.c_str(), mm, rmk.c_str(), fi.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int ScheduleSvc::freightUpdPrice(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::freightUpdPrice]bad format:", jsonString, 1);

    char sqlDelete[2048] = {'\0'};
    char sqlUpdate[2048] = {'\0'};
    char sqlInsert[2048] = {'\0'};

    string fi = root.getv("fi", "");
    Json* prices = root["fare"];

    MySql* psql = CREATE_MYSQL;
    for(int i=0; i<(int)prices->size(); i++)
    {
        TPrice price;
        sprintf(price.m_szPriceID, prices->get(i)->getv("pid", ""));
        sprintf(price.m_szLineID, prices->get(i)->getv("li", ""));
        price.m_nDeptPort = atoi(prices->get(i)->getv("lptId", ""));
        price.m_nDestPort = atoi(prices->get(i)->getv("dptId", ""));
        sprintf(price.m_szDeptPortName, prices->get(i)->getv("lptnm", ""));
        sprintf(price.m_szDestPortName, prices->get(i)->getv("dptnm", ""));
        price.m_nBoxType = prices->get(i)->getv("boxtp", 0);
        price.m_nDay = prices->get(i)->getv("day", 0);
        sprintf(price.m_szPrice, prices->get(i)->getv("price", ""));
        sprintf(price.m_szCarrierID, prices->get(i)->getv("ci", ""));
        sprintf(price.m_szCarrierName, prices->get(i)->getv("cn", ""));

        if(strlen(price.m_szPriceID)) // update
        {
            sprintf(sqlUpdate, "update t30_price set CARRIER_SRV_ID = '%s', CARRIER_ID = '%s', DEPT_PORT = '%d', DEPT_PORTNAME = '%s', DEST_PORT = '%d', DEST_PORTNAME = '%s', BOX_TYPE = '%d', WEEKDAYS = '%d', PRICE = '%s' where price_id = '%s'",
                    price.m_szLineID, price.m_szCarrierID, price.m_nDeptPort, price.m_szDeptPortName, price.m_nDestPort, price.m_szDestPortName, price.m_nBoxType, price.m_nDay, price.m_szPrice, price.m_szPriceID);
            CHECK_MYSQL_STATUS(psql->Execute(sqlUpdate)>=0, 3);
        }
        else						  // insert
        {
            sprintf(sqlInsert, "insert into t30_price(FREIGHT_ID, CARRIER_SRV_ID, CARRIER_ID, DEPT_PORT, DEPT_PORTNAME, DEST_PORT, DEST_PORTNAME, BOX_TYPE, WEEKDAYS, PRICE) values ('%s', '%s', '%s', '%d', '%s', '%d', '%s', '%d', '%d', '%s')",
                    fi.c_str(), price.m_szLineID, price.m_szCarrierID, price.m_nDeptPort, price.m_szDeptPortName, price.m_nDestPort, price.m_szDestPortName, price.m_nBoxType, price.m_nDay, price.m_szPrice);
            CHECK_MYSQL_STATUS(psql->Execute(sqlUpdate)>=0, 3);
        }
    }

    // delete
    sprintf(sqlDelete, "delete from t30_price where freight_id = '%s' and price_id in (", fi.c_str());
    Json* dels = root["del"];
    for(int i=0; i<dels->size(); i++)
    {
        if(i!=0)
            strcat(sqlDelete, ",");

        strcat(sqlDelete, dels->getv(i, ""));
    }
    strcat(sqlDelete, ")");
    if(dels->size() > 0)
    {
        CHECK_MYSQL_STATUS(psql->Execute(sqlUpdate)>=0, 3);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{ci:"01",cn:"new Freight" ,type:0}
int ScheduleSvc::freightMyMgr(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::freightMyMgr]bad format:", jsonString, 1);

    string ci = root.getv("ci", "");
    string cn = SqlReplace(root.getv("cn", ""));
    int    tp = root.getv("type", 0);

    string uid(pUid);

    if(ci.empty())
		return 1;

    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    switch(tp)
    {
    case 0:
    {
        sprintf(sql, "insert into T30_myfreight_category(user_id, categoryid, categoryname, update_dt, create_dt, trash_flag) values ('%s', '%s', '%s', CURRENT_TIMESTAMP(), CURRENT_TIMESTAMP(), 0)",
                uid.c_str(), ci.c_str(), cn.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>0, 3);
    }
    break;

    case 1:
    {
        sprintf(sql, "update T30_myfreight_category set TRASH_FLAG='1', update_dt = CURRENT_TIMESTAMP() where CATEGORYID='%s' and user_id='%s'", ci.c_str(), uid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

        sprintf(sql, "Update T30_myfreight set TRASH_FLAG='1' where CATEGORY_ID='%s' and user_id='%s'", ci.c_str(), uid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    break;

    case 2:
    {
        sprintf(sql, "delete from T30_myfreight_category where CATEGORYID='%s' and user_id='%s'", ci.c_str(), uid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

        sprintf(sql, "delete t1 from  T30_price t1, T30_myfreight t2 where t2.CATEGORY_ID='%s' and t2.user_id='%s' and t1.FREIGHT_ID = t2.FREIGHT_ID", ci.c_str(), uid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

        sprintf(sql, "delete  from  T30_myfreight  where CATEGORY_ID='%s' and user_id='%s'", ci.c_str(), uid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    break;

    case 3:
    {
        sprintf(sql, "update T30_myfreight_category set TRASH_FLAG='0', update_dt = CURRENT_TIMESTAMP() where CATEGORYID='%s' and user_id='%s'", ci.c_str(), uid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

        sprintf(sql, "Update T30_myfreight set TRASH_FLAG='0' where CATEGORY_ID='%s' and user_id='%s'", ci.c_str(), uid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    break;

    case 4:
    {
        sprintf(sql, "update T30_myfreight_category set CATEGORYNAME='%s', update_dt = CURRENT_TIMESTAMP() where CATEGORYID='%s' and user_id='%s'", cn.c_str(), ci.c_str(), uid.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    break;

    default:
        RELEASE_MYSQL_RETURN(psql, 2);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{fi:"", name:"", type:0 }
int ScheduleSvc::freightMgr(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::freightMgr]bad format:", jsonString, 1);

    string fi = root.getv("fi", "");
    string nm = root.getv("name", "");
    int    tp = root.getv("type", 0);

    if(fi.empty())
		return 1;

    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    switch(tp)
    {
    case 0:
    {
        sprintf(sql, "Update T30_myfreight set TRASH_FLAG='1' where FREIGHT_ID='%s'", fi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    break;

    case 1:
    {
        sprintf(sql, "Update T30_myfreight set TRASH_FLAG='0' where FREIGHT_ID='%s'", fi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    break;

    case 2:
    {
        sprintf(sql, "delete from T30_myfreight  where FREIGHT_ID='%s'", fi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

        sprintf(sql, "delete from T30_price where FREIGHT_ID='%s'", fi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    break;

    case 3:
    {
        sprintf(sql, "Update T30_myfreight set PAUSE_FLAG ='1' where FREIGHT_ID='%s'", fi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    break;

    case 4:
    {
        sprintf(sql, "Update T30_myfreight set PAUSE_FLAG ='0' where FREIGHT_ID='%s'", fi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
    break;

    case 5:
    {
        sprintf(sql, "Update T30_myfreight set FREIGHT_NAME ='%s' where FREIGHT_ID='%s'", nm.c_str(), fi.c_str());
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }
	break;

    default:
        RELEASE_MYSQL_RETURN(psql, 2);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{fi:"",oldCatId:"",newCatId:""}
int ScheduleSvc::freightChgCat(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::freightChgCat]bad format:", jsonString, 1);

    string fi = root.getv("fi", "");
    string oc = root.getv("oldCatId", "");
    string nc = root.getv("newCatId", "");

    if(fi.empty() || oc.empty() || nc.empty())
		return 1;

    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "Update  T30_myfreight set CATEGORY_ID='%s' where FREIGHT_ID='%s' and CATEGORY_ID='%s'", nc.c_str(), fi.c_str(), oc.c_str());
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{shipid:""}
int ScheduleSvc::voyCountForVsl(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[ScheduleSvc::voyCountForVsl]bad format:", jsonString, 1);

    std::string szShipID = root.getv("shipid", "");
    if(szShipID.empty())
		return 1;

    int Num = 0;
    MySql* psql = CREATE_MYSQL;
    char sql[1024];
    sprintf(sql, "SELECT COUNT(1) as Num \
				 FROM t30_voyage_sch_info \
				 WHERE shipid = '%s' AND DEPT_PTD >= SUBDATE(CURDATE(),INTERVAL %d MONTH) AND DEPT_PTD <= ADDDATE(CURDATE(),INTERVAL %d MONTH)",
				szShipID.c_str(), 2, 2);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    if(psql->NextRow())
        READMYSQL_INT(Num, Num, 0);

    out << "{shipid:\"" << szShipID << "\",num:\"" << Num << "\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

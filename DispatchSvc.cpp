#include "blmcom_head.h"
#include "DispatchSvc.h"
#include "DispatchCal.h"
#include "VesselSvc.h"
#include "MessageService.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include "json.h"

const char* PILOT_STATUS_DESC[] =
{
    "未知", "草稿", "待派", "待引", "开始引航", "引航结束", "取消引航"
};

const char* SL_STATUS_DESC[] =
{
    "未受理", "同意受理", "不同意受理"
};

const char*  CJ_REGION = "121.6983,31.8785|121.33299,31.3334|116.0019,29.3927|113.3871,30.5632|114.1013,32.1114|120.2756,32.6033|121.6983,31.8785";
POLYGON g_CJPolygon = StringToPolygon(CJ_REGION);

IMPLEMENT_SERVICE_MAP(DispatchSvc)

DispatchSvc::DispatchSvc()
{
    m_szMaxVoyageID[0] = '\0';
}

DispatchSvc::~DispatchSvc()
{

}

bool DispatchSvc::Start()
{
    MySql* psql = CREATE_MYSQL;
    bool bRet =
        loadUserData(psql) &&
        loadPilotApplyData(psql) &&
        g_DispatchCal::instance()->Start() &&
        g_MessageService::instance()->RegisterCmd(MID_DISPATCH, this);

	/*
    uint32 interval = g_MainConfig::instance()->GetPilotFreshTime();
    if(interval <= 0)
        interval = 30*60;

    int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
    if(timerId <= 0)
    {
        RELEASE_MYSQL_RETURN(psql, false);
    }
	*/

    SERVICE_MAP(SID_PILOT_APPLY,DispatchSvc,getPilotApply);
    SERVICE_MAP(SID_PILOT_APPLY_DETAIL,DispatchSvc,getPilotApplyDetail);
    SERVICE_MAP(SID_PILOT_MAN,DispatchSvc,getPilotMan);
    SERVICE_MAP(SID_PILOT_VOY,DispatchSvc,getPilotVoy);
    SERVICE_MAP(SID_PILOT_CHG_SQ,DispatchSvc,chgPilotStatus);
    SERVICE_MAP(SID_PILOT_CHG_SL,DispatchSvc,chgPilotSL);
    SERVICE_MAP(SID_PILOT_CHG_ZJ,DispatchSvc,chgPilotZJ);
    SERVICE_MAP(SID_PILOT_CHG_SP,DispatchSvc,chgPilotSP);
    SERVICE_MAP(SID_PILOT_STARTTIME,DispatchSvc,chgPilotStartTime);
    SERVICE_MAP(SID_PILOT_DISPATCH,DispatchSvc,dispatchPilotMan);
    SERVICE_MAP(SID_PILOT_MAN_VSL,DispatchSvc,getPilotManVsl);
    SERVICE_MAP(SID_PILOT_VSL_DIST,DispatchSvc,getPilotVslDist);
    SERVICE_MAP(SID_PILOT_MAN_STATION,DispatchSvc,getPilotManStation);
    SERVICE_MAP(SID_PILOT_PLAN,DispatchSvc,getPilotPlan);
    SERVICE_MAP(SID_PILOT_MAN_DELETE,DispatchSvc,delPilotMan);
    SERVICE_MAP(SID_PILOT_GET_LOG,DispatchSvc,getPilotLog);
    SERVICE_MAP(SID_PILOT_GET_RMK,DispatchSvc,getPilotRmk);
    SERVICE_MAP(SID_PILOT_UPD_RMK,DispatchSvc,updPilotRmk);
    SERVICE_MAP(SID_PILOT_DEL_RMK,DispatchSvc,delPilotRmk);
    SERVICE_MAP(SID_PILOTED_MAN,DispatchSvc,getPilotedMan);
    SERVICE_MAP(SID_PILOT_MAN_CONTACT,DispatchSvc,getPilotManContact);
    SERVICE_MAP(SID_PILOT_STATUS,DispatchSvc,getPilotStatus);

    DEBUG_LOG("[DispatchSvc::Start] OK......................................");
    RELEASE_MYSQL_RETURN(psql, true);
}

int DispatchSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
    MySql* psql = CREATE_MYSQL;

    SYSTEM_LOG("[DispatchSvc::loadUserData] begin ======================");
    loadUserData(psql);
    SYSTEM_LOG("[DispatchSvc::loadUserData] end ======================");

    SYSTEM_LOG("[DispatchSvc::loadPilotApplyData] begin ======================");
    loadPilotApplyData(psql);
    SYSTEM_LOG("[DispatchSvc::loadPilotApplyData] end ======================");

    RELEASE_MYSQL_RETURN(psql, 0);

}

bool DispatchSvc::loadUserData(MySql* psql)
{
    static bool bIsFirst = true;

    char sql[1024];
    if(bIsFirst)
    {
        bIsFirst = false;

        sprintf(sql, "SELECT t1.User_id, IFNULL(t4.REALNAME, concat(t4.LastName,t4.FirstName)) as UserName, t4.TELNO, t4.EMAIL, t1.Auth, t5.LineDes, t2.PilotLevel, t2.Position, t2.Grade, t2.CurStatus, UNIX_TIMESTAMP(t2.Available_Dt) as AvailableTime, t3.STATIONID, t1.OrgID, t3.ORGTYPE \
			FROM t45_pilot_cj_org_man t1 \
			LEFT JOIN t45_pilot_cj_man_cn t2 ON t1.User_id = t2.UserID \
			LEFT JOIN t45_pilot_cn_cj_org t3 ON t1.OrgID = t3.ORGID \
			LEFT JOIN t00_user t4 ON t1.User_id = t4.USER_ID \
			LEFT JOIN t91_pilot_cj_line_scope t5 ON t2.LineID = t5.LineID");


        if(psql->Query(sql))
        {
            while(psql->NextRow())
            {
                CjUserPilot* pInfo = new CjUserPilot();
                READMYSQL_STR(User_id, pInfo->m_szUserID);
                READMYSQL_STR(UserName, pInfo->m_szName);
                READMYSQL_STR(TELNO, pInfo->m_szTel);
                READMYSQL_STR(EMAIL, pInfo->m_szEmail);
                READMYSQL_INT(Auth, pInfo->m_nAuth, 0);
                READMYSQL_STR(LineDes, pInfo->m_szVoyageLine);
                READMYSQL_STR(PilotLevel, pInfo->m_szPilotLevel);
                pInfo->m_nPilotLevel = pInfo->m_szPilotLevel[0];
                READMYSQL_INT(Position, pInfo->m_nPosition, 7);
                READMYSQL_STR(Grade, pInfo->m_szGrade);
                READMYSQL_INT(CurStatus, pInfo->m_nStatus, 1);
                READMYSQL_64(AvailableTime, pInfo->m_lAvaibleTime, (long)time(NULL));
                READMYSQL_STR(STATIONID, pInfo->m_szPilotID);
                READMYSQL_STR(OrgID, pInfo->m_szOrgID);
                READMYSQL_INT(ORGTYPE, pInfo->m_nOrgType, 6);

                if(m_mapUserPilot.find(pInfo->m_szUserID) != m_mapUserPilot.end())
                {
                    delete pInfo;
                    continue;
                }

                m_mapUserPilot.insert(std::make_pair(pInfo->m_szUserID, pInfo));
            }
        }
        else
            return false;

        sprintf(sql, "SELECT PortID, StationID FROM t45_pilot_station_port");
        if(psql->Query(sql))
        {
            while(psql->NextRow())
            {
                CjPortPilot* pInfo = new CjPortPilot();
                READMYSQL_INT(PortID, pInfo->m_nPortID, 0);
                READMYSQL_STR(StationID, pInfo->m_szPilotID);

                if(pInfo->m_nPortID != 0)
                    m_mapPortPilot.insert(std::make_pair(pInfo->m_nPortID, pInfo));
                else
                    delete pInfo;
            }
        }
        else
            return false;
    }
    else
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

        sprintf(sql, "SELECT UserID, CurStatus FROM t45_pilot_cj_man_cn");
        if(psql->Query(sql))
        {
            char userid[64];
            int  status;

            while(psql->NextRow())
            {
                READMYSQL_STR(UserID, userid);
                READMYSQL_INT(CurStatus, status, 0);
                CjUserPilot* pUser = GetUserInfo(userid);
                if(!pUser)
                    continue;

                if(pUser->m_nStatus >= 3 && pUser->m_nStatus <= 5 && pUser->m_nStatus != status)
                {
                    pUser->m_nStatus = status;
                    pUser->m_lAvaibleTime = (long)time(NULL);
                }
            }
        }
    }

    return true;
}

bool DispatchSvc::loadPilotApplyData(MySql* psql)
{
    static bool bIsFirst = true;
    char sql[2*1024] = {'\0'};
    sprintf(sql, "SELECT \
		t1.VOYAGE_ID, t1.VoyStatus, t1.DeptPilot, t9.Name as DeptPilotName, t1.DeptPort, t2.LONGITUDE_dd as lon1, t2.LATITUDE_dd as lat1, t1.DestPilot, t10.Name as DestPilotName, t1.DestPort, t3.LONGITUDE_dd as lon2, t3.LATITUDE_dd as lat2, t1.Miles, \
		t1.ApplyAgent, t1.Applicant, t5.OWNER, t5.MANAGER, t5.BUILDER, \
		t4.ShipID, t4.Name_en, t4.Name_cn, t4.MMSI, t5.IMO, t4.Callsign, t4.Flag, t5.SPEED, \
		t7.LOA, t7.MAXBEAM, t7.DEPTH, t4.Draught_For, t4.Draught_Aft, t4.Height, t4.Age, \
		t6.GROSSTON, t6.NETTON, t6.DWT, t6.LOADEDDISPLACEMENT, \
		UNIX_TIMESTAMP(t1.ApplyDt) as appltdt, UNIX_TIMESTAMP(t1.APTD) as aptd, UNIX_TIMESTAMP(t1.ETD) as etd, UNIX_TIMESTAMP(t1.ATD) as atd, UNIX_TIMESTAMP(t1.ETA) as eta, UNIX_TIMESTAMP(t1.ATA) as ata, t1.Pilot_Time, \
		t1.Phase, t1.Pilot_Type, t1.Voy_Type, \
		t1.DangerLevel, t1.IsSpec, t8.CargoName, t8.Quantity, \
		UNIX_TIMESTAMP(t1.SurveyDt) as SurveyDt, t1.SurveyStatus, t1.SurveyOpinion, t1.Transactor, t1.Summary, \
		UNIX_TIMESTAMP(t1.LeaderApprDt) as LeaderApprDt, t1.LeaderApprStatus, t1.LeaderApprOpinion, t1.LeaderApprover, \
		UNIX_TIMESTAMP(t1.TechApprDt) as TechApprDt, t1.TechApprStatus, t1.TechApprOpinion, t1.TechApprover, \
		UNIX_TIMESTAMP(t1.SchApprDt) as SchApprDt, t1.SchApprStatus, t1.SchApprOpinion, t1.SchApprover, \
		UNIX_TIMESTAMP(t1.Upd_Dt) as UpdateDt \
		FROM t45_pilot_voy_info t1 \
		LEFT JOIN t41_port t2 ON t1.DeptPort = t2.PortID \
		LEFT JOIN t41_port t3 ON t1.DestPort = t3.PortID \
		LEFT JOIN t45_pilot_voy_ship t4 ON t1.VOYAGE_ID = t4.VoyageID \
		LEFT JOIN t41_ship t5 ON t4.ShipID = t5.SHIPID \
		LEFT JOIN t41_ship_tonnage t6 ON t5.SHIPID = t6.SHIPID \
		LEFT JOIN t41_ship_dimension t7 ON t5.SHIPID = t7.SHIPID \
		LEFT JOIN t45_pilot_cj_cargo t8 ON t1.VOYAGE_ID = t8.VoyageID \
		LEFT JOIN t45_pilot_cj_point t9 ON t1.DeptPilot = t9.PointID \
		LEFT JOIN t45_pilot_cj_point t10 ON t1.DestPilot = t10.PointID ");

    if(bIsFirst)
    {
        bIsFirst = false;
        strcat(sql, "WHERE t1.SurveyStatus in (0,1) and (t1.VoyStatus in (2,3,4) or (t1.VoyStatus = 5 and DATE(aptd) = CURDATE()))");
    }
    else
    {
        sprintf(sql+strlen(sql), "WHERE t1.Upd_dt > FROM_UNIXTIME(%ld)", m_lLastUpdTime);
    }

    VecPilot2 tempVector;
    VecPilot2::iterator iterTemp;
    int nEtlEnd = 0;
    int nExpired = 0;
    int nNewAdd = 0;

    //获取全部新的或者更新的引航申请
    {
        if(!psql->Query(sql))
            return false;

        CjPilotInfo pInfo;
        while(psql->NextRow())
        {
            char shipid[64];
            long updatedt;
            READMYSQL_STR(ShipID, shipid);
            READMYSQL_64(UpdateDt, updatedt, 0);
            if(shipid[0] == '\0')
                continue;

            CjPilotInfo pInfo;
            READMYSQL_STR(VOYAGE_ID, pInfo.m_szVoyageID);
            READMYSQL_INT(VoyStatus, pInfo.m_nStatus, 0);
            READMYSQL_STR(DeptPilot, pInfo.m_szDeptPilotID);
            READMYSQL_STR(DeptPilotName, pInfo.m_szDeptPilotName);
            READMYSQL_INT(DeptPort, pInfo.m_nDeptPortID, 0);
            READMYSQL_DOUBLE(lon1, pInfo.m_dbDeptLon, 0.0);
            READMYSQL_DOUBLE(lat1, pInfo.m_dbDeptLat, 0.0);
            READMYSQL_STR(DestPilot, pInfo.m_szDestPilotID);
            READMYSQL_STR(DestPilotName, pInfo.m_szDestPilotName);
            READMYSQL_INT(DestPort, pInfo.m_nDestPortID, 0);
            READMYSQL_DOUBLE(lon2, pInfo.m_dbDestLon, 0.0);
            READMYSQL_DOUBLE(lat2, pInfo.m_dbDestLat, 0.0);
            READMYSQL_INT(Miles, pInfo.m_nMiles, 0);

            READMYSQL_STR(ApplyAgent, pInfo.m_szApplyCmpID);
            READMYSQL_STR(Applicant, pInfo.m_szApplyUser);

            READMYSQL_STR(ShipID, pInfo.m_szShipID);
            READMYSQL_STR(Name_en, pInfo.m_szNameEn);
            READMYSQL_STR(Name_cn, pInfo.m_szNameCn);
            READMYSQL_STR(MMSI, pInfo.m_szMmsi);
            READMYSQL_STR(IMO, pInfo.m_szImo);
            READMYSQL_STR(Callsign, pInfo.m_szCallsign);
            READMYSQL_STR(Flag, pInfo.m_szFlag);
            READMYSQL_DOUBLE(SPEED, pInfo.m_dbSpd, 0.0);
            READMYSQL_DOUBLE(LOA, pInfo.m_dbLength, 0);
            READMYSQL_DOUBLE(MAXBEAM, pInfo.m_dbWidth, 0);
            READMYSQL_DOUBLE(DEPTH, pInfo.m_dbDepth, 0);
            READMYSQL_DOUBLE(Draught_For, pInfo.m_dbDraughtFor, 0);
            READMYSQL_DOUBLE(Draught_Aft, pInfo.m_dbDraughAfter, 0);
            READMYSQL_DOUBLE(Height, pInfo.m_dbHeight, 0);
            READMYSQL_INT(Age, pInfo.m_nAge, 0);
            READMYSQL_INT(GROSSTON, pInfo.m_nGrossTon, 0);
            READMYSQL_INT(NETTON, pInfo.m_nNetTon, 0);
            READMYSQL_INT(DWT, pInfo.m_nDWT, 0);
            READMYSQL_INT(LOADEDDISPLACEMENT, pInfo.m_nLoadedDisplament, 0);
            READMYSQL_STR(OWNER, pInfo.m_szOwner);
            READMYSQL_STR(MANAGER, pInfo.m_szManager);
            READMYSQL_STR(BUILDER, pInfo.m_szBuilder);

            long timeNow = (long)time(NULL);
            long timeNow2Days = timeNow + 2*24*3600;
            READMYSQL_64(appltdt, pInfo.m_lApply, timeNow);
            READMYSQL_64(aptd, pInfo.m_lAPDT, timeNow2Days);
            READMYSQL_64(etd, pInfo.m_lETD, 0L);
            READMYSQL_64(atd, pInfo.m_lATD, 0L);
            READMYSQL_64(eta, pInfo.m_lETA, 0L);
            READMYSQL_64(ata, pInfo.m_lATA, 0L);
            READMYSQL_INT(Pilot_Time, pInfo.m_nJT, 0);

            READMYSQL_INT(Phase, pInfo.m_nPhase, 1);
            READMYSQL_INT(Pilot_Type, pInfo.m_nPilotType, 1);
            READMYSQL_INT(Voy_Type, pInfo.m_nVoyType, 1);
            READMYSQL_INT(DangerLevel, pInfo.m_nDangerLevel, 4);
            READMYSQL_INT(IsSpec, pInfo.m_nIsSpec, 0);
            READMYSQL_STR(CargoName, pInfo.m_szCargoName);
            READMYSQL_INT(Quantity, pInfo.m_nCargoQuantity, 0);

            READMYSQL_STR(Transactor, pInfo.m_strAccepterID);
            READMYSQL_64(SurveyDt, pInfo.m_lAcceptTime, 0L);
            READMYSQL_INT(SurveyStatus, pInfo.m_nAgree, 0);
            READMYSQL_STR(SurveyOpinion, pInfo.m_strAgreement);

            READMYSQL_STR(Summary, pInfo.m_strSummary);
            READMYSQL_INT(IsSpec, pInfo.m_nAudit, 0);

            READMYSQL_STR(LeaderArrover, pInfo.m_strID_zhan);
            READMYSQL_64(LeaderApprDt, pInfo.m_lTime_zhan, 0L);
            READMYSQL_INT(LeaderApprStatus, pInfo.m_nAudit_zhan, 0);
            READMYSQL_STR(LeaderApprOpinion, pInfo.m_strAudit_zhan);

            READMYSQL_STR(TechArrover, pInfo.m_strID_anjibu);
            READMYSQL_64(TechApprDt, pInfo.m_lTime_anjibu, 0L);
            READMYSQL_INT(TechApprStatus, pInfo.m_nAudit_anjibu, 0);
            READMYSQL_STR(TechApprOpinion, pInfo.m_strAudit_anjibu);

            READMYSQL_STR(SchArrover, pInfo.m_strID_zongdiaoshi);
            READMYSQL_64(SchApprDt, pInfo.m_lTime_zongdiaoshi, 0L);
            READMYSQL_INT(SchApprStatus, pInfo.m_nAudit_zongdiaoshi, 0);
            READMYSQL_STR(SchApprOpinion, pInfo.m_strAudit_zongdiaoshi);

            if(strcmp(pInfo.m_szVoyageID, m_szMaxVoyageID) > 0)
                strcpy(m_szMaxVoyageID, pInfo.m_szVoyageID);
            if(updatedt > m_lLastUpdTime)
                m_lLastUpdTime = updatedt;

            MapPortPilot::iterator iter = m_mapPortPilot.find(pInfo.m_nDeptPortID);
            if(iter != m_mapPortPilot.end())
                sprintf(pInfo.m_szDeptStationID, iter->second->m_szPilotID);
            else
                pInfo.m_szDeptStationID[0] = '\0';

            iter = m_mapPortPilot.find(pInfo.m_nDestPortID);
            if(iter != m_mapPortPilot.end())
                sprintf(pInfo.m_szDestStationID, iter->second->m_szPilotID);
            else
                pInfo.m_szDestStationID[0] = '\0';

            sprintf(pInfo.m_szStationID, (pInfo.m_nPilotType==2?pInfo.m_szDeptStationID:pInfo.m_szDestStationID));
            tempVector.push_back(pInfo);
        }
    }

    //更新旧的引航申请
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

        for(iterTemp = tempVector.begin(); iterTemp != tempVector.end(); )
        {
            CjPilotInfo& pInfo = *iterTemp;

            MapPilot::iterator iterPilot = m_mapPilot.find(pInfo.m_szVoyageID);
            if(iterPilot != m_mapPilot.end())
            {
                iterPilot->second.m_nStatus = pInfo.m_nStatus;
                if(iterPilot->second.m_nStatus == 5)
                {
                    makePilotEnd(&(iterPilot->second), psql);
                    nEtlEnd++;
                }
                else
                {
                    if(pInfo.m_lAPDT != 0) iterPilot->second.m_lAPDT = pInfo.m_lAPDT;
                    if(pInfo.m_lETD != 0) iterPilot->second.m_lETD = pInfo.m_lETD;
                    if(pInfo.m_lATD != 0) iterPilot->second.m_lATD = pInfo.m_lATD;
                    if(pInfo.m_lETA != 0) iterPilot->second.m_lETA = pInfo.m_lETA;
                    if(pInfo.m_lATA != 0) iterPilot->second.m_lATA = pInfo.m_lATA;
                }
                iterTemp = tempVector.erase(iterTemp);
                continue;
            }
            else
            {
                if(pInfo.m_nStatus > 5)
                    iterTemp = tempVector.erase(iterTemp);
                else
                    iterTemp++;
            }
        }
    }

    //更新新的引航申请分段信息
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

        for(iterTemp = tempVector.begin(); iterTemp != tempVector.end(); )
        {
            CjPilotInfo* pInfo = &(*iterTemp);
            if(pInfo->m_nStatus == 5)
            {
                iterTemp++;
                continue;
            }

            sprintf(sql, "SELECT UNIX_TIMESTAMP(eta) eta, UNIX_TIMESTAMP(ata) ata, miles, pilotor1, pilotor2, pilotor3 FROM t45_pilot_cj_section WHERE voyage_id = '%s' ORDER BY seq", pInfo->m_szVoyageID);
            if(!psql->Query(sql))
            {
                return false;
            }

            //对应的分段表记录不存在,则忽略这条引航申请
            if(psql->GetRowCount() == 0)
            {
                SYSTEM_LOG("[DispatchSvc::loadPilotApplyData] pilotid:%s, sections not exists!", pInfo->m_szVoyageID);
                iterTemp = tempVector.erase(iterTemp);
                continue;
            }
            else
            {
                iterTemp++;
            }

            char pilotor[64];
            if(psql->NextRow())
            {
                READMYSQL_64(eta, pInfo->m_lEDC, 0L);
                READMYSQL_64(ata, pInfo->m_lADC, 0L);
                READMYSQL_INT(miles, pInfo->m_nMiles1, 0);
                READMYSQL_STR(pilotor1, pilotor);
                parsePilots(pilotor, pInfo, 0, 1);
                READMYSQL_STR(pilotor2, pilotor);
                parsePilots(pilotor, pInfo, 0, 2);
                READMYSQL_STR(pilotor3, pilotor);
                parsePilots(pilotor, pInfo, 0, 3);
            }
            if(psql->NextRow())
            {
                READMYSQL_INT(miles, pInfo->m_nMiles2, 0);
                READMYSQL_STR(pilotor1, pilotor);
                parsePilots(pilotor, pInfo, 1, 1);
                READMYSQL_STR(pilotor2, pilotor);
                parsePilots(pilotor, pInfo, 1, 2);
                READMYSQL_STR(pilotor3, pilotor);
                parsePilots(pilotor, pInfo, 1, 3);
            }
        }
    }

    //更新新的引航申请联系人信息
    {
        for(iterTemp = tempVector.begin(); iterTemp != tempVector.end(); iterTemp++)
        {
            CjPilotInfo* pInfo = &(*iterTemp);
            Tokens cmpyList;

            cmpyList.push_back(pInfo->m_szApplyCmpID);
            cmpyList.push_back(pInfo->m_szOwner);
            cmpyList.push_back(pInfo->m_szManager);
            cmpyList.push_back(pInfo->m_szBuilder);

            std::string inCondition;
            for(int i=0; i<(int)cmpyList.size(); i++)
            {
                if(i==0)
                    inCondition += "'" + cmpyList[i] + "'";
                else
                    inCondition += ",'" + cmpyList[i] + "'";
            }

            sprintf(sql, "SELECT COMPANY_KEY, NAME, TELNO, EMAIL FROM t41_company WHERE COMPANY_KEY IN (%s)", inCondition.c_str());
            if(psql->Query(sql))
            {
                char company_key[128];
                char name[128];
                char telno[128];
                char email[128];
                while(psql->NextRow())
                {
                    READMYSQL_STR(company_key, company_key);
                    READMYSQL_STR(name, name);
                    READMYSQL_STR(telno, telno);
                    READMYSQL_STR(email, email);
                    if(!strcmp(company_key, cmpyList[0].c_str()))
                    {
                        sprintf(pInfo->m_szApplyCmpName, name);
                        pInfo->m_VslContacts[0].m_nContactCatID = 1;
                        sprintf(pInfo->m_VslContacts[0].m_strContactCatID, "PROXY");
                        sprintf(pInfo->m_VslContacts[0].m_strContactUserID, company_key);
                        sprintf(pInfo->m_VslContacts[0].m_strContactName, name);
                        sprintf(pInfo->m_VslContacts[0].m_strContactTel, telno);
                        sprintf(pInfo->m_VslContacts[0].m_strContactEmail, email);
                    }
                    else if(!strcmp(company_key, cmpyList[1].c_str()))
                    {
                        pInfo->m_VslContacts[1].m_nContactCatID = 3;
                        sprintf(pInfo->m_VslContacts[1].m_strContactCatID, "OWNER");
                        sprintf(pInfo->m_VslContacts[1].m_strContactUserID, company_key);
                        sprintf(pInfo->m_VslContacts[1].m_strContactName, name);
                        sprintf(pInfo->m_VslContacts[1].m_strContactTel, telno);
                        sprintf(pInfo->m_VslContacts[1].m_strContactEmail, email);
                    }
                    else if(!strcmp(company_key, cmpyList[2].c_str()))
                    {
                        pInfo->m_VslContacts[2].m_nContactCatID = 5;
                        sprintf(pInfo->m_VslContacts[2].m_strContactCatID, "MANAGER");
                        sprintf(pInfo->m_VslContacts[2].m_strContactUserID, company_key);
                        sprintf(pInfo->m_VslContacts[2].m_strContactName, name);
                        sprintf(pInfo->m_VslContacts[2].m_strContactTel, telno);
                        sprintf(pInfo->m_VslContacts[2].m_strContactEmail, email);
                    }
                    else if(!strcmp(company_key, cmpyList[3].c_str()))
                    {
                        pInfo->m_VslContacts[3].m_nContactCatID = 2;
                        sprintf(pInfo->m_VslContacts[3].m_strContactCatID, "BUILDER");
                        sprintf(pInfo->m_VslContacts[3].m_strContactUserID, company_key);
                        sprintf(pInfo->m_VslContacts[3].m_strContactName, name);
                        sprintf(pInfo->m_VslContacts[3].m_strContactTel, telno);
                        sprintf(pInfo->m_VslContacts[3].m_strContactEmail, email);
                    }
                }
            }
            else
            {
                return false;
            }
        }
    }

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);
    for(iterTemp = tempVector.begin(); iterTemp != tempVector.end(); iterTemp++)
        m_mapPilot.insert(std::make_pair(iterTemp->m_szVoyageID, *iterTemp));

    nNewAdd = tempVector.size();

    SYSTEM_LOG("[DispatchSvc::loadPilotApplyData] etl-end %d ships ======================", nEtlEnd);
    SYSTEM_LOG("[DispatchSvc::loadPilotApplyData] new add %d ships ======================", nNewAdd);

    return true;
}

void MapPilotToMmsi(MapPilot& mapPilots, MapMmsiPilot& mapMmsis)
{
    MapPilot::iterator iter = mapPilots.begin();
    for(; iter != mapPilots.end(); iter++)
    {
        if(iter->second.m_nStatus > 4 || iter->second.m_szMmsi[0] == '\0')
            continue;

        mapMmsis.insert(std::make_pair(atoi(iter->second.m_szMmsi), &(iter->second)));
    }
}

bool DispatchSvc::makePilotStart(CjPilotInfo* pVoy, MySql* psql)
{
    char sql[1024];
    sprintf(sql, "UPDATE t45_pilot_voy_info SET VoyStatus = 4, ATD = CURRENT_TIMESTAMP() WHERE VOYAGE_ID = '%s'", pVoy->m_szVoyageID);
    if(psql->Execute(sql) < 0)
        return false;

    pVoy->m_nStatus = 4;
    pVoy->m_lATD = (long)time(NULL);

    MapUserPilot::iterator iter2;
    VecUserStatus userStatusList;
    for(StrSetIter iter=pVoy->pilotorList1.begin(); iter!=pVoy->pilotorList1.end(); iter++)
    {
        if((iter2=m_mapUserPilot.find(*iter)) != m_mapUserPilot.end())
        {
            iter2->second->m_nStatus = 6;
            userStatusList.push_back(UserStatus(iter2->second));
        }
    }
    for(StrSetIter iter=pVoy->pilotorList2.begin(); iter!=pVoy->pilotorList2.end(); iter++)
    {
        if((iter2=m_mapUserPilot.find(*iter)) != m_mapUserPilot.end())
        {
            iter2->second->m_nStatus = 6;
            userStatusList.push_back(UserStatus(iter2->second));
        }
    }
    updatePilotorStatus(userStatusList, psql);
    return true;
}

bool DispatchSvc::makePilotEnd(CjPilotInfo* pVoy, MySql* psql)
{
    char sql[1024];
    sprintf(sql, "UPDATE t45_pilot_voy_info SET VoyStatus = 5, ATA = CURRENT_TIMESTAMP() WHERE VOYAGE_ID = '%s'", pVoy->m_szVoyageID);
    if(psql->Execute(sql) < 0)
        return false;

    pVoy->m_nStatus = 5;
    pVoy->m_lATA = (long)time(NULL);

    MapUserPilot::iterator iter2;
    VecUserStatus userStatusList;
    for(StrSetIter iter=pVoy->pilotorList1.begin(); iter!=pVoy->pilotorList1.end(); iter++)
    {
        if((iter2=m_mapUserPilot.find(*iter)) != m_mapUserPilot.end())
        {
            iter2->second->m_nStatus = 2;
            iter2->second->m_lAvaibleTime = (long)time(NULL) + (2*3600); //休息时间2小时
            memset(iter2->second->m_szVoyageID, 0, sizeof(iter2->second->m_szVoyageID));
            userStatusList.push_back(UserStatus(iter2->second));
        }
    }
    for(StrSetIter iter=pVoy->pilotorList2.begin(); iter!=pVoy->pilotorList2.end(); iter++)
    {
        if((iter2=m_mapUserPilot.find(*iter)) != m_mapUserPilot.end())
        {
            iter2->second->m_nStatus = 2;
            iter2->second->m_lAvaibleTime = (long)time(NULL) +(2*3600);  //休息时间2小时
            memset(iter2->second->m_szVoyageID, 0, sizeof(iter2->second->m_szVoyageID));
            userStatusList.push_back(UserStatus(iter2->second));
        }
    }
    updatePilotorStatus(userStatusList, psql);
    return true;
}

bool DispatchSvc::makePilotExpired(CjPilotInfo* pVoy, MySql* psql)
{
    char sql[1024];
    sprintf(sql, "UPDATE t45_pilot_voy_info SET VoyStatus = 7 WHERE VOYAGE_ID = '%s'", pVoy->m_szVoyageID);
    if(psql->Execute(sql) < 0)
        return false;

    pVoy->m_nStatus = 7;

    MapUserPilot::iterator iter2;
    VecUserStatus userStatusList;
    for(StrSetIter iter=pVoy->pilotorList1.begin(); iter!=pVoy->pilotorList1.end(); iter++)
    {
        if((iter2=m_mapUserPilot.find(*iter)) != m_mapUserPilot.end())
        {
            iter2->second->m_nStatus = 1;
            memset(iter2->second->m_szVoyageID, 0, sizeof(iter2->second->m_szVoyageID));
            userStatusList.push_back(UserStatus(iter2->second));
        }
    }
    for(StrSetIter iter=pVoy->pilotorList2.begin(); iter!=pVoy->pilotorList2.end(); iter++)
    {
        if((iter2=m_mapUserPilot.find(*iter)) != m_mapUserPilot.end())
        {
            iter2->second->m_nStatus = 1;
            memset(iter2->second->m_szVoyageID, 0, sizeof(iter2->second->m_szVoyageID));
            userStatusList.push_back(UserStatus(iter2->second));
        }
    }
    updatePilotorStatus(userStatusList, psql);
    return true;
}

bool DispatchSvc::makePilotCancel(CjPilotInfo* pVoy, MySql* psql, bool bReject, const std::string& rmk, CjUserPilot* pUser)
{
    char sql[1024];

    if(bReject)
    {
        sprintf(pVoy->m_strAccepterID, pUser->m_szUserID);
        pVoy->m_nStatus = 6;
        pVoy->m_nAgree = 2;
        pVoy->m_lAcceptTime = (long)time(NULL);
        sprintf(pVoy->m_strAgreement, "%s", rmk.c_str());
        sprintf(sql, "UPDATE t45_pilot_voy_info SET VoyStatus = 6, surveystatus = 2, surveydt = CURRENT_TIMESTAMP(), surveyopinion = '%s', Transactor = '%s' WHERE VOYAGE_ID = '%s'", rmk.c_str(), pUser->m_szUserID, pVoy->m_szVoyageID);
        if(psql->Execute(sql) < 0)
            return false;
        return true;
    }

    sprintf(sql, "UPDATE t45_pilot_voy_info SET VoyStatus = 6 WHERE VOYAGE_ID = '%s'", pVoy->m_szVoyageID);
    if(psql->Execute(sql) < 0)
        return false;

    pVoy->m_nStatus = 6;

    MapUserPilot::iterator iter2;
    VecUserStatus userStatusList;
    for(StrSetIter iter=pVoy->pilotorList1.begin(); iter!=pVoy->pilotorList1.end(); iter++)
    {
        if((iter2=m_mapUserPilot.find(*iter)) != m_mapUserPilot.end())
        {
            iter2->second->m_nStatus = 1;
            memset(iter2->second->m_szVoyageID, 0, sizeof(iter2->second->m_szVoyageID));
            userStatusList.push_back(UserStatus(iter2->second));
        }
    }
    for(StrSetIter iter=pVoy->pilotorList2.begin(); iter!=pVoy->pilotorList2.end(); iter++)
    {
        if((iter2=m_mapUserPilot.find(*iter)) != m_mapUserPilot.end())
        {
            iter2->second->m_nStatus = 1;
            memset(iter2->second->m_szVoyageID, 0, sizeof(iter2->second->m_szVoyageID));
            userStatusList.push_back(UserStatus(iter2->second));
        }
    }
    updatePilotorStatus(userStatusList, psql);
    return true;
}

void DispatchSvc::updatePilotorStatus(VecUserStatus& statusList, MySql* psql)
{
    char sql[100*1024];
    //更新引航员表的引航员状态
    for(int i=0; i<(int)statusList.size(); i++)
    {
        sprintf(sql, "UPDATE t45_pilot_cj_man_cn SET CurStatus = '%d', Available_Dt = FROM_UNIXTIME(%ld) WHERE UserID = '%s'",
                statusList[i].status, statusList[i].avaible, statusList[i].userid.c_str());
        psql->Execute(sql);
    }
}

void DispatchSvc::parsePilots(const char* pilots, CjPilotInfo* pVoy, int nSeq, int nType)
{
    if(!strlen(pilots))
        return;

    Tokens tokens = StrSplit(pilots, "|");
    MapUserPilot::iterator iter;
    for(int i=0; i<(int)tokens.size(); i++)
    {
        iter = m_mapUserPilot.find(tokens[i]);
        if(iter != m_mapUserPilot.end())
        {
            if(nSeq==0)
                pVoy->pilotorList1.insert(tokens[i]);
            else
                pVoy->pilotorList2.insert(tokens[i]);

            iter->second->m_nShiRen = nType;
            strcpy(iter->second->m_szVoyageID, pVoy->m_szVoyageID);
            if(pVoy->m_nStatus==2 || pVoy->m_nStatus==3)
                iter->second->m_nStatus = 7;
            else if(pVoy->m_nStatus==4)
                iter->second->m_nStatus = 6;
            else if(pVoy->m_nStatus >= 5)
                iter->second->m_nStatus = 1;
        }
    }
}

void DispatchSvc::parsePilots(const char* pilots, VecUserPilot& pilotList, std::vector<int>& levelList, int nShiRen)
{
    if(!strlen(pilots))
        return;

    Tokens tokens = StrSplit(pilots, "|");
    for(int i=0; i<(int)tokens.size(); i++)
    {
        CjUserPilot* pUser = GetUserInfo(tokens[i]);
        if(pUser)
        {
            pilotList.push_back(pUser);
            levelList.push_back(nShiRen);
        }
    }
}

//判断经度所处的区域 0:南京及以下 1:宁汉 2:汉上
int DispatchSvc::getLonPosition(double lon)
{
    if(lon > NANJING_LON)
        return 0;

    if(lon <= NANJING_LON && lon >= WUHAN_LON)
        return 1;

    return 2;
}

int DispatchSvc::getShiRen(CjUserPilot* pUser, CjPilotInfo* pVoy)
{
    double lon = (pVoy->m_nPilotType==2?pVoy->m_dbDeptLon:pVoy->m_dbDestLon);
    int nPosition = getLonPosition(lon);
    const char* szLevel = NULL;

    //危险品规则
    if(pVoy->m_nDangerLevel >= 1 && pVoy->m_nDangerLevel <= 3)
    {
        if(nPosition == 0)  	//南京以下
        {
            if(pVoy->m_nDangerLevel == 1 || pVoy->m_nDangerLevel == 2)
            {
                if(pVoy->m_dbLength >= 175)
                    szLevel = "A2";
                else if(pVoy->m_dbLength >= 145)
                    szLevel = "A3";
                else
                    szLevel = "B1";
            }
            else
            {
                if(pVoy->m_dbLength >= 175)
                    szLevel = "A2";
                else if(pVoy->m_dbLength >= 145)
                    szLevel = "A3";
                else if(pVoy->m_dbLength >= 110)
                    szLevel = "B1";
                else if(pVoy->m_dbLength >= 85)
                    szLevel = "B2";
                else
                    szLevel = "C1";
            }
        }
        else  				//南京以上
        {
            if(pVoy->m_nDangerLevel == 1 || pVoy->m_nDangerLevel == 2)
            {
                if(pVoy->m_dbLength >= 145)
                    szLevel = "A2";
                else
                    szLevel = "A3";
            }
            else
            {
                if(pVoy->m_dbLength >= 145)
                    szLevel = "A2";
                else if(pVoy->m_dbLength >= 110)
                    szLevel = "A3";
                else if(pVoy->m_dbLength >= 85)
                    szLevel = "B1";
                else
                    szLevel = "B2";
            }
        }
    }
    if(szLevel)
        goto _label_end;

    //适任规则
    if(pVoy->m_nPilotType == 3)  		//所有人都能移泊
    {
        return 1;
    }

    if(pUser->m_nPilotLevel >= 'F')  	//实习
    {
        return 3;
    }

    if(pVoy->m_nVoyType == 1) //内贸
    {
        if(pVoy->m_dbLength >= 200)
        {
            if(pVoy->m_dbDraughtFor >= 9.7)
                szLevel = "A3";
            else
                szLevel = "B1";
        }
        else if(pVoy->m_dbLength >= 175)
        {
            if(pVoy->m_dbDraughtFor >= 9.7)
                szLevel = "B1";
            else
                szLevel = "B2";

        }
        else if(pVoy->m_dbLength >= 150)
        {
            if(pVoy->m_dbDraughtFor >= 9.7)
                szLevel = "B2";
            else
                szLevel = "C1";
        }
        else if(pVoy->m_dbLength >= 125)
        {
            if(pVoy->m_dbDraughtFor >= 9)
                szLevel = "C1";
            else
                szLevel = "C2";
        }
        else if(pVoy->m_dbLength >= 110)
        {
            if(pVoy->m_dbDraughtFor >= 8)
                szLevel = "C2";
            else
                szLevel = "C3";
        }
        else
        {
            szLevel = "D3";
        }
    }
    else  	//外贸
    {
        switch(nPosition)
        {
        case 0:
            if(pVoy->m_dbLength >= 204.99)
            {
                if(pVoy->m_dbDraughtFor >= 9.69)
                    szLevel = "A2";
                else
                    szLevel = "A3";
            }
            else if(pVoy->m_dbLength >= 175)
            {
                if(pVoy->m_dbDraughtFor >= 9.69)
                    szLevel = "A3";
                else
                    szLevel = "B1";
            }
            else if(pVoy->m_dbLength >= 145)
            {
                if(pVoy->m_dbDraughtFor >= 9)
                    szLevel = "B1";
                else
                    szLevel = "B2";
            }
            else if(pVoy->m_dbLength >= 125)
            {
                if(pVoy->m_dbDraughtFor >= 9)
                    szLevel = "B2";
                else
                    szLevel = "C1";
            }
            else if(pVoy->m_dbLength >= 110)
            {
                if(pVoy->m_dbDraughtFor >= 8)
                    szLevel = "C1";
                else
                    szLevel = "C2";
            }
            else if(pVoy->m_dbLength >= 85)
            {
                if(pVoy->m_dbDraughtFor >= 7)
                    szLevel = "C2";
                else
                    szLevel = "D3";
            }
            else
            {
                szLevel = "D3";
            }
            break;
        case 1:
            if(pVoy->m_dbLength >= 175)
            {
                if(pVoy->m_dbDraughtFor >= 9.69)
                    szLevel = "A2";
                else
                    szLevel = "A3";
            }
            else if(pVoy->m_dbLength >= 145)
            {
                if(pVoy->m_dbDraughtFor >= 9)
                    szLevel = "A3";
                else
                    szLevel = "B1";
            }
            else if(pVoy->m_dbLength >= 125)
            {
                if(pVoy->m_dbDraughtFor >= 9)
                    szLevel = "B1";
                else
                    szLevel = "B2";
            }
            else if(pVoy->m_dbLength >= 110)
            {
                if(pVoy->m_dbDraughtFor >= 8)
                    szLevel = "B2";
                else
                    szLevel = "C1";
            }
            else if(pVoy->m_dbLength >= 85)
            {
                if(pVoy->m_dbDraughtFor >= 7)
                    szLevel = "C1";
                else
                    szLevel = "C2";
            }
            else if(pVoy->m_dbLength >= 70)
            {
                if(pVoy->m_dbDraughtFor >= 6)
                    szLevel = "C2";
                else
                    szLevel = "D3";
            }
            else
            {
                szLevel = "D3";
            }
            break;
        case 2:
            if(pVoy->m_dbLength >= 125)
            {
                if(pVoy->m_dbDraughtFor >= 9)
                    szLevel = "A2";
                else
                    szLevel = "A3";
            }
            else if(pVoy->m_dbLength >= 110)
            {
                if(pVoy->m_dbDraughtFor >= 8)
                    szLevel = "A3";
                else
                    szLevel = "B1";
            }
            else if(pVoy->m_dbLength >= 85)
            {
                if(pVoy->m_dbDraughtFor >= 7)
                    szLevel = "B1";
                else
                    szLevel = "B2";
            }
            else if(pVoy->m_dbLength >= 70)
            {
                if(pVoy->m_dbDraughtFor >= 6)
                    szLevel = "B2";
                else
                    szLevel = "C1";
            }
            else
            {
                szLevel = "C2";
            }
            break;
        default:
            szLevel = "D3";
            break;
        }
    }

_label_end:
    if(strcmp(pUser->m_szGrade, szLevel) <= 0)
        return 1;

    return 2;
}


std::string TDispLog::toJson()
{
    CjUserPilot* pUser = g_DispatchSvc::instance()->GetUserInfo(m_strOperator);

    char buff[3*1024];
    sprintf(buff, "{userid:\"%s\",tm:%s,content:\"%s\",una:\"%s\"}", m_strOperator, m_strOperateDt, m_strOperateContent, pUser?pUser->m_szName:"");
    return std::string(buff);
}

std::string TDispRmk::toJson()
{
    CjUserPilot* pUser = g_DispatchSvc::instance()->GetUserInfo(m_strUserID);

    char buff[3*1024];
    sprintf(buff, "{rid:\"%s\",uid:\"%s\",tm:%ld,title:\"%s\",content:\"%s\",una:\"%s\"}", m_strRmkID, m_strUserID, m_lUpdTime, m_strTitle, m_strContent, pUser?pUser->m_szName:"");
    return buff;
}

std::string CjPilotInfo::toPilotApplyJson()
{
    char buff[5*1024];
    sprintf(buff, "[\"%s\",\"%d\",\"%d\",\"%d\",\"%s\",\"%s\",\"%s\",\"%s\",\"%ld\",\"%s\",\"%s\",\"%d\",\"%d\"]",
            m_szVoyageID, m_nStatus, m_nPilotType, m_nVoyType,
            m_szMmsi, m_szNameCn,
            m_szDeptPilotName, m_szDestPilotName, m_lAPDT,
            m_szApplyCmpName, m_szStationID, m_nDangerLevel, m_nIsSpec);

    return buff;
}

std::string CjPilotInfo::toDetailPilotApplyJson()
{
    char buff[5*1024] = {'\0'};

    strcat(buff, "{");

    //联系人信息
    char buff2[3*1024];
    sprintf(buff2, "Contact:{pid:\"%s\",pna:\"%s\",mid:\"%s\",mna:\"%s\",othercontacts:[", m_szApplyCmpID, m_szApplyCmpName, m_VslContacts[1].m_strContactUserID, m_VslContacts[1].m_strContactName);

    bool bFirst = true;
    for(int i=0; i<4; i++)
    {
        if(strlen(m_VslContacts[i].m_strContactCatID))
        {
            if(bFirst)
            {
                strcat(buff2, m_VslContacts[i].toJson().c_str());
                bFirst = false;
            }
            else
            {
                strcat(buff2, ",");
                strcat(buff2, m_VslContacts[i].toJson().c_str());
            }
        }
    }

    strcat(buff2, "]},");
    strcat(buff, buff2);
    memset(buff2, 0, sizeof(buff2));

    //船舶信息
    sprintf(buff2, "Vsl:[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"]",
            m_szNameCn, m_szImo, m_szMmsi, m_szCallsign, m_szFlag, m_dbSpd,
            m_dbLength, m_dbWidth, m_dbDepth,
            m_dbDraughtFor, m_dbDraughAfter, m_dbHeight,
            m_nGrossTon, m_nNetTon, m_nDWT, m_nLoadedDisplament,
            m_szCargoName, m_nCargoQuantity, m_nDangerLevel, m_nCargoQuantity, m_nAge, 0);
    strcat(buff2, ",");
    strcat(buff, buff2);
    memset(buff2, 0, sizeof(buff2));

    //受理信息
    CjUserPilot* pUser = g_DispatchSvc::instance()->GetUserInfo(m_strAccepterID);
    sprintf(buff2, "Accept:[\"%s\",\"%s\",\"%ld\",\"%d\",\"%s\"]", m_strAccepterID, pUser?pUser->m_szName:"", m_lAcceptTime, m_nAgree, m_strAgreement);
    strcat(buff2, ",");
    strcat(buff, buff2);
    memset(buff2, 0, sizeof(buff2));

    //审批信息
    CjUserPilot* pZhan = g_DispatchSvc::instance()->GetUserInfo(m_strID_zhan);
    CjUserPilot* pAnjibu = g_DispatchSvc::instance()->GetUserInfo(m_strID_anjibu);
    CjUserPilot* pZongdiaoshi = g_DispatchSvc::instance()->GetUserInfo(m_strID_zongdiaoshi);
    sprintf(buff2, "Audit:[\"%s\",\"%d\",\"%s\",\"%s\",\"%ld\",\"%d\",\"%s\",\"%s\",\"%s\",\"%ld\",\"%d\",\"%s\",\"%s\",\"%s\",\"%ld\",\"%d\",\"%s\"]",
            m_strSummary, m_nAudit,
            m_strID_zhan, pZhan?pZhan->m_szName:"", m_lTime_zhan, m_nAudit_zhan, m_strAudit_zhan,
            m_strID_anjibu, pAnjibu?pAnjibu->m_szName:"", m_lTime_anjibu, m_nAudit_anjibu, m_strAudit_anjibu,
            m_strID_zongdiaoshi, pZongdiaoshi?pZongdiaoshi->m_szName:"", m_lTime_zongdiaoshi, m_nAudit_zongdiaoshi, m_strAudit_zongdiaoshi);
    strcat(buff, buff2);

    strcat(buff, "}");
    return buff;
}

std::string CjPilotInfo::toPilotPlanJson()
{
    char buff[5*1024] = {'\0'};

    int dist;
    long t1 = 0, t2 = 0, t3 = 0;
    dist = g_DispatchSvc::instance()->GetVslDistETA(this, t1, t2, t3);

    sprintf(buff, "[\"%d\",\"%d\",\"%s\",\"%s\",\"%d\",\"%d\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%d\",\"%d\",\"%d\",\"%d\"]",
            m_nPilotType, (m_nPhase==2)?1:0,
            m_szDeptPilotName, m_szDestPilotName, m_nDeptPortID, m_nDestPortID,
            m_lETD, m_lATD, t1,
            m_lEDC, m_lADC, t2,
            m_lETA, m_lATA, t3,
            m_nMiles1, m_nMiles2,
            dist, m_nJT);

    return buff;
}

std::string CjUserPilot::toJson(int pilotLevel)
{
    CjPilotInfo* pInfo = g_DispatchSvc::instance()->GetPilotInfo(m_szVoyageID);
    const char* vslName = (pInfo?pInfo->m_szNameCn:"");
    const char* vslMmsi = (pInfo?pInfo->m_szMmsi:"");
    const char* pilotID = (pInfo?pInfo->m_szVoyageID:"");

    long nSecond = (long)time(NULL) - m_lAvaibleTime;
    long availableTime = (m_nStatus==1)?(nSecond):0;

    char buff[1024];
    sprintf(buff, "[\"%d\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%ld\",\"%s\",\"%s\",\"%s\",\"%s\"]",
            (pilotLevel==-1?m_nShiRen:pilotLevel), m_szUserID, m_szName, m_szPilotLevel, m_szVoyageLine, m_szTel, m_szEmail, m_szPilotID, m_nStatus, availableTime, vslName, vslMmsi, pilotID, m_szGrade);

    return buff;
}

std::string CjPilotInfo::toPilotManVslJson()
{
    char buff[1024];
    sprintf(buff, "{vid:\"%s\",vname:\"%s\",dipid:\"%s\"}", m_szMmsi, m_szNameCn, m_szVoyageID);
    return buff;
}

void DispatchSvc::GetVslETA1(CjPilotInfo* pInfo, long& t1, long tm, double lon, double lat, double spd)
{
    //进江
    if(pInfo->m_nPilotType == 1)
    {
        if(lon < BAOSHAN_LON) //已经过宝山(数据不大合理,目前有这种数据)
            t1 = 0;
        else
        {
            double dist = (int)(CalCircleDistance(lon, lat, BAOSHAN_LON, BAOSHAN_LAT)/1.852);
            double hour = dist / spd;
            t1 = (long)(tm + hour*24*3600);
        }
    }
    //出江
    else if(pInfo->m_nPilotType == 2)
    {
        if(pInfo->m_nDeptPortID == 0 || lon > pInfo->m_dbDeptLon) //已经开始出江
            t1 = 0;
        else
        {
            double dist = (int)(CalCircleDistance(lon, lat, pInfo->m_dbDeptLon, pInfo->m_dbDeptLat)/1.852);
            double hour = dist / spd;
            t1 = (long)(tm + hour*24*3600);
        }
    }
}

int DispatchSvc::GetVslDistETA(CjPilotInfo* pInfo, long& t1, long& t2, long& t3)
{
    //移泊（如果出发港和目的港相同，则也认为是移泊)
    if(pInfo->m_nPilotType == 3 || pInfo->m_nDeptPortID == pInfo->m_nDestPortID)
    {
        //已开航
        if(pInfo->m_nStatus == 4)
            return 0;

        //未开航
        t3 = (pInfo->m_lETD==0?(long)time(NULL):pInfo->m_lETD) + (long)((pInfo->m_nMiles/1.852/3)*3600); // 3 knots
        return 0;
    }

    //获取当前坐标
    double lon,lat,spd;
    long tm;
    double dist, hour;
    int vslDist = 0;

    AisRealtime* pAis = g_VesselSvc::instance()->aisShip.FindAis(atoi(pInfo->m_szMmsi));
	if(pAis)
	{
		lon = pAis->lon;
		lat = pAis->lat;
		spd = pAis->avgspeed;
		tm = pAis->time;
    }
    else
    {
        lon = pInfo->m_dbAisLon;
        lat = pInfo->m_dbAisLat;
        spd = pInfo->m_dbAisSpd;
        tm = pInfo->m_lAisTime;
    }

    //当前数据太旧(2天前)
    if((long)time(NULL) - tm >= 2*24*3600)
    {
        return 0;
    }

    if(spd <= 0.0)
    {
        spd = 5.0; // 5 knots
    }
    else
    {
        spd *= 1.944;
    }

    if(pInfo->m_nStatus == 4)   //已开航
    {
        t1 = 0;

        //进江
        if(pInfo->m_nPilotType == 1)
        {
            if(lon >= BAOSHAN_LON)
                vslDist = 0;
            else
                vslDist = (int)(CalCircleDistance(lon, lat, BAOSHAN_LON, BAOSHAN_LAT)/1.852);
        }
        //出江
        else if(pInfo->m_nPilotType == 2)
        {
            if(pInfo->m_nDeptPortID == 0)
                vslDist = 0;
            else
                vslDist = (int)(CalCircleDistance(lon, lat, pInfo->m_dbDeptLon, pInfo->m_dbDeptLat)/1.852);
        }
    }
    else  					//未开航
    {
        vslDist = 0;
        GetVslETA1(pInfo, t1, tm, lon, lat, spd);
    }

    t2 = 0;
    if(pInfo->m_nPhase == 2)  	//分段引航
    {
        if(pInfo->m_nPilotType == 2 && lon < JIANGYIN_LON)   //出江且还没经过江阴
        {
            dist = CalCircleDistance(lon, lat, JIANGYIN_LON, JIANGYIN_LAT)/1.852;
            hour = dist / spd;
            t2 = (long)(tm + hour*24*3600);
        }
        if(pInfo->m_nPilotType == 1 && lon > JIANGYIN_LON)   //进江且还没经过江阴
        {
            dist = CalCircleDistance(lon, lat, JIANGYIN_LON, JIANGYIN_LAT)/1.852;
            hour = dist / spd;
            t2 = (long)(tm + hour*24*3600);
        }
    }

    if(pInfo->m_nDestPortID == 0)
        t3 = 0;
    else
    {
        dist = CalCircleDistance(lon, lat, pInfo->m_dbDestLon, pInfo->m_dbDestLat)/1.852;
        hour = dist / spd;
        t3 = (long)(tm + hour*24*3600);
    }

    return vslDist;
}

CjPilotInfo* DispatchSvc::GetPilotInfo(const std::string& vid)
{
    CjPilotInfo* pInfo = NULL;

    MapPilot::iterator iter = m_mapPilot.find(vid);
    if(iter != m_mapPilot.end())
        pInfo = &(iter->second);

    return pInfo;
}

CjUserPilot* DispatchSvc::GetUserInfo(const std::string& uid)
{
    CjUserPilot* pInfo = NULL;

    MapUserPilot::iterator iter = m_mapUserPilot.find(uid);
    if(iter != m_mapUserPilot.end())
        pInfo = iter->second;

    return pInfo;
}

int DispatchSvc::getPilotApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotApply]bad format:", jsonString, 1);

    std::string uid = root.getv("uid", "");
    std::string dt = root.getv("dt", "");
    std::string extradt = root.getv("extradt", "");
    if(uid.empty() || (dt.empty() && extradt.empty()))
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjUserPilot* pUser = GetUserInfo(uid);
    if(!pUser)
    {
        DEBUG_LOG("[DispatchSvc::getPilotApply]user:%s not exists!", uid.c_str());
        return 2;
    }

    int nOrgType = pUser->m_nOrgType;
    char* szStationID = pUser->m_szPilotID;

    VecPilot applyList;

    //1、先根据时间来过滤(取消、过期的不返回,已经结束的只返回当天的)
    if(!extradt.empty())
    {
        for(MapPilot::iterator iter=m_mapPilot.begin(); iter!=m_mapPilot.end(); iter++)
        {
            if(iter->second.m_nStatus>=5)
                continue;
            std::string applyDate = TimeToLocalDate(iter->second.m_lAPDT);
            if(strcmp(applyDate.c_str(), extradt.c_str()) == 0)
                applyList.push_back(&(iter->second));
        }
    }
    else
    {
        for(MapPilot::iterator iter=m_mapPilot.begin(); iter!=m_mapPilot.end(); iter++)
        {
            if(iter->second.m_nStatus>5)
                continue;
            if(iter->second.m_nStatus==5)
            {
                std::string applyDate = TimeToLocalDate(iter->second.m_lAPDT);
                std::string todayDate = TimeToLocalDate(time(NULL));
                if(applyDate != todayDate)
                    continue;
            }
            applyList.push_back(&(iter->second));
        }
    }

    //2、再根据用户级别来过滤(引航中心以及基地能看到所有引航申请)
    if(nOrgType == 6)
    {
        for(VecPilot::iterator iter=applyList.begin(); iter!=applyList.end(); )
        {
            if(strcmp(szStationID, (*iter)->m_szStationID))
                iter = applyList.erase(iter);
            else
                iter++;
        }
    }

    out << "[";
    for(int i=0; i<(int)applyList.size(); i++)
    {
        if(i==0)
            out << applyList[i]->toPilotApplyJson();
        else
            out << "," << applyList[i]->toPilotApplyJson();
    }
    out << "]";

    DEBUG_LOG("[DispatchSvc::getPilotApply]return ship's count:%d", applyList.size());

    return 0;
}

int DispatchSvc::getPilotApplyDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotApplyDetail]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    if(vid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        DEBUG_LOG("[DispatchSvc::getPilotApplyDetail]voyage:%s not exists!", vid.c_str());
        return 2;
    }

    out << pInfo->toDetailPilotApplyJson();
    return 0;
}

int DispatchSvc::getPilotMan(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotMan]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    std::string level = root.getv("level", "");
    std::string zhan = root.getv("zhan", "");
    std::string st = root.getv("st", "");

    if(vid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        DEBUG_LOG("[DispatchSvc::getPilotMan]voyage:%s not exists!", vid.c_str());
        return 2;
    }

    VecUserPilot pilotList;

    //1、获取所有不是引航中状态的引航员列表
    for(MapUserPilot::iterator iter = m_mapUserPilot.begin(); iter != m_mapUserPilot.end(); iter++)
    {
        if(iter->second->m_nStatus == 6)
            continue;
        pilotList.push_back(iter->second);
    }

    //2、更新状态(休息-->待命)
    for(VecUserPilot::iterator iter = pilotList.begin(); iter != pilotList.end(); iter++)
    {
        if((*iter)->m_nStatus == 2 && (time(NULL) - (*iter)->m_lAvaibleTime) >= 0)
            (*iter)->m_nStatus = 1;
    }

    //3、根据站过滤
    if(zhan == "0" || zhan == "1")
    {
        for(VecUserPilot::iterator iter = pilotList.begin(); iter != pilotList.end(); )
        {
            bool bIsLocalStation = (0==strcmp((*iter)->m_szPilotID, pInfo->m_szStationID));
            if(zhan == "0")
            {
                if(!bIsLocalStation)
                    iter = pilotList.erase(iter);
                else
                    iter++;
            }
            else
            {
                if(bIsLocalStation)
                    iter = pilotList.erase(iter);
                else
                    iter++;
            }
        }
    }

    //4、根据状态过滤
    StrSet stSet = StrToSet(st, "|");
    for(VecUserPilot::iterator iter = pilotList.begin(); iter != pilotList.end(); )
    {
        char stStr[2];
        snprintf(stStr, 2, "%d", (*iter)->m_nStatus);
        if(stSet.find(stStr) == stSet.end())
            iter = pilotList.erase(iter);
        else
            iter++;
    }

    //5、根据适任规则过滤
    IntVector levelVec;
    StrSet levelSet = StrToSet(level, "|");
    for(VecUserPilot::iterator iter = pilotList.begin(); iter != pilotList.end(); )
    {
        char levelStr[2];
        int nLevel = getShiRen(*iter, pInfo);
        snprintf(levelStr, 2, "%d", nLevel);
        if(levelSet.find(levelStr) == levelSet.end())
            iter = pilotList.erase(iter);
        else
        {
            iter++;
            levelVec.push_back(nLevel);
        }
    }

    out << "{dispatchers:[";

    for(int i=0; i<(int)pilotList.size(); i++)
    {
        if(i==0)
            out << pilotList[i]->toJson(levelVec[i]);
        else
            out << "," << pilotList[i]->toJson(levelVec[i]);
    }

    out << "]}";

    return 0;
}

int DispatchSvc::getPilotVoy(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotVoy]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    if(vid.empty())
		return 1;

    out << "{eid:1,emsg:\"not implements\"}";
    return 0;
}

int DispatchSvc::chgPilotStatus(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::chgPilotStatus]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    std::string uid = root.getv("uid", "");
    std::string status = root.getv("nstatus", "");
    if(vid.empty() || uid.empty() || status.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        DEBUG_LOG("[DispatchSvc::chgPilotStatus]voyage:%s not exists!", vid.c_str());
        return 2;
    }

    int nStatusOld = pInfo->m_nStatus;
    int nStatus = atoi(status.c_str());
    int nRet = 0;
    char sql[1024];
    MySql* psql = CREATE_MYSQL;

    if(nStatus == 4)			//开始引航
    {
        nRet = makePilotStart(pInfo, psql)?0:3;
    }
    else if(nStatus == 5)		//引航结束
    {
        nRet = makePilotEnd(pInfo, psql)?0:3;
    }
    else if(nStatus == 6)		//取消引航
    {
        nRet = makePilotCancel(pInfo, psql)?0:3;
    }
    else
    {
        pInfo->m_nStatus = nStatus;
        sprintf(sql, "UPDATE t45_pilot_voy_info SET VoyStatus = %d WHERE VOYAGE_ID = '%s'", nStatus, pInfo->m_szVoyageID);
        nRet = (psql->Execute(sql)>=0)?0:3;
    }

    if(nRet == 0)
    {
        out << "{nsucc:0}";

        //写操作日志
        char szRmk[1024];
        sprintf(szRmk, "修改了引航状态,从 %s 到 %s ", PILOT_STATUS_DESC[nStatusOld], PILOT_STATUS_DESC[nStatus]);
        sprintf(sql,
                "INSERT INTO t45_pilot_cj_logs(VOYAGE_ID, Operator, operate_dt, Remark) VALUES ('%s', '%s', CURRENT_TIMESTAMP(), '%s')",
                vid.c_str(), uid.c_str(), szRmk);
        DB_LOG(sql);
    }

    RELEASE_MYSQL_RETURN(psql, nRet);
}

int DispatchSvc::chgPilotSL(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::chgPilotSL]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    std::string uid = root.getv("uid", "");
    int         apt = root.getv("naccept", 0);
    std::string rmk = root.getv("rmk", "");

    if(vid.empty() || uid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        DEBUG_LOG("[DispatchSvc::chgPilotSL]voyage:%s not exists!", vid.c_str());
        return 2;
    }
    CjUserPilot* pUser = GetUserInfo(uid);
    if(!pUser)
    {
        DEBUG_LOG("[DispatchSvc::chgPilotSL]user:%s not exists!", uid.c_str());
        return 2;
    }

    char sql[1024];
    MySql* psql = CREATE_MYSQL;

    if(apt == 0)   //拒绝受理
    {
        if(!makePilotCancel(pInfo, psql, true, rmk, pUser))
            RELEASE_MYSQL_RETURN(psql, 3);

        //写操作日志
        char szRmk[1024];
        sprintf(szRmk, "修改了受理意见,从 %s 到 %s ", SL_STATUS_DESC[0], SL_STATUS_DESC[2]);
        sprintf(sql,"INSERT INTO t45_pilot_cj_logs(VOYAGE_ID, Operator, operate_dt, Remark) VALUES ('%s', '%s', CURRENT_TIMESTAMP(), '%s')",
					vid.c_str(), uid.c_str(), szRmk);
        DB_LOG(sql);

        out << "{nsucc:0}";
        RELEASE_MYSQL_RETURN(psql, 0);
    }

    sprintf(sql, "UPDATE t45_pilot_voy_info SET Transactor = '%s', SurveyStatus = 1, SurveyOpinion = '%s', SurveyDt = CURRENT_TIMESTAMP() \
				WHERE VOYAGE_ID = '%s'", uid.c_str(), rmk.c_str(), vid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out << "{nsucc:0}";

    sprintf(pInfo->m_strAccepterID, "%s", uid.c_str());
    sprintf(pInfo->m_strAgreement, "%s", rmk.c_str());
    pInfo->m_nAgree = 1;
    pInfo->m_lAcceptTime = (long)time(NULL);

    //写操作日志
    char szRmk[1024];
    sprintf(szRmk, "修改了受理意见,从 %s 到 %s ", SL_STATUS_DESC[0], SL_STATUS_DESC[1]);
    sprintf(sql,"INSERT INTO t45_pilot_cj_logs(VOYAGE_ID, Operator, operate_dt, Remark) VALUES ('%s', '%s', CURRENT_TIMESTAMP(), '%s')",
				vid.c_str(), uid.c_str(), szRmk);
    DB_LOG(sql);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int DispatchSvc::chgPilotZJ(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::chgPilotZJ]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    std::string uid = root.getv("uid", "");
    std::string rmk = root.getv("rmk", "");

    if(vid.empty() || uid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        DEBUG_LOG("[DispatchSvc::chgPilotZJ]voyage:%s not exists!", vid.c_str());
        return 2;
    }

    char sql[1024];
    MySql* psql = CREATE_MYSQL;
    sprintf(sql, "UPDATE t45_pilot_voy_info SET Summary = '%s' WHERE VOYAGE_ID = '%s'", rmk.c_str(), vid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out << "{nsucc:0}";

    sprintf(pInfo->m_strSummary, "%s", rmk.c_str());

    //写操作日志
    char szRmk[1024];
    sprintf(szRmk, "%s", "修改了引航总结");
    sprintf(sql,
            "INSERT INTO t45_pilot_cj_logs(VOYAGE_ID, Operator, operate_dt, Remark) VALUES ('%s', '%s', CURRENT_TIMESTAMP(), '%s')",
            vid.c_str(), uid.c_str(), szRmk);
    DB_LOG(sql);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int DispatchSvc::chgPilotSP(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::chgPilotSP]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    std::string uid = root.getv("uid", "");
    int         agr = root.getv("nagree", 0);
    std::string rmk = root.getv("rmk", "");
    int         ntp = root.getv("ntp", 0);

    if(vid.empty() || uid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        DEBUG_LOG("[DispatchSvc::chgPilotSP]voyage:%s not exists!", vid.c_str());
        return 2;
    }

#define WHICH_ORG(X) ((X)==0?"Leader":((X)==1?"Tech":"Sch"))
#define ORG_ID(X) ((X)==0?pInfo->m_strID_zhan:((X)==1?pInfo->m_strID_anjibu:pInfo->m_strID_zongdiaoshi))
#define ORG_AUDIT(X) ((X)==0?pInfo->m_nAudit_zhan:((X)==1?pInfo->m_nAudit_anjibu:pInfo->m_nAudit_zongdiaoshi))
#define ORG_TIME(X) ((X)==0?pInfo->m_lTime_zhan:((X)==1?pInfo->m_lTime_anjibu:pInfo->m_lTime_zongdiaoshi))
#define ORG_REMARK(X) ((X)==0?pInfo->m_strAudit_zhan:((X)==1?pInfo->m_strAudit_anjibu:pInfo->m_strAudit_zongdiaoshi))

    int nRet = 0;
    char sql[1024];
    MySql* psql = CREATE_MYSQL;
    sprintf(sql, "UPDATE t45_pilot_voy_info SET %sApprover = '%s', %sApprStatus = '%d', %sApprOpinion = '%s', %sApprDt = CURRENT_TIMESTAMP() \
				WHERE VOYAGE_ID = '%s'", WHICH_ORG(ntp), uid.c_str(), WHICH_ORG(ntp), agr, WHICH_ORG(ntp), rmk.c_str(), WHICH_ORG(ntp), vid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    ORG_AUDIT(ntp) = agr;
    ORG_TIME(ntp) = (long)time(NULL);
    sprintf(ORG_REMARK(ntp), "%s", rmk.c_str());
    sprintf(ORG_ID(ntp), "%s", uid.c_str());

    //写操作日志
    char szRmk[1024];
    sprintf(szRmk, "%s", "修改了审批意见");
    sprintf(sql,
            "INSERT INTO t45_pilot_cj_logs(VOYAGE_ID, Operator, operate_dt, Remark) VALUES ('%s', '%s', CURRENT_TIMESTAMP(), '%s')",
            vid.c_str(), uid.c_str(), szRmk);
    DB_LOG(sql);

    out << "{nsucc:0}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int DispatchSvc::chgPilotStartTime(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::chgPilotStartTime]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    std::string uid = root.getv("uid", "");
    int			ltm = root.getv("ltm", 0);
    int         ntp = root.getv("ntp", 0);
    int			nport = root.getv("nport", 0);

    if(vid.empty() || uid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        DEBUG_LOG("[DispatchSvc::chgPilotStartTime]voyage:%s not exists!", vid.c_str());
        return 2;
    }

    int nRet = 0;
    char sql[1024] = {'\0'}, sql2[1024] = {'\0'};
    if(ntp == 0)
    {
        if(nport == 0)
        {
            sprintf(sql, "update t45_pilot_voy_info set etd = FROM_UNIXTIME(%d) where voyage_id = '%s'", ltm, vid.c_str());
            sprintf(sql2, "update t45_pilot_cj_section set etd = FROM_UNIXTIME(%d) where voyage_id = '%s' and seq = 0", ltm, vid.c_str());
        }
        else if(nport == 1)
        {
            sprintf(sql, "update t45_pilot_cj_section set eta = FROM_UNIXTIME(%d) where voyage_id = '%s' and seq = 0", ltm, vid.c_str());
            sprintf(sql2,"update t45_pilot_cj_section set etd = FROM_UNIXTIME(%d) where voyage_id = '%s' and seq = 1", ltm, vid.c_str());
        }
        else if(nport == 2)
        {
            sprintf(sql, "update t45_pilot_voy_info set eta = FROM_UNIXTIME(%d) where voyage_id = '%s'", ltm, vid.c_str());
            sprintf(sql2, "update t45_pilot_cj_section set eta = FROM_UNIXTIME(%d) where voyage_id = '%s' and seq = %d", ltm, vid.c_str(), pInfo->m_nPhase-1);
        }
    }
    else if(ntp == 1)
    {
        if(nport == 0)
        {
            sprintf(sql, "update t45_pilot_voy_info set atd = FROM_UNIXTIME(%d) where voyage_id = '%s'", ltm, vid.c_str());
            sprintf(sql2, "update t45_pilot_cj_section set atd = FROM_UNIXTIME(%d) where voyage_id = '%s' and seq = 0", ltm, vid.c_str());
        }
        else if(nport == 1)
        {
            sprintf(sql, "update t45_pilot_cj_section set ata = FROM_UNIXTIME(%d) where voyage_id = '%s' and seq = 0", ltm, vid.c_str());
            sprintf(sql2,"update t45_pilot_cj_section set atd = FROM_UNIXTIME(%d) where voyage_id = '%s' and seq = 1", ltm, vid.c_str());
        }
        else if(nport == 2)
        {
            sprintf(sql, "update t45_pilot_voy_info set ata = FROM_UNIXTIME(%d) where voyage_id = '%s'", ltm, vid.c_str());
            sprintf(sql2, "update t45_pilot_cj_section set ata = FROM_UNIXTIME(%d) where voyage_id = '%s' and seq = %d", ltm, vid.c_str(), pInfo->m_nPhase-1);
        }
    }
    else if(ntp == 2)
    {
        sprintf(sql2, "update t45_pilot_voy_info set pilot_time = %d where voyage_id = '%s'", ltm, vid.c_str());
    }

    MySql* psql = CREATE_MYSQL;

    if(strlen(sql) && psql->Execute(sql) == -1)
    {
        RELEASE_MYSQL_RETURN(psql, 3);
    }

    if(strlen(sql2) && psql->Execute(sql2) == -1)
    {
        RELEASE_MYSQL_RETURN(psql, 3);
    }

    out << "{nsucc:0}";

    long ltmOld, ltmNew;

    if(ntp == 0)
    {
        if(nport == 0)
        {
            ltmOld = pInfo->m_lETD;
            ltmNew = ltm;
            pInfo->m_lETD = ltm;
        }
        else if(nport == 1)
        {
            ltmOld = pInfo->m_lEDC;
            ltmNew = ltm;
            pInfo->m_lEDC = ltm;
        }
        else if(nport == 2)
        {
            ltmOld = pInfo->m_lETA;
            ltmNew = ltm;
            pInfo->m_lETA = ltm;
        }
    }
    else if(ntp == 1)
    {
        if(nport == 0)
        {
            ltmOld = pInfo->m_lATD;
            ltmNew = ltm;
            pInfo->m_lATD = ltm;
        }
        else if(nport == 1)
        {
            ltmOld = pInfo->m_lADC;
            ltmNew = ltm;
            pInfo->m_lADC = ltm;
        }
        else if(nport == 2)
        {
            ltmOld = pInfo->m_lATA;
            ltmNew = ltm;
            pInfo->m_lATA = ltm;
        }
    }
    else if(ntp == 2)
    {
        ltmOld = pInfo->m_nJT;
        ltmNew = ltm;
        pInfo->m_nJT = ltm;
    }

    //写操作日志
    char szRmk[1024];
    if(ntp == 2)
    {
        sprintf(szRmk, "修改了交通船时间,从 %d分钟 到 %d分钟 ", (int)ltmOld/60, (int)ltmNew/60);
        sprintf(sql,
                "INSERT INTO t45_pilot_cj_logs(VOYAGE_ID, Operator, operate_dt, Remark) VALUES ('%s', '%s', CURRENT_TIMESTAMP(), '%s')",
                vid.c_str(), uid.c_str(), szRmk);
    }
    else
    {
        sprintf(szRmk, "修改了%s时间,从 %s 到 %s ", (ntp==0?"预计开航":"实际开航"), TimeToTimestampStr(ltmOld).c_str(), TimeToTimestampStr(ltmNew).c_str());
        sprintf(sql,
                "INSERT INTO t45_pilot_cj_logs(VOYAGE_ID, Operator, operate_dt, Remark) VALUES ('%s', '%s', CURRENT_TIMESTAMP(), '%s')",
                vid.c_str(), uid.c_str(), szRmk);
    }
    DB_LOG(sql);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int DispatchSvc::dispatchPilotMan(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::dispatchPilot]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    std::string uid = root.getv("uid", "");
    std::string yid = root.getv("yid", "");
    int			ytp = root.getv("ytp", 0);

    if(vid.empty() || uid.empty() || yid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    CjUserPilot* pUser = GetUserInfo(uid);
    CjUserPilot* pYser = GetUserInfo(yid);
    if(!pInfo || !pUser || !pYser)
        return 2;

    int nSeq = 0;
    if(pInfo->m_nPhase==1 || pInfo->m_nPilotType==3)  	//单段（包含移泊）
    {
        nSeq = 0;
    }
    else if(pUser->m_nOrgType == 6)  					//分段：引航站调派
    {
        if(pInfo->m_nPilotType==2)
            nSeq = 0;	//出江
        else
            nSeq = 1;	//进江
    }
    else  											//分段：中心或者基地进行调派
    {
        if(pInfo->m_nPilotType==2)
            nSeq = 1;	//出江
        else
            nSeq = 0;	//进江
    }

    int nRet = 0;
    char sql[1024] = {'\0'}, sql2[1024] = {'\0'};
    MySql* psql = CREATE_MYSQL;
    sprintf(sql, "update t45_pilot_cj_section set pilotor%d = '%s' where voyage_id = '%s' and seq = %d and (pilotor%d is null)", ytp, yid.c_str(), vid.c_str(), nSeq, ytp);
    sprintf(sql2, "update t45_pilot_cj_section set pilotor%d = concat(pilotor%d, '|', '%s') where voyage_id = '%s' and seq = %d", ytp, ytp, yid.c_str(), vid.c_str(), nSeq);

    int nRes = psql->Execute(sql);
    if(nRes == -1)
    {
        RELEASE_MYSQL_RETURN(psql, 3);
    }
    if(nRes == 0)
    {
        nRes = psql->Execute(sql2);
    }

    if(nRes == -1)
    {
        RELEASE_MYSQL_RETURN(psql, 3);
    }
    if(nRes == 1)
    {
        out << "{nsucc:0}";

        if(nSeq == 0)
            pInfo->pilotorList1.insert(yid);
        else
            pInfo->pilotorList2.insert(yid);
        pYser->m_nStatus = 7;
        sprintf(pYser->m_szVoyageID, "%s", vid.c_str());
        sprintf(sql, "update T45_pilot_cj_man_cn set CurStatus = 7 where UserID = '%s'", yid.c_str());
        psql->Execute(sql);

        //写操作日志
        char szRmk[1024];
		sprintf(szRmk, "修改了引航员,增加 %s ", CodeConverter::Utf8ToGb2312(pYser->m_szName).c_str());
        sprintf(sql,
                "INSERT INTO t45_pilot_cj_logs(VOYAGE_ID, Operator, operate_dt, Remark) VALUES ('%s', '%s', CURRENT_TIMESTAMP(), '%s')",
                vid.c_str(), uid.c_str(), szRmk);
        DB_LOG(sql);

        RELEASE_MYSQL_RETURN(psql, 0);
    }

    RELEASE_MYSQL_RETURN(psql, 2);
}

int DispatchSvc::getPilotManVsl(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotManVsl]bad format:", jsonString, 1);

    std::string uid = root.getv("uid", "");
    std::string yid = root.getv("yid", "");

    if(uid.empty() || yid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjUserPilot* pUser = GetUserInfo(uid);
    CjUserPilot* pYser = GetUserInfo(yid);
    if(!pUser || !pYser)
        return 2;

    VecPilot vslList;

    // 1.还没开始航行(状态必须是2和3)
    for(MapPilot::iterator iter=m_mapPilot.begin(); iter!=m_mapPilot.end(); iter++)
    {
        if(iter->second.m_nStatus==2 || iter->second.m_nStatus==3)
            vslList.push_back(&(iter->second));
    }

    // 2.中心的人可以给引航员调派任意的船舶
    if(pUser->m_nOrgType == 6)
    {
        for(VecPilot::iterator iter=vslList.begin(); iter!=vslList.end(); )
        {
            if(strcmp((*iter)->m_szStationID, pYser->m_szPilotID))
                iter = vslList.erase(iter);
            else
                iter++;
        }
    }

    out << "[";
    for(int i=0; i<(int)vslList.size(); i++)
    {
        if(i==0)
            out << vslList[i]->toPilotManVslJson();
        else
            out << "," << vslList[i]->toPilotManVslJson();
    }
    out << "]";

    return 0;
}

int DispatchSvc::getPilotVslDist(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotVslDist]bad format:", jsonString, 1);

    std::string vid  = root.getv("vid", "");
    std::string mmsi = root.getv("mmsi", "");
    std::string stdt = root.getv("stdt", "");

    if(vid.empty() || mmsi.empty() || stdt.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);
    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        return 2;
    }

    long t1 = 0, t2 = 0, t3 = 0;
    int nDist = GetVslDistETA(pInfo, t1, t2, t3);

    out << "{dist:" << nDist << ",t1:" << t1 << ",t2:" << t2 << ",t3:" << t3 << "}";
    return 0;
}

int DispatchSvc::getPilotManStation(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotManStation]bad format:", jsonString, 1);

    std::string uid = root.getv("uid", "");
    if(uid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjUserPilot* pUser = GetUserInfo(uid);
    if(!pUser)
        out << "{org:0,zhan:\"\",audit:0,una:\"\"}";
    else
        out << "{org:" << pUser->m_nOrgType << ",zhan:\"" << pUser->m_szPilotID << "\",audit:" << pUser->m_nAuth << ",una:\"" << pUser->m_szName << "\"}";

    return 0;
}

int DispatchSvc::getPilotPlan(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotPlan]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    if(vid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        DEBUG_LOG("[DispatchSvc::getPilotPlan]voyage:%s not exists!", vid.c_str());
        return 2;
    }

    out << pInfo->toPilotPlanJson();
    return 0;
}

int DispatchSvc::delPilotMan(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::delPilotMan]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    std::string uid = root.getv("uid", "");
    std::string yid = root.getv("yid", "");
    if(vid.empty() || uid.empty() || yid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    CjUserPilot* pUser = GetUserInfo(uid);
    CjUserPilot* pYser = GetUserInfo(yid);
    if(!pInfo || !pUser || !pYser)
        return 2;

    int nShiRen = pYser->m_nShiRen;
    int nSeq = 0;
    if(pInfo->m_nPhase==1 || pInfo->m_nPilotType==3)  	//单段（包含移泊）
    {
        nSeq = 0;
    }
    else if(pUser->m_nOrgType == 6)  					//分段：引航站调派
    {
        if(pInfo->m_nPilotType==2)
            nSeq = 0;	//出江
        else
            nSeq = 1;	//进江
    }
    else  											//分段：中心或者基地进行调派
    {
        if(pInfo->m_nPilotType==2)
            nSeq = 1;	//出江
        else
            nSeq = 0;	//进江
    }

    std::string pilotors;

    if(nSeq == 0)
    {
        DeleteFromSet(pInfo->pilotorList1, yid);
        pilotors = ToStr(pInfo->pilotorList1, "|");
    }
    else
    {
        DeleteFromSet(pInfo->pilotorList2, yid);
        pilotors = ToStr(pInfo->pilotorList2, "|");
    }
    pYser->m_nStatus = 1;
    memset(pYser->m_szVoyageID, 0, sizeof(pYser->m_szVoyageID));

    //更改引航员的状态
    int nRet = 0;
    char sql[1024];
    char sql2[1024];

    if(pilotors.empty())
        sprintf(sql, "update t45_pilot_cj_section set pilotor%d = NULL where voyage_id = '%s' and seq = %d", nShiRen, vid.c_str(), nSeq);
    else
        sprintf(sql, "update t45_pilot_cj_section set pilotor%d = '%s' where voyage_id = '%s' and seq = %d", nShiRen, pilotors.c_str(), vid.c_str(), nSeq);
    sprintf(sql2, "UPDATE t45_pilot_cj_man_cn SET curstatus = '1' WHERE userid = '%s'", yid.c_str());

    MySql* psql = CREATE_MYSQL;

    int nRes1 = psql->Execute(sql);
    int nRes2 = psql->Execute(sql2);

    if(nRes1 == -1 || nRes2 == -1)
    {
        RELEASE_MYSQL_RETURN(psql, 3);
    }

    //更改引航状态(如果引航员数量为0则变成待派)
    if(pInfo->pilotorList1.empty() && pInfo->pilotorList2.empty())
    {
        sprintf(sql, "update t45_pilot_voy_info set voystatus = 2 where voyage_id = '%s'", pInfo->m_szVoyageID);
        psql->Execute(sql);
        pInfo->m_nStatus = 2;
    }

    out << "{nsucc:0}";

    //写操作日志
    char szRmk[1024];
    sprintf(szRmk, "修改了引航员,删除 %s ",  CodeConverter::Utf8ToGb2312(pYser->m_szName).c_str());
    sprintf(sql,
            "INSERT INTO t45_pilot_cj_logs(VOYAGE_ID, Operator, operate_dt, Remark) VALUES ('%s', '%s', CURRENT_TIMESTAMP(), '%s')",
            vid.c_str(), uid.c_str(), szRmk);
    DB_LOG(sql);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int DispatchSvc::getPilotLog(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotLog]bad format:", jsonString, 1);

    std::string vid = root.getv("yid", "");
    if(vid.empty())
		return 1;

    int nRet = 0;
    char sql[1024] = {'\0'};
    MySql* psql = CREATE_MYSQL;
    sprintf(sql, "SELECT Operator, UNIX_TIMESTAMP(operate_dt) as tm, Remark FROM t45_pilot_cj_logs \
				WHERE VOYAGE_ID = '%s' ORDER BY tm", vid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "[";
	bool first = true;
	while(psql->NextRow())
	{
		TDispLog dispLog;

		READMYSQL_STR(Operator, dispLog.m_strOperator);
		READMYSQL_STR(tm, dispLog.m_strOperateDt);
		READMYSQL_STR(Remark, dispLog.m_strOperateContent);
		if(first)
		{
			out << dispLog.toJson();
			first = false;
		}
		else
			out << "," << dispLog.toJson();
	}
	out << "]";

    RELEASE_MYSQL_RETURN(psql, nRet);
}

int DispatchSvc::getPilotRmk(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotRmk]bad format:", jsonString, 1);
    std::string vid = root.getv("yid", "");
		return 1;

    int nRet = 0;
    char sql[1024] = {'\0'};
    MySql* psql = CREATE_MYSQL;
    sprintf(sql, "SELECT RemarkID, UserID, Title, Content, Update_dt FROM t45_pilot_cj_remark WHERE VoyageID = '%s'", vid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    
	out << "[";
	bool bFirst = true;
	while(psql->NextRow())
	{
		TDispRmk rmk;
		READMYSQL_STR(remarkid, rmk.m_strRmkID);
		READMYSQL_STR(userid, rmk.m_strUserID);
		READMYSQL_STR(title, rmk.m_strTitle);
		READMYSQL_STR(content, rmk.m_strContent);
		READMYSQL_64(update_dt, rmk.m_lUpdTime, 0L);
		if(bFirst)
		{
			bFirst = false;
			out << rmk.toJson();
		}
		else
		{
			out << "," << rmk.toJson();
		}
	}
	out << "]";

    RELEASE_MYSQL_RETURN(psql, nRet);
}

int DispatchSvc::updPilotRmk(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::updPilotRmk]bad format:", jsonString, 1);

    std::string vid = root.getv("yid", "");
    std::string rid = root.getv("rid", "");
    std::string uid = root.getv("uid", "");
    int			tm  = root.getv("tm", 0);
    std::string title = root.getv("title", "");
    std::string content = root.getv("content", "");

    int nRemarkID = -1;

    if(vid.empty() || uid.empty())
		return 1;

    if(tm <= 0)
        tm = (int)time(NULL);
    title = StrReplace(title, "'", "\'");
    content = StrReplace(content, "'", "\'");

    int nRet = 0;
    char sql[1024] = {'\0'};
    MySql* psql = CREATE_MYSQL;

    if(rid.empty())
    {
        sprintf(sql, "insert into t45_pilot_cj_remark(voyageid, userid, title, content, update_dt) values ('%s','%s','%s','%s',%d)",
					vid.c_str(), uid.c_str(), title.c_str(), content.c_str(), tm);
    }
    else
    {
        nRemarkID = atoi(rid.c_str());
        sprintf(sql, "update t45_pilot_cj_remark set title = '%s', content = '%s', update_dt = %d where remarkid = '%s'",
					title.c_str(), content.c_str(), tm, rid.c_str());
    }

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	if(nRemarkID == -1)
		nRemarkID = psql->GetInsertId();

	out << "{nsucc:0,rid:\"" << nRemarkID << "\"}";

    RELEASE_MYSQL_RETURN(psql, nRet);
}

int DispatchSvc::delPilotRmk(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::delPilotRmk]bad format:", jsonString, 1);

    std::string rid = root.getv("rid", "");
    if(rid.empty())
		return 1;

    char sql[1024] = {'\0'};
    MySql* psql = CREATE_MYSQL;
    sprintf(sql, "delete from t45_pilot_cj_remark where remarkid = '%s'", rid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
	out << "{nsucc:0}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int DispatchSvc::getPilotedMan(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotMan]bad format:", jsonString, 1);
    std::string vid = root.getv("vid", "");
    if(vid.empty())
		return 1;

    int nRet = 0;
    char sql[1024] = {'\0'};
    MySql* psql = CREATE_MYSQL;
    sprintf(sql, "select pilotor1, pilotor2, pilotor3 from t45_pilot_cj_section where voyage_id = '%s'", vid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    VecUserPilot pilotList;
    std::vector<int> levelList;

	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);
	while(psql->NextRow())
	{
		char pilotorShiRen[1024];
		char pilotorPeiBan[1024];
		char pilotorShixi[1024];
		READMYSQL_STR(pilotor1, pilotorShiRen);
		READMYSQL_STR(pilitor2, pilotorPeiBan);
		READMYSQL_STR(pilotor3, pilotorShixi);

		parsePilots(pilotorShiRen, pilotList, levelList, 1);
		parsePilots(pilotorPeiBan, pilotList, levelList, 2);
		parsePilots(pilotorShixi, pilotList, levelList, 3);
	}

	out << "{dispatchers:[";

	for(int i=0; i<(int)pilotList.size(); i++)
	{
		if(i==0)
			out << pilotList[i]->toJson(levelList[i]);
		else
			out << "," << pilotList[i]->toJson(levelList[i]);
	}

	out << "]}";

    RELEASE_MYSQL_RETURN(psql, nRet);
}

int DispatchSvc::getPilotManContact(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotManContact]bad format:", jsonString, 1);

    int mmsi = root.getv("mmsi", 0);
    int type = root.getv("type", 0);
    if(mmsi < 100000000)
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    //1、根据mmsi找到对应的引航信息
    CjPilotInfo* pVoy = NULL;
    for(MapPilot::iterator iter = m_mapPilot.begin(); iter != m_mapPilot.end(); iter++)
    {
        if(mmsi == atoi(iter->second.m_szMmsi))
        {
            pVoy = &(iter->second);
            break;
        }
    }
    if(!pVoy)
        return 2;

    //2、从引航信息中找到引航员信息
    VecUserPilot pilotorList;
    for(StrSetIter iter_=pVoy->pilotorList1.begin(); iter_!=pVoy->pilotorList1.end(); iter_++)
    {
        MapUserPilot::iterator iter = m_mapUserPilot.find(*iter_);
        if(iter != m_mapUserPilot.end())
            pilotorList.push_back(iter->second);
    }
    for(StrSetIter iter_=pVoy->pilotorList2.begin(); iter_!=pVoy->pilotorList2.end(); iter_++)
    {
        MapUserPilot::iterator iter = m_mapUserPilot.find(*iter_);
        if(iter != m_mapUserPilot.end())
            pilotorList.push_back(iter->second);
    }

    //3、从引航员信息中获取联系方式
    out << "{mmsi:" << mmsi << ",type:" << type << ",restr:\"";
    bool bFirst = true;
    if(type == 1)
    {
        for(int i=0; i<(int)pilotorList.size(); i++)
        {
            if(pilotorList[i]->m_szEmail[0] == '\0')
                continue;
            if(bFirst)
            {
                out << pilotorList[i]->m_szEmail << "," << pilotorList[i]->m_szUserID;
                bFirst = false;
            }
            else
            {
                out << ";" << pilotorList[i]->m_szEmail << "," << pilotorList[i]->m_szUserID;
            }
        }
    }
    else if(type == 2)
    {
        for(int i=0; i<(int)pilotorList.size(); i++)
        {
            if(pilotorList[i]->m_szTel[0] == '\0')
                continue;
            if(bFirst)
            {
                out << pilotorList[i]->m_szTel << "," << pilotorList[i]->m_szUserID;
                bFirst = false;
            }
            else
            {
                out << ";" << pilotorList[i]->m_szTel << "," << pilotorList[i]->m_szUserID;
            }
        }
    }
    else if(type == 3)
    {
        for(int i=0; i<(int)pilotorList.size(); i++)
        {
            if(bFirst)
            {
                out << pilotorList[i]->m_szEmail << "," << pilotorList[i]->m_szTel << "," << pilotorList[i]->m_szUserID << "," << pilotorList[i]->m_szName;
                bFirst = false;
            }
            else
            {
                out << ";" << pilotorList[i]->m_szEmail << "," << pilotorList[i]->m_szTel << "," << pilotorList[i]->m_szUserID << "," << pilotorList[i]->m_szName;
            }
        }
    }
    out << "\"}";

    return 0;
}

int DispatchSvc::getPilotStatus(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[DispatchSvc::getPilotStatus]bad format:", jsonString, 1);

    std::string vid = root.getv("vid", "");
    if(vid.empty())
		return 1;

    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard2(m_ThreadLock);

    CjPilotInfo* pInfo = GetPilotInfo(vid);
    if(!pInfo)
    {
        DEBUG_LOG("[DispatchSvc::getPilotStatus]voyage:%s not exists!", vid.c_str());
        return 2;
    }

    out << "{nst:" << pInfo->m_nStatus << "}";
    return 0;
}

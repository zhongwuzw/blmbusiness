#include "DispatchCal.h"
#include "DispatchSvc.h"
#include "VesselSvc.h"
#include "LogMgr.h"
#include "PortSvc.h"
#include "ObjectPool.h"
#include "kSQL.h"
#include "ClientSocketMgr.h"
#include "MainConfig.h"

#define PORT_CIRCLE_RADIUS 0.37 //KM
bool PtInPortCircle(PT& pt, PT& port)
{
    double dist = pt.distance(port) / 1000;
    return (dist <= PORT_CIRCLE_RADIUS);
}

//�Զ�ά������״̬
//1�������ǽ������ǳ���������ʼ��Ϊ�����ڸۿڣ���������ɽ���أ�����������ʼ�ۿ�����ϵ����ɶ������л��ٶȱ�ɴ���10ʱ�����Զ���״̬��Ϊ�����У�дʵ��������ʼʱ��Ϊ״̬�ı�ʱ�䣩
//2�������ǽ������ǳ�������Ŀ�ĸ�Ϊ�����ڸۿڣ���������ɽ���أ���������Ŀ�ĸۿ����ɺ��б��ϵ�������Զ���״̬��Ϊ����ɣ�дʵ�����ʱ��Ϊ��ϵ�����ٶȱ�Ϊ0��ʱ�䣩
//3������������ʼ��Ϊ��ɽ���ػ򳤽���ۿڣ��򵱴����������һ�Ÿ������Զ���״̬�ĳ������У�дʵ��������ʼʱ��Ϊ��һ�Ÿ�ʱ�䣩
//4����������Ŀ�ĸ�Ϊ��ɽ���ػ򳤽���ۿڣ��򵱴����������һ�Ÿ������Զ���״̬�ĳ�����ɣ�дʵ�����ʱ��Ϊ��һ�Ÿ�ʱ�䣩
//5���Ʋ��ģ�ֻҪ���˼ƻ�����ʱ�䣬�������

//��������ʱ��Ԥ��
//1��(�¼�ID��78����������ʱ��Ԥ�����ڳ�����êê���Ĵ���������ó������ƻ�����ʱ�����4Сʱ��������Ϊ0����״̬��δ���Ϊ��������Ԥ��
//2��(�¼�ID��79����������ʱ��Ԥ������̫��(56615)������(56616)������Ԥ����ƻ�ʱ������Сʱ��Ԥ��������վ������Ԥ����ƻ����1Сʱ��Ԥ����Ԥ��ʱ��Ϊ��ǰ2Сʱ��

int AlertRule(CjPilotInfo* pilot)
{
    if(pilot->m_nStatus >= 4 || pilot->m_bAlert)
        return 0;

    long t1 = 0;
    g_DispatchSvc::instance()->GetVslETA1(pilot, t1, pilot->m_lAisTime, pilot->m_dbAisLon, pilot->m_dbAisLat, pilot->m_dbAisSpd);
    if(t1 == 0)
        return 0;

    if(pilot->m_nPilotType == 1) //����
    {
        double dist = CalCircleDistance(pilot->m_dbAisLon, pilot->m_dbAisLat, FLOAT1_LON, FLOAT1_LAT)/1.852;
        if((pilot->m_nVoyType == 2) && (pilot->m_dbAisSpd == 0) && (pilot->m_lAPDT - time(NULL) <= 4*3600) && (dist >= FLOAT1_CJK_DIST) && (pilot->m_dbAisLon > FLOAT1_LON))
        {
            pilot->m_bAlert = true;
            return 1;
        }
    }
    else if(pilot->m_nPilotType == 2) //����
    {
        if((pilot->m_nDeptPortID == 56615 || pilot->m_nDeptPortID == 56616) && (abs(t1 - pilot->m_lAPDT) >= 0.5*3600))
        {
            pilot->m_bAlert = true;
            return 2;
        }
        if(abs(t1 - pilot->m_lAPDT) >= 1*3600)
        {
            pilot->m_bAlert = true;
            return 2;
        }
    }

    return 0;
}

bool DispatchCalRule1(CjPilotInfo* pilot, int preAisStatus, MySql* psql)
{
    PT ais(pilot->m_dbAisLon, pilot->m_dbAisLat), port(pilot->m_dbDeptLon, pilot->m_dbDestLat);
    if((pilot->m_nStatus==2||pilot->m_nStatus==3) &&
            pilot->m_nDeptPortID > 0 &&
            (PtInPolygon(g_CJPolygon, port) && pilot->m_nDeptPortID != 40616) &&
            PtInPortCircle(ais, port) &&
            ((preAisStatus==5&&pilot->m_nAisStatus!=5) || pilot->m_dbAisSpd >= 10)
      )
    {
        g_DispatchSvc::instance()->makePilotStart(pilot, psql);
        return true;
    }

    return false;
}

bool DispatchCalRule2(CjPilotInfo* pilot, int preAisStatus, MySql* psql)
{
    PT ais(pilot->m_dbAisLon, pilot->m_dbAisLat), port(pilot->m_dbDestLon, pilot->m_dbDestLat);
    if(pilot->m_nStatus==4 &&
            pilot->m_nDestPortID > 0 &&
            (PtInPolygon(g_CJPolygon, port) && pilot->m_nDestPortID != 40616) &&
            PtInPortCircle(ais, port) &&
            ((preAisStatus!=5&&pilot->m_nAisStatus==5) || pilot->m_dbAisSpd == 0)
      )
    {
        g_DispatchSvc::instance()->makePilotEnd(pilot, psql);
        return true;
    }

    return false;
}

bool DispatchCalRule3(CjPilotInfo* pilot, MySql* psql)
{
    PT port(pilot->m_dbDeptLon, pilot->m_dbDestLat);
    if((pilot->m_nStatus==2||pilot->m_nStatus==3) &&
            pilot->m_nPilotType==1 &&
            pilot->m_nDeptPortID > 0 &&
            (!PtInPolygon(g_CJPolygon, port) || pilot->m_nDeptPortID == 40616) &&
            pilot->m_dbAisLon <= FLOAT1_LON
      )
    {
        g_DispatchSvc::instance()->makePilotStart(pilot, psql);
        return true;
    }

    return false;
}

bool DispatchCalRule4(CjPilotInfo* pilot, MySql* psql)
{
    PT port(pilot->m_dbDestLon, pilot->m_dbDestLat);
    if(pilot->m_nStatus==4 &&
            pilot->m_nPilotType==2 &&
            pilot->m_nDestPortID > 0 &&
            (!PtInPolygon(g_CJPolygon, port) || pilot->m_nDestPortID == 40616) &&
            pilot->m_dbAisLon >= FLOAT1_LON
      )
    {
        g_DispatchSvc::instance()->makePilotEnd(pilot, psql);
        return true;
    }

    return false;
}

bool DispatchCalRule5(CjPilotInfo* pilot, MySql* psql)
{
    if(pilot->m_nStatus < 5 &&
            pilot->m_nPilotType == 3 &&
            pilot->m_lAPDT > 0 && time(NULL) > pilot->m_lAPDT
      )
    {
        g_DispatchSvc::instance()->makePilotEnd(pilot, psql);
        return true;
    }

    return false;
}

bool DispatchCalExpired(CjPilotInfo* pilot, MySql* psql)
{
    uint32 u4ExpiredTime = g_MainConfig::instance()->GetExpiredTime();
    if(u4ExpiredTime <= 0)
        u4ExpiredTime = 3*24*3600;

    if(pilot->m_nStatus <= 4 && time(NULL) - pilot->m_lAPDT >= u4ExpiredTime)
    {
        g_DispatchSvc::instance()->makePilotExpired(pilot, psql);
        return true;
    }

    return false;
}

void BroadcastMsg(uint16 cmd, const std::string& msg)
{
    MapUserPilot& users = g_DispatchSvc::instance()->GetUsers();

    MapUserPilot::iterator iter;
    for(iter=users.begin(); iter!=users.end(); iter++)
        g_ClientSocketMgr::instance()->SendPacket(iter->first, 0, cmd, msg.c_str(), msg.length());
}

DispatchCal::DispatchCal()
{

}

DispatchCal::~DispatchCal()
{

}

bool DispatchCal::Start()
{
	/*
    uint32 interval = g_MainConfig::instance()->GetPilotCalTime();
    if(interval <= 0)
        interval = 300;

    int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
    if(timerId <= 0)
        return false;

	*/

	DEBUG_LOG("[DispatchCal::Start] OK ===================");
    return true;
}

int DispatchCal::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
    ACE_Recursive_Thread_Mutex& m_ThreadLock = g_DispatchSvc::instance()->GetThreadLock();
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadLock);

    MapPilot& m_mapPilot = g_DispatchSvc::instance()->GetPilots();
    MapPilot::iterator pilotIter;

    SYSTEM_LOG("[DispatchCal::handle_timeout] begin ===========================, total pilot ships:%d", m_mapPilot.size());

    int nExpired = 0;
    int nStart = 0;
    int nEnd = 0;
    MySql* psql = CREATE_MYSQL;

    //������ڵ�����
    for(pilotIter = m_mapPilot.begin(); pilotIter != m_mapPilot.end(); pilotIter++)
    {
        if(DispatchCalExpired(&(pilotIter->second), psql))
            nExpired++;
    }

    MapMmsiPilot m_mapMmsi;
    MapMmsiPilot::iterator mmsiIter;
    MapPilotToMmsi(m_mapPilot, m_mapMmsi);
    if(m_mapMmsi.empty())
    {
        SYSTEM_LOG("[DispatchCal::handle_timeout] end, makeExpired:%d, makeStart:%d, makeEnd:%d ===========================", nExpired, nStart, nEnd);
        RELEASE_MYSQL_RETURN(psql, 0);
    }

    int mmsi;
    long tt;
    int nav_status;
    int sog;
    double lon;
    double lat;
    long currTime = (long)time(NULL);

    for(mmsiIter = m_mapMmsi.begin(); mmsiIter != m_mapMmsi.end(); mmsiIter++)
    {
		mmsi = mmsiIter->first;
		AisRealtime* pAis = g_VesselSvc::instance()->aisShip.FindAis(mmsi);
		if(!pAis || currTime-pAis->time >= 12*3600)
			continue;

		tt = pAis->time;
		nav_status = pAis->nav_status;
		sog = pAis->sog;
		lon = pAis->lon;
		lat = pAis->lat;

        bool bCalStatus = true;

        int nPreAisStatus = -1;
        if(mmsiIter->second->m_nAisStatus == -1) //��һ��
        {
            mmsiIter->second->m_nAisStatus = nav_status;
            mmsiIter->second->m_lAisTime = tt;
            mmsiIter->second->m_dbAisSpd = sog / 10.0;
            mmsiIter->second->m_dbAisLon = lon;
            mmsiIter->second->m_dbAisLat = lat;
        }
        else if(mmsiIter->second->m_lAisTime == tt)     //����δ����
        {
            bCalStatus = false;
        }
        else
        {
            nPreAisStatus = mmsiIter->second->m_nAisStatus;
            mmsiIter->second->m_nAisStatus = nav_status;
            mmsiIter->second->m_lAisTime = tt;
            mmsiIter->second->m_dbAisSpd = sog / 10.0;
            mmsiIter->second->m_dbAisLon = lon;
            mmsiIter->second->m_dbAisLat = lat;
        }

        int nAlert = AlertRule(mmsiIter->second);
        if(nAlert > 0)
        {
            double dist, degree;
            PortInfo* pPort = g_PortSvc::instance()->FindNearestPort(mmsiIter->second->m_dbAisLon, mmsiIter->second->m_dbAisLat, dist, degree, true);
            char pushmsg[1024];
            snprintf(pushmsg,sizeof(pushmsg),"{tid:\"%d%ld\",mm:\"%d\",ev:\"%d\",lat:\"%f\",lon:\"%f\",dt:\"%d\",st:\"%d\",des:\"%s\",aid:\"%s\",area:\"%s\",pn:\"%s\",pid:\"%d\",dg:\"%d\",dis:\"%d\",achpn:\"%s\"}",
                     mmsiIter->first,mmsiIter->second->m_lAisTime, mmsiIter->first, (nAlert==1?78:79), mmsiIter->second->m_dbAisLat, mmsiIter->second->m_dbAisLon, mmsiIter->second->m_lAisTime, mmsiIter->second->m_nAisStatus,
                     "", "", "", pPort->portname.c_str(), pPort->portid, degree, dist, "");
            SYSTEM_LOG("[DispatchCal::handle_timeout] JinJiang/ChuJiang Alert:%s.=====================", pushmsg);
            BroadcastMsg(0xFE01, pushmsg);
        }

        if(bCalStatus)
        {
            if(DispatchCalRule1(mmsiIter->second, nPreAisStatus, psql))
                nStart++;
            else if(DispatchCalRule2(mmsiIter->second, nPreAisStatus, psql))
                nEnd++;
            else if(DispatchCalRule3(mmsiIter->second, psql))
                nStart++;
            else if(DispatchCalRule4(mmsiIter->second, psql))
                nEnd++;
            else if(DispatchCalRule5(mmsiIter->second, psql))
                nEnd++;
        }
    }

    SYSTEM_LOG("[DispatchCal::handle_timeout] end, makeExpired:%d, makeStart:%d, makeEnd:%d ===========================", nExpired, nStart, nEnd);
    RELEASE_MYSQL_RETURN(psql, 0);
}

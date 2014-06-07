#ifndef __SCHEDULE_SVC_H__
#define __SCHEDULE_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include "Util.h"

struct TCarrier
{
    char	m_szCarrierSrvId[64];
    char	m_szCarrierSrvNameCn[64];
    char	m_szCarrierSrvNameEn[64];
    char	m_szLineNameCn[64];
    char	m_szLineNameEn[64];

    char	m_szCarrierId[64];
    char	m_szCarrierName[64]; // en-cn
};
typedef std::map<std::string, TCarrier> MapCarrier;
typedef MapCarrier::iterator MapCarrierIter;

struct TSchedule;
struct TScheduleSection
{
    char	m_szVoyId[64];
    int		m_nSeq;
    char	m_szPortName[64];
    int		m_nPortId;
    char	m_szPortISO3[20];
    long	m_lPta;
    long	m_lAta;
    long	m_lPtd;
    long	m_lAtd;
    double	m_dbDist;
    double	m_dbDuration;

    long	m_lCalAta;
    long	m_lCalAtd;
    int		m_nCalLeftDist;
    int		m_nCalLeftTime;
    int		m_nMinIdx;

    TSchedule* pSche;

    std::string toJson()
    {
        char buff[1024];
        sprintf(buff, "{portId:\"%d\",portnm:\"%s\",iso3:\"%s\",seq:%d,pta:\"%s\",ptd:\"%s\",dis:\"%f\"}", m_nPortId, m_szPortName, m_szPortISO3, m_nSeq, TimeToLocalDate(m_lPta).c_str(), TimeToLocalDate(m_lPtd).c_str(), m_dbDist);
        return buff;
    }

    std::string toJson2()
    {
        char buff[1024];
        sprintf(buff, "{portId:\"%d\",portnm:\"%s\",seq:%d,ata:\"%d\",atd:\"%d\",udis:\"%d\",utime:\"%d\"}", m_nPortId, m_szPortName, m_nSeq, m_lCalAta, m_lCalAtd, m_nCalLeftDist, m_nCalLeftTime);
        return buff;
    }
};
typedef std::vector<TScheduleSection> VecScheduleSection;

struct TSchedule
{
    char	m_szVoyId[64];
    char	m_szVoyName[64];
    char	m_szCarrierId[64];
    char	m_szCarrierSrvId[64];
    char	m_szShipId[64];
    int		m_nMmsi;
    char	m_szShipName[64];
    int		m_nPortCount;
    double	m_dbDist;
    double	m_dbDuration;
    int		m_nDeptWkd;
    int		m_nDestWkd;
    long	m_lDeptPta;
    long	m_lDeptPtd;
    long	m_lDestPta;
    int		m_nStatus;
    char	m_szRmk[1024];

    VecScheduleSection	m_vecSection;
    IntIntMap			m_mapPortIdx;

    std::vector<TSchedule*> m_vecTransit;		 //中转(可以有多个)
    std::vector<int>		m_vecTransitPortId;  //中转港口ID(和上面字段对应)

    TSchedule() {}

    TSchedule* GetHowToPort(int portid, int& tportid)
    {
        tportid = 0;

        //直达
        if(m_mapPortIdx.find(portid) != m_mapPortIdx.end())
            return this;

        //中转
        for(int i=0; i<(int)m_vecTransit.size(); i++)
        {
            if(m_vecTransit[i]->m_mapPortIdx.find(portid) != m_vecTransit[i]->m_mapPortIdx.end())
            {
                tportid = m_vecTransitPortId[i];
                return m_vecTransit[i];
            }
        }

        //到不了
        return NULL;
    }

    TScheduleSection* GetSection(int portid)
    {
        int tportid = 0;
        TSchedule* pSche = GetHowToPort(portid, tportid);
        if(pSche)
        {
            int idx = pSche->m_mapPortIdx[portid];
            return &(pSche->m_vecSection[idx]);
        }
        return NULL;
    }

    std::string toJson2(int dpid)
    {
        int tportid = 0;
        TSchedule* pTran = GetHowToPort(dpid, tportid);

        std::stringstream out;

        if(tportid) // 中转
        {
            out << "{voyid:\"" << m_szVoyId << "\",voynm:\"" << m_szVoyName << "\",shipid:\"" << m_szShipId << "\",shipnm:\"" << m_szShipName << "\",btrans:0,nextvoyId:\"" << pTran->m_szVoyId << "\",ports:[";
            for(int i=0; i<(int)m_vecSection.size(); i++)
            {
                if(i==0)
                    out << m_vecSection[i].toJson();
                else
                    out << "," << m_vecSection[i].toJson();
            }
            out << "]}";
            out << ",";
            out << "{voyid:\"" << pTran->m_szVoyId << "\",voynm:\"" << pTran->m_szVoyName << "\",shipid:\"" << pTran->m_szShipId << "\",shipnm:\"" << pTran->m_szShipName << "\",btrans:1,nextvoyId:\"\",ports:[";
            for(int i=0; i<(int)pTran->m_vecSection.size(); i++)
            {
                if(i==0)
                    out << pTran->m_vecSection[i].toJson();
                else
                    out << "," << pTran->m_vecSection[i].toJson();
            }
            out << "]}";
        }
        else
        {
            out << "{voyid:\"" << m_szVoyId << "\",voynm:\"" << m_szVoyName << "\",shipid:\"" << m_szShipId << "\",shipnm:\"" << m_szShipName << "\",btrans:0,nextvoyId:\"\",ports:[";
            for(int i=0; i<(int)m_vecSection.size(); i++)
            {
                if(i==0)
                    out << m_vecSection[i].toJson();
                else
                    out << "," << m_vecSection[i].toJson();
            }
            out << "]}";
        }


        return out.str();
    }

    std::string toJson3();
};

typedef std::vector<TSchedule*> VecSchedule;

typedef std::map<std::string, TSchedule*> MapSchedule;
typedef MapSchedule::iterator MapScheduleIter;

typedef std::multimap<int, TSchedule*> MapPortSchedule;
typedef MapPortSchedule::iterator MapPortScheduleIter;
typedef std::pair<MapPortScheduleIter, MapPortScheduleIter> MapPortScheduleRange;

typedef std::multimap<std::string, TSchedule*> MapCarrierSchedule;
typedef MapCarrierSchedule::iterator MapCarrierScheduleIter;
typedef std::pair<MapCarrierScheduleIter, MapCarrierScheduleIter> MapCarrierScheduleRange;

typedef std::multimap<std::string, TSchedule*> MapShipSchedule;
typedef MapShipSchedule::iterator MapShipScheduleIter;
typedef std::pair<MapShipScheduleIter, MapShipScheduleIter> MapShipScheduleRange;

struct TPrice
{
    char	m_szPriceID[64];
    char	m_szLineID[64];
    int		m_nDeptPort;
    int		m_nDestPort;
    char	m_szDeptPortName[64];
    char	m_szDestPortName[64];
    char	m_szPrice[128];
    int		m_nBoxType;
    int		m_nDay;
    char	m_szCarrierID[64];
    char	m_szCarrierName[128];

    std::string toJson()
    {
        char json[2048];
        sprintf(json, "{pid:\"%s\",lineId:\"%s\",lptId:\"%d\",dptId:\"%d\",lptnm:\"%s\",dptnm:\"%s\",boxtp:\"%d\",day:\"%d\",price:\"%s\",carrierid:\"%s\",carrier:\"%s\"}",
                m_szPriceID, m_szLineID, m_nDeptPort, m_nDestPort, m_szDeptPortName, m_szDestPortName, m_nBoxType, m_nDay, m_szPrice, m_szCarrierID, m_szCarrierName);
        return json;
    }
};

typedef std::vector<TPrice> VecPrice;

struct TFreight
{
    char		m_szFreightID[64];
    char		m_szFreightName[128];
    char		m_szFreightCatID[64];
    long		m_lCreateTime;
    long		m_lValidTime;
    int			m_nPayType;
    int			m_nBillType;
    char		m_szLimitMt[128];
    int			m_nMinTeu;
    char		m_szRemark[1024];
    int			m_nRmkNum;
    int			m_nFlag, m_nPause, m_nTrash;

    VecPrice	m_vecPrice;

    std::string toJson()
    {
        std::stringstream out;
        out << "{fi:\"" << m_szFreightID << "\",fn:\"" << m_szFreightName << "\",catid:\"" << m_szFreightCatID << "\",validdt:" << m_lValidTime << ",paytp:" << m_nPayType << ",bill:" << m_nBillType << ",lwt:\"" << m_szLimitMt << "\",min:" << m_nMinTeu << ",des:\"" << m_szRemark << "\",fare:[";
        for(int i=0; i<(int)m_vecPrice.size(); i++)
        {
            if(i==0)
                out << m_vecPrice[i].toJson();
            else
                out << "," << m_vecPrice[i].toJson();
        }
        out << "]}";

        return out.str();
    }

    std::string toJson2()
    {
        char buff[1024];
        sprintf(buff, "{fi:\"%s\",fn:\"%s\",cdt:\"%d\",vdt:\"%d\",rn:%d,fg:%d,bp:%d,ts:%d}", m_szFreightID, m_szFreightName, m_lCreateTime, m_lValidTime, m_nRmkNum, m_nFlag, m_nPause, m_nTrash);
        return buff;
    }
};
typedef std::vector<TFreight> VecFreight;

struct TFreightCategory
{
    char		m_szCatId[64];
    char		m_szCatName[128];
    long		m_lCreate;
    int			m_nTrash;
    int			m_nRmkNum;
    VecFreight	m_vecFreight;

    std::string toJson()
    {
        std::stringstream out;
        out << "{ci:\"" << m_szCatId << "\",cn:\"" << m_szCatName << "\",cdt:\"" << m_lCreate << "\",ts:" << m_nTrash << ",rn:" << m_nRmkNum << ",fs:[";
        for(int i=0; i<(int)m_vecFreight.size(); i++)
        {
            if(i==0)
                out << m_vecFreight[i].toJson2();
            else
                out << "," << m_vecFreight[i].toJson2();
        }
        out << "]}";

        return out.str();
    }
};
typedef std::map<std::string, TFreightCategory> MapFreightCategory;
typedef MapFreightCategory::iterator MapFreightCatIter;

class ScheduleSvc : IBusinessService, ACE_Event_Handler
{
public:
    ScheduleSvc();
    ~ScheduleSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
    virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);
    TCarrier*    GetCarrierBySvrId(const std::string& svrId);

    DECLARE_SERVICE_MAP(ScheduleSvc)

private:
    int scheduleSearch(const char* pUid, const char* jsonString, std::stringstream& out);
    int carrierCompany(const char* pUid, const char* jsonString, std::stringstream& out);
    int scheduleLineDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int scheduleByVsl(const char* pUid, const char* jsonString, std::stringstream& out);
    int scheduleRealtime(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightPort(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightDetail(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightMy(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightPublish(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightMyCurr(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightUpdBase(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightUpdPrice(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightMyMgr(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightMgr(const char* pUid, const char* jsonString, std::stringstream& out);
    int freightChgCat(const char* pUid, const char* jsonString, std::stringstream& out);
    int voyCountForVsl(const char* pUid, const char* jsonString, std::stringstream& out);

private:
    bool loadCarrier();
    bool loadSchedule();
    bool checkConnect();

    MapCarrier					m_mapCarrier;
    MapSchedule					m_mapSchedule;
    MapPortSchedule				m_mapPortSchedule;
    MapCarrierSchedule			m_mapCarrierSchedule;
    MapShipSchedule				m_mapShipSchedule;
    long						m_lastLoadTime;

    ACE_SOCK_Connector			m_Connector;
    ACE_SOCK_Stream				m_TrackPeer;
    bool						m_bTrackConnected;
    ACE_SOCK_Stream				m_VoyPeer;
    bool						m_bVoyConnected;
    ACE_Thread_Mutex			m_SockLock;

private:
    //ACE_RW_Thread_Mutex			m_ThreadLock;      //线程锁
};

typedef ACE_Singleton<ScheduleSvc, ACE_Null_Mutex> g_ScheduleSvc;

#endif

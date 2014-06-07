#ifndef _MAINCONFIG_H
#define _MAINCONFIG_H

#include "AppConfig.h"

class CMainConfig
{
public:
    CMainConfig(void);
    ~CMainConfig(void);

    bool Init(const char* szConfigPath);
    
    const char* GetAddr();
    uint16      GetPort();
    const char*	GetModulePath();
    bool		GetTcpNodelay();
    int         GetNetworkThreads();

    uint16		GetThreadCount();
    uint16		GetThreadTimeOut();
    uint16		GetThreadTimeCheck();

    uint8		GetCacheSupport();
    uint16		GetCacheCheckTime();

	const char* GetGeoipPath()
	{
		return m_sGeoIp_path;
	}
	const char* GetPinyinPath()
	{
		return m_sPinyinPath;
	}

    bool		GetMainEnable()
    {
        return m_bMainEnable;
    }

    bool		GetAccountEnable()
    {
        return m_bAccountEnable;
    }

	bool		GetEquipmentEnable()
    {
        return m_bEquipmentEnable;
    }

	bool		GetBerthEnable()
    {
        return m_bBerthEnable;
    }


	bool        GetOilSpillEnable()
	{
		return m_bOilSpillEnable;
	}

    bool		GetAdEnable()
    {
        return m_bAdEnable;
    }

    bool		GetMonitorEnable()
    {
        return m_bMonitorEnable;
    }

	bool		GetGpsEnable()
    {
        return m_bGpsEnable;
    }

    bool		GetVesselEnable()
    {
        return m_bVesselEnable;
    }

    bool		GetPortEnable()
    {
        return m_bPortEnable;
    }

    bool		GetFleetEnable()
    {
        return m_bFleetEnable;
    }
	bool		GetEmopMaterialsEnable()
    {
        return m_bEmopMaterialsEnable;
    }
	bool		GetDFCalEnable()
	{	
		return m_bDFCalEnable;
	}
    uint32		GetCjDFCalTime()
    {
        return m_u4CjDFCalTime;
    }

    // Dispatch config
    bool		GetDispatchEnable()
    {
        return m_bDispatchEnable;
    }
    uint32		GetPilotFreshTime()
    {
        return m_u4PilotFreshTime;
    }
    uint32		GetPilotCalTime()
    {
        return m_u4PilotCalTime;
    }
    uint32		GetExpiredTime()
    {
        return m_u4ExpiredTime;
    }

    // SmsMail config
    bool		GetSmsEnable()
    {
        return m_bSmsEnable;
    }
    uint16		GetSmsReplyTimeout()
    {
        return m_u2SmsReplyTimeout;
    }
    bool		GetSmsEnableReply()
    {
        return m_bSmsEnableReply;
    }

    // Voyage config
    bool		GetVoyageEnable()
    {
        return m_bVoyageEnable;
    }
    ACE_INET_Addr&  GetVoyAddr()
    {
        return m_addrVoy;
    }

	// Schedule config
    bool		GetScheduleEnable()
    {
        return m_bScheduleEnable;
    }
    uint32		GetScheduleFreshTime()
    {
        return m_u4ScheduleFreshTime;
    }
    ACE_INET_Addr&	GetTrackAddr()
    {
        return m_addrTrack;
    }

	ACE_INET_Addr&	GetTrackAddr_udp()
	{
		return m_addrTrack_udp;
	}

    //Phone config
    bool GetPhoneEnable()
    {
        return m_bPhoneEnable;
    }
    const char* GetPhoneHost()
    {
        return m_sPhoneHost;
    }
	const char* GetPhoneOther()
	{
		return m_sPhoneOther;
	}
    uint16    GetPhonePort()
    {
        return m_uPhonePort;
    }
    double     GetPhoneFree()
    {
        return m_dbPhoneFree;
    }

	//Mobile config
    bool GetMobileEnable()
    {
        return m_bMobileEnable;
    }
	uint16 GetFreeSms()
	{
		return m_uFreeSms;
	}

	//AIS config
	bool GetAisEnable()
	{
		return m_bAisEnable;
	}

	//Trace config
    bool GetTraceEnable()
    {
        return m_bTraceEnable;
    }

	//Area config
	bool GetAreaEnable()
	{
		return m_bAreaEnable;
	}

    //Weather config
    bool GetWeatherEnable()
    {
        return m_bWeatherEnable;
    }
   
	//Company config
    bool   GetCompanyEnable()
    {
        return m_bCompanyEnable;
    }

	//Messenger config
    bool   GetMessengerEnable()
    {
        return m_bMessengerEnable;
    }

	//Family config
	bool	GetFamilyNotify()
	{
		return m_bFamilyNotify;
	}
	int		GetFamilyInterval()
	{
		return m_nFamilyInterval;
	}

private:
    CAppConfig m_AppConfig;

    char       m_szAddr[MAX_BUFF_128];
    uint16     m_u2Port;
    char       m_szModulePath[MAX_BUFF_128];
    bool	   m_bTcpNodelay;
    int		   m_nNetworkThreads;
	uint16     m_u2ThreadCount;

    char       m_szDBHost[MAX_BUFF_128];
	char       m_szDBHost_bj[MAX_BUFF_128];
    uint16     m_u2DBPort;
    char       m_szDBDB[MAX_BUFF_128];
    char       m_szDBUser[MAX_BUFF_128];
    char       m_szDBPwd[MAX_BUFF_128];

    char       m_sGeoIp_path[MAX_BUFF_1024];
	char       m_sPinyinPath[MAX_BUFF_1024];

    uint8      m_u1CacheSupport;
    uint16     m_u2CacheCheckTime;

    //我的账户
    bool	   m_bAccountEnable;
	
	//船舶配置
	bool	   m_bEquipmentEnable;

	//泊位信息
	bool	   m_bBerthEnable;

	//溢油
	bool       m_bOilSpillEnable;

    //广告
    bool	   m_bAdEnable;

    //长江调派
    bool	   m_bDispatchEnable;
    uint32	   m_u4PilotFreshTime;
    uint32	   m_u4PilotCalTime;
    uint32	   m_u4ExpiredTime;

    //船队
    bool	   m_bFleetEnable;
	bool	   m_bDFCalEnable;
    uint32	   m_u4CjDFCalTime;

    //主服务
    bool	   m_bMainEnable;

    //短信、邮件
    bool	   m_bSmsEnable;
    uint16	   m_u2SmsReplyTimeout;
    bool	   m_bSmsEnableReply;

    //船舶
    bool	   m_bVesselEnable;

    //港口
    bool	   m_bPortEnable;

    //航线
    bool	   m_bVoyageEnable;
    char	   m_szVoyHost[MAX_BUFF_128];
    int		   m_nVoyPort;
    ACE_INET_Addr m_addrVoy;

    //船期和运价
    bool	   m_bScheduleEnable;
    uint32	   m_u4ScheduleFreshTime;
    char	   m_szTrackHost[MAX_BUFF_128];
    int		   m_nTrackPort;
    ACE_INET_Addr m_addrTrack;
	
    //视频监控
    bool	   m_bMonitorEnable;

	//Gps及传感器
    bool	   m_bGpsEnable;

    //IP电话服务
    bool	   m_bPhoneEnable;
    char       m_sPhoneHost[MAX_BUFF_128];
	char	   m_sPhoneOther[MAX_BUFF_1024];
    uint16     m_uPhonePort;
    double     m_dbPhoneFree;

    //天气服务
    bool m_bWeatherEnable;

    //手机服务
    bool	   m_bMobileEnable;
    uint16     m_uFreeSms;

	//定位及区域服务
	bool m_bAreaEnable;

	//AIS服务
	bool m_bAisEnable;
	char m_szTrackHost_udp[MAX_BUFF_128];
	int	 m_nTrackPort_udp;
	ACE_INET_Addr m_addrTrack_udp;

	//轨迹服务
    bool m_bTraceEnable;

	//公司服务
    bool m_bCompanyEnable;

	//IM服务
    bool m_bMessengerEnable;

	//Family短信通知
	bool m_bFamilyNotify;
	int  m_nFamilyInterval;
	// emop
	bool m_bEmopMaterialsEnable;
};

typedef ACE_Singleton<CMainConfig, ACE_Null_Mutex> g_MainConfig;

#endif

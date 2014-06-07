#include "MainConfig.h"
#include "Util.h"

const char* g_szDBHost;
int			g_nDBPort;
const char* g_szDBDB;
const char* g_szDBUser;
const char* g_szDBPwd;
const char* g_szDBHost_bj;

CMainConfig::CMainConfig(void)
{
    m_szModulePath[0]		= '\0';
    m_bTcpNodelay           = true;
    m_nNetworkThreads       = 1;
	m_u2ThreadCount			= 4;

    m_szDBHost[0]			= '\0';
    m_u2DBPort              = 0;
    m_szDBDB[0]             = '\0';
    m_szDBUser[0]           = '\0';
    m_szDBPwd[0]            = '\0';

    m_u1CacheSupport        = 0;
    m_u2CacheCheckTime      = 0;

    m_bAccountEnable		= false;

	m_bEquipmentEnable		= false;

	m_bBerthEnable		= false;
	m_bOilSpillEnable   =false;

    m_bAdEnable				= false;

    m_bDispatchEnable		= false;
    m_u4PilotFreshTime		= 0;
    m_u4PilotCalTime		= 0;
    m_u4ExpiredTime			= 0;

    m_bFleetEnable			= false;
	m_bDFCalEnable			= false;
    m_u4CjDFCalTime			= 0;

    m_bMainEnable			= false;

    m_bSmsEnable			= false;
    m_u2SmsReplyTimeout		= 0;
    m_bSmsEnableReply		= 0;

    m_bVesselEnable			= false;

    m_bPortEnable			= false;

    m_bVoyageEnable			= false;

    m_bScheduleEnable		= false;
    m_u4ScheduleFreshTime	= 0;

    m_bMonitorEnable		= false;
	m_bGpsEnable			= false;

    m_bPhoneEnable			= false;
    m_sPhoneHost[0]			= '\0';
	m_sPhoneOther[0]		= '\0';
    m_uPhonePort			= 0;
    m_dbPhoneFree			= 0;

    m_bWeatherEnable		= false;

	m_bFamilyNotify			= false;
	m_nFamilyInterval		= 0;

	m_bEmopMaterialsEnable=false;
}

CMainConfig::~CMainConfig(void)
{

}

bool CMainConfig::Init(const char* szConfigPath)
{
    if(!m_AppConfig.ReadConfig(szConfigPath))
    {
        return false;
    }

    ACE_TString strValue;

    //获得数据库信息
    m_AppConfig.GetValue("ServerIP", strValue, "\\MYSQL");
    snprintf(m_szDBHost, MAX_BUFF_128, "%s", strValue.c_str());
    m_AppConfig.GetValue("ServerPort", strValue, "\\MYSQL");
    m_u2DBPort = ACE_OS::atoi((char*)strValue.c_str());
    m_AppConfig.GetValue("Database", strValue, "\\MYSQL");
    snprintf(m_szDBDB, MAX_BUFF_128, "%s", strValue.c_str());
    m_AppConfig.GetValue("Username", strValue, "\\MYSQL");
    snprintf(m_szDBUser, MAX_BUFF_128, "%s", strValue.c_str());
    m_AppConfig.GetValue("Password", strValue, "\\MYSQL");
    snprintf(m_szDBPwd, MAX_BUFF_128, "%s", strValue.c_str());

    m_AppConfig.GetValue("ServerIP_BJ", strValue, "\\MYSQL");
    snprintf(m_szDBHost_bj, MAX_BUFF_128, "%s", strValue.c_str());

    g_szDBHost	= m_szDBHost;
    g_nDBPort	= m_u2DBPort;
    g_szDBDB	= m_szDBDB;
    g_szDBUser	= m_szDBUser;
    g_szDBPwd	= m_szDBPwd;

    g_szDBHost_bj = m_szDBHost_bj;

	//数据文件路径
	m_AppConfig.GetValue("Path", strValue, "\\GEOIP");
	snprintf(m_sGeoIp_path, MAX_BUFF_1024, strValue.c_str());
	m_AppConfig.GetValue("Path", strValue, "\\PINYIN");
	snprintf(m_sPinyinPath, MAX_BUFF_1024, strValue.c_str());

    //获得服务器信息
    m_AppConfig.GetValue("ServerIP", strValue, "\\SERVER");
    snprintf(m_szAddr, MAX_BUFF_128, "%s", strValue.c_str());
    m_AppConfig.GetValue("ServerPort", strValue, "\\SERVER");
    m_u2Port = ACE_OS::atoi((char*)strValue.c_str());
    m_AppConfig.GetValue("ModulePath", strValue, "\\SERVER");
    snprintf(m_szModulePath, MAX_BUFF_128, "%s", strValue.c_str());
    m_AppConfig.GetValue("TcpNodelay", strValue, "\\SERVER");
    m_bTcpNodelay = (atoi(strValue.c_str())!=0);
    m_AppConfig.GetValue("NetworkThreads", strValue, "\\SERVER");
    m_nNetworkThreads = atoi(strValue.c_str());
	m_AppConfig.GetValue("ThreadCount", strValue, "\\SERVER");
	m_u2ThreadCount = (uint16)ACE_OS::atoi((char*)strValue.c_str());

    //获取缓存参数
    m_AppConfig.GetValue("CacheSupport", strValue, "\\SERVER");
    m_u1CacheSupport = (uint8)ACE_OS::atoi((char*)strValue.c_str());
    m_AppConfig.GetValue("CacheCheckTime", strValue, "\\SERVER");
    m_u2CacheCheckTime = (uint16)ACE_OS::atoi((char*)strValue.c_str());

    //我的账户参数
    m_AppConfig.GetValue("Enable", strValue, "\\ACCOUNT");
    m_bAccountEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

	//船舶配置
    m_AppConfig.GetValue("Enable", strValue, "\\EQUIPMENT");
    m_bEquipmentEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

	//泊位信息
    m_AppConfig.GetValue("Enable", strValue, "\\BERTH");
    m_bBerthEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));


	//溢油
	m_AppConfig.GetValue("Enable", strValue, "\\OILSPILL");
	m_bOilSpillEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

	
    //广告参数
    m_AppConfig.GetValue("Enable", strValue, "\\AD");
    m_bAdEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

    //调派参数
    m_AppConfig.GetValue("Enable", strValue, "\\DISPATCH");
    m_bDispatchEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));
    m_AppConfig.GetValue("PilotFreshTime", strValue, "\\DISPATCH");
    m_u4PilotFreshTime = (uint32)ACE_OS::atoi((char*)strValue.c_str());
    m_AppConfig.GetValue("PilotCalTime", strValue, "\\DISPATCH");
    m_u4PilotCalTime = (uint32)ACE_OS::atoi((char*)strValue.c_str());
    m_AppConfig.GetValue("ExpiredTime", strValue, "\\DISPATCH");
    m_u4ExpiredTime = (uint32)ACE_OS::atoi((char*)strValue.c_str());

    //船队参数
    m_AppConfig.GetValue("Enable", strValue, "\\FLEET");
    m_bFleetEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));
	m_AppConfig.GetValue("DFCal", strValue, "\\FLEET");
	m_bDFCalEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));
    m_AppConfig.GetValue("CjDFCalTime", strValue, "\\FLEET");
    m_u4CjDFCalTime = (uint32)ACE_OS::atoi((char*)strValue.c_str());

    //主服务参数
    m_AppConfig.GetValue("Enable", strValue, "\\MAIN");
    m_bMainEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

    //短信、邮件参数
    m_AppConfig.GetValue("Enable", strValue, "\\SMS");
    m_bSmsEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));
    m_AppConfig.GetValue("BYTimeout", strValue, "\\SMS");
    m_u2SmsReplyTimeout = ACE_OS::atoi((char*)strValue.c_str());
    m_AppConfig.GetValue("EnableReply", strValue, "\\SMS");
    m_bSmsEnableReply = (0 != ACE_OS::atoi((char*)strValue.c_str()));

    //船舶参数
    m_AppConfig.GetValue("Enable", strValue, "\\VESSEL");
    m_bVesselEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

    //港口参数
    m_AppConfig.GetValue("Enable", strValue, "\\PORT");
    m_bPortEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

    //航线计算参数
    m_AppConfig.GetValue("Enable", strValue, "\\VOYAGE");
    m_bVoyageEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));
    m_AppConfig.GetValue("VoyHost", strValue, "\\VOYAGE");
    snprintf(m_szVoyHost, MAX_BUFF_128, strValue.c_str());
    m_AppConfig.GetValue("VoyPort", strValue, "\\VOYAGE");
    m_nVoyPort = atoi((char*)strValue.c_str());
    m_addrVoy = ACE_INET_Addr(m_nVoyPort, m_szVoyHost);

    //船期运价参数
    m_AppConfig.GetValue("Enable", strValue, "\\SCHEDULE");
    m_bScheduleEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));
    m_AppConfig.GetValue("ScheduleFreshTime", strValue, "\\SCHEDULE");
    m_u4ScheduleFreshTime = ACE_OS::atoi((char*)strValue.c_str());
    m_AppConfig.GetValue("TrackHost", strValue, "\\SCHEDULE");
    snprintf(m_szTrackHost, MAX_BUFF_128, strValue.c_str());
    m_AppConfig.GetValue("TrackPort", strValue, "\\SCHEDULE");
    m_nTrackPort = atoi((char*)strValue.c_str());
    m_addrTrack = ACE_INET_Addr(m_nTrackPort, m_szTrackHost);

    //视频监控参数
    m_AppConfig.GetValue("Enable", strValue, "\\MONITOR");
    m_bMonitorEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

	//Gps及传感器参数
    m_AppConfig.GetValue("Enable", strValue, "\\GPS");
    m_bGpsEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

    //电话服务参数
    m_AppConfig.GetValue("Enable", strValue, "\\PHONE");
    m_bPhoneEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));
    m_AppConfig.GetValue("PhoneIP", strValue, "\\PHONE");
    snprintf(m_sPhoneHost, MAX_BUFF_128, "%s", strValue.c_str());
	m_AppConfig.GetValue("PhoneOther", strValue, "\\PHONE");
	snprintf(m_sPhoneOther, MAX_BUFF_1024, "%s", strValue.c_str());
    m_AppConfig.GetValue("PhonePort", strValue, "\\PHONE");
    m_uPhonePort = ACE_OS::atoi((char*)strValue.c_str());
    m_AppConfig.GetValue("PhoneFree", strValue, "\\PHONE");
    m_dbPhoneFree = ACE_OS::atof((char*)strValue.c_str());

    //天气服务
    m_AppConfig.GetValue("Enable", strValue, "\\WEATHER");
    m_bWeatherEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

    //手机绑定
    m_AppConfig.GetValue("Enable", strValue, "\\MOBILE");
    m_bMobileEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));
    m_AppConfig.GetValue("FreeSms", strValue, "\\MOBILE");
    m_uFreeSms = ACE_OS::atoi((char*)strValue.c_str());

	//AIS服务
	m_AppConfig.GetValue("Enable", strValue, "\\AIS");
	m_bAisEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));
	m_AppConfig.GetValue("TrackHostUdp", strValue, "\\AIS");
	snprintf(m_szTrackHost_udp, MAX_BUFF_128, strValue.c_str());
	m_AppConfig.GetValue("TrackPortUdp", strValue, "\\AIS");
	m_nTrackPort_udp = atoi((char*)strValue.c_str());
	m_addrTrack_udp = ACE_INET_Addr(m_nTrackPort_udp, m_szTrackHost_udp);

	//轨迹服务
    m_AppConfig.GetValue("Enable", strValue, "\\TRACE");
    m_bTraceEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

	//定位及区域服务
	m_AppConfig.GetValue("Enable", strValue, "\\AREA");
	m_bAreaEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

	//公司服务
    m_AppConfig.GetValue("Enable", strValue, "\\COMPANY");
    m_bCompanyEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

	//IM服务
    m_AppConfig.GetValue("Enable", strValue, "\\MESSENGER");
    m_bMessengerEnable = (0 != ACE_OS::atoi((char*)strValue.c_str()));

	//亲情版短信通知
	m_AppConfig.GetValue("Enable", strValue, "\\FAMILY");
	m_bFamilyNotify = (0 != ACE_OS::atoi((char*)strValue.c_str()));
	m_AppConfig.GetValue("Interval", strValue, "\\FAMILY");
	m_nFamilyInterval = ACE_OS::atoi((char*)strValue.c_str());
	// 临时设成true
	m_bEmopMaterialsEnable=true;

    return true;
}

const char* CMainConfig::GetAddr()
{
    return m_szAddr;
}

uint16 CMainConfig::GetPort()
{
    return m_u2Port;
}

const char* CMainConfig::GetModulePath()
{
    return m_szModulePath;
}

bool CMainConfig::GetTcpNodelay()
{
    return m_bTcpNodelay;
}

int CMainConfig::GetNetworkThreads()
{
    return m_nNetworkThreads;
}

uint16 CMainConfig::GetThreadCount()
{
    return m_u2ThreadCount;
}

uint8 CMainConfig::GetCacheSupport()
{
    return m_u1CacheSupport;
}

uint16 CMainConfig::GetCacheCheckTime()
{
    return m_u2CacheCheckTime;
}

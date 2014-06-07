#include "ServerMgr.h"
#include "MainConfig.h"
#include "LogMgr.h"
#include "Message.h"
#include "ObjectPool.h"
#include "ClientSocketMgr.h"
#include "MessageService.h"
#include "NotifyService.h"
#include "SigHandler.h"

#include "GeoipHandler.h"
#include "Pinyin4j.h"
#include "EquipmentSvc.h"
#include "BerthSvc.h"
#include "AccountSvc.h"
#include "AdSvc.h"
#include "DispatchSvc.h"
#include "FleetSvc.h"
#include "MainSvc.h"
#include "SmsMailSvc.h"
#include "VesselSvc.h"
#include "VoyageSvc.h"
#include "ScheduleSvc.h"
#include "PortSvc.h"
#include "MonitorSvc.h"
#include "GpsSvc.h"
#include "WeatherSvc.h"
#include "PhoneSvc.h"
#include "AisSvc.h"
#include "TraceSvc.h"
#include "CompanySvc.h"
#include "MessengerSvc.h"
#include "MobileSvc.h"
#include "AreaSvc.h"
#include "WebSvc.h"
#include "OilSpillSvc.h"
#include "eMOPMaterialsSvc.h"
#include "eMOPEquipSvc.h"
#include "eMOPUserMgrSvc.h"
#include "eMOPMaintenanceSvc.h"
#include "SequenceManager.h"
#include "eMopCommDataSvr.h" 
#include "eMOPRepairSvc.h" 
#include "eMOPSurveySvc.h" 
#include "SARSvc.h" 
#include "BDSvc.h"
#include "eMOPSMSSvc.h"

CServerMgr::CServerMgr(void)
{
}

CServerMgr::~CServerMgr(void)
{

	
}

bool CServerMgr::Init()
{
    bool bState = false;

    //初始化配置
    bState = g_MainConfig::instance()->Init("mmain.conf");
    if(!bState)
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Init] g_MainConfig.Init failed.\n"));
        return false;
    }

    //初始化日志
    if(!g_LogMgr::instance()->Init("mlogger.conf", g_MainConfig::instance()->GetModulePath()))
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Init]g_LogMgr.Init failed.\n"));
        return false;
    }

    //初始化对象池
    g_MessagePool::instance()->Init(DEFAULT_OBJECT_POOL_SIZE);
    g_DBConnPool::instance()->Init(10, 2);
	g_DBConnPoolBj::instance()->Init(3, 2);

    //初始化逻辑处理线程
    g_MessageService::instance()->Init(g_MainConfig::instance()->GetThreadCount());

    g_NotifyService::instance()->Init(4);

    return true;
}

bool CServerMgr::Start()
{
    //启动日志服务线程
    if(!g_LogMgr::instance()->Start())
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Start]g_LogMgr::instance()->Start() error.\n"));
        return false;
    }

    //注册信号量
    if(0 != g_SigHandler::instance()->RegisterSignal(ACE_Reactor::instance()))
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Start]RegisterSignal failed.\n"));
        return false;
    }

    //启动定时器
    if(0 != g_TimerManager::instance()->activate())
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Start]g_TimerManager::instance()->Start() is error.\n"));
        return false;
    }


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////      启动业务服务(全部的服务都在这里启动)      ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
	if(!g_GeoipHandler::instance()->Start(g_MainConfig::instance()->GetGeoipPath()))
	{
		DEBUG_LOG("[g_GeoipHandler::Start]g_GeoipHandler.Start failed.");
		return false;
	}
	if(!g_Pinyin4j::instance()->Start(g_MainConfig::instance()->GetPinyinPath()))
	{
		DEBUG_LOG("[g_Pinyin4j::Start]g_Pinyin4j.Start failed.");
		return false;
	}

	if(false == g_NotifyService::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_NotifyService.Start failed.");
		return false;
	}
	DEBUG_LOG("[NotifyService::Start] OK......................................");

	if(g_MainConfig::instance()->GetSmsEnable() && !g_SmsMailSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_SmsMailSvc.Start failed.");
		return false;
	}

	if(!g_WebSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_WebSvc.Start failed.");
		return false;
	}

	if(g_MainConfig::instance()->GetAccountEnable() && !g_AccountSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_AccountSvc.Start failed.");
		return false;
	}

	if(g_MainConfig::instance()->GetEquipmentEnable() && !g_EquipmentSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_EquipmentSvc.Start failed.");
		return false;
	}

	if(g_MainConfig::instance()->GetBerthEnable() && !g_BerthSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_BerthSvc.Start failed.");
		return false;
	}

	if(g_MainConfig::instance()->GetOilSpillEnable() && !g_OilSpillSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_OilSpillSvc.Start failed.");
		return false;
	}


	if(g_MainConfig::instance()->GetAdEnable() && !g_AdSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_AdSvc.Start failed.");
		return false;
	}
	
    if(g_MainConfig::instance()->GetPortEnable() && !g_PortSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_PortSvc.Start failed.");
        return false;
    }

	// 必须先启动
	if(g_MainConfig::instance()->GetVesselEnable() && !g_VesselSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_VesselSvc.Start failed.");
		return false;
	}
	
    if(g_MainConfig::instance()->GetDispatchEnable() && !g_DispatchSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_DispatchSvc.Start failed.");
        return false;
    }
	
    if(g_MainConfig::instance()->GetFleetEnable() && !g_FleetSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_FleetSvr.Start failed.");
        return false;
    }
	
    if(g_MainConfig::instance()->GetMainEnable() && !g_MainSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_MainSvc.Start failed.");
        return false;
    }
	
    if(g_MainConfig::instance()->GetVoyageEnable() && !g_VoyageSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_VoyageSvc.Start failed.");
        return false;
    }
	
    if(g_MainConfig::instance()->GetScheduleEnable() && !g_ScheduleSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_ScheduleSvc.Start failed.");
        return false;
    }
	
    if(g_MainConfig::instance()->GetMonitorEnable() && !g_MonitorSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_MonitorSvc.Start failed.");
        return false;
    }
	
	if(g_MainConfig::instance()->GetGpsEnable() && !g_GpsSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_GpsSvc.Start failed.");
        return false;
    }

    if(g_MainConfig::instance()->GetWeatherEnable() && !g_WeatherSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_WeatherSvc.Start failed.");
        return false;
    }
	
	if(g_MainConfig::instance()->GetPhoneEnable() && !g_PhoneSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_PhoneSvc.Start failed.");
		return false;
	}

    if(g_MainConfig::instance()->GetMobileEnable() && !g_MobileSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_MobileSvc.Start failed.");
        return false;
    }
    if(g_MainConfig::instance()->GetTraceEnable() && !g_TraceSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_TraceSvc.Start failed.");
        return false;
    }
    if(g_MainConfig::instance()->GetAisEnable() && !g_AisSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_AisSvc.Start failed.");
        return false;
    }
    if(g_MainConfig::instance()->GetCompanyEnable() && !g_CompanySvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_CompanySvc.Start failed.");
        return false;
    }
    if(g_MainConfig::instance()->GetMessengerEnable() && !g_MessageSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_CompanySvc.Start failed.");
        return false;
    }
	if(g_MainConfig::instance()->GetAreaEnable() && !g_AreaSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_AreaSvc.Start failed.");
		return false;
	}
	if(g_MainConfig::instance()->GetEmopMaterialsEnable() && !g_eMOPMaterialsSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_EmopMaterialsSvr.Start failed.");
        return false;
    }


	if (!g_Sequence::instance()->Init())
	{
		DEBUG_LOG("[CServerMgr::Start]g_Sequence Init failed.");
		return false;
	}

	if (!g_SARSvc::instance()->Start())
	{
		DEBUG_LOG("[SARSvc::Start]g_SARSvr.Start failed.");
		return false;
	}


	
	g_eMOPCommDataSvr::instance()->Init();


	if(!g_eMOPEquipSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_eMOPEquipSvr.Start failed.");
        return false;
    }
	if(!g_eMOPUserMgrSvc::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_eMOPUserMgrSvr.Start failed.");
        return false;
    }
	
	if (!g_eMOPMaintenanceSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPMaintenanceSvc.Start failed.");
		return false; 
	}

	if (!g_eMOPRepairSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPRepairSvc.Start failed.");
		return false; 
	}

	if (!g_eMOPSurveySvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPSurveySvc.Start failed.");
		return false; 
	}

	if(!g_eMOPSMSSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPSMSSvc.Start failed.");
		return false; 
	}

	if(!g_BDSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_BDSvc.Start failed.");
		return false; 
	}

 // 
  
////////////////////////////////////////////////////////////////////////////////////////////////////////

    //启动逻辑处理线程
    if(false == g_MessageService::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_MessageService.Start failed.");
        return false;
    }

    //启动TCP网络模块
    uint16 port = g_MainConfig::instance()->GetPort();
    std::string address = g_MainConfig::instance()->GetAddr();
    if(0 != g_ClientSocketMgr::instance()->StartNetwork(port, address))
    {
        DEBUG_LOG("[CServerMgr::Start]g_ClientSocketMgr.StartNetwork failed.");
        return false;
    }

    g_ClientSocketMgr::instance()->Wait();

    return true;
}

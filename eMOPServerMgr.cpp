#include "ServerMgr.h"
#include "MainConfig.h"
#include "LogMgr.h"
#include "Message.h"
#include "ObjectPool.h"
#include "ClientSocketMgr.h"
#include "MessageService.h"
#include "SigHandler.h"
#include "eMopCommDataSvr.h"
#include "eMOPEquipSvc.h"
#include "eMOPMaintenanceSvc.h"
#include "eMOPMaterialsSvc.h"
#include "eMOPRepairSvc.h"
#include "eMOPSMSSvc.h"
#include "eMOPSurveySvc.h"
#include "eMOPUserMgrSvc.h"

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
    g_DBConnPool::instance()->Init(2, 2);
	g_DBConnPoolBj::instance()->Init(2, 2);

    //初始化逻辑处理线程
    g_MessageService::instance()->Init(g_MainConfig::instance()->GetThreadCount());

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

	g_eMOPCommDataSvr::instance()->Init();

	if(!g_eMOPEquipSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPEquipSvc.Start failed.");
		return false; 
	}
	if(!g_eMOPMaintenanceSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPMaintenanceSvc.Start failed.");
		return false; 
	}
	if(!g_eMOPMaterialsSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPMaterialsSvc.Start failed.");
		return false; 
	}
	if(!g_eMOPRepairSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPRepairSvc.Start failed.");
		return false; 
	}
	if(!g_eMOPSMSSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPSMSSvc.Start failed.");
		return false; 
	}
	if(!g_eMOPSurveySvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPSurveySvc.Start failed.");
		return false; 
	}
	if(!g_eMOPUserMgrSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_eMOPUserMgrSvc.Start failed.");
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

#include "ServerMgr.h"
#include "MainConfig.h"
#include "LogMgr.h"
#include "Message.h"
#include "ObjectPool.h"
#include "ClientSocketMgr.h"
#include "MessageService.h"
#include "SigHandler.h"
#include "BDSvc.h"

CServerMgr::CServerMgr(void)
{
}

CServerMgr::~CServerMgr(void)
{

	
}

bool CServerMgr::Init()
{
    bool bState = false;

    //��ʼ������
    bState = g_MainConfig::instance()->Init("mmain.conf");
    if(!bState)
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Init] g_MainConfig.Init failed.\n"));
        return false;
    }

    //��ʼ����־
    if(!g_LogMgr::instance()->Init("mlogger.conf", g_MainConfig::instance()->GetModulePath()))
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Init]g_LogMgr.Init failed.\n"));
        return false;
    }

    //��ʼ�������
    g_MessagePool::instance()->Init(DEFAULT_OBJECT_POOL_SIZE);
    g_DBConnPool::instance()->Init(10, 2);
	g_DBConnPoolBj::instance()->Init(3, 2);

    //��ʼ���߼������߳�
    g_MessageService::instance()->Init(g_MainConfig::instance()->GetThreadCount());

    return true;
}

bool CServerMgr::Start()
{
    //������־�����߳�
    if(!g_LogMgr::instance()->Start())
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Start]g_LogMgr::instance()->Start() error.\n"));
        return false;
    }

    //ע���ź���
    if(0 != g_SigHandler::instance()->RegisterSignal(ACE_Reactor::instance()))
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Start]RegisterSignal failed.\n"));
        return false;
    }

    //������ʱ��
    if(0 != g_TimerManager::instance()->activate())
    {
        DEBUG_PRT((LM_ERROR, "[CServerMgr::Start]g_TimerManager::instance()->Start() is error.\n"));
        return false;
    }


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////      ����ҵ�����(ȫ���ķ�������������)      ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(!g_BDSvc::instance()->Start())
	{
		DEBUG_LOG("[CServerMgr::Start]g_BDSvc.Start failed.");
		return false; 
	}
 // 
  
////////////////////////////////////////////////////////////////////////////////////////////////////////

    //�����߼������߳�
    if(false == g_MessageService::instance()->Start())
    {
        DEBUG_LOG("[CServerMgr::Start]g_MessageService.Start failed.");
        return false;
    }

    //����TCP����ģ��
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

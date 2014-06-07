#include "MessageService.h"
#include "MainConfig.h"
#include "Message.h"
#include "LogMgr.h"
#include "CacheMgr.h"
#include "ClientSocketMgr.h"
#include "UniqueNumber.h"
#include "IBusinessService.h"
#include "ObjectPool.h"

CMessageService::CMessageService(void)
{

}

bool CMessageService::DoMessage(void* pMessage)
{
    CMessage* pMsg = reinterpret_cast<CMessage*>(pMessage);

    const char* szUid  = pMsg->GetMessageBase()->m_szUid;
    uint32 u4ConnectID = pMsg->GetMessageBase()->m_u4ConnectID;
    uint16 u2CommandID = pMsg->GetMessageBase()->m_u2Cmd;
    uint64 u8JobID     = pMsg->GetMessageBase()->m_u8JobID;

    uint16 mt, st;
    SplitCommandID(u2CommandID, mt, st);

    IterMapBusinessSvc iter = m_mapSvc.find(mt);
    if(iter == m_mapSvc.end())
    {
        DEBUG_LOG("[CMessageService::DoMessage] mid 0x%02x not found.", mt);
        pMsg->Clear();
        g_MessagePool::instance()->Delete(pMsg);
        g_ClientSocketMgr::instance()->SendPacket(u4ConnectID, u8JobID, u2CommandID, MSG_FAIL, ACE_OS::strlen(MSG_FAIL));
        return false;
    }

    bool bSucc = true;
    ACE_Message_Block* pRes = NULL;
    if(pMsg->GetMessageBody() != NULL)
        bSucc = iter->second->ExecuteCmd(szUid, st, pMsg->GetMessageBody()->rd_ptr()+8, pRes);
    else
        bSucc = iter->second->ExecuteCmd(szUid, st, NULL, pRes);

    if(pRes)
    {
        g_ClientSocketMgr::instance()->SendPacket(u4ConnectID, u8JobID, u2CommandID, pRes->rd_ptr(), pRes->length());
    }
    else if(!bSucc)
    {
        g_ClientSocketMgr::instance()->SendPacket(u4ConnectID, u8JobID, u2CommandID, MSG_FAIL, ACE_OS::strlen(MSG_FAIL));
    }

    pMsg->Clear();
    g_MessagePool::instance()->Delete(pMsg);
    SAFE_RELEASE(pRes);
    return bSucc;
}

uint16 CMessageService::GetMessageCmd(void* pMessage)
{
    CMessage* msg = reinterpret_cast<CMessage*>(pMessage);
    return msg->GetMessageBase()->m_u2Cmd;
}

bool CMessageService::RegisterCmd(uint16 u2Mid, IBusinessService* svc)
{
    IterMapBusinessSvc iter = m_mapSvc.find(u2Mid);
    if(iter != m_mapSvc.end())
    {
        DEBUG_LOG("[CMessageService::RegisterCmd] register mid failed: 0x%02x already registered.", u2Mid);
        return false;
    }

	svc->SetMid(u2Mid);
    m_mapSvc.insert(std::make_pair(u2Mid, svc));
    return true;
}

int CMessageService::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	CheckMessagePool();
	return 0;
}

void CMessageService::CheckMessagePool()
{
    //检查消息池使用情况
    SYSTEM_LOG("[MessagePool][Used:%d][Free:%d]", g_MessagePool::instance()->GetUsedCount(), g_MessagePool::instance()->GetFreeCount());
	SYSTEM_LOG("[DBPool][Used:%d][Free:%d]", g_DBConnPool::instance()->GetUsedCount(), g_DBConnPool::instance()->GetFreeCount());
	g_DBConnPool::instance()->ReduceTo(10);
	g_DBConnPoolBj::instance()->ReduceTo(3);
}

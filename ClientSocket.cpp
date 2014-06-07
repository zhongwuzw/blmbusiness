#include "ClientSocketMgr.h"
#include "MessageService.h"
#include "Message.h"
#include "Util.h"
#include "MainConfig.h"
#include "SmsMailSvc.h"
#include "ObjectPool.h"

ClientSocket::ClientSocket (void) :
    BaseSocket(),
    m_u4ConnectID(-1)
{
    m_u2HeadLen = MsgHeadSize;
    m_KeepAlive = false;
    m_SocketMgr = g_ClientSocketMgr::instance();
}

ClientSocket::~ClientSocket (void)
{

}

int ClientSocket::handle_input_header (void)
{
    ACE_ASSERT (m_Body == NULL);

    MsgHead* pHead = (MsgHead*)m_Header->rd_ptr();
    ACE_NEW_RETURN(m_Body, ACE_Message_Block(pHead->len), -1);

    return 0;
}

int ClientSocket::handle_input_payload (void)
{
    MsgHead* pHead = (MsgHead*)m_Header->rd_ptr();
    uint16 commandID = Combine(pHead->mt, pHead->st);
    int len = pHead->len;
    uint64 jobid = *(uint64*)m_Body->rd_ptr();
    const char* reqStr = m_Body->rd_ptr()+8;
    const char* uid = reqStr + strlen(reqStr) + 5;

    if(commandID == COMMAND_USER_IN)
    {
        Tokens users = StrSplit(reqStr, "|");

        if(!users.empty())
        {
            m_SocketMgr->OnUserLogin(users, this);
        }
    }
    else if(commandID == COMMAND_USER_OUT)
    {
        m_SocketMgr->OnUserLogout(reqStr);
    }
    else if(commandID == COMMAND_OF_LOGIN || commandID == COMMAND_OF_LOGINS)  //openfire登录,服务器如果需要用到这个信息自行处理
    {
        Tokens users = StrSplit(reqStr, "|");
        if(!users.empty() && g_MainConfig::instance()->GetSmsEnable() && g_MainConfig::instance()->GetSmsEnableReply())
        {
            
        }
    }
    else
    {
        CMessage* pMsg = g_MessagePool::instance()->Create();
        snprintf(pMsg->GetMessageBase()->m_szUid, sizeof(pMsg->GetMessageBase()->m_szUid)-1, uid);
        pMsg->GetMessageBase()->m_u4ConnectID = m_u4ConnectID;
        pMsg->GetMessageBase()->m_u8JobID = jobid;
        pMsg->GetMessageBase()->m_u2Cmd = commandID;
        pMsg->SetMessageBody(m_Body);
        g_MessageService::instance()->PutMessage(pMsg);

        m_Header->reset();
        m_Body = NULL;
        return 0;
    }

    m_Header->reset ();
    SAFE_RELEASE(m_Body);

    return 0;
}

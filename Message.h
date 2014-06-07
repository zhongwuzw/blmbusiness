#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "define.h"
#include "ObjectPool.h"

//记录消息的一些参数
struct _MessageBase
{
    char m_szUid[20];
    uint32 m_u4ConnectID;
    uint64 m_u8JobID;
    uint16 m_u2Cmd;
    ACE_Time_Value m_tvMsg;

    _MessageBase()
    {
        Clear();
    }

    void Clear()
    {
        memset(m_szUid, 0, sizeof(m_szUid));
        m_u4ConnectID	= 0;
        m_u8JobID	    = 0;
        m_u2Cmd			= 0;
        m_tvMsg		    = ACE_OS::gettimeofday();
    }
};

class CMessage
{
public:
    CMessage(void);
    ~CMessage(void);

    void Clear();

    _MessageBase* GetMessageBase();
    void SetMessageBody(ACE_Message_Block* pmbBody);
    ACE_Message_Block* GetMessageBody();

private:
    _MessageBase       m_Base;
    ACE_Message_Block* m_pmbBody;
};

typedef ObjectPool<CMessage> MessagePool;
typedef ACE_Singleton<MessagePool, ACE_Null_Mutex> g_MessagePool;

#endif

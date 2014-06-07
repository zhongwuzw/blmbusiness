#include "Message.h"

CMessage::CMessage(void) : m_pmbBody(NULL)
{

}

CMessage::~CMessage(void)
{
    Clear();
}

_MessageBase* CMessage::GetMessageBase()
{
    return &m_Base;
}

ACE_Message_Block* CMessage::GetMessageBody()
{
    return m_pmbBody;
}

void CMessage::SetMessageBody(ACE_Message_Block* pmbBody)
{
    m_pmbBody = pmbBody;
}

void CMessage::Clear()
{
    SAFE_RELEASE(m_pmbBody);
    m_Base.Clear();
}

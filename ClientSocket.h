#ifndef _CLIENT_SOCKET_H
#define _CLIENT_SOCKET_H

#include "BaseSocket.h"

//ÏûÏ¢Í·
struct MsgHead
{
	int len;
	short st;
	short mt;
	MsgHead() : len(0), st(0), mt(0) {}
	MsgHead(int mt1, int st1, int len1): len(len1), st((short)st1), mt((short)mt1) {}
};
#define MsgHeadSize sizeof(MsgHead)

class ClientSocket : public BaseSocket
{
public:
    /// Declare the acceptor for this class
    typedef ACE_Acceptor< ClientSocket, ACE_SOCK_ACCEPTOR > Acceptor;

public:
    ClientSocket (void);
    virtual ~ClientSocket (void);

    uint32 GetConnectID(void) const
    {
        return m_u4ConnectID;
    }
    void   SetConnectID(uint32 u2ConnectID)
    {
        m_u4ConnectID = u2ConnectID;
    }

    virtual int handle_input_header (void);
    virtual int handle_input_payload (void);

private:
    uint32 m_u4ConnectID;
};

#endif  /* _CLIENT_SOCKET_H */

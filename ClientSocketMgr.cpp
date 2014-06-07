#include "ClientSocket.h"
#include "ClientSocketMgr.h"
#include "ReactorRunnable.h"
#include "MainConfig.h"
#include "LogMgr.h"

ClientSocketMgr::ClientSocketMgr () :
    m_NetThreadsCount (0),
    m_NetThreads (0),
    m_SockOutKBuff (-1),
    m_SockOutUBuff (65536),
    m_UseNoDelay (true),
    m_u4CurrentID (0),
    m_Acceptor (0)
{
}

ClientSocketMgr::~ClientSocketMgr ()
{
    if (m_NetThreads)
        delete [] m_NetThreads;

    if(m_Acceptor)
        delete m_Acceptor;
}

int
ClientSocketMgr::StartReactiveIO (uint16 port, const char* address)
{
    m_UseNoDelay = true;
    int num_threads = 1;

    m_NetThreadsCount = static_cast<size_t> (num_threads + 1);
    m_NetThreads = new ReactorRunnable[m_NetThreadsCount];

    ClientSocket::Acceptor* acc = new ClientSocket::Acceptor;
    m_Acceptor = acc;

    ACE_INET_Addr listen_addr (port, address);
    if (acc->open (listen_addr, m_NetThreads[0].GetReactor (), ACE_NONBLOCK) == -1)
    {
        printf("[ClientSocketMgr::StartReactiveIO]Failed to open acceptor ,check if the port(%d) is free.\n", port);
        return -1;
    }

    for (size_t i = 0; i < m_NetThreadsCount; ++i)
    {
        std::stringstream threadName;
        threadName << "ClientSocketMgr Net" << (i+1);
        m_NetThreads[i].Start (threadName.str());
    }

/*
	if(g_MainConfig::instance()->GetFamilyNotify())
	{
		ACE_INET_Addr ais_udp_addr(6002);
		m_UdpSocket = new UdpSocket(ais_udp_addr);
		if(m_NetThreads[0].GetReactor()->register_handler(m_UdpSocket, ACE_Event_Handler::READ_MASK) == -1)
		{
			printf("[ClientSocketMgr::StartNetwork]Failed to open ais udp handler.\n");
			return -1;
		}
	}
*/

    DEBUG_LOG("[ClientSocketMgr::StartReactiveIO]Max allowed socket connections %d.", ACE::max_handles ());
    return 0;
}

int
ClientSocketMgr::StartNetwork (uint16 port, std::string& address)
{
    m_addr = address;
    m_port = port;

    if (StartReactiveIO (port, address.c_str()) == -1)
        return -1;

    return 0;
}

void
ClientSocketMgr::StopNetwork ()
{
    if (m_Acceptor)
    {
        ClientSocket::Acceptor* acc = dynamic_cast<ClientSocket::Acceptor*> (m_Acceptor);

        if (acc)
            acc->close ();
    }

    if (m_NetThreadsCount != 0)
    {
        for (size_t i = 0; i < m_NetThreadsCount; ++i)
            m_NetThreads[i].Stop ();
    }

    Wait ();
}

void
ClientSocketMgr::Wait ()
{
    if (m_NetThreadsCount != 0)
    {
        for (size_t i = 0; i < m_NetThreadsCount; ++i)
            m_NetThreads[i].Wait ();
    }
}

int
ClientSocketMgr::OnSocketOpen (BaseSocket* sock)
{
    ClientSocket* pSock = reinterpret_cast<ClientSocket*>(sock);

    // set some options here
    if (m_SockOutKBuff >= 0)
    {
        if (pSock->peer ().set_option (SOL_SOCKET,
                                       SO_SNDBUF,
                                       (void*) & m_SockOutKBuff,
                                       sizeof (int)) == -1 && errno != ENOTSUP)
        {
            DEBUG_LOG("[ClientSocketMgr::OnSocketOpen] set_option SO_SNDBUF errno = %s.", ACE_OS::strerror(errno));
            return -1;
        }
    }

    static const int ndoption = 1;

    // Set TCP_NODELAY.
    if (m_UseNoDelay)
    {
        if (pSock->peer ().set_option (ACE_IPPROTO_TCP,
                                       TCP_NODELAY,
                                       (void*)&ndoption,
                                       sizeof (int)) == -1)
        {
            DEBUG_LOG("[ClientSocketMgr::OnSocketOpen] peer ().set_option TCP_NODELAY errno = %s.", ACE_OS::strerror (errno));
            return -1;
        }
    }

    pSock->SetOutBufferSize (static_cast<size_t> (m_SockOutUBuff));

    // add to manager
    AddSocket(pSock);

    // we skip the Acceptor Thread
    size_t min = 1;
    ACE_ASSERT (m_NetThreadsCount >= 1);
    for (size_t i = 1; i < m_NetThreadsCount; ++i)
        if (m_NetThreads[i].Connections () < m_NetThreads[min].Connections ())
            min = i;

    int ret = m_NetThreads[min].AddSocket (pSock);
    return ret;
}

int
ClientSocketMgr::OnSocketClose (BaseSocket* sock)
{
    ClientSocket* pSock = reinterpret_cast<ClientSocket*>(sock);

    if(pSock->GetConnectID() != -1)
    {
        DelSocket(pSock);
    }

    return 0;
}

int
ClientSocketMgr::OnUserLogin(const Tokens& users, BaseSocket* sock)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_socketLock);

    for(int i=0; i<(int)users.size(); i++)
        m_mapUsers[users[i]] = (ClientSocket*)sock;
    return 0;
}

int
ClientSocketMgr::OnUserLogout(const std::string& user)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_socketLock);

    mapUser::iterator f = m_mapUsers.find(user);
    if(f != m_mapUsers.end())
        m_mapUsers.erase(f);

    return 0;
}

void
ClientSocketMgr::AddSocket(ClientSocket* sock)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_socketLock);

    sock->SetConnectID(m_u4CurrentID);
    sock->AddReference();

    m_mapSockets.insert(mapSocket::value_type(m_u4CurrentID, sock));
    m_u4CurrentID++;
}

void
ClientSocketMgr::DelSocket(ClientSocket* sock)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_socketLock);

    mapSocket::iterator f = m_mapSockets.find(sock->GetConnectID());
    if(f != m_mapSockets.end())
    {
        mapUser::iterator _f = m_mapUsers.find(sock->GetName());
        if(_f != m_mapUsers.end())
        {
            m_mapUsers.erase(_f);
        }

        f->second->RemoveReference();
        m_mapSockets.erase(f);
    }
}

int
ClientSocketMgr::SendPacket(const std::string& user, ACE_Message_Block* pct)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_socketLock);

    mapUser::iterator f = m_mapUsers.find(user);
    if(f != m_mapUsers.end())
    {
        return f->second->SendPacket(pct);
    }
    else
    {
        DEBUG_LOG("[ClientSocketMgr::SendPacket] not found user:%s.", user.c_str());
        pct->release();
        return -1;
    }

    return 0;
}

int
ClientSocketMgr::SendPacket(uint32 u4ConnectID, ACE_Message_Block* pct)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_socketLock);

    mapSocket::iterator f = m_mapSockets.find(u4ConnectID);
    if(f != m_mapSockets.end())
    {
        return f->second->SendPacket(pct);
    }
    else
    {
        DEBUG_LOG("[ClientSocketMgr::SendPacket] not found connectID:%d.", u4ConnectID);
        pct->release();
        return -1;
    }

    return 0;
}

void split(uint16 u2CommandID, uint16& mt, uint16& st)
{
    int iMt = 0, iSt = 0;

    char buf[10] = {'\0'};
    snprintf(buf, sizeof(buf), "%04x", u2CommandID);
    ACE_OS::memcpy(buf+4, buf+2, 2);
    buf[2] = '\0';

    sscanf(buf, "%x", &iMt);
    sscanf(buf+4, "%x", &iSt);

    mt = iMt;
    st = iSt;
}

ACE_Message_Block* assemble(int mt, int st, uint64 jobid, const char* src, int srcLen)
{
    int len = 12 + srcLen;
    ACE_Message_Block* pmb;
    ACE_NEW_NORETURN(pmb, ACE_Message_Block(len+8));
    char* p = pmb->wr_ptr();

    *(MsgHead*)p = MsgHead(mt, st, len);
    *(int*)(p+8) = 0; //ignore
    *(uint64*)(p+12) = jobid;
    memcpy(p+20, src, srcLen);

    pmb->wr_ptr(len+8);
    return pmb;
}

int
ClientSocketMgr::SendPacket(const std::string& userid, uint64 jobid, uint16 commandID, const char* data, int len)
{
    uint16 mt, st;
    split(commandID, mt, st);

    ACE_Message_Block* pct = assemble(mt, st, jobid, data, len);
    int ret = SendPacket(userid, pct);
    return ret;
}

int
ClientSocketMgr::SendPacket(uint32 connectID, uint64 jobid, uint16 commandID, const char* data, int len)
{
    uint16 mt, st;
    split(commandID, mt, st);

    ACE_Message_Block* pct = assemble(mt, st, jobid, data, len);
    int ret = SendPacket(connectID, pct);
    return ret;
}

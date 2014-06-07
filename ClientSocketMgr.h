#ifndef __CLIENT_SOCKET_MGR_H
#define __CLIENT_SOCKET_MGR_H

#include "ISocketMgr.h"
#include "ClientSocket.h"
#include "UdpSocket.h"

class ReactorRunnable;

/// Manages all sockets connected to peers and network threads
class ClientSocketMgr : public ISocketMgr
{
public:
    ClientSocketMgr ();
    ~ClientSocketMgr ();

    virtual int OnSocketOpen(BaseSocket* sock);
    virtual int OnSocketClose(BaseSocket* sock);
    virtual int OnSocketSend(int n)
    {
        return 0;
    }
    virtual int OnSocketRecv(int n)
    {
        return 0;
    }

    virtual int OnUserLogin(const Tokens& users, BaseSocket* sock);
    virtual int OnUserLogout(const std::string& user);

public:
    /// Start network, listen at address:port .
    int StartNetwork (uint16 port, std::string& address);

    /// Stops all network threads, It will wait for all running threads .
    void StopNetwork ();

    /// Wait untill all network threads have "joined" .
    void Wait ();

    std::string& GetBindAddress()
    {
        return m_addr;
    }
    uint16 GetBindPort()
    {
        return m_port;
    }

    /// Send packet to socket via manager, this function should release pct's memory
    int SendPacket(const std::string& user, ACE_Message_Block* pct);
    int SendPacket(uint32 u4ConnectID, ACE_Message_Block* pct);
    int SendPacket(const std::string& userid, uint64 jobid, uint16 commandID, const char* data, int len);
    int SendPacket(uint32 u4ConnectID, uint64 jobid, uint16 commandID, const char* data, int len);

private:
    int StartReactiveIO(uint16 port, const char* address);

    void AddSocket(ClientSocket* sock);
    void DelSocket(ClientSocket* sock);

private:
    ReactorRunnable* m_NetThreads;
    size_t m_NetThreadsCount;

    int m_SockOutKBuff;
    int m_SockOutUBuff;
    bool m_UseNoDelay;

    std::string m_addr;
    uint16 m_port;

    typedef std::map<uint32, ClientSocket*> mapSocket;
    mapSocket m_mapSockets;
    uint32 m_u4CurrentID;

    typedef std::map<std::string, ClientSocket*> mapUser;
    mapUser m_mapUsers;

    ACE_Recursive_Thread_Mutex m_socketLock;

    ACE_Event_Handler* m_Acceptor;
	UdpSocket* m_UdpSocket;
};

typedef ACE_Singleton<ClientSocketMgr, ACE_Thread_Mutex> g_ClientSocketMgr;

#endif

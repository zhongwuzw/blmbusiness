#ifndef _UDP_SOCKET_H_
#define _UDP_SOCKET_H_

#include "define.h"
#include "AisSvc.h"

class UdpSocket : public ACE_Event_Handler
{
public:
	UdpSocket (const ACE_INET_Addr& local_addr) : endpoint(local_addr)
	{
		endpoint.enable(ACE_NONBLOCK);
	}

	~UdpSocket()
	{

	}

	virtual ACE_HANDLE get_handle (void) const
	{
		return endpoint.get_handle();
	}

	virtual int handle_input (ACE_HANDLE handle)
	{
		ACE_INET_Addr from_addr;
		char buf[2*1024] = {'\0'};

		ACE_Time_Value timeout(5);
		ssize_t n = endpoint.recv (buf, sizeof(buf)-1, from_addr, 0, &timeout);
		if(n > 0)
		{
			g_AisSvc::instance()->OnAisEvent(buf);
		}

		return 0;
	}

private:
	ACE_SOCK_Dgram	endpoint;
};

#endif

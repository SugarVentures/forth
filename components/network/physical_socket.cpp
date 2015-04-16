
#include "physical_socket.hpp"

#include <stdio.h>

namespace oppvs
{
	int PhysicalSocket::Bind(const SocketAddress& addr)
	{
		struct sockaddr_in sockAddr;
		addr.toSocketAddr(&sockAddr);
		m_localAddress = addr;
		return bind(m_socketfd, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
	}

	int PhysicalSocket::Create(int family, int type, int protocol)
	{
		int on = 1;
		m_socketfd = socket(family, type, protocol);
		int status = setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on));
		if (status == -1)
			printf("Error in setsockopt\n");
		return m_socketfd;
	}

	
	int PhysicalSocket::Connect(const SocketAddress& dest)
	{
		struct sockaddr_in addr;
		dest.toSocketAddr(&addr);
		m_remoteAddress = dest;
		return connect(m_socketfd, (struct sockaddr*)&addr, sizeof(addr));
	}

	int PhysicalSocket::Listen()
	{
		return listen(m_socketfd, m_backlog);
	}

	int PhysicalSocket::Accept(const SocketAddress& remote)
	{
		struct  sockaddr_in addr;
		socklen_t len;
		int newsockfd = accept(m_socketfd, (struct sockaddr*)&addr, &len);
		m_remoteAddress = remote;
		return newsockfd;
	}

	int PhysicalSocket::Send(int destsock, void* msg, uint16_t len)
	{
		return send(destsock, msg, len, 0);
	}

	int PhysicalSocket::Receive(void* msg, uint16_t len)
	{
		return recv(m_socketfd, msg, len, 0);
	}

	int PhysicalSocket::Close(int destsock)
	{
		return close(destsock);
	}

	int PhysicalSocket::Close()
	{
		return close(m_socketfd);
	}

	void PhysicalSocket::setReceiveTimeOut(int expire)
	{
		struct timeval tv;
		tv.tv_sec = expire;
		tv.tv_usec = 0;
		if (setsockopt(m_socketfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval)) < 0)
			printf("Can not set receive time out\n");
	}

	SocketAddress PhysicalSocket::getLocalAddress()
	{
		if (m_socketfd == -1)
		{
			return m_localAddress;
		}
		struct sockaddr_in saddr;
		socklen_t len;
		if (getsockname(m_socketfd, (struct sockaddr*)&saddr, &len) == -1)
			return m_localAddress;
		m_localAddress.setPort(saddr.sin_port);
		return m_localAddress;
	}
}
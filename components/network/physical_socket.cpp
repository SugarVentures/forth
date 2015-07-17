
#include "physical_socket.hpp"

#include <stdio.h>
#include <errno.h>


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
		if (m_socketfd != - 1)
			Close();
		
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

	int PhysicalSocket::Accept(SocketAddress& remote)
	{
		struct  sockaddr_in addr;
		socklen_t len = sizeof(addr);
		int newsockfd = accept(m_socketfd, (struct sockaddr*)&addr, &len);
		remote = SocketAddress(addr);
		return newsockfd;
	}

	int PhysicalSocket::Send(int destsock, void* msg, uint16_t len)
	{
		return send(destsock, msg, len, 0);
	}

	int PhysicalSocket::Send(int sockfd, void* msg, uint16_t len, const SocketAddress& dest)
	{
		struct sockaddr_in sockAddr;
		dest.toSocketAddr(&sockAddr);
		return sendto(sockfd, msg, len, 0, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
	}

	int PhysicalSocket::Receive(void* msg, uint16_t len)
	{
		return recv(m_socketfd, msg, len, 0);
	}

	int PhysicalSocket::enablePacketInfoOptionCommon(int level, int option1, int option2, bool enable)
	{
		int isEnable = enable ? 1 : 0;
		if (option1 == -1 && option2 == -1)
			return -1;

		int ret = -1;
		if (option1 != -1)
		{
			ret = setsockopt(m_socketfd, level, option1, &isEnable, sizeof(isEnable));
		}

		if (ret < 0 && option2 != -1)
		{
			ret = setsockopt(m_socketfd, level, option2, &isEnable, sizeof(isEnable));
		}
		return ret;
	}

	int PhysicalSocket::enablePacketInfoOptionIpv4(bool enable)
	{
		int level = IPPROTO_IP;
		int option1 = -1;
		int option2 = -1;
    
#ifdef IP_PKTINFO
    	option1 = IP_PKTINFO;
#endif
    
#ifdef IP_RECVDSTADDR
    	option2 = IP_RECVDSTADDR;
#endif
    	return enablePacketInfoOptionCommon(level, option1, option2, enable);
	}

	int PhysicalSocket::enablePacketInfoOptionIpv6(bool enable)
	{
		int level = IPPROTO_IPV6;
		int option1 = -1;
		int option2 = -1;

#ifdef IPV6_RECVPKTINFO
    	option1 = IPV6_RECVPKTINFO;
#endif
    
#ifdef IPV6_PKTINFO
    	option2 = IPV6_PKTINFO;
#endif
    	return enablePacketInfoOptionCommon(level, option1, option2, enable);
	}

	int PhysicalSocket::enablePacketInfoOption(bool enable)
	{
		int family = m_localAddress.getIP().getAddressFamily();
		if (family == AF_INET)
			return enablePacketInfoOptionIpv4(enable);
		else if (family == AF_INET6)
			return enablePacketInfoOptionIpv6(enable);
		else
			return -1;
	}

	int PhysicalSocket::ReceiveMsg(int sockfd, void* buf, uint16_t len, int flags, SocketAddress& src, SocketAddress& dest)
	{
		
		char controlData[1024];
		struct sockaddr_storage srcAddr;

		struct iovec iov;
		iov.iov_base = buf;
		iov.iov_len = len;

		struct msghdr message;
		message.msg_name = &srcAddr;
		message.msg_namelen = sizeof(srcAddr);
		message.msg_iov = &iov;
		message.msg_iovlen = 1;
		message.msg_control = controlData;
		message.msg_controllen = sizeof(controlData);

		ssize_t ret = recvmsg(sockfd, &message, flags);
		if (ret == -1) {
			printf("%s\n", strerror(errno));
		}
		else if (message.msg_flags & MSG_TRUNC) {
		    printf("datagram too large for buffer: truncated\n");
		}
		else {
			struct sockaddr_in* addr;
		    addr = (struct sockaddr_in*)&srcAddr;		    
		    src = SocketAddress(*addr);

		    struct cmsghdr* pmsg = NULL;
		    dest = SocketAddress(srcAddr.ss_family);

		    for (pmsg = CMSG_FIRSTHDR(&message); pmsg != NULL; pmsg = CMSG_NXTHDR(&message, pmsg))
            {
                // IPV6 address ----------------------------------------------------------
                if ((pmsg->cmsg_level == IPPROTO_IPV6) && (pmsg->cmsg_type == IPV6_PKTINFO) && CMSG_DATA(pmsg))
                {
                    struct in6_pktinfo* pInfo = (in6_pktinfo*)CMSG_DATA(pmsg);
                    sockaddr_in6 addr = {};
                    addr.sin6_family = AF_INET6;
                    addr.sin6_addr = pInfo->ipi6_addr;
                    dest = SocketAddress(addr);
                    break;
                }

#ifdef IP_PKTINFO
             
                if ((pmsg->cmsg_level == IPPROTO_IP) && (pmsg->cmsg_type == IP_PKTINFO) && CMSG_DATA(pmsg))
                {
                    struct in_pktinfo* pInfo = (in_pktinfo*)CMSG_DATA(pmsg);
                    sockaddr_in addr = {};
                    addr.sin_family = AF_INET;
                    addr.sin_addr = pInfo->ipi_addr;
                    dest = SocketAddress(addr);
                    break;
                }
#endif
                
#ifdef IP_RECVDSTADDR
                
                // This code path for MacOSX and likely BSD as well
                if ((pmsg->cmsg_level == IPPROTO_IP) && (pmsg->cmsg_type == IP_RECVDSTADDR) && CMSG_DATA(pmsg))
                {
                    sockaddr_in addr = {};
                    addr.sin_family = AF_INET;
                    addr.sin_addr = *(in_addr*)CMSG_DATA(pmsg);
                    dest = SocketAddress(addr);                    
                    break;
                }
#endif
            }

		}


		return ret;
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

	SocketAddress& PhysicalSocket::getLocalAddress()
	{
		if (m_socketfd == -1)
		{
			return m_localAddress;
		}
		struct sockaddr_in saddr;
		socklen_t len = sizeof(saddr);
		if (getsockname(m_socketfd, (struct sockaddr*)&saddr, &len) == -1)
			return m_localAddress;

		m_localAddress.setPort(ntohs(saddr.sin_port));
		return m_localAddress;
	}

	SocketAddress& PhysicalSocket::getRemoteAddress()
	{
		return m_remoteAddress;
	}

	int PhysicalSocket::getSocketHandle()
	{
		return m_socketfd;
	}
}
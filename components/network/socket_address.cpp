
#include "socket_address.hpp"
#include <sstream>

namespace oppvs
{
	SocketAddress::SocketAddress()
	{
		clear();
	}

	SocketAddress::SocketAddress(const struct sockaddr_in& addr)
	{
		m_ip = IPAddress(addr.sin_addr);
		m_port = ntohs(addr.sin_port);
	}

	SocketAddress::SocketAddress(const struct sockaddr& addr)
	{
		if (addr.sa_family == AF_INET)
		{
			struct sockaddr_in* paddr = (struct sockaddr_in*)&addr;
			m_ip = IPAddress(paddr->sin_addr);
			m_port = ntohs(paddr->sin_port);
		}
	}

	SocketAddress::SocketAddress(int family)
	{
		m_ip = IPAddress(family);
	}

	SocketAddress::SocketAddress(const struct sockaddr_in6 addr)
	{
		m_ip = IPAddress(addr.sin6_addr);
		m_port = ntohs(addr.sin6_port);
	}

	void SocketAddress::clear()
	{
		m_ip = IPAddress();
		m_port = 0;
	}

	void SocketAddress::setIP(const IPAddress& ip)
	{
		m_ip = ip;
	}

	void SocketAddress::setPort(int port)
	{
		m_port = port;
	}

	void SocketAddress::toSocketAddr(struct sockaddr_in* addr) const
	{
		memset(addr, 0, sizeof(sockaddr_in));
		addr->sin_family = AF_INET;
		addr->sin_port = htons(m_port);
		addr->sin_addr = m_ip.getIPv4();
	}

	const IPAddress& SocketAddress::getIP() const
	{
		return m_ip;
	}

	uint16_t SocketAddress::getPort() const
	{
		return m_port;
	}

	std::string SocketAddress::toString() const
	{
		std::stringstream out;
		out << m_ip.toString() << ":" << m_port;
		return out.str();
	}
}
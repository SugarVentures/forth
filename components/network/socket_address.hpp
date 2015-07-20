/*
	Socket Address: IP address + Port
*/

#ifndef OPPVS_SOCKET_ADDRESS_HPP
#define OPPVS_SOCKET_ADDRESS_HPP

#include "ipaddress.hpp"
#include <string.h>

namespace oppvs
{
	class SocketAddress {
	public:
		SocketAddress();
		SocketAddress(const struct sockaddr_in& addr);
		SocketAddress(const struct sockaddr& addr);
		SocketAddress(const struct sockaddr_in6 addr);
		SocketAddress(int family);
		SocketAddress(uint32_t ipHostByteOrder, uint16_t port);

		void setIP(const IPAddress& ip);
		const IPAddress& getIP() const;

		void setPort(int port);
		uint16_t getPort() const;

		void clear();

		void toSocketAddr(struct sockaddr_in* addr) const;
		std::string toString() const;

	
	protected:
		IPAddress m_ip;
		uint16_t m_port;
	};

	struct SocketAddressPair
	{
		SocketAddress local;
		SocketAddress remote;
	};

}

#endif
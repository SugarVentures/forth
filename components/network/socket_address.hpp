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

		void setIP(const IPAddress& ip);
		const IPAddress& getIP() const;

		void setPort(int port);
		uint16_t getPort() const;

		void clear();

		void toSocketAddr(struct sockaddr_in* addr) const;

	
	private:
		IPAddress m_ip;
		uint16_t m_port;
	};
}

#endif
#ifndef OPPVS_STUN_SOCKET_ADDRESS_HPP
#define OPPVS_STUN_SOCKET_ADDRESS_HPP

#include "socket_address.hpp"
#include "stun_header.hpp"

namespace oppvs
{
	class StunSocketAddress : public SocketAddress
	{
	public:
		StunSocketAddress(uint32_t ipHostByteOrder, uint16_t port) : SocketAddress(ipHostByteOrder, port) {}
		StunSocketAddress(const struct sockaddr_in6 addr) : SocketAddress(addr) {}
		void applyXorMap(const StunTransactionId&);
		bool isZeroAddress() const;
	};
}

#endif
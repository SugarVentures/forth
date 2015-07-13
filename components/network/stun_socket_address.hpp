#ifndef OPPVS_STUN_SOCKET_ADDRESS_HPP
#define OPPVS_STUN_SOCKET_ADDRESS_HPP

#include "socket_address.hpp"
#include "stun_header.hpp"

namespace oppvs
{
	class StunSocketAddress : public SocketAddress
	{
	public:
		void applyXorMap(const StunTransactionId&);
		bool isZeroAddress() const;
	};
}

#endif
#ifndef OPPVS_STUN_SOCKET_HPP
#define OPPVS_STUN_SOCKET_HPP

#include "physical_socket.hpp"
#include "stun_header.hpp"

namespace oppvs
{
	class StunSocket : public PhysicalSocket
	{
	private:
		SocketRole m_role;

		void reset();
	public:
		StunSocket();
		int Close();
	};
}

#endif
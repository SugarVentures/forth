#ifndef OPPVS_STUN_SERVER_HPP
#define OPPVS_STUN_SERVER_HPP

#include "stun_socket_address.hpp"

namespace oppvs
{
	class StunServer
	{
	public:
		int init();
		int shutdown();

		int start();
		int stop();
	private:
		StunSocketAddress m_sockets[4];

		StunServer();
		~StunServer();
	};
}

#endif
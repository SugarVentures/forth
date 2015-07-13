#ifndef OPPVS_STUN_SERVER_THREAD_HPP
#define OPPVS_STUN_SERVER_THREAD_HPP


#include "thread.hpp"

#include "stun_socket.hpp"
#include "stun_message_handler.hpp"
#include <vector>

namespace oppvs
{
	class StunServerThread : public Thread 
	{
	public:
		StunServerThread();
		~StunServerThread();

		int init(StunSocket* sockets, StunTransportAddressSet* stas, SocketRole role);
	private:
		StunSocket* m_sendSockets;
		std::vector<StunSocket*> m_listenSockets;

		StunTransportAddressSet m_stas;
	};
}

#endif
/*
	Server socket
*/

#ifndef OPPVS_SERVER_SOCKET_HPP
#define OPPVS_SERVER_SOCKET_HPP

#include "physical_socket.hpp"
#include <pthread.h>

namespace oppvs
{
	class ServerSocket : public PhysicalSocket
	{
	public:
		ServerSocket();
		uint16_t getNumSubscribers();
		void increaseNumSubscribers();
	private:
		uint16_t m_numSubscribers;
		pthread_mutex_t m_mutex;
	};
}

#endif
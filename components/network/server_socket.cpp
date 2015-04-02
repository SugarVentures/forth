
#include "server_socket.hpp"

namespace oppvs
{
	ServerSocket::ServerSocket() : PhysicalSocket(), m_numSubscribers(0)
	{
		pthread_mutex_init(&m_mutex, NULL);
	}
	void ServerSocket::increaseNumSubscribers()
	{
		pthread_mutex_lock(&m_mutex);
		m_numSubscribers++;
		pthread_mutex_unlock(&m_mutex);
	}

	uint16_t ServerSocket::getNumSubscribers()
	{
		return m_numSubscribers;
	}
	
}
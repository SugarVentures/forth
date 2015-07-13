#include "stun_server_thread.hpp"
#include "common.hpp"

namespace oppvs
{
	StunServerThread::StunServerThread() : m_stas()
	{
		m_sendSockets = NULL;
		m_listenSockets.clear();
	}

	StunServerThread::~StunServerThread()
	{

	}

	int StunServerThread::init(StunSocket* sockets, StunTransportAddressSet* stas, SocketRole role)
	{
		bool singleSocket = IsValidSocketRole(role);
		if (!sockets)
		{
			printf("Invalid stun sockets\n");
			return -1;
		}
		if (!stas)
		{
			printf("Invalid transport address set\n");
			return -1;
		}

		if (singleSocket)
		{
			if (sockets[role].isValid() == false)
			{
				printf("Invalid stun socket role\n");
				return -1;
			}
		}

		m_sendSockets = sockets;
		m_stas = *stas;
		if (singleSocket)
		{
			m_listenSockets.push_back(&sockets[role]);
		}

		return 0;
	}
}
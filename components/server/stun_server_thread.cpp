#include "stun_server_thread.hpp"
#include "common.hpp"

namespace oppvs
{
	StunServerThread::StunServerThread() : Thread(threadExecuteFunction, this), m_stas(), m_exitThread(false)
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

	int StunServerThread::start()
	{
		create();
		return 0;
	}

	void* StunServerThread::threadExecuteFunction(void* pthis)
	{
		StunServerThread* thread = (StunServerThread*)pthis;
		thread->run();
		return NULL;
	}

	int StunServerThread::run()
	{
		while (!m_exitThread)
		{
			usleep(100);
		}
		return 0;
	}
}
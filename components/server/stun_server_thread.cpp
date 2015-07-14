#include "stun_server_thread.hpp"
#include "common.hpp"

namespace oppvs
{
	StunServerThread::StunServerThread() : Thread(threadExecuteFunction, this), m_rotation(0), m_stas(), m_exitThread(false)
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
		else
		{
			for (int i = 0; i < 4; i++)
			{
				if (sockets[i].isValid())
				{
					m_listenSockets.push_back(&sockets[i]);
				}
			}
		}

		m_exitThread = false;
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

	StunSocket* StunServerThread::waitForSocketData()
	{
		fd_set set = {};
		int highestSocketValue = 0;
		int ret;
		StunSocket* pReadySocket = NULL;
		size_t noSockets = m_listenSockets.size();

		m_rotation = (m_rotation + 1) % noSockets;

		FD_ZERO(&set);

		for (size_t index = 0; index < noSockets; index++)
	    {
	        ASSERT(m_listenSockets[index] != NULL);
	        int sock = m_listenSockets[index]->getSocketHandle();
	        ASSERT(sock != -1);
	        FD_SET(sock, &set);
	        highestSocketValue = (sock > highestSocketValue) ? sock : highestSocketValue;
	    }

	    // wait indefinitely for a socket
    	ret = select(highestSocketValue+1, &set, NULL, NULL, NULL);
    	if (ret < 0)
    	{
    		printf("Select error\n");
    		return NULL;
    	}
    	
	    for (size_t index = 0; index < noSockets; index++)
	    {
	        int indexconverted = (index + m_rotation) % noSockets;
	        int sock = m_listenSockets[indexconverted]->getSocketHandle();
	        
	        ASSERT(sock != -1);
	        
	        if (FD_ISSET(sock, &set))
	        {
	            pReadySocket = m_listenSockets[indexconverted];
	            break;
	        }
	    }
	    
	    ASSERT(pReadySocket != NULL);
	    
	    return pReadySocket;
	}

	int StunServerThread::run()
	{
		size_t noSockets = m_listenSockets.size();
		bool isMultiSockets = (noSockets > 1);
		StunSocket* psocket = m_listenSockets[0];
		int noSendSockets = 0;
		noSendSockets += (int)(m_stas.set[RolePP].isValid);
		noSendSockets += (int)(m_stas.set[RolePA].isValid);
		noSendSockets += (int)(m_stas.set[RoleAP].isValid);
		noSendSockets += (int)(m_stas.set[RoleAA].isValid);

		printf("Starting listener: %d send %d recv\n", noSockets, noSendSockets);

		while (!m_exitThread)
		{
			if (isMultiSockets)
			{
				psocket = waitForSocketData();
				if (psocket)
					printf("Selected socket: %s\n", psocket->getLocalAddress().toString().c_str());
				psocket = NULL;
			}
			usleep(100);
		}
		return 0;
	}
}
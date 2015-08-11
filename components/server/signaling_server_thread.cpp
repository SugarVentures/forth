#include "signaling_server_thread.hpp"
#include <iostream>

namespace oppvs {
	SignalingServerThread::SignalingServerThread() : Thread(threadExecuteFunction, this), m_exitThread(false)
	{
		m_sendSockets = NULL;
	}

	SignalingServerThread::~SignalingServerThread()
	{

	}

	int SignalingServerThread::init(PhysicalSocket* socket)
	{
		if (!socket)
		{
			std::cout << "Invalid socket" << std::endl;
			return -1;
		}
		m_sendSockets = socket;

		if (socket[0].isValid())
		{
			m_listenSockets.push_back(&socket[0]);
		}
		m_exitThread = false;
		return 0;
	}

	void* SignalingServerThread::threadExecuteFunction(void* pthis)
	{
		SignalingServerThread* thread = (SignalingServerThread*)pthis;
		thread->run();
		return NULL;
	}

	int SignalingServerThread::run()
	{
		size_t noSockets = m_listenSockets.size();
		bool isMultiSockets = (noSockets > 1);
		int flags = isMultiSockets ? MSG_DONTWAIT : 0;
		if (m_listenSockets.size() > 0)
			PhysicalSocket* psocket = m_listenSockets[0];
		else
		{
			std::cout << "No valid listening socket" << std::endl;
			return -1;
		}
		int noSendSockets = 1;
		
		printf("Starting listener: %lu send %d recv\n", noSockets, noSendSockets);

		while (!m_exitThread)
		{
			
		}
		return 0;
	}

	int SignalingServerThread::start()
	{
		create();
		return 0;
	}
} // oppvs
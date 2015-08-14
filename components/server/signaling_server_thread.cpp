#include "signaling_server_thread.hpp"
#include <iostream>

namespace oppvs {
	SignalingServerThread::SignalingServerThread() : Thread(threadExecuteFunction, this), m_exitThread(false)
	{
		m_sendSockets = NULL;
	}

	SignalingServerThread::~SignalingServerThread()
	{
		signalForStop(true);
		waitForStopAndClose();
		for (int i = 0; i < m_threads.size(); i++)
		{
			SignalingServerSubThread* thread = m_threads[i];
			delete thread;
			m_threads[i] = NULL;
		}
		m_threads.clear();
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
		if (noSockets == 0)
			return -1;
		PhysicalSocket* psocket = m_listenSockets[0];
		psocket->Listen();
		SocketAddress remoteAddress;
		while (!m_exitThread)
		{
			int sockfd = psocket->Accept(remoteAddress);
			if (sockfd < 0)
				continue;

			std::cout << "Accept connection from client: " << remoteAddress.toString() << std::endl;
			SignalingServerSubThread *thread = new SignalingServerSubThread();
			thread->init(psocket, sockfd, remoteAddress);
			//thread->start();
			m_threads.push_back(thread);
		}

		return 0;
	}

	int SignalingServerThread::start()
	{
		create();
		return 0;
	}

	
	int SignalingServerThread::signalForStop(bool postMessages)
	{
		for(unsigned i = 0; i < m_threads.size(); ++i) {
			SignalingServerSubThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->signalForStop();
			}
		}
		for(unsigned i = 0; i < m_threads.size(); ++i) {
			SignalingServerSubThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->waitForStopAndClose();
			}
		}

		m_exitThread = true;
		return 0;    	
	}

	int SignalingServerThread::waitForStopAndClose()
	{
		m_listenSockets[0]->Close();
		waitUntilEnding();
		m_listenSockets.clear();
		m_sendSockets = NULL;
		return 0;
	}

	
} // oppvs
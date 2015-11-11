#include "forwarding_server_thread.h"

namespace oppvs {
	ForwardingServerThread::ForwardingServerThread() : Thread(threadExecuteFunction, this), m_exitThread(false)
	{
		m_sendSockets = NULL;
	}

	ForwardingServerThread::~ForwardingServerThread()
	{
		signalForStop(true);
		waitForStopAndClose();
		for (int i = 0; i < m_threads.size(); i++)
		{
			ForwardingServerSubThread* thread = m_threads[i];
			delete thread;
			m_threads[i] = NULL;
		}
		m_threads.clear();
	}

	void* ForwardingServerThread::threadExecuteFunction(void* pthis)
	{
		ForwardingServerThread* thread = (ForwardingServerThread*)pthis;
		thread->run();
		return NULL;
	}

	int ForwardingServerThread::init(PhysicalSocket* psocket)
	{
		if (psocket == nullptr)
		{
			printf("Invalid socket\n");
			return -1;
		}
		if (psocket->isValid())
		{
			m_listenSockets.push_back(psocket);
		}
		m_exitThread = false;
		return 0;
	}

	int ForwardingServerThread::run()
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

			printf("Accept connection from client: %s \n", remoteAddress.toString().c_str());
			ForwardingServerSubThread *thread = new ForwardingServerSubThread();
			thread->init(psocket, sockfd, remoteAddress);
			thread->start();
			m_threads.push_back(thread);
		}

		return 0;
	}

	int ForwardingServerThread::start()
	{
		create();
		return 0;
	}

	int ForwardingServerThread::signalForStop(bool postMessages)
	{
		for(unsigned i = 0; i < m_threads.size(); ++i) {
			ForwardingServerSubThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->signalForStop();
			}
		}
		for(unsigned i = 0; i < m_threads.size(); ++i) {
			ForwardingServerSubThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->waitForStopAndClose();
			}
		}

		m_exitThread = true;
		return 0;    	
	}

	int ForwardingServerThread::waitForStopAndClose()
	{
		while (m_listenSockets.size() > 0)
		{
			PhysicalSocket* ps = m_listenSockets.back();
			ps->Close();
			m_listenSockets.pop_back();
		}
		waitUntilEnding();
		m_listenSockets.clear();
		m_sendSockets = NULL;
		return 0;
	}
} // oppvs
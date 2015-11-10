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

	int ForwardingServerThread::run()
	{
		

		return 0;
	}

	int ForwardingServerThread::start()
	{
		create();
		return 0;
	}

	int ForwardingServerThread::signalForStop(bool postMessages)
	{
		printf("Size : %lu\n", m_threads.size());
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
		for (unsigned i = 0; i < m_listenSockets.size(); i++)
		{
			m_listenSockets[i]->Close();
		}
		waitUntilEnding();
		m_listenSockets.clear();
		m_sendSockets = NULL;
		return 0;
	}
} // oppvs
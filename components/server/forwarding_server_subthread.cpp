#include "forwarding_server_subthread.h"

namespace oppvs {
	ForwardingServerSubThread::ForwardingServerSubThread(): Thread(threadExecuteFunction, this), m_socket(NULL), m_sockfd(-1), m_exitThread(false)
	{
	}

	ForwardingServerSubThread::~ForwardingServerSubThread()
	{
		
	}

	void* ForwardingServerSubThread::threadExecuteFunction(void* pthis)
	{
		ForwardingServerSubThread* thread = (ForwardingServerSubThread*)pthis;
		thread->run();
		return NULL;
	}

	void ForwardingServerSubThread::signalForStop()
	{
		m_exitThread = true;
	}

	void ForwardingServerSubThread::waitForStopAndClose()
	{
		if (m_socket != NULL)
			m_socket->Close(m_sockfd);
		m_socket = NULL;
	}

	void ForwardingServerSubThread::run()
	{
		
	}
} // oppvs
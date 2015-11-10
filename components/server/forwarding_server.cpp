#include "forwarding_server.h"

namespace oppvs {
	ForwardingServer::ForwardingServer()
	{

	}

	ForwardingServer::~ForwardingServer()
	{
		shutdown();
	}

	void ForwardingServer::init(const ForwardingServerConfiguration& config)
	{
		shutdown();

		PhysicalSocket* psock = nullptr;
		if (addSocket(config.addressListen, psock) < 0)
			return;

		printf("Add socket for forwarding server: %s\n", psock->getLocalAddress().toString().c_str());

		ForwardingServerThread* thread = new ForwardingServerThread();
		m_threads.push_back(thread);
	}
		
	int ForwardingServer::start()
	{
		for (int i = 0; i < m_threads.size(); i++)
		{
			ForwardingServerThread* thread = m_threads[i];
			thread->start();
		}
		return 0;
	}

	int ForwardingServer::stop()
	{
		for(unsigned int i = 0; i < m_threads.size(); ++i) {
			ForwardingServerThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->signalForStop(true);
			}
		}

		for(unsigned int i = 0; i < m_threads.size(); ++i) {
			ForwardingServerThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->waitForStopAndClose();
			}
		}
		return 0;
	}

	int ForwardingServer::shutdown()
	{
		stop();
		for (int i = 0; i < m_threads.size(); i++)
		{
			ForwardingServerThread* thread = m_threads[i];
			delete thread;
			m_threads[i] = NULL;
		}
		m_threads.clear();

		return 0;
	}

	int ForwardingServer::addSocket(const SocketAddress& addressListen, PhysicalSocket*& psock)
	{
		psock = new PhysicalSocket;
		if (psock->Create(AF_INET, SOCK_STREAM, 0) < 0)
		{
			printf("Open socket error %s\n", strerror(errno));
			delete psock;
			return -1;
		}
		if (psock->Bind(addressListen) < 0)
		{
			delete psock;
			printf("Binding error %s\n", strerror(errno));
			return -1;
		}

		
		return 0;
	}
} // oppvs
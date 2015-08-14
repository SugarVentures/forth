#include "signaling_server.hpp"
#include <errno.h>

namespace oppvs {
	SignalingServer::SignalingServer()
	{

	}

	SignalingServer::~SignalingServer()
	{
		shutdown();
	}

	void SignalingServer::init(const SignalingServerConfiguration& config)
	{
		shutdown();

		if (addSocket(config.addressListen, config.addressAdvertised) < 0)
			return;

		SignalingServerThread* thread = new SignalingServerThread();
		m_threads.push_back(thread);
		thread->init(&m_socket);
	}

	int SignalingServer::addSocket(const SocketAddress& addressListen, const SocketAddress& addressAdvertised)
	{
		if (m_socket.Create(AF_INET, SOCK_STREAM, 0) < 0)
		{
			printf("Open socket error %s\n", strerror(errno));
			return -1;
		}
		if (m_socket.Bind(addressListen) < 0)
		{
			printf("Binding error %s\n", strerror(errno));
			return -1;
		}

		printf("Add socket for signaling: %s\n", m_socket.getLocalAddress().toString().c_str());
		return 0;		
	}

	int SignalingServer::start()
	{
		for (int i = 0; i < m_threads.size(); i++)
		{
			SignalingServerThread* thread = m_threads[i];
			thread->start();
		}
		return 0;
	}

	int SignalingServer::stop()
	{
		for(unsigned i = 0; i < m_threads.size(); ++i) {
			SignalingServerThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->signalForStop(true);
			}
		}

		for(unsigned i = 0; i < m_threads.size(); ++i) {
			SignalingServerThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->waitForStopAndClose();
			}
		}
		return 0;
	}

	int SignalingServer::shutdown()
	{
		stop();
		m_socket.Close();
		for (int i = 0; i < m_threads.size(); i++)
		{
			SignalingServerThread* thread = m_threads[i];
			delete thread;
			m_threads[i] = NULL;
		}
		m_threads.clear();
		return 0;
	}

} // oppvs
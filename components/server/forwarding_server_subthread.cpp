#include "forwarding_server_subthread.h"

namespace oppvs {
	ForwardingServerSubThread::ForwardingServerSubThread(): Thread(threadExecuteFunction, this), m_socket(NULL), m_sockfd(-1), m_exitThread(false)
	{
	}

	ForwardingServerSubThread::~ForwardingServerSubThread()
	{
		releaseBuffers();
	}

	int ForwardingServerSubThread::init(PhysicalSocket* socket, int sockfd, const SocketAddress &remote)
	{
		m_socket = socket;
		m_sockfd = sockfd;
		m_remoteAddress = remote;
		m_exitThread = false;
		allocBuffers();
		return 0;
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
		printf("Run forwarding subthread \n");
		while (!m_exitThread)
		{
			int rcvlen = m_socket->Receive(m_sockfd, m_incomingBuffer->data(), m_incomingBuffer->capacity());

			if (rcvlen <= 0)
			{
				printf("Error in receiving packets %s\n", strerror(errno));
				break;
			}

			printf("Receive %d bytes\n", rcvlen);
			m_incomingBuffer->setSize(rcvlen);
			handleMessage();
		}
		m_socket->Close(m_sockfd);
	}

	void ForwardingServerSubThread::start()
	{
		create();
	}

	void ForwardingServerSubThread::allocBuffers()
	{
		m_incomingBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_incomingBuffer->setSize(OPPVS_NETWORK_PACKET_LENGTH);
	}

	void ForwardingServerSubThread::releaseBuffers()
	{
		m_incomingBuffer->reset();
	}

	void ForwardingServerSubThread::handleMessage()
	{

	}
} // oppvs
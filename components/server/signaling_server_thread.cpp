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
		allocBuffers();
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
		bool isMultiSockets = (noSockets > 1);
		int flags = isMultiSockets ? MSG_DONTWAIT : 0;
		PhysicalSocket* psocket = m_listenSockets[0];
		
		psocket->Listen();
		SocketAddress localAddress, remoteAddress;
		while (!m_exitThread)
		{
			int sockfd = psocket->Accept(remoteAddress);
			if (sockfd < 0)
				continue;

			int rcvlen = psocket->Receive(sockfd, m_incomingBuffer->data(), m_incomingBuffer->size());

			if (rcvlen < 0)
			{
				continue;
			}
			std::cout << "Receive msg from " << remoteAddress.toString() << " at: " << psocket->getLocalAddress().toString() << std::endl;
		}
		return 0;
	}

	int SignalingServerThread::start()
	{
		create();
		return 0;
	}

	void SignalingServerThread::allocBuffers()
	{
		m_incomingBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_incomingBuffer->setSize(MAX_SIGNALING_MESSAGE_SIZE);
		m_outgoingBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_outgoingBuffer->setSize(MAX_SIGNALING_MESSAGE_SIZE);
		
	}

	void SignalingServerThread::releaseBuffers()
	{
		m_incomingBuffer->reset();
		m_outgoingBuffer->reset();		
	}

	int SignalingServerThread::signalForStop(bool postMessages)
	{

		m_exitThread = true;

	    // have the socket send a message to itself
	    // if another thread is sharing the same socket, this may wake that thread up to
	    // but all the threads should be started and shutdown together
	    if (postMessages)
	    {
	        for (size_t index = 0; index < m_listenSockets.size(); index++)
	        {
	            char data = 'x';
	            
	            ASSERT(m_listenSockets[index] != NULL);
	            
	            SocketAddress addr = m_listenSockets[index]->getLocalAddress();
	            // If no specific adapter was binded to, IP will be 0.0.0.0
	            // Linux evidently treats 0.0.0.0 IP as loopback (and works)
	            // On Windows you can't send to 0.0.0.0. sendto will fail - switch to sending to localhost
	            if (addr.getIP().isZero())
	            {
	                SocketAddress addrLocal;
	                addrLocal.setIP(IPAddress());
	                addrLocal.setPort(addr.getPort());
	                addr = addrLocal;

	            }
	            m_listenSockets[index]->Send(m_listenSockets[index]->getSocketHandle(), &data, 1, addr);
	        }
	    }
		return 0;    	
	}

	int SignalingServerThread::waitForStopAndClose()
	{
		waitUntilEnding();
		releaseBuffers();
		m_listenSockets.clear();
		m_sendSockets = NULL;
		return 0;
	}
} // oppvs
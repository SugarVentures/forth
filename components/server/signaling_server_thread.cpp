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
		PhysicalSocket* psocket = m_listenSockets[0];
		psocket->Listen();
		SocketAddress remoteAddress;
		while (!m_exitThread)
		{
			int sockfd = psocket->Accept(remoteAddress);
			if (sockfd < 0)
				continue;

			int rcvlen = psocket->Receive(sockfd, m_incomingBuffer->data(), m_incomingBuffer->size());

			if (rcvlen <= 0)
			{
				continue;
			}
			
			std::cout << "Receive msg from " << remoteAddress.toString() << " at: " << psocket->getLocalAddress().toString() << std::endl;
			std::cout << "Bytes: " << rcvlen << std::endl;
			m_incomingBuffer->setSize(rcvlen);
			m_outgoingMessage.sock = sockfd;
			m_outgoingMessage.destination = remoteAddress;
			handleMessage();
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
	
		m_readerBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_readerBuffer->setSize(MAX_SIGNALING_MESSAGE_SIZE);
		
		m_messageReader.reset();
		m_messageReader.getStream().attach(m_readerBuffer, true);
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

	void SignalingServerThread::handleMessage()
	{
		m_messageReader.reset();
		m_readerBuffer->setSize(0);
		m_messageReader.getStream().attach(m_readerBuffer, true);

		m_messageReader.addBytes(m_incomingBuffer->data(), m_incomingBuffer->size());
		switch (m_messageReader.getMessageType())
		{
			case SignalingStreamRegister:
				if (buildIceRequest() < 0)
					return;
				sendResponse();
				break;
		}

		std::vector<IceCandidate>& candidates = m_messageReader.getIceCandidates();

		for (int i = 0; i < candidates.size(); i++)
		{
	        std::cout << "Candidate: " << candidates[i].component << " "
				  << candidates[i].foundation << " "
				  << candidates[i].priority << " "
				  << candidates[i].ip << " "
				  << candidates[i].protocol << " "
				  << candidates[i].port << " "
				  << candidates[i].type << std::endl;
		}
	}

	int SignalingServerThread::buildIceRequest()
	{
		m_messageBuilder.reset();
		if (m_messageBuilder.addMessageType(SignalingIceRequest) < 0)
			return -1;

		if (m_messageBuilder.addStreamKey(m_messageReader.getStreamKey()) < 0)
			return -1;

		return 0;
	}

	void SignalingServerThread::sendResponse()
	{
		SharedDynamicBufferRef buffer;
		if (m_messageBuilder.getResult(buffer) < 0)
		{
			std::cout << "Can not build the message to send" << std::endl;
			return;
		}

		PhysicalSocket* psocket = &m_sendSockets[0];
		
		if (psocket->Send(m_outgoingMessage.sock, buffer->data(), buffer->size()) >= 0)
			printf("Sent response to %s\n", m_outgoingMessage.destination.toString().c_str());
	}
} // oppvs
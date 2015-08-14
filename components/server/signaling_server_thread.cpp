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

			std::cout << "Accept connection from client: " << remoteAddress.toString() << std::endl;
			while (true)
			{
				int rcvlen = psocket->Receive(sockfd, m_incomingBuffer->data(), m_incomingBuffer->capacity());

				if (rcvlen <= 0)
				{
					std::cout << "Error in receiving packet" << std::endl;
					break;
				}

				std::cout << "Receive msg from " << remoteAddress.toString() << " at: " << psocket->getLocalAddress().toString() << std::endl;
				std::cout << "Bytes: " << rcvlen << std::endl;
				m_incomingBuffer->setSize(rcvlen);
				m_outgoingMessage.sock = sockfd;
				m_outgoingMessage.destination = remoteAddress;
				handleMessage();

			}
			psocket->Close(sockfd);
		
			
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
		return 0;    	
	}

	int SignalingServerThread::waitForStopAndClose()
	{
		m_listenSockets[0]->Close();
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

		if (m_messageReader.addBytes(m_incomingBuffer->data(), m_incomingBuffer->size()) < 0)
			return;
		switch (m_messageReader.getMessageType())
		{
			case SignalingStreamRegister:
				if (buildIceRequest() < 0)
					return;
				sendResponse();
				break;
			case SignalingIceResponse:
				std::cout << "Receive Ice Response" << std::endl;
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
				break;
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
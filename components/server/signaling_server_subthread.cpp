#include "signaling_server_subthread.hpp"
#include <iostream>

namespace oppvs {
	SignalingServerSubThread::SignalingServerSubThread(): Thread(threadExecuteFunction, this), m_socket(NULL), m_sockfd(-1), m_exitThread(false)
	{
	}

	SignalingServerSubThread::~SignalingServerSubThread()
	{
		releaseBuffers();
	}

	int SignalingServerSubThread::init(PhysicalSocket* socket, int sockfd, const SocketAddress& remote)
	{
		m_socket = socket;
		m_sockfd = sockfd;
		m_remote = remote;
		m_exitThread = false;
		allocBuffers();
		return 0;
	}

	void* SignalingServerSubThread::threadExecuteFunction(void* pthis)
	{
		SignalingServerSubThread* thread = (SignalingServerSubThread*)pthis;
		thread->run();
		return NULL;
	}

	void SignalingServerSubThread::run()
	{
		std::cout << "Run subthread" << std::endl;

		while (!m_exitThread)
		{
			int rcvlen = m_socket->Receive(m_sockfd, m_incomingBuffer->data(), m_incomingBuffer->capacity());

			if (rcvlen <= 0)
			{
				std::cout << "Error in receiving packet" << std::endl;
				m_cbDisconnect(m_sockfd);
				break;
			}

			std::cout << "Receives Bytes: " << rcvlen << std::endl;
			m_incomingBuffer->setSize(rcvlen);
			handleMessage();

		}
		m_socket->Close(m_sockfd);
		
	}

	void SignalingServerSubThread::start()
	{
		create();
	}

	void SignalingServerSubThread::signalForStop()
	{
		m_exitThread = true;
		releaseBuffers();
	}

	void SignalingServerSubThread::waitForStopAndClose()
	{
		if (m_socket != NULL)
			m_socket->Close(m_sockfd);
		waitUntilEnding();
		m_socket = NULL;
	}

	void SignalingServerSubThread::allocBuffers()
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

	void SignalingServerSubThread::releaseBuffers()
	{
		m_incomingBuffer->reset();
		m_outgoingBuffer->reset();		
	}

	void SignalingServerSubThread::handleMessage()
	{
		m_messageReader.reset();
		m_readerBuffer->setSize(0);
		m_messageReader.getStream().attach(m_readerBuffer, true);

		if (m_messageReader.addBytes(m_incomingBuffer->data(), m_incomingBuffer->size()) < 0)
			return;
		switch (m_messageReader.getMessageType())
		{
			case SignalingStreamRegister:
			{
				std::cout << "Receive Stream Register" << std::endl;
				m_cbStreamRegister(m_messageReader.getStreamKey(), m_sockfd, m_messageReader.getVideoStreamInfo());
			}
				break;
			case SignalingIceResponse:
			{
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
			}
				break;
			case SignalingStreamRequest:
			{
				std::cout << "Receive Stream Request" << std::endl;
				std::cout << "Requested stream key: " << m_messageReader.getStreamKey() << std::endl;
				VideoStreamInfo info;
				int broadcasterfd = -1;
				if (m_cbStreamRequest(m_messageReader.getStreamKey(), &broadcasterfd, info) < 0)
				{
					return;
				}
				if (buildStreamResponse(m_messageReader.getStreamKey(), info) < 0)
					return;

				printf("%d %d\n", m_sockfd, broadcasterfd);
				sendResponse(m_sockfd);

				if (buildIceResponse(m_messageReader.getStreamKey(), m_messageReader.getUsername(), 
					m_messageReader.getPassword(), m_messageReader.getIceCandidates()) < 0)
					return;

				sendResponse(broadcasterfd);
			}
				break;
		}

		
	}

	int SignalingServerSubThread::buildIceRequest()
	{
		m_messageBuilder.reset();
		if (m_messageBuilder.addMessageType(SignalingIceRequest) < 0)
			return -1;

		if (m_messageBuilder.addStreamKey(m_messageReader.getStreamKey()) < 0)
			return -1;

		return 0;
	}

	int SignalingServerSubThread::buildIceResponse(const std::string& streamKey, const std::string& username, const std::string& password, const std::vector<IceCandidate>& candidates)
	{
		m_messageBuilder.reset();

		if (m_messageBuilder.addMessageType(SignalingIceResponse) < 0)
			return -1;

		if (m_messageBuilder.addStreamKey(streamKey) < 0)
			return -1;

		if (m_messageBuilder.addIceUsername(username) < 0)
			return -1;

		if (m_messageBuilder.addIcePassword(password) < 0)
			return -1;

		if (m_messageBuilder.addIceCandidates(candidates) < 0)
			return -1;

		return 0;
	}

	int SignalingServerSubThread::buildStreamResponse(const std::string& streamKey, const VideoStreamInfo& videoInfo)
	{
		m_messageBuilder.reset();

		if (m_messageBuilder.addMessageType(SignalingStreamResponse) < 0)
			return -1;

		if (m_messageBuilder.addStreamKey(streamKey) < 0)
			return -1;

		if (m_messageBuilder.addVideoSources(videoInfo) < 0)
			return -1;
		return 0;
	}

	void SignalingServerSubThread::sendResponse()
	{
		SharedDynamicBufferRef buffer;
		if (m_messageBuilder.getResult(buffer) < 0)
		{
			std::cout << "Can not build the message to send" << std::endl;
			return;
		}
		if (m_socket->Send(m_sockfd, buffer->data(), buffer->size()) >= 0)
			printf("Sent response to %s\n", m_remote.toString().c_str());
	}

	void SignalingServerSubThread::sendResponse(int dest)
	{
		SharedDynamicBufferRef buffer;
		if (m_messageBuilder.getResult(buffer) < 0)
		{
			std::cout << "Can not build the message to send" << std::endl;
			return;
		}
		if (m_socket->Send(dest, buffer->data(), buffer->size()) >= 0)
			printf("Sent response\n");
	}

	void SignalingServerSubThread::attachCallback(callbackStreamRegister cb)
	{
		m_cbStreamRegister = cb;
	}

	void SignalingServerSubThread::attachCallback(callbackStreamRequest cb)
	{
		m_cbStreamRequest = cb;
	}

	void SignalingServerSubThread::attachCallback(callbackDisconnect cb)
	{
		m_cbDisconnect = cb;
	}

} // oppvs
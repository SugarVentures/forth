#include "signaling_manager.hpp"
#include <errno.h>
#include <iostream>

namespace oppvs {
	SignalingManager::SignalingManager(const SocketAddress& address): m_serverAddress(address), m_interrupt(false)
	{

	}

	SignalingManager::~SignalingManager()
	{

	}

	int SignalingManager::init()
	{
		if (m_socket.Create(AF_INET, SOCK_STREAM, 0) < 0)
		{
			std::cout << "Open socket error "  << strerror(errno) << std::endl;
			return -1;
		}
		SocketAddress localAddress;
		if (m_socket.Bind(localAddress) < 0)
		{
			std::cout << "Binding error " << strerror(errno) << std::endl;
			return -1;
		}

		if (m_socket.Connect(m_serverAddress) < 0)
		{
			std::cout << "Cannot connect to signaling server:" << strerror(errno) << std::endl;
			return -1;
		}
		std::cout << "Address for signaling: " << m_socket.getLocalAddress().toString() << std::endl;
		//m_socket.setReceiveTimeOut(OPPVS_NETWORK_MAX_WAIT_TIME);

		//Init buffer for response
		m_incomingBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_incomingBuffer->setSize(MAX_SIGNALING_MESSAGE_SIZE);

		m_readerBuffer = SharedDynamicBufferRef(new DynamicBuffer());

		return 0;
	}


	void SignalingManager::attachCallbackEvent(callbackOnReceiveIceResponse cb, void* object)
	{
		cbOnReceiveIceResponse = cb;
		m_object = object;
	}

	int SignalingManager::sendIceResponse(std::string username, std::string password, std::vector<IceCandidate>& candidates)
	{
		m_messageBuilder.reset();

		if (m_messageBuilder.addMessageType(SignalingIceResponse) < 0)
			return -1;

		if (m_messageBuilder.addStreamKey(m_streamKey) < 0)
			return -1;

		if (m_messageBuilder.addIceUsername(username) < 0)
			return -1;

		if (m_messageBuilder.addIcePassword(password) < 0)
			return -1;

		if (m_messageBuilder.addIceCandidates(candidates) < 0)
			return -1;

		return sendSignal();
	}

	int SignalingManager::sendStreamRegister(const std::string& streamKey, const VideoStreamInfo& info)
	{
		if (streamKey == "")
			return -1;
		m_streamKey = streamKey;
		m_messageBuilder.reset();
		if (m_messageBuilder.addMessageType(SignalingStreamRegister) < 0)
			return -1;
		if (m_messageBuilder.addStreamKey(streamKey) < 0)
			return -1;

		if (m_messageBuilder.addVideoSources(info) < 0)
			return -1;
		return sendSignal();
	}

	int SignalingManager::sendStreamRequest(const std::string& username, const std::string& password, 
			const std::vector<IceCandidate>& candidates)
	{
		if (m_streamKey == "" || username == "" || password == "" || candidates.size() == 0)
			return -1;
		m_messageBuilder.reset();
		if (m_messageBuilder.addMessageType(SignalingStreamRequest) < 0)
			return -1;
		if (m_messageBuilder.addStreamKey(m_streamKey) < 0)
			return -1;

		if (m_messageBuilder.addIceUsername(username) < 0)
			return -1;

		if (m_messageBuilder.addIcePassword(password) < 0)
			return -1;

		if (m_messageBuilder.addIceCandidates(candidates) < 0)
			return -1;

		return sendSignal();
	}

	int SignalingManager::sendSignal()
	{
		SharedDynamicBufferRef buffer;
		if (m_messageBuilder.getResult(buffer) < 0)
		{
			std::cout << "Can not build the message to send" << std::endl;
			return -1;
		}

		if (m_socket.Send(m_socket.getSocketHandle(), buffer->data(), buffer->size(), m_serverAddress) < 0)
		{
			std::cout << "Send error " << strerror(errno);
			return -1;
		}
		else
		{
			std::cout << "sent done " << buffer->size() << " bytes" << std::endl;
		}
		return 0;
	}

	void SignalingManager::waitResponse()
	{
    	int sock = m_socket.getSocketHandle();

    	m_incomingBuffer->setSize(0);

		while (!m_interrupt)
		{
			
    		int ret = m_socket.Receive(sock, m_incomingBuffer->data(), m_incomingBuffer->capacity());
			if (ret > 0)
			{
				m_incomingBuffer->setSize(ret);
				std::cout << "Receive " <<  ret << " bytes " << m_socket.getLocalAddress().toString() << " - " 
				<< m_socket.getRemoteAddress().toString() << std::endl;
				processResponse();
			}
			else
			{
				if (ret == 0)
					break;
				std::cout << "Receive error " << strerror(errno) << std::endl;
			}
		}
	}

	void SignalingManager::release()
	{
		m_incomingBuffer.reset();
		m_messageReader.reset();
	}

	void SignalingManager::signalForStop()
	{
		m_interrupt = true;
		m_socket.Close();
	}

	void SignalingManager::processResponse()
	{
		m_messageReader.reset();
		m_readerBuffer->setSize(0);
		m_messageReader.getStream().attach(m_readerBuffer, true);

		m_messageReader.addBytes(m_incomingBuffer->data(), m_incomingBuffer->size());
		switch (m_messageReader.getMessageType())
		{
			case SignalingIceRequest:
				std::cout << "Receive Ice Request" << std::endl;
				m_interrupt = true;
				break;
			case SignalingIceResponse:
			{
				std::cout << "Receive Ice Response" << std::endl;
				std::vector<IceCandidate>& candidates = m_messageReader.getIceCandidates();
				cbOnReceiveIceResponse(m_object, m_messageReader.getUsername(), m_messageReader.getPassword(), candidates);

			}

			default:
				break;
		}
	}

	void SignalingManager::setStreamKey(const std::string& streamkey)
	{
		m_streamKey = streamkey;
	}
} // oppvs
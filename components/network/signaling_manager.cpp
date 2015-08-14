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
			std::cout << "Cannot connect to signaling server" << std::endl;
			return -1;
		}
		std::cout << "Address for signaling: " << m_socket.getLocalAddress().toString() << std::endl;
		m_socket.setReceiveTimeOut(OPPVS_NETWORK_MAX_WAIT_TIME);

		//Init buffer for response
		m_incomingBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_incomingBuffer->setSize(MAX_SIGNALING_MESSAGE_SIZE);

		m_readerBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		return 0;
	}

	void SignalingManager::registerCallback(callbackOnReceiveIceRequest cb, void* object)
	{
		cbOnReceiveIceRequest = cb;
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

	int SignalingManager::sendStreamRegister(const std::string& streamKey)
	{
		if (streamKey == "")
			return -1;
		m_streamKey = streamKey;
		m_messageBuilder.reset();
		if (m_messageBuilder.addMessageType(SignalingStreamRegister) < 0)
			return -1;
		if (m_messageBuilder.addStreamKey(streamKey) < 0)
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
		fd_set set;
    	timeval tv = {};
    	int sock = m_socket.getSocketHandle();

    	m_incomingBuffer->setSize(0);
        FD_ZERO(&set);
        FD_SET(sock, &set);
        tv.tv_usec = 500000; // half-second
        tv.tv_sec = OPPVS_NETWORK_MAX_WAIT_TIME;

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
				std::cout << "Receive error " << strerror(errno) << std::endl;
				break;
			}
		}
		m_socket.Close();
	}

	void SignalingManager::release()
	{
		m_incomingBuffer.reset();
		m_messageReader.reset();
	}

	void SignalingManager::signalForStop()
	{
		m_interrupt = true;
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
				cbOnReceiveIceRequest(m_object);
				m_interrupt = true;
				break;
			default:
				break;
		}
	}
} // oppvs
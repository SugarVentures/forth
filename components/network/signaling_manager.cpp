#include "signaling_manager.hpp"
#include <errno.h>
#include <iostream>

namespace oppvs {
	SignalingManager::SignalingManager(const SocketAddress& address): m_serverAddress(address)
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

		std::cout << "Address for signaling: " << m_socket.getLocalAddress().toString() << std::endl;

		if (m_socket.Connect(m_serverAddress) < 0)
		{
			std::cout << "Cannot connect to signaling server" << std::endl;
			return -1;
		}
		return 0;
	}

	int SignalingManager::sendRequest()
	{
		if (m_messageBuilder.addMessageType(SignalingStreamRegister) < 0)
			return -1;

		uint32_t streamKey = 1234567;
		if (m_messageBuilder.addStreamKey(streamKey) < 0)
			return -1;

		SharedDynamicBufferRef buffer;
		m_messageBuilder.getResult(buffer);
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
} // oppvs
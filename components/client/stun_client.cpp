#include "stun_client.hpp"

namespace oppvs {
	StunClient::StunClient() : m_socket(), m_transactionId()
	{

	}

	int StunClient::init(StunClientConfiguration& config)
	{
		if (config.serverAddress.getIP().isZero() || config.serverAddress.getPort() == 0)
		{
			printf("StunClient: init - Invalid input parameters\n");
			return -1;
		}

		m_configuration = config;
		if (m_configuration.ignoreTimeOut)
		{
			m_configuration.timeOut = 0;
		}
		else if (m_configuration.timeOut == 0)
		{
			m_configuration.timeOut = 3;
		}

		if (m_configuration.maxAttempts == 0)
		{
			m_configuration.maxAttempts = 2;
		}

		//Init UDP connection to server
		int ret = m_socket.initUDP(m_configuration.localAddress, oppvs::RolePP);

		if (ret < 0)
			return -1;

		if (m_socket.enablePacketInfoOption(true) < 0)
		{
			std::cout << "Cannot enable packet info option\n";
			return -1;
		}

		return 0;
	}

	int StunClient::sendBindingRequest()
	{
		if (m_messageBuilder.addBindingRequestHeader() < 0)
		{
			std::cout << "Failed to add binding request header" << std::endl;
			return -1;
		}

        if (IsValidTransactionId(m_transactionId))
			m_messageBuilder.addTransactionID(m_transactionId);
		else
			m_messageBuilder.addRandomTransactionID(&m_transactionId);

		m_messageBuilder.getResult(m_messageBuffer);

		std::cout << "Current size: " << m_messageBuffer->size() << std::endl;

		if (m_socket.Send(m_socket.getSocketHandle(), m_messageBuffer->data(), m_messageBuffer->size(), m_configuration.serverAddress) < 0)
		{
			std::cout << "Send error " << strerror(errno);
			return -1;
		}
		else
		{
			std::cout << "sent to " << m_configuration.serverAddress.toString() << std::endl;
		}

		return 0;
	}

	void StunClient::waitResponse()
	{
		fd_set set;
    	timeval tv = {};
    	int sock = m_socket.getSocketHandle();

    	m_messageBuffer->setSize(0);
        FD_ZERO(&set);
        FD_SET(sock, &set);
        tv.tv_usec = 500000; // half-second
        tv.tv_sec = m_configuration.timeOut;

		while (true)
		{
			oppvs::SocketAddress remote, local;
			int ret = select(sock + 1, &set, NULL, NULL, &tv);
			if (ret > 0)
        	{
        		ret = m_socket.ReceiveMsg(sock, m_messageBuffer->data(), m_messageBuffer->capacity(), MSG_DONTWAIT, remote, local);
				if (ret >= 0)
				{
					local.setPort(m_socket.getLocalAddress().getPort());
					m_messageBuffer->setSize(ret);
					std::cout << "Receive " <<  ret << " bytes " << local.toString() << " - " << remote.toString() << std::endl;
				}
				else
				{
					std::cout << "Receive error " << strerror(errno);
				}
			}
		}
	}

	int StunClient::processResponse(SharedDynamicBufferRef& pmsg, StunSocketAddress& remoteAddr, StunSocketAddress& localAddr)
	{

		return 0;
	}

} // oppvs
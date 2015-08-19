#include "network_stream.hpp"

namespace oppvs
{
	NetworkStream::NetworkStream(NetworkRole role, uint32_t ssrc) : m_role(role), m_socket(ssrc),
	m_busy(false)
	{
		m_owner = NULL;
		m_error = 0;
		m_timestamp = 0;
		m_buffer = NULL;
	}

	int NetworkStream::setup(uint32_t port)
	{
		/*IPAddress ipAddr;
		SocketAddress socketAddr;
		socketAddr.setIP(ipAddr);
		socketAddr.setPort(port);

		if (m_socket.Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP) < 0)
			return -1;
		if (m_socket.Bind(socketAddr) < 0)
			return -1;

		m_socket.setPolicy(m_srtpKey);*/
		return 0;
	}

	int NetworkStream::setup(IceStream* stream)
	{
		m_iceStream = stream;
		return 0;
	}

	void NetworkStream::setSender(const SocketAddress& dest)
	{
		if (m_socket.Connect(dest) < 0)
			return;
		m_socket.initSender();
	}

	void NetworkStream::setReceiver(const SocketAddress& dest)
	{
		if (m_socket.Connect(dest) < 0)
			return;
		m_socket.initReceiver();
		m_socket.setReceiveTimeOut(EXPIRE_TIME);
	}

	void NetworkStream::releaseSender()
	{
		m_socket.releaseSender();
		m_socket.Close();
	}

	void NetworkStream::releaseReceiver()
	{
		m_socket.releaseReceiver();
		m_socket.Close();
	}


	void NetworkStream::registerCallback(void* owner, MessageHandling* message_handler, on_send_done_event event)
	{
		m_owner = owner;
		m_sendDoneEvent = event;
		//p_sendingQueue = (ConQueue<RawData*> *)squeue;
		m_messageHandler = message_handler;
	}

	void NetworkStream::registerCallback(void* owner, MessageParsing* message_parser, on_receive_event event)
	{
		m_owner = owner;
		m_receiveEvent = event;
		m_messageParser = message_parser;
	}

	void NetworkStream::sendStream()
	{
		if (!m_messageHandler->isEmptyPool() && !m_busy)
		{
			uint8_t* data = NULL;
			uint16_t length = 0;
			uint32_t ts = 0;
			
			m_messageHandler->getNextMessage(&data, &length, &ts);

			/*if (m_socket.SendTo(data, length, ts) < 0)
			{
				printf("Failed to send message\n");
				m_error = -1;
			}*/
			//else
			//	printf("Sent message %d\n", length);

			m_iceStream->send(length, data);
			m_busy = !m_messageHandler->releaseMessage();
			if (!m_busy)
				m_error = 1;

			m_sendDoneEvent(m_owner, m_error);
		}
		
	}

	void NetworkStream::unlock()
	{
		m_busy = false;
	}

	void NetworkStream::waitStream()
	{
		printf("Waiting data\n");
		int retries = 0;

		Message message;
		int len = OPPVS_NETWORK_PACKET_LENGTH;
		uint32_t timestamp;
		while (1)
		{
			int rcvLen = m_socket.RecvFrom(message.getData(), &len, &timestamp);
			if (rcvLen < 0)
			{
				printf("Error while reading from network\n");
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					if (retries++ >= MAX_RETRY_TIMES - 1)
					{
						printf("Stop waiting\n");
						break;
					}
				}
			}
			else if (rcvLen > 0)

			{
				//printf("Recv len: %d\n", rcvLen);
				message.setLength(rcvLen);
				message.setTimestamp(timestamp);
				m_messageParser->updateMessage(message);
			}
			
		}
	}

	void NetworkStream::onReceive(void* object, uint8_t* data, uint32_t len)
	{
		Message message;
		uint32_t timestamp = 0;
		NetworkStream* nstream = (NetworkStream*)object;
		if (len > 0)
		{
			memcpy(message.getData(), data, len);
			message.setLength(len);
			message.setTimestamp(timestamp);
			nstream->getParser()->updateMessage(message);
		}
	}

	void NetworkStream::sendDone(int* error)
	{

	}

	SocketAddress& NetworkStream::getLocalAddress()
	{
		return m_socket.getLocalAddress();
	}

	void NetworkStream::addSource(FrameInfo& inf)
	{

	}

	MessageParsing* NetworkStream::getParser()
	{
		return m_messageParser;
	}
}

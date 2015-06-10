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
		IPAddress ipAddr;
		SocketAddress socketAddr;
		socketAddr.setIP(ipAddr);
		socketAddr.setPort(port);

		if (m_socket.Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP) < 0)
			return -1;
		if (m_socket.Bind(socketAddr) < 0)
			return -1;

		m_socket.setPolicy(m_srtpKey);
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

	int NetworkStream::write(const uint8_t* data, uint32_t length, uint32_t* written)
	{
		int msgLength = length;
		int sendLength = msgLength > OPPVS_NETWORK_PACKET_LENGTH ? OPPVS_NETWORK_PACKET_LENGTH : msgLength;
		const uint8_t* curPos = data;
		
		*written = 0;
		m_error = 0;
		uint count = 0;

		while (msgLength > 0)
		{
			int len = m_socket.SendTo(curPos, sendLength, m_timestamp);
			if (len < 0)
			{
				printf("Send failed %s\n", strerror(errno));
				m_error = -1;
				break;
			}
			*written += len;
			curPos += sendLength;
			msgLength -= sendLength;
			count++;
			sendLength = msgLength > OPPVS_NETWORK_PACKET_LENGTH ? OPPVS_NETWORK_PACKET_LENGTH : msgLength;
		}
		return 0;
	}

	int NetworkStream::read(uint8_t* &buffer, uint32_t length, uint32_t* read, FrameInfo& info)
	{
		uint8_t localbuffer[OPPVS_NETWORK_PACKET_LENGTH];
		int len = OPPVS_NETWORK_PACKET_LENGTH;
		bool isNextFrame = false;
		int msgLength = 0;
		uint8_t* curPos = NULL;
		
		uint8_t *data = NULL;
		info.width = 0;
		info.height = 0;
		FrameBegin controlmsg;
		int rcvLen = m_socket.RecvFrom(localbuffer, &len, &isNextFrame);
		if (rcvLen <= 0)
			return -1;

		while (rcvLen > 0)
		{
			if (rcvLen == controlmsg.size())
			{
				printf("Receive control msg\n");				
				memcpy(&controlmsg, localbuffer, controlmsg.size());
				info.width = controlmsg.width;
				info.height = controlmsg.height;
				info.source = controlmsg.source;
				info.order = controlmsg.order;
				info.stride = controlmsg.stride;
				data = new uint8_t[info.stride * info.height];
				curPos = data;
				printf("Width: %d Height: %d Source %d Order: %d\n", controlmsg.width, controlmsg.height, controlmsg.source, controlmsg.order);
			}
			else if (rcvLen == sizeof(FrameEnd) || msgLength >= info.width*info.height*4)
			{
				break;
			}
			else
			{
				if (curPos)
				{
					memcpy(curPos, localbuffer, rcvLen);
					msgLength += rcvLen;
					curPos += rcvLen;
				}
			}
			rcvLen = m_socket.RecvFrom(localbuffer, &len, &isNextFrame);
		}

		if (info.width == 0 || info.height == 0)
			return -1;

		if (msgLength != info.width * info.height * 4)
		{
			delete [] data;
			return -1;
		}
		printf("Read %u bytes\n", msgLength);
		if (buffer)
		{
			delete [] buffer;
		}
		buffer = data;
		/*uint16_t stride = info.width * 4;
		for (int i = 0; i < info.height; i++)
	    {
	        uint32_t offset_buffer = m_buffer->width[0]*4*(0 + i) + 0*4;
	        uint32_t offset_data = info.width*4*i;
	        memcpy(buffer + offset_buffer, data + offset_data, stride);
	    }
	    delete [] data;*/
		return 0;
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
			
			m_messageHandler->getNextMessage(&data, &length);

			m_timestamp++;
			if (m_socket.SendTo(data, length, m_timestamp) < 0)
			{
				printf("Failed to send message\n");
				m_error = -1;
			}

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
		uint32_t readBytes = 0;
		printf("Waiting data\n");
		int retries = 0;

		FrameInfo info;
		Message message;
		int len = OPPVS_NETWORK_PACKET_LENGTH;
		bool isNextFrame;
		while (1)
		{
			int rcvLen = m_socket.RecvFrom(message.getData(), &len, &isNextFrame);
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
			else
			{
				message.setLength(rcvLen);
				m_messageParser->updateMessage(message);
				switch (message.getFlag())
				{
					case FLAG_START_FRAME:						
						break;
					case FLAG_MIDDLE_FRAME:
						//printf("Midlle segement\n");
						break;
					case FLAG_END_FRAME:
						//printf("End frame\n");
						m_receiveEvent(m_owner, message.getSource(), 0);
						break;
				}
				
			}
			
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
}

#include "network_stream.hpp"

namespace oppvs
{
	NetworkStream::NetworkStream(NetworkRole role, uint32_t ssrc) : m_role(role), m_socket(ssrc),
	m_busy(false)
	{
		m_owner = NULL;
		m_error = 0;
		m_timestamp = 0;
		m_buffer = new RawData();
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
			usleep(10);
		}
		return 0;
	}

	int NetworkStream::read(uint8_t* buffer, uint32_t length, uint32_t* read)
	{
		uint8_t localbuffer[OPPVS_NETWORK_PACKET_LENGTH];
		int len = OPPVS_NETWORK_PACKET_LENGTH;
		bool isNextFrame = false;
		int msgLength = 0;
		uint8_t* curPos = buffer;
		while (!isNextFrame)
		{
			int rcvLen = m_socket.RecvFrom(localbuffer, &len, &isNextFrame);
			if (rcvLen == -1)
			{
				return -1;
			}
			printf("Curpos %u\n", msgLength);
			memcpy(curPos, localbuffer, rcvLen);
			msgLength += rcvLen;
			curPos += rcvLen;
		}
		if (msgLength != length)
			return -1;
		printf("Read %u bytes\n", msgLength);
		return 0;
	}

	void NetworkStream::registerCallback(void* owner, void* squeue, on_send_done_event event)
	{
		m_owner = owner;
		m_sendDoneEvent = event;
		p_sendingQueue = (ConQueue<RawData*> *)squeue;
	}

	void NetworkStream::registerCallback(void* owner, uint8_t* data, uint32_t length, on_receive_event event)
	{
		m_owner = owner;
		m_receiveEvent = event;
		m_buffer->data = data;
		m_buffer->length = length;
	}

	void NetworkStream::sendStream()
	{
		if (!p_sendingQueue->empty() && !m_busy)
		{
			uint32_t written = 0;
			RawData *raw = p_sendingQueue->front();			
			printf("send %u %x\n", raw->length, raw);
			m_timestamp++;
			if (write(raw->data, raw->length, &written) < 0)
			{
				printf("Send failed\n");
				m_error = -1;
			}
			else
				printf("Sent %u bytes\n", written);

			m_busy = true;
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

		while (1)
		{
			if (read(m_buffer->data, m_buffer->length, &readBytes) < 0)
			{
				printf("Error while reading from network\n");
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					if (retries++ >= MAX_RETRY_TIMES)
					{
						printf("Stop waiting\n");
						break;
					}
				}
			}
			else
				m_receiveEvent(m_owner, 0);
		}
	}

	void NetworkStream::sendDone(int* error)
	{

	}

	SocketAddress& NetworkStream::getLocalAddress()
	{
		return m_socket.getLocalAddress();
	}
}

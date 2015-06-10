#include "message_handling.hpp"
#include <string.h>

namespace oppvs
{
	Message::Message(): m_length(0)
	{
	}

	Message::~Message()
	{
		m_length = 0;
	}

	void Message::setFlag(uint8_t flag)
	{
		m_data[0] = flag;
	}

	uint8_t Message::getFlag()
	{
		return m_data[0];
	}

	void Message::setSource(uint8_t sid)
	{
		m_data[1] = sid;
	}

	uint8_t Message::getSource()
	{
		return m_data[1];
	}

	void Message::setSegID(uint16_t seg)
	{
		memcpy(m_data + 2, &seg, 2);
	}

	uint16_t Message::getSegID()
	{
		uint16_t seg = 0;
		memcpy(&seg, m_data + 2, 2);
		return seg;
	}

	void Message::setData(const uint8_t* data, uint16_t length)
	{
		if (length > OPPVS_NETWORK_PACKET_LENGTH - MESSAGE_HEADER_SIZE)
			length = OPPVS_NETWORK_PACKET_LENGTH - MESSAGE_HEADER_SIZE;
		memcpy(m_data + MESSAGE_HEADER_SIZE, data, length);
		m_length = length + MESSAGE_HEADER_SIZE;
	}

	uint8_t* Message::getData()
	{
		return m_data;
	}

	uint16_t Message::getLength()
	{
		return m_length;
	}

	void Message::setLength(uint16_t len)
	{
		m_length = len;
	}

	MessageHandling::MessageHandling() : m_numFramesInPool(0), m_sentClients(0)
	{

	}

	MessageHandling::~MessageHandling()
	{

	}


	bool MessageHandling::isEmptyPool()
	{
		return (m_messagePool.size() == 0);
	}

	void MessageHandling::addMessage(PixelBuffer& pf)
	{
		if (m_numFramesInPool >= MAX_FRAMES_IN_POOL)
			return;


		int msgLength = pf.nbytes;
		int sendLength = msgLength > (OPPVS_NETWORK_PACKET_LENGTH - MESSAGE_HEADER_SIZE) ? (OPPVS_NETWORK_PACKET_LENGTH - MESSAGE_HEADER_SIZE) : msgLength;
		const uint8_t* curPos = pf.plane[0];
		uint16_t count = 0;
		while (msgLength > 0)
		{

			std::shared_ptr<Message> message(new Message);
			message->setSource(pf.source);
			message->setData(curPos, sendLength);
			message->setSegID(count);

			curPos += sendLength;
			msgLength -= sendLength;

			if (count == 0)
			{
				message->setFlag(FLAG_START_FRAME);
			}
			else if (msgLength <= 0)
			{
				message->setFlag(FLAG_END_FRAME);
			}
			else
			{
				message->setFlag(FLAG_MIDDLE_FRAME);
			}

			m_messagePool.push(message);
			count++;
			sendLength = msgLength > (OPPVS_NETWORK_PACKET_LENGTH - MESSAGE_HEADER_SIZE) ? (OPPVS_NETWORK_PACKET_LENGTH - MESSAGE_HEADER_SIZE) : msgLength;
		}
		m_numFramesInPool++;
	}

	void MessageHandling::getNextMessage(uint8_t** pdata, uint16_t* length)
	{
		if (m_messagePool.size() == 0)
		{
			return;
		}
		std::shared_ptr<std::shared_ptr<Message>> ptrmsg = m_messagePool.try_front();
		if (ptrmsg.get() != NULL)
		{
			std::shared_ptr<Message> message = *ptrmsg;
			*pdata = message->getData();
			*length = message->getLength();
			m_sentClients++;
		}
	}

	void MessageHandling::setNumClients(uint8_t clients)
	{
		m_numClients = clients;
	}

	bool MessageHandling::releaseMessage()
	{
		if (m_sentClients >= m_numClients)
		{
			std::shared_ptr<std::shared_ptr<Message>> ptrmsg = m_messagePool.try_pop();
			
			m_sentClients = 0;
			std::shared_ptr<Message> message = *ptrmsg;
			if (message->getFlag() == FLAG_END_FRAME)
				m_numFramesInPool--;

			return true;
		}
		else
		{			
			return false;
		}
	}

	MessageParsing::MessageParsing()
	{
		m_cacheBuffer = NULL;
	}

	MessageParsing::~MessageParsing()
	{

	}

	void MessageParsing::setCacheBuffer(CacheBuffer *cb)
	{
		m_cacheBuffer = cb;
	}

	void MessageParsing::updateMessage(Message& message)
	{
		if (!m_cacheBuffer)
			return;
		uint32_t loc = 0;
		//printf("Seg: %d len: %d source: %d\n", message.getSegID(), message.getLength(), message.getSource());
		loc = message.getSegID() * (message.getLength() - MESSAGE_HEADER_SIZE);
		uint8_t* dest = m_cacheBuffer->getBufferAddress(message.getSource(), loc);
		if (dest)
		{
			memcpy(dest, message.getData() + MESSAGE_HEADER_SIZE, message.getLength() - MESSAGE_HEADER_SIZE);
		}
	}
}
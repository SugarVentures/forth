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

	void Message::setData(const uint8_t* data, uint16_t length)
	{
		if (length > OPPVS_NETWORK_PACKET_LENGTH - 2)
			length = OPPVS_NETWORK_PACKET_LENGTH - 2;
		memcpy(m_data + 2, data, length);
		m_length = length + 2;
	}

	uint8_t* Message::getData()
	{
		return m_data;
	}

	uint16_t Message::getLength()
	{
		return m_length;
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
		int sendLength = msgLength > (OPPVS_NETWORK_PACKET_LENGTH - 2) ? (OPPVS_NETWORK_PACKET_LENGTH - 2) : msgLength;
		const uint8_t* curPos = pf.plane[0];
		int count = 0;
		while (msgLength > 0)
		{

			std::shared_ptr<Message> message(new Message);
			message->setSource(pf.source);
			message->setData(curPos, sendLength);

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
			sendLength = msgLength > (OPPVS_NETWORK_PACKET_LENGTH - 2) ? (OPPVS_NETWORK_PACKET_LENGTH - 2) : msgLength;
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
		if (m_sentClients == m_numClients)
		{
			std::shared_ptr<std::shared_ptr<Message>> ptrmsg = m_messagePool.try_pop();
			if (ptrmsg.get() == NULL)
				return true;

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
}
#include "message_handling.hpp"
#include <string.h>

namespace oppvs
{
	Message::Message(): m_length(0)
	{

	}

	Message::~Message()
	{

	}

	void Message::setFlag(uint8_t flag)
	{
		m_data[0] = flag;
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

	Message* MessageHandling::getNewMessage()
	{
		if (m_numFramesInPool >= MAX_FRAMES_IN_POOL)
			return NULL;
		Message* msg = new Message();
		return m_messagePool.push_and_back(msg);
	}

	bool MessageHandling::isEmptyPool()
	{
		return (m_numFramesInPool == 0);
	}

	void MessageHandling::addMessage(PixelBuffer& pf)
	{
		int msgLength = pf.nbytes;
		int sendLength = msgLength > (OPPVS_NETWORK_PACKET_LENGTH - 2) ? (OPPVS_NETWORK_PACKET_LENGTH - 2) : msgLength;
		const uint8_t* curPos = pf.plane[0];
		int count = 0;
		while (msgLength > 0)
		{
			Message *message = getNewMessage();
			if (message == NULL)
				return;
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


			count++;
			sendLength = msgLength > (OPPVS_NETWORK_PACKET_LENGTH - 2) ? (OPPVS_NETWORK_PACKET_LENGTH - 2) : msgLength;
		}

		m_numFramesInPool++;
	}

	void MessageHandling::getNextMessage(uint8_t** pdata, uint16_t* length)
	{
		Message* message = m_messagePool.front();
		
		*pdata = message->getData();
		if (message == NULL)
			printf("Null message\n");
		*length = 1;
		//*length = message->getLength();
		m_sentClients++;
		
	}

	void MessageHandling::setNumClients(uint8_t clients)
	{
		m_numClients = clients;
	}

	bool MessageHandling::releaseMessage()
	{
		if (m_sentClients == m_numClients)
		{
			m_messagePool.pop();
			m_sentClients = 0;
			return true;
		}
		else
		{			
			return false;
		}
	}
}
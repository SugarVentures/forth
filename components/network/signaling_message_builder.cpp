#include "signaling_message_builder.hpp"

namespace oppvs {
	SignalingMessageBuilder::SignalingMessageBuilder() 
	{

	}

	SignalingMessageBuilder::~SignalingMessageBuilder()
	{

	}

	DataStream& SignalingMessageBuilder::getDataStream()
	{
		return m_dataStream;
	}

	int SignalingMessageBuilder::getResult(SharedDynamicBufferRef& buffer)
	{
		int ret = addMessageLength();
		if (ret == 0)
			buffer = m_dataStream.getBuffer();
		return ret;
	}

	int SignalingMessageBuilder::addMessageType(SignalingMessageType type)
	{
		uint8_t msgType = type;
		m_dataStream.grow(100);
		if (m_dataStream.writeUInt8(msgType) < 0)
			return -1;

		//Reserve space for message length
		if (m_dataStream.writeUInt16(0) < 0)
	    	return -1;
		return 0;
	}

	int SignalingMessageBuilder::addStreamKey(uint32_t streamKey)
	{
		if (m_dataStream.writeUInt32(htonl(streamKey)) < 0)
			return -1;

		return 0;
	}

	int SignalingMessageBuilder::addMessageLength()
	{
		uint16_t len = m_dataStream.size();
    	size_t currentPos = m_dataStream.getPosition();
    	if (len < SIGNALING_HEADER_SIZE)
    	{
    		printf("Wrong size in signaling message\n");
    		return -1;
    	}

    	len -= SIGNALING_HEADER_SIZE;
    	m_dataStream.setAbsolutePosition(1);	//Message length is at 2 & 3rd byte
    	m_dataStream.writeUInt16(htons(len));
    	m_dataStream.setAbsolutePosition(currentPos);
		return 0;
	}
} // oppvs
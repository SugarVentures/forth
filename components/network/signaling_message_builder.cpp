#include "signaling_message_builder.hpp"

namespace oppvs {
	SignalingMessageBuilder::SignalingMessageBuilder() : m_streamKey(0)
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
		buffer = m_dataStream.getBuffer();
		return 0;
	}

	int SignalingMessageBuilder::addMessageType(SignalingMessageType type)
	{
		uint8_t msgType = type;
		m_dataStream.grow(100);
		if (m_dataStream.writeUInt8(htons(msgType)) < 0)
			return -1;

		return 0;
	}
} // oppvs
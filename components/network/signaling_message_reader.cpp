#include "signaling_message_reader.hpp"

namespace oppvs {
	SignalingMessageReader::SignalingMessageReader()
	{
		reset();	
	}

	SignalingMessageReader::~SignalingMessageReader()
	{
			
	}

	void SignalingMessageReader::reset()
	{
		m_dataStream.reset();
	}

	DataStream& SignalingMessageReader::getStream()
	{
		return m_dataStream;
	}

	int SignalingMessageReader::addBytes(const uint8_t* pData, uint32_t size)
	{
		// seek to the end of the stream
	    m_dataStream.setAbsolutePosition(m_dataStream.size());

	    if (m_dataStream.write(pData, size) < 0)
	    {
	        return -1;
	    }

	    size_t currentSize = m_dataStream.size();
	    std::cout << "Read " << currentSize << " bytes" << std::endl;
	    if (readHeader() < 0)
	    {
	    	std::cout << "Invalid header" << std::endl;
	    	return -1;
	    }
	    if (readBody() < 0)
	    {
	    	std::cout << "Invalid body" << std::endl;
	    	return -1;
	    }
		return 0;
	}

	int SignalingMessageReader::readHeader()
	{
		uint16_t msgType = 0;
		uint16_t msgLength = 0;
		uint32_t streamKey = 0;
		if (m_dataStream.setAbsolutePosition(0) < 0)
	    	return -1;
	    if (m_dataStream.readUInt16(&msgType) < 0)
	    	return -1;
	    if (m_dataStream.readUInt16(&msgLength) < 0)
	    	return -1;

	    if (m_dataStream.readUInt32(&streamKey) < 0)
	    	return -1;
	    msgType = ntohs(msgType);
	    msgLength = ntohs(msgLength);
	    streamKey = ntohl(streamKey);

	    printf("%d %d %u\n", msgType, msgLength, streamKey);
	    m_messageType = convertToSignalingMessageType(msgType);
		return 0;
	}

	int SignalingMessageReader::readBody()
	{
		size_t currentSize = m_dataStream.size();
    	size_t bytesConsumed = SIGNALING_HEADER_SIZE;

    	if (m_dataStream.setAbsolutePosition(SIGNALING_HEADER_SIZE) < 0)
    		return -1;
    	
    	if (m_messageType == SignalingIceResponse)
    	{
    		std::string username, password;
    		if (readStringAttribute(SIGNALING_ATTRIBUTE_ICE_USERNAME, username) < 0)
    		{
    			return -1;
    		}
    		if (readStringAttribute(SIGNALING_ATTRIBUTE_ICE_PASSWORD, password) < 0)
    		{
    			return -1;
    		}

    		std::cout << "Username: " << username << " password: " << password << std::endl;
    	}
    	return 0;
	}

	int SignalingMessageReader::readStringAttribute(uint16_t type, std::string& s)
	{
		uint16_t attributeType = 0, attributeLength = 0;
		uint8_t paddingLength = 0;
		if (m_dataStream.readUInt16(&attributeType) < 0)
		{
			return -1;
		}
		attributeType = ntohs(attributeType);
		if (attributeType != type)
		{
			return -1;
		}
		if (m_dataStream.readUInt16(&attributeLength) < 0)
		{
			return -1;
		}
		attributeLength = ntohs(attributeLength);
		if (attributeLength % 4)
        {
            paddingLength = 4 - attributeLength % 4;
        }
		char localbuf[attributeLength];
		if (m_dataStream.read((void*)localbuf, attributeLength) < 0)
		{
			return -1;
		}
		s = std::string(localbuf);
		m_dataStream.setRelativePosition(paddingLength);
		return 0;
	}
} // oppvs
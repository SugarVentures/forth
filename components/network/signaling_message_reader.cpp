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
    		uint16_t noCandidates = 0;
    		if (readUInt16Attribute(SIGNALING_ATTRIBUTE_ICE_NO_CANDIDATES, &noCandidates) < 0)
    		{
    			return -1;
    		}
    		std::cout << "Username: " << username << " password: " << password << std::endl;
    		std::cout << "Candidates: " << noCandidates << std::endl;
    		m_candidates.clear();
    		m_candidates.reserve(noCandidates);

    		for (int i = 0; i < noCandidates; ++i)
    		{
    			IceCandidate candidate;
    			if (readStringAttribute(SIGNALING_ATTRIBUTE_ICE_FOUNDATION, candidate.foundation) < 0)    			
    			{
    				return -1;
    			}
    			if (readUInt32Attribute(SIGNALING_ATTRIBUTE_ICE_PRIORITY, &candidate.priority) < 0)
    			{
    				return -1;
    			}
    			if (readStringAttribute(SIGNALING_ATTRIBUTE_ICE_IP_ADDRESS, candidate.ip) < 0)
    			{
    				return -1;
    			}
    			if (readUInt16Attribute(SIGNALING_ATTRIBUTE_ICE_PORT, &candidate.port) < 0)
    			{
    				return -1;
    			}
    			if (readStringAttribute(SIGNALING_ATTRIBUTE_ICE_TYPE, candidate.type) < 0)
    			{
    				return -1;
    			}

    			m_candidates.push_back(candidate);
    		}
    	}
    	return 0;
	}

	int SignalingMessageReader::readStringAttribute(uint16_t type, std::string& s)
	{
		uint16_t attributeLength = 0;
		uint8_t paddingLength = 0;
		
		if (readAttributeLength(type, &attributeLength) < 0)
			return -1;

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
		s = std::string(localbuf, attributeLength);
		m_dataStream.setRelativePosition(paddingLength);
		return 0;
	}

	int SignalingMessageReader::readUInt16Attribute(uint16_t type, uint16_t* attr)
	{
		uint16_t attributeLength = 0;
		if (readAttributeLength(type, &attributeLength) < 0)
			return -1;

		attributeLength = ntohs(attributeLength);
		if (attributeLength != 2)
			return -1;

		if (m_dataStream.read((void*)attr, attributeLength) < 0)
		{
			return -1;
		}
		*attr = ntohs(*attr);
		m_dataStream.setRelativePosition(2); //Move 2 bytes padding
		return 0;	
	}

	int SignalingMessageReader::readUInt32Attribute(uint16_t type, uint32_t* attr)
	{
		uint16_t attributeLength = 0;
		if (readAttributeLength(type, &attributeLength) < 0)
			return -1;

		attributeLength = ntohs(attributeLength);
		if (attributeLength != 4)
			return -1;

		if (m_dataStream.read((void*)attr, attributeLength) < 0)
		{
			return -1;
		}
		*attr = ntohl(*attr);

		return 0;
	}

	int SignalingMessageReader::readAttributeLength(uint16_t type, uint16_t* length)
	{
		uint16_t attributeType = 0, attributeLength = 0;
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
		*length = attributeLength;
		return 0;
	}

	std::vector<IceCandidate>& SignalingMessageReader::getIceCandidates()
	{
		return m_candidates;
	}
} // oppvs
#include "signaling_message_reader.hpp"

namespace oppvs {
	SignalingMessageReader::SignalingMessageReader()
	{
		m_serviceInfo.videoStreamInfo.sources = NULL;
		m_serviceInfo.videoStreamInfo.noSources = 0;
		reset();	
	}

	SignalingMessageReader::~SignalingMessageReader()
	{
		reset();
	}

	void SignalingMessageReader::reset()
	{
		m_dataStream.reset();
		if (m_serviceInfo.videoStreamInfo.noSources > 0)
		{
			delete [] m_serviceInfo.videoStreamInfo.sources;
			m_serviceInfo.videoStreamInfo.noSources = 0;
			m_serviceInfo.videoStreamInfo.sources = NULL;
		}
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
		char streamKey[STREAM_KEY_SIZE];
		if (m_dataStream.setAbsolutePosition(0) < 0)
	    	return -1;
	    if (m_dataStream.readUInt16(&msgType) < 0)
	    	return -1;
	    if (m_dataStream.readUInt16(&msgLength) < 0)
	    	return -1;

	    if (m_dataStream.read(streamKey, STREAM_KEY_SIZE) < 0)
	    	return -1;

	    msgType = ntohs(msgType);
	    msgLength = ntohs(msgLength);

	    printf("%d %d %s\n", msgType, msgLength, streamKey);
	    m_messageType = convertToSignalingMessageType(msgType);
	    m_streamKey = std::string(streamKey, STREAM_KEY_SIZE);
		return 0;
	}

	int SignalingMessageReader::readBody()
	{
    	if (m_dataStream.setAbsolutePosition(SIGNALING_HEADER_SIZE) < 0)
    		return -1;
    	if (m_messageType == SignalingIceResponse || m_messageType == SignalingStreamRequest)
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
    		m_username = username;
    		m_password = password;
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
    	else if (m_messageType == SignalingStreamRegister || m_messageType == SignalingStreamResponse)
    	{
    		uint8_t noVideoSources = 0;
    		uint8_t noAudioSources = 0;
    		int ret = 0;
    		int currentPos = m_dataStream.getPosition();
    		if (readUInt8Attribute(SIGNALING_ATTRIBUTE_VIDEO_NOSOURCES, &noVideoSources) >= 0)
    		{
	    		m_serviceInfo.videoStreamInfo.noSources = noVideoSources;
	    		m_serviceInfo.videoStreamInfo.sources = new VideoSourceInfo[noVideoSources];
	    		for (uint8_t i = 0; i < noVideoSources; ++i)
	    		{
	    			if (readUInt8Attribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_ID, &m_serviceInfo.videoStreamInfo.sources[i].source) <  0)
		    		{
		    			return -1;
		    		}
		    		if (readUInt8Attribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_ORDER, &m_serviceInfo.videoStreamInfo.sources[i].order) <  0)
		    		{
		    			return -1;
		    		}
		    		if (readUInt8Attribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_PIXEL_FORMAT, &m_serviceInfo.videoStreamInfo.sources[i].format) <  0)
		    		{
		    			return -1;
		    		}
	    			if (readUInt16Attribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_WIDTH, &m_serviceInfo.videoStreamInfo.sources[i].width) < 0)
	    			{
	    				return -1;
	    			}
	    			if (readUInt16Attribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_HEIGHT, &m_serviceInfo.videoStreamInfo.sources[i].height) < 0)
	    			{
	    				return -1;
	    			}
	    			if (readUInt16Attribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_STRIDE, &m_serviceInfo.videoStreamInfo.sources[i].stride) < 0)
	    			{
	    				return -1;
	    			}
	    			printf("Source: %d order: %d format: %d width: %d height: %d stride: %d\n", m_serviceInfo.videoStreamInfo.sources[i].source,
	    				m_serviceInfo.videoStreamInfo.sources[i].order,
	    				m_serviceInfo.videoStreamInfo.sources[i].format,
	    				m_serviceInfo.videoStreamInfo.sources[i].width,
	    				m_serviceInfo.videoStreamInfo.sources[i].height,
	    				m_serviceInfo.videoStreamInfo.sources[i].stride);
	    		}
    		}
    		else
    		{
    			ret = -1;
    			m_dataStream.setAbsolutePosition(currentPos);
    		}

    		if (readUInt8Attribute(SIGNALING_ATTRIBUTE_AUDIO_NOSOURCES, &noAudioSources) >= 0)
    		{

    			m_serviceInfo.audioStreamInfo.noSources = noAudioSources;
    			m_serviceInfo.audioStreamInfo.sources = new AudioSourceInfo[noAudioSources];
    			for (uint8_t i = 0; i < noAudioSources; ++i)
    			{
    				if (readUInt8Attribute(SIGNALING_ATTRIBUTE_SOURCE_AUDIO_ID, &m_serviceInfo.audioStreamInfo.sources[i].source) <  0)
		    		{
		    			return -1;
		    		}
		    		if (readUInt16Attribute(SIGNALING_ATTRIBUTE_SOURCE_AUDIO_CHANNELS, &m_serviceInfo.audioStreamInfo.sources[i].numberChannels) < 0)
	    			{
	    				return -1;
	    			}
	    			if (readUInt32Attribute(SIGNALING_ATTRIBUTE_SOURCE_AUDIO_SAMPLE_RATE, &m_serviceInfo.audioStreamInfo.sources[i].sampleRate) < 0)
	    			{
	    				return -1;
	    			}
	    			printf("Source %d channels: %d sample rate: %d\n", m_serviceInfo.audioStreamInfo.sources[i].source, 
	    				m_serviceInfo.audioStreamInfo.sources[i].numberChannels,
	    				m_serviceInfo.audioStreamInfo.sources[i].sampleRate);
    			}
    			ret = 0;
    		}
    		else
    		{
    			if (ret < 0)
    				return -1;
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

	int SignalingMessageReader::readUInt8Attribute(uint16_t type, uint8_t* attr)
	{
		uint16_t attributeLength = 0;
		if (readAttributeLength(type, &attributeLength) < 0)
			return -1;

		attributeLength = ntohs(attributeLength);
		if (attributeLength != 1)
			return -1;

		if (m_dataStream.read((void*)attr, attributeLength) < 0)
		{
			return -1;
		}

		m_dataStream.setRelativePosition(3); //Move 3 bytes padding
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

	SignalingMessageType SignalingMessageReader::getMessageType()
	{
		return m_messageType;
	}

	std::string& SignalingMessageReader::getStreamKey()
	{
		return m_streamKey;
	}

	std::string& SignalingMessageReader::getUsername()
	{
		return m_username;
	}

	std::string& SignalingMessageReader::getPassword()
	{
		return m_password;
	}

	ServiceInfo& SignalingMessageReader::getServiceInfo()
	{
		return m_serviceInfo;
	}
} // oppvs
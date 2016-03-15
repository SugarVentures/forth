#include "signaling_message_builder.hpp"

namespace oppvs {
	SignalingMessageBuilder::SignalingMessageBuilder() 
	{
		reset();
	}

	SignalingMessageBuilder::~SignalingMessageBuilder()
	{
		reset();
	}

	void SignalingMessageBuilder::reset()
	{
		m_dataStream.reset();
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
		uint16_t msgType = type;
		m_dataStream.grow(100);
		if (m_dataStream.writeUInt16(htons(msgType)) < 0)
			return -1;

		//Reserve space for message length
		if (m_dataStream.writeUInt16(0) < 0)
	    	return -1;
		return 0;
	}

	int SignalingMessageBuilder::addStreamKey(const std::string& streamKey)
	{
		if (streamKey.length() != STREAM_KEY_SIZE)
			return -1;
		if (m_dataStream.write(streamKey.c_str(), STREAM_KEY_SIZE) < 0)
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
    	m_dataStream.setAbsolutePosition(2);	//Message length is at 3 & 4th byte
    	m_dataStream.writeUInt16(htons(len));
    	m_dataStream.setAbsolutePosition(currentPos);
		return 0;
	}

	int SignalingMessageBuilder::addAttributeHeader(uint16_t type, uint16_t size)
	{
		int ret = 0;
    	ret = m_dataStream.writeUInt16(htons(type));
    	if (ret >= 0)
    		ret = m_dataStream.writeUInt16(htons(size));
    	return ret;
	}

	int SignalingMessageBuilder::addAttribute(uint16_t type, const void* data, uint16_t size)
    {
        uint8_t padBytes[4] = {0};
        size_t padding = 0;

        uint16_t sizeheader = size;

        if (data == NULL)
        {
            size = 0;
        }

        // attributes always start on a 4-byte boundary
        padding = (size % 4) ? (4 - (size % 4)) : 0;

        //zero length attributes as an indicator of something
        if (addAttributeHeader(type, sizeheader) < 0)
        	return -1;

        if (size > 0)
        {
        	if (m_dataStream.write(data, size) < 0)
            	return -1;
        }

        // pad with zeros to get the 4-byte alignment
        if (padding > 0)
        {
        	if (m_dataStream.write(padBytes, padding) < 0)
            	return -1;
        }
        return 0;
    }

	int SignalingMessageBuilder::addStringAttribute(uint16_t type, std::string s)
	{
		if (s == "")
			return -1;

		return addAttribute(type, s.c_str(), s.length());
	}

	int SignalingMessageBuilder::addIceUsername(const std::string& username)
	{
		return addStringAttribute(SIGNALING_ATTRIBUTE_ICE_USERNAME, username);
	}

	int SignalingMessageBuilder::addIcePassword(const std::string& password)
	{
		return addStringAttribute(SIGNALING_ATTRIBUTE_ICE_PASSWORD, password);
	}

	int SignalingMessageBuilder::addIceCandidates(const std::vector<IceCandidate>& candidates)
	{
		uint16_t noCandidates = candidates.size();
		uint32_t priority = 0;
		uint16_t port = 0;
		if (noCandidates == 0)
			return -1;

		noCandidates = htons(noCandidates);
		if (addAttribute(SIGNALING_ATTRIBUTE_ICE_NO_CANDIDATES, &noCandidates, 2) < 0)
			return -1;

		for (uint16_t i = 0; i < candidates.size(); i++)
		{
			if (addStringAttribute(SIGNALING_ATTRIBUTE_ICE_FOUNDATION, candidates[i].foundation) < 0)
			{
				return -1;
			}
			priority = htonl(candidates[i].priority);
			if (addAttribute(SIGNALING_ATTRIBUTE_ICE_PRIORITY, &priority, 4) < 0)
			{
				return -1;
			}
			if (addStringAttribute(SIGNALING_ATTRIBUTE_ICE_IP_ADDRESS, candidates[i].ip) < 0)
			{
				return -1;
			}
			port = htons(candidates[i].port);
			if (addAttribute(SIGNALING_ATTRIBUTE_ICE_PORT, &port, 2) < 0)
			{
				return -1;
			}
			if (addStringAttribute(SIGNALING_ATTRIBUTE_ICE_TYPE, candidates[i].type) < 0)
			{
				return -1;
			}
		}
		return 0;
	}

	int SignalingMessageBuilder::addVideoSources(const VideoStreamInfo& info)
	{
		uint8_t noSources = info.noSources;
		uint16_t width, height, stride;
		uint8_t sid, order, format;
		if (noSources == 0)
			return -1;

		int ret = 0;
		int currentPos = m_dataStream.getPosition();

		if (addAttribute(SIGNALING_ATTRIBUTE_VIDEO_NOSOURCES, &noSources, 1) < 0)
		{
			m_dataStream.setAbsolutePosition(currentPos);
			return -1;
		}

		for (uint8_t i = 0; i < noSources; i++)
		{
			sid = info.sources[i].source;
			order = info.sources[i].order;
			width = htons(info.sources[i].width);
			height = htons(info.sources[i].height);
			stride = htons(info.sources[i].stride);
			format = info.sources[i].format;
			if ((ret = addAttribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_ID, &sid, 1)) < 0)
				break;
			if ((ret = addAttribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_ORDER, &order, 1)) < 0)
				break;
			if ((ret = addAttribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_PIXEL_FORMAT, &format, 1)) < 0)
				break;
			if ((ret = addAttribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_WIDTH, &width, 2)) < 0)
				break;
			if ((ret = addAttribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_HEIGHT, &height, 2)) < 0)
				break;
			if ((ret = addAttribute(SIGNALING_ATTRIBUTE_SOURCE_VIDEO_STRIDE, &stride, 2)) < 0)
				break;
				
		}
		if (ret < 0)
		{
			m_dataStream.setAbsolutePosition(currentPos);
			return -1;	
		}
		return 0;
	}

	int SignalingMessageBuilder::addAudioSources(const AudioStreamInfo& info)
	{
		uint8_t noSources = info.noSources;
		uint8_t aid;
		uint16_t channels;
		uint32_t sampleRate;

		if (noSources == 0)
			return -1;

		int ret = 0;
		int currentPos = m_dataStream.getPosition();

		if (addAttribute(SIGNALING_ATTRIBUTE_AUDIO_NOSOURCES, &noSources, 1) < 0)
		{
			m_dataStream.setAbsolutePosition(currentPos);
			return -1;
		}

		for (uint8_t i = 0; i < noSources; i++)
		{
			aid = info.sources[i].source;
			channels = htons(info.sources[i].numberChannels);
			sampleRate = htonl(info.sources[i].sampleRate);
			if ((ret = addAttribute(SIGNALING_ATTRIBUTE_SOURCE_AUDIO_ID, &aid, 1)) < 0)
				break;
			if ((ret = addAttribute(SIGNALING_ATTRIBUTE_SOURCE_AUDIO_CHANNELS, &channels, 2)) < 0)
				break;
			if ((ret = addAttribute(SIGNALING_ATTRIBUTE_SOURCE_AUDIO_SAMPLE_RATE, &sampleRate, 4)) < 0)
				break;
		}
		if (ret < 0)
		{
			m_dataStream.setAbsolutePosition(currentPos);
			return -1;	
		}
		printf("Added audio source\n");
		return 0;
	}

	int SignalingMessageBuilder::addDuration(const AvailableDuration& dur)
	{
		int ret = 0;
		int currentPos = m_dataStream.getPosition();

		uint32_t start = htonl(dur.start);
		uint32_t end = htonl(dur.end);

		if ((ret = addAttribute(SIGNALING_ATTRIBUTE_DURATION_START, &start, 4)) < 0)
			return -1;
		if ((ret = addAttribute(SIGNALING_ATTRIBUTE_DURATION_END, &end, 4)) < 0)
			return -1;

		printf("Added duration %u %u\n", dur.start, dur.end);
		return 0;
	}
} // oppvs
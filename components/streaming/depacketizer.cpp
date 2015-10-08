#include "depacketizer.h"

namespace oppvs {
	Depacketizer::Depacketizer(): p_recvPool(NULL)
	{

	}

	Depacketizer::~Depacketizer()
	{
		m_videoDecoder.release();
	}

	void Depacketizer::init(ServiceInfo& info, tsqueue<IncomingStreamingFrame*>* p)
	{
		if (info.videoStreamInfo.noSources > 0)
			m_videoDecoder.init(info.videoStreamInfo);
		if (info.audioStreamInfo.noSources > 0)
			m_audioDecoder.init(info.audioStreamInfo);
		p_recvPool = p;
		for(unsigned i = 0; i < info.videoStreamInfo.noSources; ++i) {
			IncomingStreamingMessage* msg = new IncomingStreamingMessage();
			msg->sourceid = info.videoStreamInfo.sources[i].source;
			m_readers.push_back(msg);
		}
		for(unsigned i = 0; i < info.audioStreamInfo.noSources; ++i) {
			IncomingStreamingMessage* msg = new IncomingStreamingMessage();
			msg->sourceid = info.audioStreamInfo.sources[i].source;
			m_readers.push_back(msg);
		}
	}

	SegmentReader* Depacketizer::getReader(uint8_t sourceid)
	{
		for(unsigned i = 0; i < m_readers.size(); ++i) {
			if (m_readers[i]->sourceid == sourceid)
				return &m_readers[i]->reader;
		}
		return NULL;
	}

	void Depacketizer::pushSegment(uint8_t* data, uint32_t len)
	{
		uint32_t timestamp = 0;
		uint8_t sourceid = 0;
		uint16_t type = 0;

		if (len < RTP_HEADER_SIZE)
			return;

		memcpy(&timestamp, data, 4);
		timestamp = ntohl(timestamp);
		memcpy(&sourceid, data + 4, 1);
		memcpy(&type, data + 5, 2);
		type = ntohs(type);
		printf("timestamp %u len: %u source: %d type: %d\n", timestamp, len, sourceid, type);
		
		SegmentReader* reader = getReader(sourceid);
		if (reader == NULL)
			return;
	
		int ret = -1;
		switch (type)
		{
		 	case VP8_PAYLOAD_TYPE:
		 		ret = reader->addVP8Bytes(data, len);
		 		break;
		 	case OPUS_PAYLOAD_TYPE:
		 		ret = reader->addOpusBytes(data, len);
		 		break;
		 	default:
		 		return;
		}

		if (ret < 0)
			return;
		else if (ret == 1)
		{
			IncomingStreamingFrame* frame = new IncomingStreamingFrame();
			frame->sourceid = sourceid;
			frame->type = type;
			frame->data = reader->getBuffer();
			p_recvPool->push(frame);
		}
	}

	int Depacketizer::pullFrame(PixelBuffer& pf, SharedDynamicBufferRef frame)
	{
		if (m_videoDecoder.decode(pf, frame->size(), frame->data()) < 0)
			return -1;

		return 0;
	}

	int Depacketizer::pullFrame(SharedDynamicBufferRef frame, uint8_t source)
	{
		float* out = new float[AUDIO_MAX_ENCODING_PACKET_SIZE];
		int len = m_audioDecoder.decode(frame->data(), frame->size(), source, out);
		printf("audio out decode len %d\n", len);
		delete [] out;
		return 0;
	}
} // oppvs
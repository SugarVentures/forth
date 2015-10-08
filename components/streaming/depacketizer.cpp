#include "depacketizer.h"

namespace oppvs {
	Depacketizer::Depacketizer(): p_recvPool(NULL)
	{

	}

	Depacketizer::~Depacketizer()
	{
		m_decoder.release();
	}

	void Depacketizer::init(VideoStreamInfo& info, tsqueue<IncomingStreamingFrame*>* p)
	{
		m_decoder.init(info);
		p_recvPool = p;
		for(unsigned i = 0; i < info.noSources; ++i) {
			IncomingStreamingMessage* msg = new IncomingStreamingMessage();
			msg->sourceid = info.sources[i].source;
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

		if (len < RTP_HEADER_SIZE)
			return;

		memcpy(&timestamp, data, 4);
		timestamp = ntohl(timestamp);
		memcpy(&sourceid, data + 4, 1);
		//printf("timestamp %u len: %u source: %d\n", timestamp, len, sourceid);
		SegmentReader* reader = getReader(sourceid);
		if (reader == NULL)
			return;
	
		int ret = reader->addBytes(data, len);
		if (ret < 0)
			return;
		else if (ret == 1)
		{
			IncomingStreamingFrame* frame = new IncomingStreamingFrame();
			frame->sourceid = sourceid;
			frame->data = reader->getBuffer();
			p_recvPool->push(frame);
		}
	}

	int Depacketizer::pullFrame(PixelBuffer& pf, SharedDynamicBufferRef frame)
	{
		if (m_decoder.decode(pf, frame->size(), frame->data()) < 0)
			return -1;

		return 0;
	}
} // oppvs
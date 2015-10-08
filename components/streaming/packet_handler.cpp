#include "packet_handler.hpp"

namespace oppvs {

	Packetizer::Packetizer(): m_isRunning(true)
	{
		p_thread = new Thread(Packetizer::run, this);
		p_segmentPool = NULL;
		m_timestamp = 0;
	}

	Packetizer::~Packetizer()
	{
		delete p_thread;
		p_segmentPool = NULL;
		m_encoder.release();
	}

	void Packetizer::init(VideoStreamInfo& info, tsqueue<SharedDynamicBufferRef>* queue)
	{
		m_encoder.init(info);
		p_segmentPool = queue;
	}

	void Packetizer::start()
	{
		p_thread->create();
	}

	bool Packetizer::isRunning()
	{
		return m_isRunning;
	}

	void Packetizer::pushFrame(const PixelBuffer& pf)
	{
		if (m_framePool.size() >= MAX_FRAMES_IN_POOL)
			return;
		std::shared_ptr<PixelBuffer> pixelbuffer(new PixelBuffer);
		pixelbuffer->source = pf.source;
		pixelbuffer->order = pf.order;
		pixelbuffer->plane[0] = new uint8_t[pf.nbytes];
		memcpy(pixelbuffer->plane[0], pf.plane[0], pf.nbytes);
		pixelbuffer->nbytes = pf.nbytes;
		pixelbuffer->width[0] = pf.width[0];
		pixelbuffer->height[0] = pf.height[0];
		pixelbuffer->stride[0] = pf.stride[0];

		m_framePool.push(pixelbuffer);
	}

	void Packetizer::pullFrame()
	{
		uint8_t* data = NULL;
		uint32_t encodingLength, sendLength, sentLength = 0;
		bool isKey = false;
		int picID = -1;

		if (m_framePool.empty())
			return;

		std::shared_ptr<std::shared_ptr<PixelBuffer>> ptrFrame = m_framePool.pop();
		if (ptrFrame.get() == NULL)
		{
			return;
		}

		std::shared_ptr<PixelBuffer> frame = *ptrFrame;
		PixelBuffer pf = *frame;
		
		//Encoding		
		if (m_encoder.encode(pf, &encodingLength, &data, &picID, &isKey) < 0)
		{
			delete [] pf.plane[0];
			return;
		}
		delete [] pf.plane[0];

		//printf("Encoding Length %u Key: %d timestamp: %u\n", encodingLength, isKey, m_timestamp);
		sendLength = encodingLength;
		uint8_t* curPos = data;
		do
		{
			bool flag = (sendLength == encodingLength);
			SharedDynamicBufferRef segment = SharedDynamicBufferRef(new DynamicBuffer());
			segment->setSize(100);
			m_builder.reset();
			m_builder.getStream().attach(segment, true);

			if (m_builder.addRTPHeader(m_timestamp, pf.source, VP8_PAYLOAD_TYPE) < 0)
				return;

			if (m_builder.addVideoCommonHeader(flag, picID) < 0)
				return;
			
			if (flag)
			{
				if (m_builder.addVideoPayloadHeader(isKey, encodingLength) < 0)
					return;
				sentLength = sendLength > (OPPVS_NETWORK_PACKET_LENGTH - VP8_MAX_HEADER_SIZE) ? (OPPVS_NETWORK_PACKET_LENGTH - VP8_MAX_HEADER_SIZE) : sendLength;
			}
			else
			{
				sentLength = sendLength > (OPPVS_NETWORK_PACKET_LENGTH - VP8_COMMON_HEADER_SIZE - RTP_HEADER_SIZE) ? 
				(OPPVS_NETWORK_PACKET_LENGTH - VP8_COMMON_HEADER_SIZE - RTP_HEADER_SIZE) : sendLength;
			}
			if (m_builder.addPayload(curPos, sentLength) < 0)
				return;
			sendLength -= sentLength;
			curPos += sentLength;

			//printf("Sent length: %u\n", sentLength);
			p_segmentPool->push(segment);
		}
		while (sendLength > 0);
		m_timestamp++;
	}

	void* Packetizer::run(void* object)
	{
		Packetizer* handler = (Packetizer*)object;
		while (handler->isRunning())
		{
			handler->pullFrame();
			usleep(30000);
		}
		return NULL;
	}


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
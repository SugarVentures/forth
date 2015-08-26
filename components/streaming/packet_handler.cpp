#include "packet_handler.hpp"

namespace oppvs {
	SegmentBuilder::SegmentBuilder()
	{
		reset();
	}

	SegmentBuilder::~SegmentBuilder()
	{
		reset();
	}

	void SegmentBuilder::reset()
	{
		m_dataStream.reset();
	}

	int SegmentBuilder::addPayload(uint8_t* data, uint32_t size)
	{
		return m_dataStream.write(data, size);
	}

	int SegmentBuilder::addCommonHeader(bool flag, int picID)
	{
		uint8_t req = XBit;
		if (flag)
			req |= SBit;
		uint8_t optX = IBit;
		uint8_t optI = (picID & 127);

		//Add VP8 Required
		if (m_dataStream.writeUInt8(req) < 0)
			return -1;
		if (m_dataStream.writeUInt8(optX) < 0)
			return -1;
		if (m_dataStream.writeUInt8(optI) < 0)
			return -1;
		return 0;
	}

	int SegmentBuilder::addPayloadHeader(bool isKeyFrame, uint32_t size)
	{
		//VP8 Payload Header
		uint8_t o1 = (size & Size0BitMask) << Size0BitShift; // Size0
		o1 |= HBit; // H (show frame)
		if (!isKeyFrame)
		{
			o1 |= 1; //P (Inverse frame)
		}

		if (m_dataStream.writeUInt8(o1) < 0)
			return -1;
		if (m_dataStream.writeUInt8(static_cast<uint8_t>(size >> 3)) < 0)	//Size 1
			return -1;

		if (m_dataStream.writeUInt8(static_cast<uint8_t>(size >> 11)) < 0) //Size 2
			return -1;
		return 0;
	}

	int SegmentBuilder::addRTPHeader(uint32_t timestamp, uint8_t sourceid)
	{
		if (m_dataStream.writeUInt32(htonl(timestamp)) < 0)
			return -1;
		if (m_dataStream.writeUInt8(sourceid) < 0)
			return -1;	
		return 0;
	}

	DataStream& SegmentBuilder::getStream()
	{
		return m_dataStream;
	}

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

		printf("Encoding Length %u Key: %d timestamp: %u\n", encodingLength, isKey, m_timestamp);
		sendLength = encodingLength;
		uint8_t* curPos = data;
		do
		{
			bool flag = (sendLength == encodingLength);
			SharedDynamicBufferRef segment = SharedDynamicBufferRef(new DynamicBuffer());
			segment->setSize(100);
			m_builder.reset();
			m_builder.getStream().attach(segment, true);

			if (m_builder.addRTPHeader(m_timestamp, pf.source) < 0)
				return;

			if (m_builder.addCommonHeader(flag, picID) < 0)
				return;
			
			if (flag)
			{
				if (m_builder.addPayloadHeader(isKey, encodingLength) < 0)
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

			printf("Sent length: %u\n", sentLength);
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

	SegmentReader::SegmentReader(): m_keyFrame(false)
	{
		reset();
	}

	SegmentReader::~SegmentReader()
	{
		reset();
	}

	void SegmentReader::reset()
	{
		m_sourceId = -1;
		m_dataStream.reset();
	}

	SharedDynamicBufferRef SegmentReader::getBuffer()
	{
		return m_dataStream.getBuffer();
	}

	int SegmentReader::addBytes(uint8_t* data, uint32_t len)
	{
		uint8_t req = 0;
		uint8_t optx = 0;
		uint8_t opti = 0;
		int picID = -1;
		uint32_t curPos = 0;
		bool showFrame = false;
		
		curPos = RTP_HEADER_SIZE;
		req = data[curPos];
		if (req & XBit)
		{
			optx = data[curPos + 1];
			if (optx & IBit)
			{
				opti = data[curPos + 2];
				picID = opti >> 1;
			}
		}
		if (req & SBit)
		{
			//First segment => Read payload header
			curPos += VP8_COMMON_HEADER_SIZE;
			uint8_t o1 = data[curPos];
			showFrame = o1 & HBit;
			m_keyFrame = !(o1 & 1);
			o1 >>= Size0BitShift;
			uint32_t frameSize = o1 + 8 * data[curPos + 1] + 2048 * data[curPos + 2];
			printf("Size: %u key: %d\n", frameSize, m_keyFrame);
			//Create buffer for new frame
			m_dataStream.reset();
			SharedDynamicBufferRef buffer = SharedDynamicBufferRef(new DynamicBuffer());
			buffer->setCapacity(frameSize);
			m_dataStream.attach(buffer, true);
			if (m_dataStream.write(data + VP8_MAX_HEADER_SIZE, len - VP8_MAX_HEADER_SIZE) < 0)
				return -1;
		}
		else
		{
			if (m_dataStream.capacity() == 0)
				return -1;
			if (m_dataStream.size() + len - VP8_COMMON_HEADER_SIZE - RTP_HEADER_SIZE > m_dataStream.capacity())
			{
				printf("Wrong segment\n");
				m_dataStream.reset();
				return -1;
			}
			if (m_dataStream.write(data + VP8_COMMON_HEADER_SIZE + RTP_HEADER_SIZE, len - VP8_COMMON_HEADER_SIZE - RTP_HEADER_SIZE) < 0)
				return -1;
			//printf("Current size: %u cap: %u\n", m_dataStream.size(), m_dataStream.capacity());
		}
		if (m_dataStream.size() == m_dataStream.capacity())
		{
			//printf("Frame size: %u\n", m_dataStream.size());
			return 1;
		}

		return 0;
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
		printf("timestamp %u len: %u source: %d\n", timestamp, len, sourceid);
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
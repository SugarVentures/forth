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

	DataStream& SegmentBuilder::getStream()
	{
		return m_dataStream;
	}

	Packetizer::Packetizer(): m_isRunning(true)
	{
		p_thread = new Thread(Packetizer::run, this);
		p_segmentPool = NULL;
	}

	Packetizer::~Packetizer()
	{
		delete p_thread;
		p_segmentPool = NULL;
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

		printf("Encoding Length %u Key: %d\n", encodingLength, isKey);
		sendLength = encodingLength;
		uint8_t* curPos = data;
		do
		{
			bool flag = (sendLength == encodingLength);
			SharedDynamicBufferRef segment = SharedDynamicBufferRef(new DynamicBuffer());
			segment->setSize(100);
			m_builder.reset();
			m_builder.getStream().attach(segment, true);

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
				sentLength = sendLength > (OPPVS_NETWORK_PACKET_LENGTH - VP8_COMMON_HEADER_SIZE) ? (OPPVS_NETWORK_PACKET_LENGTH - VP8_COMMON_HEADER_SIZE) : sendLength;
			}
			if (m_builder.addPayload(curPos, sentLength) < 0)
				return;
			sendLength -= sentLength;
			curPos += sentLength;

			printf("Sent length: %u\n", sentLength);
			p_segmentPool->push(segment);
		}
		while (sendLength > 0);
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


} // oppvs
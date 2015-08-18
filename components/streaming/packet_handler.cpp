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

	int SegmentBuilder::addBytes(uint8_t* data, uint16_t size, bool flag, bool isKeyFrame, int picID)
	{
		m_dataStream.grow(100);
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

		if (flag)
		{
			//VP8 Payload Header
			uint8_t o1 = (size & Size0BitMask) << Size0BitShift; // Size0
			o1 |= HBit; // H (show frame)
			if (!isKeyFrame)
			{
				o1 |= 1; //P (Inverse frame)
			}

			m_dataStream.writeUInt8(o1);
			m_dataStream.writeUInt8(static_cast<uint8_t>(size >> 3));	//Size 1
			m_dataStream.writeUInt8(static_cast<uint8_t>(size >> 11));	//Size 2
		}
		return 0;
	}


	PacketHandler::PacketHandler()
	{

	}

	PacketHandler::~PacketHandler()
	{

	}

	void PacketHandler::pushFrame(const PixelBuffer& pf)
	{
		printf("Number of frames in pool %u\n", m_framePool.size());
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

} // oppvs
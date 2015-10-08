#include "segment_builder.h"

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

	int SegmentBuilder::addVideoCommonHeader(bool flag, int picID)
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

	int SegmentBuilder::addVideoPayloadHeader(bool isKeyFrame, uint32_t size)
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

	int SegmentBuilder::addRTPHeader(uint32_t timestamp, uint8_t sourceid, uint16_t type)
	{
		if (m_dataStream.writeUInt32(htonl(timestamp)) < 0)
			return -1;
		if (m_dataStream.writeUInt8(sourceid) < 0)
			return -1;
		if (m_dataStream.writeUInt16(type) < 0)
			return -1;
		return 0;
	}

	DataStream& SegmentBuilder::getStream()
	{
		return m_dataStream;
	}
} // oppvs	
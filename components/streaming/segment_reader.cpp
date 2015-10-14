#include "segment_reader.h"

namespace oppvs {
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

	int SegmentReader::addVP8Bytes(uint8_t* data, uint32_t len)
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
			//printf("Size: %u key: %d\n", frameSize, m_keyFrame);
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

	int SegmentReader::addOpusBytes(uint8_t* data, uint32_t len)
	{
		m_dataStream.reset();
		SharedDynamicBufferRef buffer = SharedDynamicBufferRef(new DynamicBuffer());
		uint32_t frameSize = len - OPUS_MAX_HEADER_SIZE;
		buffer->setCapacity(frameSize);
		m_dataStream.attach(buffer, true);
		if (m_dataStream.write(data + OPUS_MAX_HEADER_SIZE, len - OPUS_MAX_HEADER_SIZE) < 0)
			return -1;

		if (m_dataStream.size() == m_dataStream.capacity())
		{
			return 1;
		}
		return 0;
	}
} // oppvs
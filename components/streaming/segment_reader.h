#ifndef OPPVS_SEGMENT_READER_H
#define OPPVS_SEGMENT_READER_H

#include "datatypes.hpp"
#include "data_stream.hpp"

namespace oppvs {
	class SegmentReader
	{
	private:
		DataStream m_dataStream;
		uint8_t SBit = 1 << 4;
		uint8_t XBit = 1 << 7;
		uint8_t IBit = 1 << 7;
		uint8_t HBit = 1 << 4;
		uint8_t Size0BitMask = 7;
		uint8_t Size0BitShift = 5;

		bool m_keyFrame;
		int m_sourceId;

	public:
		SegmentReader();
		~SegmentReader();

		void reset();
		SharedDynamicBufferRef getBuffer();

		int addBytes(uint8_t* data, uint32_t len);
	};

} // oppvs

#endif // OPPVS_SEGMENT_READER_H

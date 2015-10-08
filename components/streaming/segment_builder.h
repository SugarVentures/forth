#ifndef OPPVS_SEGMENT_BUILDER_H
#define OPPVS_SEGMENT_BUILDER_H

#include "datatypes.hpp"
#include "data_stream.hpp"

namespace oppvs {

	class SegmentBuilder {
	private:
		DataStream m_dataStream;

		uint8_t SBit = 1 << 4;
		uint8_t XBit = 1 << 7;
		uint8_t IBit = 1 << 7;
		uint8_t HBit = 1 << 4;
		uint8_t Size0BitMask = 7;
		uint8_t Size0BitShift = 5;

	public:
		SegmentBuilder();
		~SegmentBuilder();

		void reset();
		DataStream& getStream();

		int addVideoCommonHeader(bool flag, int picID);
		int addVideoPayloadHeader(bool isKeyFrame, uint32_t length);
		int addPayload(uint8_t* data, uint32_t size);

		int addRTPHeader(uint32_t timestamp, uint8_t sourceid, uint16_t type);
	};
} // oppvs
	
#endif // OPPVS_SEGMENT_BUILDER_H

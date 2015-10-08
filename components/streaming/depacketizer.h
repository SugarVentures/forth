#ifndef OPPVS_DEPACKETIZER_H
#define OPPVS_DEPACKETIZER_H

#include "datatypes.hpp"
#include "tsqueue.hpp"
#include "data_stream.hpp"
#include "thread.hpp"

#include "segment_reader.h"
#include "video_decoding_vp.hpp"

namespace oppvs {
	struct IncomingStreamingMessage
	{
		uint8_t sourceid;
		SegmentReader reader;
	};

	struct IncomingStreamingFrame
	{
		uint8_t sourceid;
		SharedDynamicBufferRef data;
	};

	class Depacketizer {
	private:
		VPVideoDecoder m_decoder;
		std::vector<IncomingStreamingMessage*> m_readers;
		tsqueue<IncomingStreamingFrame*>* p_recvPool;

		SegmentReader* getReader(uint8_t sourceid);
	public:
		Depacketizer();
		~Depacketizer();

		void init(ServiceInfo&, tsqueue<IncomingStreamingFrame*>*);
		void pushSegment(uint8_t* data, uint32_t len);
		int pullFrame(PixelBuffer&, SharedDynamicBufferRef);
	};
} // oppvs

#endif // OPPVS_DEPACKETIZER_H

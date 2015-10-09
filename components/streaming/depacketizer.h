#ifndef OPPVS_DEPACKETIZER_H
#define OPPVS_DEPACKETIZER_H

#include "datatypes.hpp"
#include "tsqueue.hpp"
#include "data_stream.hpp"
#include "thread.hpp"

#include "segment_reader.h"
#include "video_decoding_vp.hpp"
#include "audio_opus_decoder.hpp"

#include "audio_ring_buffer.h"

namespace oppvs {
	struct IncomingStreamingMessage
	{
		uint8_t sourceid;
		SegmentReader reader;
	};

	struct IncomingStreamingFrame
	{
		uint8_t sourceid;
		uint16_t type;
		SharedDynamicBufferRef data;
	};

	class Depacketizer {
	private:
		VPVideoDecoder m_videoDecoder;
		AudioOpusDecoder m_audioDecoder;

		std::vector<IncomingStreamingMessage*> m_readers;
		tsqueue<IncomingStreamingFrame*>* p_recvPool;

		SegmentReader* getReader(uint8_t sourceid);
		AudioRingBuffer* p_audioRingBuffer;
		uint32_t m_timestamp;
	public:
		Depacketizer();
		~Depacketizer();

		void init(ServiceInfo&, tsqueue<IncomingStreamingFrame*>*, AudioRingBuffer* pbuf);
		void pushSegment(uint8_t* data, uint32_t len);
		int pullFrame(PixelBuffer&, SharedDynamicBufferRef);
		int pullFrame(SharedDynamicBufferRef, uint8_t source);
	};
} // oppvs

#endif // OPPVS_DEPACKETIZER_H

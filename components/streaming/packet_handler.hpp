#ifndef OPPVS_PACKET_HANDLER_HPP
#define OPPVS_PACKET_HANDLER_HPP

#include "datatypes.hpp"
#include "tsqueue.hpp"
#include "data_stream.hpp"
#include "thread.hpp"
#include "video_encoding_vp.hpp"
#include "video_decoding_vp.hpp"

#include "segment_builder.h"
#include "segment_reader.h"

namespace oppvs {

	class Packetizer {
	private:
		tsqueue<std::shared_ptr<PixelBuffer>> m_framePool;
		tsqueue<SharedDynamicBufferRef>* p_segmentPool;

		const static uint16_t MAX_FRAMES_IN_POOL = 10;
		Thread* p_thread;
		bool m_isRunning;
		VPVideoEncoder m_encoder;
		SegmentBuilder m_builder;
		uint32_t m_timestamp;

	public:
		Packetizer();
		~Packetizer();

		void init(VideoStreamInfo&, tsqueue<SharedDynamicBufferRef>*);
		void start();
		void pushFrame(const PixelBuffer& pf);
		void pullFrame();
		static void* run(void* object);

		bool isRunning();
	};
	
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

		void init(VideoStreamInfo&, tsqueue<IncomingStreamingFrame*>*);
		void pushSegment(uint8_t* data, uint32_t len);
		int pullFrame(PixelBuffer&, SharedDynamicBufferRef);
	};
} // oppvs

#endif // OPPVS_PACKET_HANDLER_HPP

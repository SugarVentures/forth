#ifndef OPPVS_VIDEO_PACKETIZER_H
#define OPPVS_VIDEO_PACKETIZER_H

#include "datatypes.hpp"
#include "tsqueue.hpp"
#include "data_stream.hpp"
#include "thread.hpp"
#include "video_encoding_vp.hpp"
#include "segment_builder.h"

namespace oppvs {

	class VideoPacketizer {
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
		VideoPacketizer();
		~VideoPacketizer();

		void init(VideoStreamInfo&, tsqueue<SharedDynamicBufferRef>*);
		void start();
		void pushFrame(const PixelBuffer& pf);
		void pullFrame();
		static void* run(void* object);

		bool isRunning();
	};
	
} // oppvs

#endif // OPPVS_VIDEO_PACKETIZER_H

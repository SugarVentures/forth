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
#include "video_frame_buffer.h"

namespace oppvs {
	

	class DepacketizerThread;

	struct IncomingStreamingMessage
	{
		uint8_t sourceid;
		SegmentReader reader;
		DepacketizerThread* thread;
	};

	struct IncomingStreamingFrame
	{
		uint8_t sourceid;
		uint16_t type;
		uint32_t delay;
		uint32_t timestamp;
		SharedDynamicBufferRef data;
	};

	typedef std::shared_ptr<IncomingStreamingFrame> SharedIncomingStreamingFrame;

	class Depacketizer {
	private:
		VPVideoDecoder m_videoDecoder;
		AudioOpusDecoder m_audioDecoder;

		std::vector<IncomingStreamingMessage*> m_readers;
		tsqueue<IncomingStreamingFrame*>* p_recvPool;

		SegmentReader* getReader(uint8_t sourceid);
		DepacketizerThread* getThread(uint8_t sourceid);

		AudioRingBuffer* p_audioRingBuffer;
		VideoFrameBuffer* p_videoFrameBuffer;

		uint32_t m_timestamp;

		ServiceInfo* p_serviceInfo;
		frame_callback m_callback;

		int pullFrame(PixelBuffer&, SharedDynamicBufferRef);
		int pullFrame(SharedDynamicBufferRef, uint8_t source, uint32_t ts);
        
        void* m_user;
	public:
		Depacketizer();
		~Depacketizer();

		void init(ServiceInfo*, VideoFrameBuffer* pvideoBuf, AudioRingBuffer* paudioBuf);
		void attachCallback(frame_callback cb, void* user);
        void attachCallback(frame_callback cb);
		void start();
		void pushSegment(uint8_t* data, uint32_t len);
		void pullFrame(SharedIncomingStreamingFrame frame);
	};

	class DepacketizerThread : public Thread
	{
	public:
		DepacketizerThread(Depacketizer*);
		~DepacketizerThread();

		void pushFrame(SharedIncomingStreamingFrame);
		uint32_t getLastPTS();
	private:
		bool m_exitThread;
		Depacketizer* p_depacketizer;
		tsqueue<SharedIncomingStreamingFrame> m_queue;
		static void* run(void* object);

		void processFrame();
		uint64_t m_idleTime;
		uint32_t m_lastPTS;
	};
} // oppvs

#endif // OPPVS_DEPACKETIZER_H

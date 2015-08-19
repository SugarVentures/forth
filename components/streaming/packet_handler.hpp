#ifndef OPPVS_PACKET_HANDLER_HPP
#define OPPVS_PACKET_HANDLER_HPP

#include "datatypes.hpp"
#include "tsqueue.hpp"
#include "data_stream.hpp"
#include "thread.hpp"
#include "video_encoding_vp.hpp"

namespace oppvs {
	const int VP8_PAYLOAD_HEADER_SIZE = 3;
	const int VP8_COMMON_HEADER_SIZE = 3;
	const int VP8_MAX_HEADER_SIZE = VP8_COMMON_HEADER_SIZE + VP8_PAYLOAD_HEADER_SIZE;

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

		int addCommonHeader(bool flag, int picID);
		int addPayloadHeader(bool isKeyFrame, uint32_t length);
		int addPayload(uint8_t* data, uint32_t size);
	};

	class Packetizer {
	private:
		tsqueue<std::shared_ptr<PixelBuffer>> m_framePool;
		tsqueue<SharedDynamicBufferRef> m_segmentPool;

		const static uint16_t MAX_FRAMES_IN_POOL = 10;
		Thread* p_thread;
		bool m_isRunning;
		VPVideoEncoder m_encoder;
		SegmentBuilder m_builder;

	public:
		Packetizer();
		~Packetizer();

		void init(VideoStreamInfo&);
		void start();
		void pushFrame(const PixelBuffer& pf);
		void pullFrame();
		static void* run(void* object);

		bool isRunning();
	};
} // oppvs

#endif // OPPVS_PACKET_HANDLER_HPP

#ifndef OPPVS_PACKET_HANDLER_HPP
#define OPPVS_PACKET_HANDLER_HPP

#include "datatypes.hpp"
#include "tsqueue.hpp"
#include "data_stream.hpp"
#include "thread.hpp"
#include "video_encoding_vp.hpp"
#include "video_decoding_vp.hpp"

namespace oppvs {
	const int VP8_PAYLOAD_HEADER_SIZE = 3;
	const int VP8_COMMON_HEADER_SIZE = 3;
	const int RTP_HEADER_SIZE = 5;
	const int VP8_MAX_HEADER_SIZE = VP8_COMMON_HEADER_SIZE + VP8_PAYLOAD_HEADER_SIZE + RTP_HEADER_SIZE;

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

		int addRTPHeader(uint32_t timestamp, uint8_t sourceid);
	};

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

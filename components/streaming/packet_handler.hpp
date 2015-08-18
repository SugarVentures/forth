#ifndef OPPVS_PACKET_HANDLER_HPP
#define OPPVS_PACKET_HANDLER_HPP

#include "datatypes.hpp"
#include "tsqueue.hpp"
#include "data_stream.hpp"
#include "thread.hpp"

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

		int addBytes(uint8_t* data, uint16_t size, bool flag, bool isKeyFrame, int picID);	//Flag is true in case of first segment
	};

	class PacketHandler {
	private:
		tsqueue<std::shared_ptr<PixelBuffer>> m_framePool;
		const static uint16_t MAX_FRAMES_IN_POOL = 10;
		Thread* p_thread;
		bool m_isRunning;


	public:
		PacketHandler();
		~PacketHandler();

		void pushFrame(const PixelBuffer& pf);
		void pullFrame();
		static void* run(void* object);

		bool isRunning();
	};
} // oppvs

#endif // OPPVS_PACKET_HANDLER_HPP

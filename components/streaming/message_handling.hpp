#ifndef OPPVS_MESSAGE_HANDLING_HPP
#define OPPVS_MESSAGE_HANDLING_HPP

#include "datatypes.hpp"
#include "concurrent_queue.hpp"
#include "tsqueue.hpp"
#include "cache_buffer.hpp"

#include "video_encoding.hpp"
#include "video_encoding_vp.hpp"

namespace oppvs
{

	const static uint8_t FLAG_START_FRAME = 1;
	const static uint8_t FLAG_MIDDLE_FRAME = 2;
	const static uint8_t FLAG_END_FRAME = 3;
	const static uint8_t FLAG_ONE_FRAME = 4;
	const static uint16_t MAX_FRAMES_IN_POOL = 10;
	const static uint8_t MESSAGE_HEADER_SIZE = 10;

	class Message
	{
	public:
		Message();
		virtual ~Message();
		void setFlag(uint8_t);
		uint8_t getFlag();
		void setSource(uint8_t);
		uint8_t getSource();
		void setSegID(uint16_t seg);
		uint16_t getSegID();

		//Header for VP8 packets
		void setVP8Required(uint8_t req);
		uint8_t getVP8Required();
		void setVP8OptX(uint8_t optx);
		uint8_t getVP8OptX();
		void setVP8OptY(uint8_t opty);
		uint8_t getVP8OptY();
		void setSize0(uint8_t size0);
		uint8_t getSize0();
		void setSize1(uint8_t size1);
		uint8_t getSize1();
		void setSize2(uint8_t size2);
		uint8_t getSize2();

		void setData(const uint8_t *data, uint16_t length);

		uint16_t getLength();
		void setLength(uint16_t);
		uint8_t* getData();
		void setTimestamp(uint32_t);
		uint32_t getTimestamp();
	private:
		uint16_t m_length;
		uint32_t m_timestamp;
		uint8_t m_data[OPPVS_NETWORK_PACKET_LENGTH];

	};

	class MessageHandling
	{
	public:
		MessageHandling();
		virtual ~MessageHandling();

		void setNumClients(uint8_t clients);
		void setEncoder(VPVideoEncoding*);
		void addMessage(PixelBuffer& pf);
		void getNextMessage(uint8_t** pdata, uint16_t* length, uint32_t* ts);
		bool releaseMessage();
		bool isEmptyPool();
	private:
		tsqueue<std::shared_ptr<Message>> m_messagePool;
		uint16_t m_numFramesInPool;
		uint8_t m_numClients;
		uint8_t m_sentClients;
		uint32_t m_timestamp;

		VPVideoEncoding *m_encoder;

		static const uint8_t SBit = 1 << 4;
		static const uint8_t XBit = 1 << 7;
		static const uint8_t IBit = 1 << 7;
		static const uint8_t HBit = 1 << 4;
		static const uint8_t Size0BitMask = 7;
		static const uint8_t Size0BitShift = 5;

	};

	class MessageParsing
	{
	public:
		MessageParsing();
		virtual ~MessageParsing();

		void updateMessage(Message&);
		void setCacheBuffer(CacheBuffer *cb);
	private:
		CacheBuffer *m_cacheBuffer;
		//Test
		int oldseq;
		int count;
		uint32_t m_currentTimestamp;
		long int m_totalLength;

		static const uint8_t SBit = 1 << 4;
		static const uint8_t XBit = 1 << 7;
		static const uint8_t IBit = 1 << 7;
		static const uint8_t HBit = 1 << 4;
		static const uint8_t Size0BitShift = 5;

	};

};


#endif
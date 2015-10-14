#ifndef OPPVS_AUDIO_PACKET_HANDLER_H
#define OPPVS_AUDIO_PACKET_HANDLER_H

#include "datatypes.hpp"
#include "tsqueue.hpp"
#include "audio_ring_buffer.h"
#include "data_stream.hpp"
#include "thread.hpp"
#include "audio_opus_encoder.hpp"

#include "segment_builder.h"
#include "segment_reader.h"

namespace oppvs {
	class AudioPacketizer
	{
	private:
		tsqueue<SharedDynamicBufferRef>* p_segmentPool;
		AudioRingBuffer* p_audioBuffer;
		uint8_t m_source;
		uint8_t m_size;
		uint8_t m_channels;
		uint32_t m_sampleRate;
		uint32_t m_timestamp;
		uint64_t m_firstTime;

		AudioOpusEncoder m_encoder;
		SegmentBuilder m_builder;
		Thread* p_thread;
		bool m_isRunning;
		float m_inBuffer[AUDIO_MAX_ENCODING_PACKET_SIZE];

	public:
		AudioPacketizer();
		~AudioPacketizer();

		int init(const AudioStreamInfo&, tsqueue<SharedDynamicBufferRef>*);
		void start();
		void push(const GenericAudioBufferList& ab);
		void pull();

		bool isRunning();

		static void* run(void* object);
	};
} // oppvs

#endif // OPPVS_AUDIO_PACKET_HANDLER_H

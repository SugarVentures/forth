#ifndef OPPVS_AUDIO_RING_BUFFER_H
#define OPPVS_AUDIO_RING_BUFFER_H

#include "ring_buffer.h"

namespace oppvs {
	class AudioRingBuffer : public RingBuffer 
	{
	public:
		AudioRingBuffer();

		void allocate(uint32_t bytesPerFrame, uint32_t maxFrames);
        RingBufferError store(uint32_t* ioFrames, const void* data, uint64_t timeStamp);
        RingBufferError fetch(uint32_t ioFrames, void* data, uint64_t timeStamp);
        uint32_t getNumberFrames();
        uint64_t getStartTime();
	protected:
		uint32_t m_bytesPerFrame;
		uint32_t m_maxFrames;
        
        volatile uint64_t m_startTime;
        volatile uint64_t m_endTime;
	};
} // oppvs
	
#endif // OPPVS_AUDIO_RING_BUFFER_H

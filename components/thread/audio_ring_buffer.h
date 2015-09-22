#ifndef OPPVS_AUDIO_RING_BUFFER_H
#define OPPVS_AUDIO_RING_BUFFER_H

#include "ring_buffer.h"

namespace oppvs {
	class AudioRingBuffer : public RingBuffer 
	{
	public:
		AudioRingBuffer();

		void allocate(int noChannels, uint32_t bytesPerFrame, uint32_t maxFrames);
	protected:
		int m_noChannels;
		uint32_t m_bytesPerFrame;
		uint32_t m_maxFrames;
	};
} // oppvs
	
#endif // OPPVS_AUDIO_RING_BUFFER_H

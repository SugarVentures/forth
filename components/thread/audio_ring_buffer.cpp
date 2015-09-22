#include "audio_ring_buffer.h"

namespace oppvs {
	AudioRingBuffer::AudioRingBuffer() : RingBuffer(), m_noChannels(0), m_bytesPerFrame(0),
		m_maxFrames(0)
	{

	}
} // oppvs
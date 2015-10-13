#ifndef OPPVS_VIDEO_FRAME_BUFFER_H
#define OPPVS_VIDEO_FRAME_BUFFER_H

#include "ring_buffer.h"

namespace oppvs {

	class VideoFrameBuffer : public RingBuffer
	{
	public:
		VideoFrameBuffer();
		void allocate(uint32_t maxFrames);
		RingBufferError store(PixelBuffer* pf, uint32_t timestamp);
		RingBufferError fetch(PixelBuffer* pf, uint32_t timestamp);
	protected:
		uint32_t m_maxFrames;
		uint32_t m_bytesPerFrame;
	};
} // oppvs

#endif // OPPVS_VIDEO_FRAME_BUFFER_H

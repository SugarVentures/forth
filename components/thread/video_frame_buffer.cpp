#include "video_frame_buffer.h"

namespace oppvs {
	VideoFrameBuffer::VideoFrameBuffer() : m_maxFrames(10), m_bytesPerFrame(0)
	{

	}

	void VideoFrameBuffer::allocate(uint32_t maxFrames)
	{
		PixelBuffer* pf;
		RingBuffer::allocate(sizeof(pf) * maxFrames);

		m_bytesPerFrame = sizeof(pf);
		m_maxFrames = maxFrames;
	}

	RingBufferError VideoFrameBuffer::store(PixelBuffer* pf, uint32_t timestamp)
	{
		RingBufferError err = RingBufferError_OK;
		int inLen = sizeof(pf);
		int outLen = 0;
		
		
		return err;	
	}

	RingBufferError VideoFrameBuffer::fetch(PixelBuffer* pf, uint32_t timestamp)
	{
		RingBufferError err = RingBufferError_OK;
		
		return err;	
	}
} // oppvs
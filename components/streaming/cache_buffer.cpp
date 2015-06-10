#include "cache_buffer.hpp"

namespace oppvs
{
	CacheBuffer::CacheBuffer(VideoStreamInfo& info)
	{
		m_numSources = info.noSources;
		if (m_numSources > 0 && m_numSources < OPPVS_MAX_CAPTURE_SOURCES)
		{
			for (int i = 0; i < m_numSources; i++)
			{
				pixelBuffer[i].width[0] = info.sources[i].width;
				pixelBuffer[i].height[0] = info.sources[i].height;
				pixelBuffer[i].stride[0] = info.sources[i].stride;
				pixelBuffer[i].order = info.sources[i].order;
				pixelBuffer[i].nbytes = info.sources[i].height * info.sources[i].stride;
				pixelBuffer[i].plane[0] = new uint8_t[pixelBuffer[i].nbytes];
				pixelBuffer[i].source = info.sources[i].source;
			}
		}
	}


	CacheBuffer::~CacheBuffer()
	{
		for (int i = 0; i < m_numSources; i++)
		{
			delete [] pixelBuffer[i].plane[0];
		}
	}

	uint8_t* CacheBuffer::getBufferAddress(uint8_t source, uint32_t loc)
	{
		for (int i = 0; i < m_numSources; i++)
		{
			if (pixelBuffer[i].source == source)
			{
				if (pixelBuffer[i].nbytes < loc)
					return NULL;
				return pixelBuffer[i].plane[0] + loc;
			}
		}
		return NULL;
	}

	PixelBuffer* CacheBuffer::getBuffer(uint8_t source)
	{
		for (int i = 0; i < m_numSources; i++)
		{
			if (pixelBuffer[i].source == source)
			{
				return &pixelBuffer[i];
			}
		}
		return NULL;
	}
}
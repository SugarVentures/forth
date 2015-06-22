#include "cache_buffer.hpp"

namespace oppvs
{
	CacheBuffer::CacheBuffer(VideoStreamInfo& info): isWaiting(true)
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
				//pixelBuffer[i].plane[0] = NULL;
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

	bool CacheBuffer::allocateBuffer(uint8_t source)
	{
		for (int i = 0; i < m_numSources; i++)
		{
			if (pixelBuffer[i].source == source)
			{	
				//pixelBuffer[i].plane[0] = new uint8_t[pixelBuffer[i].nbytes];
				//pixelBuffer[i].plane[0] = new uint8_t[pixelBuffer[i].width[0] * pixelBuffer[i].height[0] * 3 / 2];
				return true;
			}
		}
		return false;
	}

	void CacheBuffer::delocateBuffer(uint8_t source)
	{
		for (int i = 0; i < m_numSources; i++)
		{
			if (pixelBuffer[i].source == source)
			{	
				//delete [] pixelBuffer[i].plane[0];
			}
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
				if (pixelBuffer[i].plane[0] == NULL)
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

	void CacheBuffer::push(uint8_t source, uint32_t length)
	{
		for (int i = 0; i < m_numSources; i++)
		{
			if (pixelBuffer[i].source == source)
			{
				std::shared_ptr<PixelBuffer> pf(new PixelBuffer);
				pf->width[0] = pixelBuffer[i].width[0];
				pf->height[0] = pixelBuffer[i].height[0];
				pf->stride[0] = pixelBuffer[i].stride[0];
				pf->order = pixelBuffer[i].order;
				printf("Length: %u\n", length);
				pf->nbytes = length;
				pf->plane[0] = new uint8_t[length];
				memcpy(pf->plane[0], pixelBuffer[i].plane[0], length);
				//pixelBuffer[i].plane[0] = NULL;
				pf->source = source;
				m_framePool.push(pf);
			}
		}
	}

	std::shared_ptr<PixelBuffer> CacheBuffer::pop()
	{
		if (m_framePool.size() > OPPVS_MAX_CACHE_POOL_SIZE)
		{
			isWaiting = false;
		}
		if (!isWaiting)
		{
			/*if (m_framePool.size() < 5)
			{
				isWaiting = true;
				return std::shared_ptr<PixelBuffer>();
			}*/
			
			std::shared_ptr<std::shared_ptr<PixelBuffer>> ptr = m_framePool.try_pop();
			if (ptr.get() != NULL)
			{
				return *ptr;
			}
		}

		return std::shared_ptr<PixelBuffer>();
	}
}

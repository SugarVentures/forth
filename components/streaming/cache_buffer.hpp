#ifndef OPPVS_CACHE_BUFFER_HPP
#define OPPVS_CACHE_BUFFER_HPP

#include "datatypes.hpp"

namespace oppvs
{

	class CacheBuffer
	{
		public:
			CacheBuffer(VideoStreamInfo&);
			virtual ~CacheBuffer();
			uint8_t *getBufferAddress(uint8_t source, uint32_t loc);
			PixelBuffer* getBuffer(uint8_t source);
			
		private:
			PixelBuffer pixelBuffer[OPPVS_MAX_CAPTURE_SOURCES];
			int m_numSources;
	};
}

#endif
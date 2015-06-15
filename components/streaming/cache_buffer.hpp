#ifndef OPPVS_CACHE_BUFFER_HPP
#define OPPVS_CACHE_BUFFER_HPP

#include "datatypes.hpp"
#include "tsqueue.hpp"
#include "video_encoding.hpp"

namespace oppvs
{
	const static int OPPVS_MAX_CACHE_POOL_SIZE = 20;

	class CacheBuffer
	{
		public:
			CacheBuffer(VideoStreamInfo&);
			virtual ~CacheBuffer();
			uint8_t *getBufferAddress(uint8_t source, uint32_t loc);
			PixelBuffer* getBuffer(uint8_t source);
			
			bool allocateBuffer(uint8_t source);
			void delocateBuffer(uint8_t source);

			void push(uint8_t source);	//Push frame of source to pool
			std::shared_ptr<PixelBuffer> pop();
		private:
			PixelBuffer pixelBuffer[OPPVS_MAX_CAPTURE_SOURCES];
			int m_numSources;
			tsqueue<std::shared_ptr<PixelBuffer>> m_framePool;
			bool isWaiting;
	};

}

#endif
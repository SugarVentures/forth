/*
	Bitstream = video stream + audio stream
*/

#ifndef OPPVS_BITS_STREAM_HPP
#define OPPVS_BITS_STREAM_HPP

#include "datatypes.hpp"
#include <stddef.h>
#include <stdio.h>
#include "thread.hpp"
#include <vector>

namespace oppvs
{

	class BitsStream
	{
		public:
			BitsStream();
			virtual ~BitsStream();

			static void* init(void* object);
			bool isOverloading();
			void setOverloading(bool value);

			void pushData(const PixelBuffer& pf);
			void throttle();

		private:
			const static int MAX_WAITING_QUEUE_SIZE = 20;
			bool m_isOverloading;
			Thread* m_thread;
			std::vector<PixelBuffer> m_waitingQueue;
	};
}

#endif
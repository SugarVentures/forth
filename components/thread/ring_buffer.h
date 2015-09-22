#ifndef OPPVS_RING_BUFFER_H
#define OPPVS_RING_BUFFER_H

#include "datatypes.hpp"

namespace oppvs {
	class RingBuffer
	{
	public:
		RingBuffer();
		~RingBuffer();

		void allocate(uint32_t capacity);
		void deallocate();
	protected:
		uint8_t* m_buffer;
		uint32_t m_capacity;

		volatile uint32_t m_fetchIndex;
		volatile uint32_t m_storeIndex;
	};
} // oppvs

#endif // OPPVS_RING_BUFFER_H

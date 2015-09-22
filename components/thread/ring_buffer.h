#ifndef OPPVS_RING_BUFFER_H
#define OPPVS_RING_BUFFER_H

#include "datatypes.hpp"

namespace oppvs {
	enum RingBufferError
	{
		RingBufferError_OK = 0,
		RingBufferError_Overload = -2,

	};

	class RingBuffer
	{
	public:
		RingBuffer();
		~RingBuffer();

		void allocate(uint32_t capacity);
		void deallocate();

		uint32_t size();
		uint32_t capacity();

		RingBufferError store(const uint8_t* data, uint32_t inLen, uint32_t* outLen);
		RingBufferError fetch(uint8_t* data, uint32_t inLen, uint32_t* outLen);
	protected:
		uint8_t* m_buffer;
		uint32_t m_capacity;

		volatile uint32_t m_fetchIndex;
		volatile uint32_t m_storeIndex;
	};
} // oppvs

#endif // OPPVS_RING_BUFFER_H

#include "ring_buffer.h"

namespace oppvs {
	RingBuffer::RingBuffer() : m_buffer(NULL), m_capacity(0), m_fetchIndex(0), m_storeIndex(0)
	{

	}

	RingBuffer::~RingBuffer()
	{
		deallocate();
	}

	void RingBuffer::allocate(uint32_t capacity)
	{
		deallocate();
		m_buffer = new uint8_t[capacity];
		m_capacity = capacity;
	}

	void RingBuffer::deallocate()
	{
		delete [] m_buffer;
	}
} // oppvs
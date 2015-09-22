#include "ring_buffer.h"
#include "CAAtomic.h"

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
		if (m_buffer)
		{
			delete [] m_buffer;
			m_buffer = NULL;
		}
		m_capacity = 0;
	}

	uint32_t RingBuffer::size()
	{
        if (m_storeIndex >= m_fetchIndex)
            return m_storeIndex - m_fetchIndex;
        return (m_capacity - m_fetchIndex) + m_storeIndex;
	}

	uint32_t RingBuffer::capacity()
	{
		return m_capacity;
	}

	RingBufferError RingBuffer::store(const uint8_t* data, uint32_t inLen, uint32_t* outLen)
	{
		//Data is too big
		if (inLen > m_capacity)
			return RingBufferError_Overload;

		//Nothing to push
		if (inLen == 0)
			return RingBufferError_OK;

        
		return RingBufferError_OK;
	}

	RingBufferError RingBuffer::fetch(uint8_t* data, uint32_t inLen, uint32_t* outLen)
	{
		return RingBufferError_OK;
	}	
} // oppvs
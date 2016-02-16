#include "ring_buffer.h"
#include <algorithm>

namespace oppvs {
	RingBuffer::RingBuffer() : m_buffer(NULL), m_capacity(0), m_size(0), m_fetchIndex(0), m_storeIndex(0)
	{

	}

	RingBuffer::~RingBuffer()
	{
		deallocate();
	}
    
    void RingBuffer::reset()
    {
        m_fetchIndex = m_storeIndex = 0;
        m_size = 0;
    }

	void RingBuffer::allocate(uint32_t capacity)
	{
		deallocate();
		m_buffer = new uint8_t[capacity];
        memset(m_buffer, 0, capacity);
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
        return m_size;
	}

	uint32_t RingBuffer::capacity()
	{
		return m_capacity;
	}

	RingBufferError RingBuffer::store(const uint8_t* data, uint32_t inLen, uint32_t* outLen)
	{
		if (data == NULL && inLen > 0)
			return RingBufferError_InvalidArgument;
		//Data is too big
		if (inLen > m_capacity)
			return RingBufferError_Overload;

		//Nothing to push
		if (inLen == 0)
			return RingBufferError_OK;
        
        //printf("Size %d cap: %d\n", m_size, m_capacity);
        if (m_fetchIndex == m_storeIndex && size() > 0)
            return RingBufferError_FullBuffer;

        uint32_t writeBytes = std::min(inLen, m_capacity - size());

        if (writeBytes <= m_capacity - m_storeIndex)
        {
        	memcpy(m_buffer + m_storeIndex, data, writeBytes);
        	m_storeIndex += writeBytes;
        	if (m_storeIndex == m_capacity)
        		m_storeIndex = 0;
        }
        else
        {
        	uint32_t size1 = m_capacity - m_storeIndex;
        	memcpy(m_buffer + m_storeIndex, data, size1);
        	uint32_t size2 = writeBytes - size1;
        	memcpy(m_buffer, data + size1, size2);
        	m_storeIndex = size2;
        }

        m_size += writeBytes;
        *outLen = writeBytes;
       	return RingBufferError_OK;
	}

	RingBufferError RingBuffer::fetch(uint8_t* data, uint32_t inLen, uint32_t* outLen)
	{
		if (inLen == 0)
			return RingBufferError_OK;
		if ((data == NULL) && (inLen > 0))
		{
			return RingBufferError_InvalidArgument;
		}

		uint32_t readBytes = std::min(inLen, size());
		if (readBytes <= m_capacity - m_fetchIndex)
		{
			memcpy(data, m_buffer + m_fetchIndex, readBytes);
			m_fetchIndex += readBytes;
			if (m_fetchIndex == m_capacity)
				m_fetchIndex = 0;
		}
		else
		{
			uint32_t size1 = m_capacity - m_fetchIndex;
			memcpy(data, m_buffer + m_fetchIndex, size1);
			uint32_t size2 = readBytes - size1;
			memcpy(data + size1, m_buffer, size2);
			m_fetchIndex = size2;
		}
        m_size -= readBytes;
		*outLen = readBytes;
		return RingBufferError_OK;
	}
    
    bool RingBuffer::isFull()
    {
        return (m_capacity == m_size);
    }
} // oppvs
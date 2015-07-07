#include "dynamic_buffer.hpp"

namespace oppvs
{
	DynamicBuffer::DynamicBuffer()
	{
		init(NULL, 0, 0);
	}

	DynamicBuffer::DynamicBuffer(const void* data, size_t length)
	{
		init(data, length, length);
	}

	size_t DynamicBuffer::size()
	{
		return m_allocatedSize;
	}

	size_t DynamicBuffer::capacity()
	{
		return m_capacity;
	}

	void DynamicBuffer::setSize(size_t size)
	{
		setCapacity(size);
		m_allocatedSize = size;
	}	

	void DynamicBuffer::setData(const void* data, size_t length)
	{
		ASSERT(data != NULL || length == 0);
		setSize(length);
		memcpy(m_data.get(), data, length);
	}

	void DynamicBuffer::setCapacity(size_t capacity)
	{
		if (capacity > m_capacity)
		{
			unique_ptr<uint8_t[]> data(new uint8_t[capacity]);
			memcpy(data, m_data.get(), m_allocatedSize);
			m_data.swap(data);
			m_capacity = capacity;
		}
	}

	uint8_t* DynamicBuffer::data()
	{
		return m_data.get();
	}

	void DynamicBuffer::init(const void* data, size_t length, size_t capacity)
	{
		m_data.reset(new uint8_t[m_capacity = capacity]);
		setData(data, length);
	}


}
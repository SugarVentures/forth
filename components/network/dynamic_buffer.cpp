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

	DynamicBuffer::DynamicBuffer(const void* data, size_t length, size_t capacity)
	{
		init(data, length, capacity);
	}

	DynamicBuffer::DynamicBuffer(const DynamicBuffer& buffer)
	{
		init(buffer.dataC(), buffer.size(), buffer.capacity());
	}

	DynamicBuffer& DynamicBuffer::operator=(const DynamicBuffer& buf) {
    	if (&buf != this) {
    		init(buf.dataC(), buf.size(), buf.size());
    	}
    	return *this;
	}

	bool DynamicBuffer::operator==(const DynamicBuffer& buf) const {
		return (m_allocatedSize == buf.size() &&
	        memcmp(m_data.get(), buf.dataC(), m_allocatedSize) == 0);
	}
	
	bool DynamicBuffer::operator!=(const DynamicBuffer& buf) const {
		return !operator==(buf);
	}

	size_t DynamicBuffer::size() const
	{
		return m_allocatedSize;
	}

	size_t DynamicBuffer::capacity() const
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
			std::unique_ptr<uint8_t[]> data(new uint8_t[capacity]);
			memcpy(data.get(), m_data.get(), m_allocatedSize);
			m_data.swap(data);
			m_capacity = capacity;
		}
	}

	uint8_t* DynamicBuffer::data()
	{
		return m_data.get();
	}

	const uint8_t* DynamicBuffer::dataC() const
	{
		return m_data.get();
	}	

	void DynamicBuffer::init(const void* data, size_t length, size_t capacity)
	{
		m_data.reset(new uint8_t[m_capacity = capacity]);
		setData(data, length);
	}

	void DynamicBuffer::appendData(const void* data, size_t length)
	{
		ASSERT(data != NULL || length == 0);
		size_t oldSize = m_allocatedSize;
		setSize(oldSize + length);
		memcpy(m_data.get() + oldSize, data, length);
	}

	void DynamicBuffer::moveTo(DynamicBuffer* buffer)
	{
		ASSERT(buffer != NULL);
		buffer->setData(m_data.get(), m_allocatedSize);
		buffer->setCapacity(m_capacity);
		m_data.release();
		m_allocatedSize = 0;
		m_capacity = 0;
	}
}
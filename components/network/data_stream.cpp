#include "data_stream.hpp"

namespace oppvs
{
	DataStream::DataStream() : p_buffer(NULL), m_currentPos(0), m_noGrow(false)
	{

	}

	DataStream::DataStream(SharedDynamicBufferRef& spBuffer) : sp_buffer(spBuffer), m_currentPos(0), m_noGrow(false)
	{
		p_buffer = spBuffer.get();
	}

	void DataStream::attach(SharedDynamicBufferRef& spBuffer, bool allowWritting)
	{
		reset();
		sp_buffer = spBuffer;
		p_buffer = sp_buffer.get();
		if (sp_buffer && allowWritting)
		{
			sp_buffer->setSize(0);
		}
	}

	void DataStream::grow(size_t size)
	{
		if (sp_buffer.get() == nullptr)
		{
			sp_buffer = SharedDynamicBufferRef(new DynamicBuffer(NULL, 0, size));
			p_buffer = sp_buffer.get();
		}
		else
		{
			sp_buffer->setCapacity(size);
		}
	}

	void DataStream::reset()
	{
		sp_buffer.reset();
		p_buffer = NULL;
		m_currentPos = 0;
	}

	size_t DataStream::size()
	{
		return (p_buffer ? p_buffer->size() : 0);
	}

	size_t DataStream::capacity()
	{
		return (p_buffer ? p_buffer->capacity() : 0);	
	}

	uint8_t* DataStream::getUnSafeDataPointer()
	{
		if (!p_buffer)
			return NULL;
		return p_buffer->data();
	}

	SharedDynamicBufferRef DataStream::getBuffer()
	{
		return sp_buffer;
	}

	int DataStream::write(const void* data, size_t length)
	{
		if (data == NULL || length == 0)
		{
			return -1;
		}

		std::lock_guard<std::mutex> lk(m_io_mutex);
		size_t curentSize = size();
		if (m_currentPos == curentSize)
		{
			sp_buffer->appendData(data, length);
			m_currentPos = curentSize + length;
			return 0;
		}
		else
		{
			size_t newPos = m_currentPos + length;
			if (newPos > capacity())
				sp_buffer->setCapacity(newPos);

			sp_buffer->updateData(data, length, m_currentPos);
			m_currentPos = newPos;
		}
		return 0;
	}

	int DataStream::read(void* data, size_t length)
	{
		std::lock_guard<std::mutex> lk(m_io_mutex);
		if (m_currentPos + length > size())
		{
			return -1;
		}
		memcpy(data, sp_buffer->data() + m_currentPos, length);
		m_currentPos += length;
		return 0;
	}

	void DataStream::disableGrowing(bool value)
	{
		m_noGrow = value;
	}

	size_t DataStream::getPosition()
	{
		return m_currentPos;
	}

	int DataStream::setAbsolutePosition(size_t pos)
	{
		if (pos <= capacity())
		{
			m_currentPos = pos;
			return 0;
		}
		else
			return -1;
	}

	int DataStream::setRelativePosition(int offset)
	{
		if (offset < 0 && abs(offset) > m_currentPos)
		{
			return -1;
		}
		return setAbsolutePosition(m_currentPos + offset);
	}
}

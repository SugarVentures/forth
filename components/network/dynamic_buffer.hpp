#ifndef OPPVS_DYNAMIC_BUFFER_HPP
#define OPPVS_DYNAMIC_BUFFER_HPP

#include "datatypes.hpp"
#include "common.hpp"
#include <memory>

namespace oppvs
{
	class DynamicBuffer
	{
	private:
		std::unique_ptr<uint8_t[]> m_data;
		size_t m_allocatedSize;
		size_t m_capacity;

		void init(const void* data, size_t length, size_t capacity);
	public:
		DynamicBuffer();
		DynamicBuffer(const void* data, size_t length);
		DynamicBuffer(const void* data, size_t length, size_t capacity);
		DynamicBuffer(const DynamicBuffer& buffer);
		size_t size() const;
		void setData(const void* data, size_t length);
		void setSize(size_t);
		void setCapacity(size_t);
		void appendData(const void* data, size_t length);
		void moveTo(DynamicBuffer* buffer);

		DynamicBuffer& operator=(const DynamicBuffer& buf);
		bool operator==(const DynamicBuffer& buf) const;
		bool operator!=(const DynamicBuffer& buf) const;

		uint8_t* data();
		const uint8_t* dataC() const;
		size_t capacity() const;

	};

	typedef std::shared_ptr<DynamicBuffer> SharedDynamicBufferRef;
}

#endif
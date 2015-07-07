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
		size_t size();
		void setData(const void* data, size_t length);
		void setSize(size_t);
		void setCapacity(size_t);

		uint8_t* data();
		size_t capacity();
	};
}

#endif
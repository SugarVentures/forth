#ifndef OPPVS_CACHE_BUFFER_H
#define OPPVS_CACHE_BUFFER_H

#include "data_stream.hpp"
#include <map>
#include <vector>

namespace oppvs {
	class CacheItem
	{
	private:
		std::vector<SharedDynamicBufferRef> m_data;

	public:
		CacheItem() = default;
		CacheItem(const SharedDynamicBufferRef& input_data)
		{
			m_data.clear();
			m_data.push_back(input_data);
		}

		void add(const SharedDynamicBufferRef& input_data)
		{
			m_data.push_back(input_data);
		}

		void print() const
		{
			for (const auto &p : m_data)
				printf("Item %d\n", p->size());
		}
	};

	class CacheBuffer
	{
	public:
		CacheBuffer();
		~CacheBuffer();

		int add(uint64_t timestamp, SharedDynamicBufferRef data);
		size_t size();

		void printMap();
	private:
		std::map<uint64_t, CacheItem> m_map;

		typedef std::map<uint64_t, CacheItem>::iterator CacheBufferIterator;
		typedef std::map<uint64_t, CacheItem>::reverse_iterator CacheBufferReverseIterator;
	};
} // oppvs

#endif // OPPVS_CACHE_BUFFER_H

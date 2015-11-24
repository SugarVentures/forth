#include "cache_buffer.h"

namespace oppvs {
	CacheBuffer::CacheBuffer()
	{

	}

	CacheBuffer::~CacheBuffer()
	{

	}

	size_t CacheBuffer::size()
	{
		return m_map.size();
	}

	void CacheBuffer::printMap()
	{
		for (const auto &p : m_map) {
            printf("m[%d]\n", p.first);
            p.second.print();
        }
	}

	int CacheBuffer::add(uint64_t timestamp, SharedDynamicBufferRef data)
	{
		CacheBufferIterator iter;
		CacheBufferReverseIterator riter;
		iter = m_map.upper_bound(timestamp);
		if (iter == m_map.end())
		{
			CacheItem item(data);
			if (m_map.size() == 0)
			{
				printf("Insert new item to begin\n");
				m_map.insert(m_map.begin(), std::make_pair(timestamp, data));
			}
			else
			{
				printf("Insert to last position\n");
				riter = m_map.rbegin();
				riter++;

				m_map.insert(riter.base(), std::make_pair(timestamp, data));
			}
		}
		else
		{
			//The new item is in the middle
			printf("The item is in the middle\n");
			iter--;
			if (iter->first == timestamp)
			{
				CacheItem& item = iter->second;
				item.add(data);
			}
			else
			{
				printf("Insert to position %lu\n", iter->first);
				m_map.insert(iter, std::make_pair(timestamp, data));
			}
		}
		return 0;
	}
} // oppvs
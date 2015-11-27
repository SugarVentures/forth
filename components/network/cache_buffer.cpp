#include "cache_buffer.h"

namespace oppvs {
	CacheBuffer::CacheBuffer(): m_startTime(-1), m_endTime(-1)
	{

	}

	CacheBuffer::~CacheBuffer()
	{

	}

	size_t CacheBuffer::size()
	{
		return m_data.size();
	}

	void CacheBuffer::printBuffer()
	{
		printf("Current duration: %lld %lld\n", m_startTime, m_endTime);
		for (const auto &p : m_data) {
            p.print();
        }
	}

	int CacheBuffer::add(int64_t timestamp, SharedDynamicBufferRef data)
	{
		printf("Add item ts: %lld\n", timestamp);
		if (!checkTimeBoundary(timestamp))
		{
			printf("Timestamp is out of boundary\n");
			return -1;
		}
		
		CacheBufferIterator iter = upperBound(timestamp);
		if (iter == m_data.end())
		{
			if (m_data.size() > 0)
			{
				CacheBufferReverseIterator riter = m_data.rbegin();
				if (riter->getTimestamp() == timestamp)
					riter->add(data);
				else
				{
					CacheItem item(timestamp, data);
					m_data.push_back(item);
					updateTimeBoundary(timestamp);		
				}
			}
			else
			{
				CacheItem item(timestamp, data);
				m_data.push_back(item);

				updateTimeBoundary(timestamp);
			}
			
		}
		else
		{
			printf("Found ts: %d\n", iter->getTimestamp());
			--iter;
			if (iter->getTimestamp() == timestamp)
			{
				iter->add(data);
			}
			else
			{
				CacheItem item(timestamp, data);
				m_data.emplace(iter + 1, item);
			}
			printf("Found ts: %d\n", iter->getTimestamp());
		}
		return 0;
	}

	void CacheBuffer::updateTimeBoundary(int64_t timestamp)
	{
		if (timestamp > m_endTime)
			m_endTime = timestamp;

		if (m_startTime == -1)
			m_startTime = timestamp;


	}

	bool CacheBuffer::checkTimeBoundary(int64_t timestamp)
	{
		if (timestamp < m_startTime)
			return false;
		return true;
	}

	CacheBuffer::CacheBufferIterator CacheBuffer::find(int64_t timestamp)
	{		
		return std::find_if(m_data.begin(), m_data.end(), MatchCachePredicate(timestamp, 0));
	}

	CacheBuffer::CacheBufferIterator CacheBuffer::upperBound(int64_t timestamp)
	{
		return std::find_if(m_data.begin(), m_data.end(), MatchCachePredicate(timestamp, 1));	
	}
} // oppvs
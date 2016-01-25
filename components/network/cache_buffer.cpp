#include "cache_buffer.hpp"

namespace oppvs {
	CacheBuffer::CacheBuffer(): m_startTime(-1), m_endTime(-1), m_isRemoving(false), mp_eventHandler(nullptr)
	{
		mp_eventHandler = EventHandler::getInstance();
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
		int64_t startTime, endTime;
		startTime = m_startTime;
		endTime = m_endTime;
		printf("Current duration: %lld %lld\n", startTime, endTime);
		for (const auto &p : m_data) {
            p.print();
        }
	}

	int CacheBuffer::add(int64_t timestamp, SharedDynamicBufferRef data)
	{
		if (!checkTimeBoundary(timestamp))
		{
			int64_t startTime, endTime;
			startTime = m_startTime;
			endTime = m_endTime;
			printf("Timestamp is out of boundary %lld [%lld %lld]\n", timestamp, startTime, endTime);
			return -1;
		}
		std::lock_guard<std::mutex> lk(m_mutex);

		CacheBufferIterator iter = upperBound(timestamp);
		if (iter == m_data.end())
		{
			if (m_data.size() > 0)
			{
				CacheBufferReverseIterator riter = m_data.rbegin();
				if (riter->getTimestamp() == timestamp)
				{
					riter->add(data);
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
				CacheItem item(timestamp, data);
				m_data.push_back(item);
				updateTimeBoundary(timestamp);
			}
			
		}
		else
		{
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
		}
		return 0;
	}

	void CacheBuffer::updateTimeBoundary(int64_t timestamp)
	{
		if (timestamp > m_endTime)
			m_endTime = timestamp;

		if (m_startTime == -1)
			m_startTime = timestamp;

		if (m_endTime - m_startTime > MAX_CACHE_TIME_DURATION && !m_isRemoving)
		{
			ASSERT(mp_eventHandler != nullptr);
			m_isRemoving = true;
			mp_eventHandler->sendSignal(EVENT_SIGNAL_EXECUTE, [this](void* obj) { remove(obj); }, nullptr);
		}
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

	void CacheBuffer::remove(void* param)
	{
		remove(m_data.size() / 4);
		m_isRemoving = false;
	}

	int CacheBuffer::remove(size_t n)
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		if (m_data.size() == 0)
		{
			printf("Nothing to remove\n");
			return 0;
		}
		if (m_data.size() > n)
		{
			//printf("Need to update timestamp\n");
			int64_t timestamp = m_data[n].getTimestamp();
			m_startTime = timestamp;
		}

		//printf("Remove %ld item in %ld\n", n, m_data.size());
		m_data.erase(m_data.begin(), m_data.begin() + std::min(n, m_data.size()));
		
		m_startTime = m_data[0].getTimestamp();
		return 0;
	}

} // oppvs
#ifndef OPPVS_CACHE_BUFFER_H
#define OPPVS_CACHE_BUFFER_H

#include "data_stream.hpp"
#include "thread.hpp"
#include "event_handler.hpp"
#include <vector>
#include <mutex>

namespace oppvs {
	class CacheItem
	{
	private:
		int64_t m_timestamp;
		std::vector<SharedDynamicBufferRef> m_data;

	public:
		CacheItem() = default;
		CacheItem(int64_t ts, const SharedDynamicBufferRef& input_data): m_timestamp(ts)
		{
			m_data.clear();
			m_data.push_back(input_data);
		}

		int64_t getTimestamp() const
		{
			return m_timestamp;
		}

		void add(const SharedDynamicBufferRef& input_data)
		{
			m_data.push_back(input_data);
		}

		void print() const
		{
			printf("Timestamp: %lld\n", m_timestamp);
			for (const auto &p : m_data)
				printf("Item %zu\n", p->size());
		}
	};

	class MatchCachePredicate
	{
		int64_t m_timestamp;
		int m_comparisonType;	//0: equal, 1: >
	public:
		MatchCachePredicate(int64_t timestamp, int64_t type) : m_timestamp(timestamp), m_comparisonType(type) {}
		bool operator()(const CacheItem& item) const
		{
			if (m_comparisonType == 0)
				return item.getTimestamp() == m_timestamp;
			else
				return item.getTimestamp() > m_timestamp;
		}
	};


	class CacheBuffer
	{
	private:
		typedef std::vector<CacheItem>::iterator CacheBufferIterator;
		typedef std::vector<CacheItem>::reverse_iterator CacheBufferReverseIterator;

		const static int MAX_CACHE_TIME_DURATION = 50;	

	public:
		CacheBuffer();
		~CacheBuffer();

		int add(int64_t timestamp, SharedDynamicBufferRef data);
		CacheBufferIterator find(int64_t timestamp);
		CacheBufferIterator upperBound(int64_t timestamp);
		int remove(size_t n);
		void remove(void* param);

		size_t size();

		void printBuffer();
		void attachHandler(EventHandler* handler);
	private:
		std::vector<CacheItem> m_data;


		std::atomic<int64_t> m_startTime;
		std::atomic<int64_t> m_endTime;

		std::atomic<bool> m_isRemoving;

		std::mutex m_mutex;

		EventHandler* mp_eventHandler;
		bool checkTimeBoundary(int64_t timestamp);
		void updateTimeBoundary(int64_t timestamp);
		
	};
} // oppvs

#endif // OPPVS_CACHE_BUFFER_H

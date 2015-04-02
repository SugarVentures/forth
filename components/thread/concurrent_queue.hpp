/*
	Concurrent Queue
*/

#ifndef OPPVS_CONCURRENT_QUEUE_HPP
#define OPPVS_CONCURRENT_QUEUE_HPP

#include <queue>
#include <pthread.h>

namespace oppvs
{
	struct mutex_t
	{
		public:
	        mutex_t(pthread_mutex_t& lock) : isLock(false), m_mutex(lock) {
	        }

	        void Acquire() { pthread_mutex_lock(&m_mutex);  isLock = true; }
	        void Release() { pthread_mutex_unlock(&m_mutex); isLock = false; }
	        bool isLock;
	    private:
	        pthread_mutex_t m_mutex;
	        
	};

	struct LockGuard
	{
		public:
		    LockGuard(pthread_mutex_t& mutex) : _ref(mutex) 
		    { 
		        _ref.Acquire();
		    };

		    ~LockGuard() 
		    {
		     	if (_ref.isLock)
		        	_ref.Release();
		    }

		    void unlock()
		    {
		     	if (_ref.isLock)
		     		_ref.Release();
		    }

		private:
		    mutex_t _ref;
	};

	template <typename T>
	class ConQueue
	{
	public:
		void pop()
		{
			LockGuard lock(m_mutex);
			m_queue.pop();
		}

		T& front()
		{
			LockGuard lock(m_mutex);
			return m_queue.front();
		}

		void lockpop()
		{
			LockGuard lock(m_mutex);
			isWait = true;
		}

		void unlockpop()
		{
			LockGuard lock(m_mutex);
			isWait = false;
			lock.unlock();
			pthread_cond_signal(&m_condition);
		}

		void push(const T& data)
		{
			LockGuard lock(m_mutex);
			m_queue.push(data);
			lock.unlock();
			pthread_cond_signal(&m_condition);
		}

		bool empty()
		{
			LockGuard lock(m_mutex);
			return m_queue.empty();
		}

		bool try_pop(T& popped_value)
		{
			LockGuard lock(m_mutex);
			if (m_queue.empty())
				return false;
			popped_value = m_queue.front();
			m_queue.pop();
			return true;
		}

		void wait_and_pop(T& popped_value)
		{
			LockGuard lock(m_mutex);
			while (m_queue.empty() || isWait)
			{
				pthread_cond_wait(&m_condition, &m_mutex);
			}
			popped_value = m_queue.front();
			m_queue.pop();
		}

		size_t size()
		{
			LockGuard lock(m_mutex);
			return m_queue.size();
		}

		ConQueue() : isWait(false)
		{	
			pthread_mutex_init(&m_mutex, NULL);		
			pthread_cond_init(&m_condition, NULL);
		}

	private:
		std::queue<T> m_queue;
		pthread_mutex_t m_mutex;
		pthread_cond_t m_condition;
		bool isWait;
	};
}

#endif
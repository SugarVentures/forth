#ifndef OPPVS_TIMER_H
#define OPPVS_TIMER_H

#include <chrono>
#include <ctime>
#include <string>
#include <condition_variable>
#include <mutex>


namespace oppvs {
	template <class T>
	class Timer
	{
	public:
		Timer()
		{
			m_start = Clock::now();
		}
		~Timer()
		{

		}

		void create(uint64_t when, uint64_t period, bool isOnce = true)
		{

		}

		int64_t diff()
		{
			return std::chrono::duration_cast<Duration>(Clock::now() - m_start).count();
		}

		void reset()
		{
			m_start = Clock::now();
		}

	private:
		typedef std::chrono::steady_clock Clock;
		typedef std::chrono::time_point<Clock> Timestamp;
		typedef std::chrono::milliseconds Duration;
		typedef std::unique_lock<std::mutex> ScopedLock;

		std::condition_variable m_wakeUp;
		std::mutex m_sync;

		Timestamp m_start;				
	};

	std::string getCurrentTime();
} // oppvs

#endif // OPPVS_TIMER_H

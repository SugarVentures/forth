#ifndef OPPVS_EVENT_HANDLER_HPP
#define OPPVS_EVENT_HANDLER_HPP

#include <functional>
#include "datatypes.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>

#include "thread.hpp"
#include "tsqueue.hpp"

namespace oppvs {

	const int EVENT_SIGNAL_STOP = 0x0000;
	const int MESSAGE_CACHE_CLEAN = 0x0001;

	typedef std::function<int(const std::string&)> callbackFunctionType;

	struct Event
	{
		int eventId;
		callbackFunctionType cbFunction;
	};

	struct ActiveEvent
	{
		int signal;
	};
	
	class EventHandler {
	private:
		Thread* p_thread;
		std::mutex m_mutex;
		std::mutex m_writeMutex;
		std::condition_variable m_conditionVariable;

		callbackFunctionType cbFunction;
		std::vector<Event> m_eventsList;
		tsqueue<ActiveEvent> m_activeEventsList;

		static void* run(void* object);
		void runImpl();

		std::atomic<int> m_numEvents;

		int processSignal();

	public:
		EventHandler();
		~EventHandler();
		void registerEvent(callbackFunctionType);
		void sendSignal();
		void start();
		void stop();
	};
} // oppvs

#endif // OPPVS_EVENT_HANDLER_HPP

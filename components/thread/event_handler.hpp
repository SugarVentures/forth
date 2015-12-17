#ifndef OPPVS_EVENT_HANDLER_HPP
#define OPPVS_EVENT_HANDLER_HPP

#include <functional>
#include "datatypes.hpp"

#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <future>

#include "thread.hpp"
#include "tsqueue.hpp"

namespace oppvs {

	const int EVENT_SIGNAL_STOP = 0x0000;
	const int EVENT_SIGNAL_EXECUTE = 0x0001;

	typedef std::function<void(void* params)> callbackFunctionType;

	struct ActiveEvent
	{
		int signal;
		callbackFunctionType cb;
		void* params;
	};
	
	class EventHandler {
	private:
		Thread* p_thread;
		std::mutex m_mutex;
		std::mutex m_writeMutex;
		std::condition_variable m_conditionVariable;

		callbackFunctionType cbFunction;
		tsqueue<ActiveEvent> m_activeEventsList;

		std::vector<std::future<void>> m_futures;

		static void* run(void* object);
		void runImpl();

		int processSignal();

	public:
		EventHandler();
		~EventHandler();
		void sendSignal(int signal, callbackFunctionType cb, void* param);
		void start();
		void stop();
	};
} // oppvs

#endif // OPPVS_EVENT_HANDLER_HPP

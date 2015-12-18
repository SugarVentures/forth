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

template< class Function, class... Args>
std::future<typename std::result_of<Function(Args...)>::type> async( Function&& f, Args&&... args ) 
{
    typedef typename std::result_of<Function(Args...)>::type R;
    auto bound_task = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
    std::packaged_task<R()> task(std::move(bound_task));
    auto ret = task.get_future();
    std::thread t(std::move(task));
    t.detach();
    return ret;   
}

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
		static bool instanceFlag;
		static EventHandler* single;

		Thread* p_thread;
		std::mutex m_mutex;
		std::condition_variable m_conditionVariable;

		tsqueue<ActiveEvent> m_activeEventsList;
		std::vector<std::future<void>> m_futures;

		bool m_isRunning;

		static void* run(void* object);
		void runImpl();				
		int processSignal();

		EventHandler();
	public:
		
		~EventHandler();
		void sendSignal(int signal, callbackFunctionType cb, void* param);
		void start();
		void stop();

		static EventHandler* getInstance();
	};

	
} // oppvs

#endif // OPPVS_EVENT_HANDLER_HPP

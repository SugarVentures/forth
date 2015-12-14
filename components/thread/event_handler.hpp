#ifndef OPPVS_EVENT_HANDLER_HPP
#define OPPVS_EVENT_HANDLER_HPP

#include <functional>
#include "datatypes.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>

#include "thread.hpp"

namespace oppvs {

	const int MESSAGE_CACHE_CLEAN = 0x0001;
	bool m_hasMessage = false;
	
	class EventHandler {
	private:
		Thread* p_thread;
		std::mutex m_mutex;
		std::condition_variable m_conditionVariable;

		

		static void* run(void* object);
		void runImpl();
	public:
		EventHandler();
		~EventHandler();
		void registerEvent();
		void start();
	};
} // oppvs

#endif // OPPVS_EVENT_HANDLER_HPP

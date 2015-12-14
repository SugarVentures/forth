#include "event_handler.hpp"

namespace oppvs {
	EventHandler::EventHandler()
	{
		p_thread = new Thread(EventHandler::run, this);
	}

	EventHandler::~EventHandler()
	{
		delete p_thread;
	}

	void* EventHandler::run(void* object)
	{
		EventHandler* handler = (EventHandler*)object;
		handler->runImpl();
		return NULL;
	}

	void EventHandler::runImpl()
	{
		std::unique_lock<std::mutex> lk(m_mutex);
		m_conditionVariable.wait(lk, []{return m_hasMessage;});
		printf("Wake up\n");
	}	

	void EventHandler::start()
	{
	    p_thread->create();
	    p_thread->join();
	}
} // oppvs
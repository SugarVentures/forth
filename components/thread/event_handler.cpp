#include "event_handler.hpp"

namespace oppvs {
	bool EventHandler::instanceFlag = false;
	EventHandler* EventHandler::single = NULL;
	
	EventHandler::EventHandler(): m_isRunning(false)
	{
		p_thread = new Thread(EventHandler::run, this);
	}

	EventHandler::~EventHandler()
	{
		delete p_thread;
		instanceFlag = false;
	}

	EventHandler* EventHandler::getInstance()
	{
		if (!instanceFlag)
		{
			single = new EventHandler();
			instanceFlag = true;
			single->start();
			return single;
		}
		else
		{
			single->start();
			return single;
		}
	}

	void* EventHandler::run(void* object)
	{
		EventHandler* handler = (EventHandler*)object;
		handler->runImpl();
		return NULL;
	}

	void EventHandler::runImpl()
	{
		for (;;)
		{
			std::unique_lock<std::mutex> lk(m_mutex);
			m_conditionVariable.wait(lk, [this]{return (m_activeEventsList.size() > 0);});
			printf("Wake up. There is new event.\n");
			if (processSignal() == 1)
				break;
		}
		m_isRunning = true;
	}	

	void EventHandler::start()
	{
		if (!m_isRunning)
		{
		    p_thread->create();
		    m_isRunning = true;
		}
	}

	void EventHandler::stop()
	{
		ActiveEvent event;
		event.signal = EVENT_SIGNAL_STOP;
		m_activeEventsList.push(event);
		m_conditionVariable.notify_one();
	}

	void EventHandler::sendSignal(int sig, callbackFunctionType cb, void* param)
	{
		printf("Receive signal %d\n", sig);
		ActiveEvent event;
		event.signal = sig;
		event.cb = cb;
		event.params = param;
		m_activeEventsList.push(event);
		m_conditionVariable.notify_one();
	}

	int EventHandler::processSignal()
	{
		while (m_activeEventsList.size() > 0)
		{
			std::shared_ptr<ActiveEvent> event = m_activeEventsList.pop();
			if (event->signal == EVENT_SIGNAL_STOP)
				return 1;

			if (event->signal == EVENT_SIGNAL_EXECUTE)
			{
				m_futures.push_back(std::async(event->cb, event->params));	
			}
		}
		for (auto &e : m_futures) {
			e.get();
		}
		m_futures.clear();
		return 0;
	}
} // oppvs
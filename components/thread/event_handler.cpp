#include "event_handler.hpp"

namespace oppvs {
	EventHandler::EventHandler(): m_numEvents(0)
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
		for (;;)
		{
			std::unique_lock<std::mutex> lk(m_mutex);
			m_conditionVariable.wait(lk, [this]{return (m_activeEventsList.size() > 0);});
			printf("Wake up\n");
			if (processSignal() == 1)
				break;
		}
	}	

	void EventHandler::start()
	{
	    p_thread->create();
	}

	void EventHandler::stop()
	{
		ActiveEvent event;
		event.signal = EVENT_SIGNAL_STOP;
		m_activeEventsList.push(event);
		m_conditionVariable.notify_one();
	}

	void EventHandler::registerEvent(callbackFunctionType fnc)
	{
		cbFunction = fnc;
		int localNumEvents = m_numEvents++;
		printf("current id: %d\n", localNumEvents);
		Event event;
		event.eventId = localNumEvents;
		event.cbFunction = fnc;
		{
			std::lock_guard<std::mutex> lk(m_writeMutex);
			m_eventsList.push_back(event);
		}
	}

	void EventHandler::sendSignal()
	{
		printf("Receive signal\n");
		ActiveEvent event;
		m_activeEventsList.push(event);
		m_conditionVariable.notify_one();
	}

	int EventHandler::processSignal()
	{
		while (m_activeEventsList.size() > 0)
		{
			std::shared_ptr<ActiveEvent> event = m_activeEventsList.pop();
			printf("Hello\n");
			if (event->signal == EVENT_SIGNAL_STOP)
				return 1;
		}
		return 0;
	}
} // oppvs
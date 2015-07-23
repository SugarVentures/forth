
#include "thread.hpp"

#include <stdio.h>

namespace oppvs
{
	void print_thread_id(pthread_t id)
	{
	    size_t i;
	    for (i = sizeof(i); i; --i)
	    	printf("%02x", *(((unsigned char*) &id) + i - 1));
	    printf("\n");
	}


	Thread::Thread(thread_routine routine, void* params)
	{
		m_routine = routine;
		m_params = params;
		m_isValid = false;
	}

	Thread::~Thread()
	{
		pthread_cancel(m_threadId);
	}

	int Thread::create()
	{
		int error = pthread_create(&m_threadId, NULL, m_routine, m_params);
		//print_thread_id(m_threadId);
		m_isValid = true;
		return error;
	}

	void Thread::waitUntilNextEvent()
	{

	}

	void Thread::waitUntilEnding()
	{
		void* pRetValFromThread = NULL;
		if (m_isValid)
			pthread_join(m_threadId, &pRetValFromThread);
		m_isValid = false;
		m_threadId = (pthread_t)-1;
	}

}
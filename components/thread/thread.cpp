
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
	}

	Thread::~Thread()
	{
		waitUntilEnding();
	}

	int Thread::create()
	{
		m_thread = std::thread(m_routine, m_params);
		return 0;
	}

	void Thread::waitUntilEnding()
	{
		join();
	}

	void Thread::join()
	{
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	void* Thread::defaultRun(void* object)
	{
		LOGD("Default Run\n");
		return NULL;
	}

}

#include "thread.hpp"

namespace oppvs
{
	Thread::Thread(thread_routine routine, void* params)
	{
		m_routine = routine;
		m_params = params;
	}

	int Thread::create()
	{
		return pthread_create(&m_threadId, NULL, m_routine, m_params);
	}


}
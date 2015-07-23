/*
	Thread Management
*/

#ifndef OPPVS_THREAD_HPP
#define OPPVS_THREAD_HPP

#include <pthread.h>

namespace oppvs
{
	typedef void* (*thread_routine)(void* param);	//Pointer to function that executes when a thread starts
	class Thread
	{
	public:
		Thread() {}
		Thread(thread_routine routine, void* param);		
		int create();
		void waitUntilNextEvent();
		void waitUntilEnding();
		virtual ~Thread();
	private:
		pthread_t m_threadId;
		thread_routine m_routine;
		void* m_params;
		bool m_isValid;
	};
}

#endif
/*
	Thread Management
*/

#ifndef OPPVS_THREAD_HPP
#define OPPVS_THREAD_HPP

//#include <pthread.h>
#include <thread>
#include "../include/logs.h"

namespace oppvs
{
	typedef void* (*thread_routine)(void* param);	//Pointer to function that executes when a thread starts
	class Thread
	{
	public:
		Thread() {}
		Thread(thread_routine routine, void* param);
		~Thread();

		int create();
		void join();
		void waitUntilEnding();
		static void* defaultRun(void* object);
	private:
		std::thread m_thread;
		thread_routine m_routine;
		void* m_params;
	};
}

#endif
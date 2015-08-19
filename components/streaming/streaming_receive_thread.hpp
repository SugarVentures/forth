#ifndef OPPVS_STREAMING_RECEIVE_THREAD_HPP
#define OPPVS_STREAMING_RECEIVE_THREAD_HPP

#include "thread.hpp"

namespace oppvs {
	class StreamingReceiveThread : public Thread
	{
	public:
		StreamingReceiveThread();
		~StreamingReceiveThread();
	private:
		static void* run(void* object);
		bool m_exitThread;
	};
} // oppvs

#endif // OPPVS_STREAMING_RECEIVE_THREAD_HPP

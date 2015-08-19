#ifndef OPPVS_STREAMING_SEND_THREAD_HPP
#define OPPVS_STREAMING_SEND_THREAD_HPP

#include "thread.hpp"
#include "ice_manager.hpp"
#include "tsqueue.hpp"
#include "dynamic_buffer.hpp"

namespace oppvs {
	class StreamingSendThread : public Thread
	{
	public:
		StreamingSendThread(IceStream*);
		~StreamingSendThread();

		void pushSegment(SharedDynamicBufferRef segment);
		void send();
	private:
		IceStream* m_stream;
		bool m_exitThread;
		tsqueue<SharedDynamicBufferRef> m_queue;
		static void* run(void* object);
	};
} // oppvs

#endif // OPPVS_STREAMING_SEND_THREAD_HPP

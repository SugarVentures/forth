#include "streaming_send_thread.hpp"

namespace oppvs {
	StreamingSendThread::StreamingSendThread(IceStream* stream) : Thread(run, this), m_stream(stream), m_exitThread(false)
	{

	}

	StreamingSendThread::~StreamingSendThread()
	{
		m_exitThread = true;
		waitUntilEnding();
	}

	void* StreamingSendThread::run(void* object)
	{
		StreamingSendThread* thread = (StreamingSendThread*)object;
		thread->send();
		return NULL;
	}

	void StreamingSendThread::pushSegment(SharedDynamicBufferRef segment)
	{
		m_queue.push(segment);
	}

	void StreamingSendThread::send()
	{
		while (!m_exitThread)
		{
			if (m_queue.size() > 0 && m_stream != NULL)
			{
				SharedDynamicBufferRef segment = *m_queue.pop();
				m_stream->send(segment->size(), segment->data());
			}
			usleep(1000);
		}
	}
} // oppvs
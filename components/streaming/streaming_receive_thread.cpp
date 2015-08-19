#include "streaming_receive_thread.hpp"

namespace oppvs {
	StreamingReceiveThread::StreamingReceiveThread() : Thread(run, this), m_exitThread(false) {

	}

	StreamingReceiveThread::~StreamingReceiveThread()
	{
		m_exitThread = true;
		waitUntilEnding();
	}

	void* StreamingReceiveThread::run(void* object)
	{
		StreamingReceiveThread* thread = (StreamingReceiveThread*)object;
		
		return NULL;
	}
} // oppvs
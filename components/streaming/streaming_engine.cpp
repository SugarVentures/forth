#include "streaming_engine.hpp"

namespace oppvs
{
	void* initUS(void* p)
	{
		printf("Waiting subscribers\n");
		PublishChannel* publisher = (PublishChannel*)p;
		publisher->waitingSubscribers();
		return NULL;
	}

	int StreamingEngine::initUploadStream()
	{
		
		//int error = m_upload->initListeningServer();
		return 0;
	}

	int StreamingEngine::initPublishChannel()
	{
		m_publisher = new PublishChannel();
		m_publisher->start();
		Thread *thread = new Thread(initUS, (void*)m_publisher);
		thread->create();
		return 0;
	}
}
#include "streaming_engine.hpp"

namespace oppvs
{
	void* initUS(void* p)
	{
		
		PublishChannel* publisher = (PublishChannel*)p;
		publisher->waitingSubscribers();
		return NULL;
	}

	void onNewSubscriberEvent(void* owner, SocketAddress& addr)
	{
		printf("New subcriber event\n");
		StreamingEngine* engine = (StreamingEngine*)owner;
		engine->initUploadStream();
	}

	int StreamingEngine::initUploadStream()
	{
		printf("Setup upload stream\n");
		return 0;
	}

	void StreamingEngine::pushData(const PixelBuffer& pf)
	{

	}

	void StreamingEngine::pullData(PixelBuffer& pf)
	{
		
	}

	int StreamingEngine::initPublishChannel()
	{
		m_publisher = new PublishChannel((void*)this, onNewSubscriberEvent);
		m_publisher->setServiceInfo(ST_VIDEO_STREAMING, generateSSRC());
		m_publisher->start();
		Thread *thread = new Thread(initUS, (void*)m_publisher);
		thread->create();
		printf("SSRC: %u\n", m_publisher->getServiceKey());
		return 0;
	}

	int StreamingEngine::initSubscribeChannel(const std::string& publisher, uint16_t port, const ServiceInfo& service)
	{
		m_subscribe = new SubscribeChannel(publisher, port, service);
		m_subscribe->registerInterest();
		printf("SSRC: %u\n", m_subscribe->getServiceKey());
		return 0;
	}

	uint32_t StreamingEngine::generateSSRC()
	{
		return random32(0);
	}
}
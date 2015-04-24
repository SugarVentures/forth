#include "streaming_engine.hpp"

namespace oppvs
{
	StreamingEngine::StreamingEngine(PixelBuffer* pf): m_ssrc(0), m_isRunning(false), pixelBuffer(pf)
	{
		m_broadcaster = NULL;
		m_receiver = NULL;

		m_sendThread = NULL;
		m_receiveThread = NULL;
		m_thread = NULL;
		
		SRTPSocket::initSRTPLib();
		if (pthread_mutex_init(&m_mutex, NULL) != 0)
		{
			printf("Cannot init mutex\n");
		}
	}

	StreamingEngine::~StreamingEngine()
	{
		delete m_broadcaster;
		delete m_sendThread;

		delete m_receiver;
		delete m_receiveThread;

		m_broadcaster = NULL;
		m_receiver = NULL;

		m_sendThread = NULL;
		m_receiveThread = NULL;
	}

	void* initUS(void* p)
	{
		
		PublishChannel* publisher = (PublishChannel*)p;
		publisher->waitingSubscribers();
		return NULL;
	}

	void* runStreaming(void* p)
	{
		NetworkStream* stream = (NetworkStream*)p;
		while (1)
		{
			stream->sendStream();
			usleep(1000);
		}
		return NULL;
	}

	void* waitStreaming(void* p)
	{
		NetworkStream* stream = (NetworkStream*)p;
		stream->waitStream();
		stream->releaseReceiver();
		return NULL;
	}

	void onNewSubscriberEvent(void* owner, const SocketAddress& remoteaddr, SocketAddress& localaddr)
	{
		StreamingEngine* engine = (StreamingEngine*)owner;
		engine->initUploadStream(localaddr, remoteaddr);
	}

	void onSendDoneEvent(void* owner, int error)
	{
		printf("Error: %d\n", error);
		StreamingEngine* engine = (StreamingEngine*)owner;
		engine->updateQueue();
	}

	void onReceiveEvent(void* owner, int error)
	{
		printf("Receive pkt\n");
		StreamingEngine* engine = (StreamingEngine*)owner;
		engine->pullData();
	}

	void StreamingEngine::updateQueue()
	{
		pthread_mutex_lock(&m_mutex);
		RawData* raw = m_sendingQueue.front();
		printf("Count: %d\n", raw->count);
		raw->count--;
		if (raw->count <= 0)
		{
			m_sendingQueue.pop();
			delete raw;
			std::vector<NetworkStream*>::const_iterator it;
			for (it = m_subscribers.begin(); it != m_subscribers.end(); ++it)
			{
				NetworkStream* stream = (NetworkStream*)*it;
				stream->unlock();
			}
		}
		pthread_mutex_unlock(&m_mutex);
	}


	int StreamingEngine::initUploadStream(SocketAddress& localaddr, const SocketAddress& remoteaddr)
	{
		NetworkStream *stream = new NetworkStream(SENDER_ROLE, m_publisher->getServiceKey());
		stream->registerCallback((void*)this, &m_sendingQueue, onSendDoneEvent);
		if (stream->setup(0) < 0)
		{
			printf("Cannot setup network stream for uploading data for %s\n", remoteaddr.toString().c_str());
			delete stream;
			return -1;
		}
		
		localaddr = stream->getLocalAddress();
		printf("Setup upload stream at %s\n", localaddr.toString().c_str());
		stream->setSender(remoteaddr);
		m_subscribers.push_back(stream);
		
		setIsRunning(true);
		m_sendThread = new Thread(runStreaming, (void*)stream);
		m_sendThread->create();

		return 0;
	}

	int StreamingEngine::initDownloadStream()
	{
		if (m_receiveThread)
			printf("strange error here\n");
		m_receiver = new NetworkStream(RECEIVER_ROLE, m_subscribe->getServiceKey());
		m_receiver->registerCallback((void*)this, pixelBuffer->plane[0], pixelBuffer->nbytes, onReceiveEvent);
		if (m_receiver->setup(m_subscribe->getLocalAddress().getPort()) < 0)
		{
			printf("Cannot setup network stream for downloading data\n");
			return -1;
		}
		m_receiver->setReceiver(m_subscribe->getRemoteAddress());
		printf("Set a download stream (%s %s)\n", m_subscribe->getLocalAddress().toString().c_str(), 
			m_subscribe->getRemoteAddress().toString().c_str());

		m_receiveThread = new Thread(waitStreaming, (void*)m_receiver);
		m_receiveThread->create();

		return 0;
	}

	void StreamingEngine::pushData(PixelBuffer& pf)
	{
		if (m_subscribers.size() > 0)
		{
			RawData *raw = new RawData(pf.plane[0], pf.nbytes, m_subscribers.size());
			m_sendingQueue.push(raw);
			printf("Push data %u bytes size: %lu\n", pf.nbytes, m_sendingQueue.size());
		}
	}

	void StreamingEngine::pullData()
	{
		m_callback(*pixelBuffer);
	}

	int StreamingEngine::initPublishChannel()
	{
		m_publisher = new PublishChannel((void*)this, pixelBuffer, onNewSubscriberEvent);
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
		if (m_subscribe->registerInterest(pixelBuffer) < 0)
			return -1;
		printf("SSRC: %u\n", m_subscribe->getServiceKey());
		pixelBuffer->plane[0] = new uint8_t[pixelBuffer->nbytes];
		if (initDownloadStream() < 0)
			return -1;
		return 0;
	}

	uint32_t StreamingEngine::generateSSRC()
	{
		return random32(0);
	}

	bool StreamingEngine::getIsRunning()
	{
		return m_isRunning;
	}

	void StreamingEngine::setIsRunning(bool value)
	{
		m_isRunning = value;
	}

	void StreamingEngine::registerCallback(frame_callback cb)
	{
		m_callback = cb;
	}

}
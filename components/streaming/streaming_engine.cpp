#include "streaming_engine.hpp"

namespace oppvs
{
	StreamingEngine::StreamingEngine()
	{
		m_sendThread = NULL;
		m_receiveThread = NULL;

		m_publisher = NULL;
		m_subscribe = NULL;

		m_cacheBuffer = NULL;
		m_messageParser = MessageParsing();
	}

	void StreamingEngine::setup()
	{
		m_ssrc = 0;
		m_isRunning = false;

		m_broadcaster = NULL;
		m_receiver = NULL;

		m_sendThread = NULL;
		m_receiveThread = NULL;
		m_thread = NULL;

		m_subscribe = NULL;
		
		SRTPSocket::initSRTPLib();
		if (pthread_mutex_init(&m_mutex, NULL) != 0)
		{
			printf("Cannot init mutex\n");
		}
	}


	StreamingEngine::~StreamingEngine()
	{
		delete m_sendThread;
		delete m_receiveThread;

		delete m_publisher;
		delete m_subscribe;

		if (m_serviceInfo.videoStreamInfo.noSources > 0)
		{			
			delete [] m_serviceInfo.videoStreamInfo.sources;			
		}

		if (m_cacheBuffer)
			delete m_cacheBuffer;
	}

	void* runStreaming(void* p)
	{
		NetworkStream* stream = (NetworkStream*)p;
		while (1)
		{
			stream->sendStream();
			usleep(50);
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
		//printf("SendDone Event Error: %d\n", error);
		StreamingEngine* engine = (StreamingEngine*)owner;
		//Unlock streams
		if (error == 1)
			engine->updateQueue();
	}

	void onReceiveEvent(void* owner, uint8_t source, int error)
	{
		//printf("Receive pkt\n");
		StreamingEngine* engine = (StreamingEngine*)owner;
		engine->pullData(source);
	}

	void StreamingEngine::updateQueue()
	{
		std::vector<NetworkStream*>::const_iterator it;
		for (it = m_subscribers.begin(); it != m_subscribers.end(); ++it)
		{
			NetworkStream* stream = (NetworkStream*)*it;
			stream->unlock();
		}
	}


	int StreamingEngine::initUploadStream(SocketAddress& localaddr, const SocketAddress& remoteaddr)
	{
		NetworkStream *stream = new NetworkStream(SENDER_ROLE, m_publisher->getServiceKey());
		//stream->registerCallback((void*)this, &m_sendingQueue, onSendDoneEvent);
		stream->registerCallback((void*)this, &m_messageHandler, onSendDoneEvent);
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

		m_messageHandler.setNumClients(m_subscribers.size());
		
		setIsRunning(true);
		m_sendThread = new Thread(runStreaming, (void*)stream);
		m_sendThread->create();

		return 0;
	}

	int StreamingEngine::initDownloadStream()
	{
		m_receiver = new NetworkStream(RECEIVER_ROLE, m_subscribe->getServiceKey());
		m_messageParser.setCacheBuffer(m_cacheBuffer);
		m_receiver->registerCallback((void*)this, &m_messageParser, onReceiveEvent);
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
			m_messageHandler.addMessage(pf);
		}
	}

	void StreamingEngine::pullData(uint8_t source)
	{
		PixelBuffer* pf = m_cacheBuffer->getBuffer(source);
		if (pf)
		{
			//pf->user = pixelBuffer->user;
			m_callback(*pf);
		}
	}

	int StreamingEngine::initPublishChannel()
	{
		m_publisher = new PublishChannel((void*)this, onNewSubscriberEvent);

		m_publisher->setServiceInfo(ST_VIDEO_STREAMING, generateSSRC());
		m_publisher->setServiceInfo(&m_serviceInfo);
		printServiceInfo();
		m_publisher->start();
		printf("SSRC: %u\n", m_publisher->getServiceKey());
		
		
		return 0;
	}

	int StreamingEngine::initSubscribeChannel(const std::string& publisher, uint16_t port, const ServiceInfo& service)
	{
		m_subscribe = new SubscribeChannel(publisher, port, service);
		int len;
		uint8_t info[OPPVS_NETWORK_PACKET_LENGTH];
		if (m_subscribe->registerInterest((uint8_t*)&info, OPPVS_NETWORK_PACKET_LENGTH, &len) < 0)
			return -1;

		int curPos = sizeof(sockaddr); //Skip the destination address
		len -= curPos;
		setStreamInfo(info + curPos, len);
		//Setup cache
		m_cacheBuffer = new CacheBuffer(m_serviceInfo.videoStreamInfo);

		printServiceInfo();
		printf("SSRC: %u\n", m_subscribe->getServiceKey());
		if (initDownloadStream() < 0)
			return -1;
		return 0;
	}

	uint32_t StreamingEngine::generateSSRC()
	{
		return random32(0);
	}

	bool StreamingEngine::isRunning()
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

	void StreamingEngine::initBitsStream()
	{
		//m_bitsstream = new BitsStream();
	}

	std::string StreamingEngine::getStreamInfo() const
	{
		if (!m_publisher)
			return std::string();
		return m_publisher->getLocalAddress().toString();
	}

	void StreamingEngine::setStreamInfo(const std::vector<VideoActiveSource>& sources)
	{
		m_serviceInfo.type = ST_VIDEO_STREAMING;
		m_serviceInfo.key = m_ssrc;
		VideoStreamInfo* vsi = &m_serviceInfo.videoStreamInfo;
		vsi->videoWidth = DEFAULT_VIDEO_FRAME_WIDTH;
		vsi->videoHeight = DEFAULT_VIDEO_FRAME_HEIGHT;
		if (sources.size() == 0)
			return;
		vsi->noSources = sources.size();
		vsi->sources = new VideoSourceInfo[vsi->noSources];		

		std::vector<VideoActiveSource>::const_iterator it;
		int i = 0;
		for (it = sources.begin(); it != sources.end(); ++it)
		{
			vsi->sources[i].source = it->id;
			vsi->sources[i].order = it->order;
			vsi->sources[i].width = it->rect.right - it->rect.left;
			vsi->sources[i].height = it->rect.top - it->rect.bottom;
			vsi->sources[i].stride = it->stride;
			i++;
		}
	}

	void StreamingEngine::setStreamInfo(uint8_t *info, int len)
	{
		int curPos = 0;
		m_serviceInfo.type = ST_VIDEO_STREAMING;
		m_serviceInfo.key = m_ssrc;
		VideoStreamInfo* vsi = &m_serviceInfo.videoStreamInfo;
		if (len < sizeof(uint16_t))
			return;
		memcpy(&vsi->videoWidth, info + curPos, sizeof(uint16_t));
		curPos += sizeof(uint16_t);
		if (len < curPos + sizeof(uint16_t))
			return;
		memcpy(&vsi->videoHeight, info + curPos, sizeof(uint16_t));
		curPos += sizeof(uint16_t);
		if (len < curPos + sizeof(uint8_t))
			return;
		memcpy(&vsi->noSources, info + curPos, sizeof(uint8_t));
		curPos += sizeof(uint8_t);
		VideoSourceInfo vsinfo;
		if (len - curPos != vsi->noSources * vsinfo.size())
			return;
		vsi->sources = new VideoSourceInfo[vsi->noSources];


		for (int i = 0; i < vsi->noSources; i++)
		{
			memcpy(&vsi->sources[i].source, info + curPos, sizeof(uint8_t));
			curPos += sizeof(uint8_t);
			memcpy(&vsi->sources[i].order, info + curPos, sizeof(uint8_t));
			curPos += sizeof(uint8_t);
			memcpy(&vsi->sources[i].width, info + curPos, sizeof(uint16_t));
			curPos += sizeof(uint16_t);
			memcpy(&vsi->sources[i].height, info + curPos, sizeof(uint16_t));
			curPos += sizeof(uint16_t);
			memcpy(&vsi->sources[i].stride, info + curPos, sizeof(uint16_t));
			curPos += sizeof(uint16_t);
		}
	}

	void StreamingEngine::printServiceInfo()
	{
		printf("Stream Info: Width %d Height %d \n", m_serviceInfo.videoStreamInfo.videoWidth, m_serviceInfo.videoStreamInfo.videoHeight);
		printf("Stream Info: Number of capture sources: %d\n", m_serviceInfo.videoStreamInfo.noSources);
		for (int i = 0; i < m_serviceInfo.videoStreamInfo.noSources; i++)
		{
			printf("Source: %d Width: %d Height: %d Stride: %d Order: %d\n", m_serviceInfo.videoStreamInfo.sources[i].source, 
				m_serviceInfo.videoStreamInfo.sources[i].width, m_serviceInfo.videoStreamInfo.sources[i].height,
				m_serviceInfo.videoStreamInfo.sources[i].stride, m_serviceInfo.videoStreamInfo.sources[i].order);
		}

	}
}

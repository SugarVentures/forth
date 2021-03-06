#include "streaming_engine.hpp"

namespace oppvs
{
	
	StreamingEngine::StreamingEngine()
	{
		m_exitMainThread = false;
		m_mainThread = NULL;
		m_isRunning = false;
		p_audioRingBuffer = NULL;
		p_videoFrameBuffer = NULL;
	}

	StreamingEngine::~StreamingEngine()
	{
		if (m_serviceInfo.videoStreamInfo.noSources > 0)
		{			
			delete [] m_serviceInfo.videoStreamInfo.sources;			
		}

		if (m_serviceInfo.audioStreamInfo.noSources > 0)
		{			
			delete [] m_serviceInfo.audioStreamInfo.sources;			
		}

		//Close and release sending threads
		for (int i = 0; i < m_sendingThreads.size(); i++)
		{
			StreamingSendThread* thread = m_sendingThreads[i];
			delete thread;
			m_sendingThreads[i] = NULL;
		}
		m_sendingThreads.clear();

		//Stop main thread
		m_exitMainThread = true;
		if (m_mainThread != NULL)
		{
			m_mainThread->waitUntilEnding();
			delete m_mainThread;
		}
	}


	int StreamingEngine::init(StreamingRole role, const std::string& stun, const std::string& turn, 
			const std::string& username, const std::string& password, const std::string& signaling, uint16_t port)
	{
		//Update configuration
		m_configuration.stunServer = IceServerInfo(stun, oppvs::DEFAULT_STUN_PORT, "", "");
		m_configuration.turnServer = IceServerInfo(turn, oppvs::DEFAULT_STUN_PORT, username, password);
		m_configuration.role = role;

		m_configuration.signalingServerAddress.setIP(oppvs::IPAddress(signaling));
		m_configuration.signalingServerAddress.setPort(port);

		if (m_signaler.init(m_configuration.stunServer, m_configuration.turnServer, m_configuration.signalingServerAddress, 
			role, &m_serviceInfo) < 0)
			return -1;

		m_signaler.attachCallback(StreamingEngine::onNewSubscriber, this);
		m_signaler.attachCallback(StreamingEngine::onReceiveSegment, this);
		m_signaler.attachCallback([this](const ServiceInfo& info) { return updateStreamInfo(info); });
		if (role == ROLE_BROADCASTER)
		{
			m_videoPacketizer.init(m_serviceInfo.videoStreamInfo, &m_sendPool);
			m_audioPacketizer.init(m_serviceInfo.audioStreamInfo, &m_sendPool);
		}
		else if (role == ROLE_VIEWER)
		{
			m_serviceInfo.videoStreamInfo.noSources = 1;
			m_serviceInfo.videoStreamInfo.sources = new VideoSourceInfo[1];
			m_serviceInfo.videoStreamInfo.sources[0].width = DEFAULT_VIDEO_FRAME_WIDTH;
			m_serviceInfo.videoStreamInfo.sources[0].height = DEFAULT_VIDEO_FRAME_HEIGHT;
			m_serviceInfo.videoStreamInfo.sources[0].source = 0;
			m_serviceInfo.videoStreamInfo.sources[0].order = 0;
			m_serviceInfo.videoStreamInfo.sources[0].stride = 4 * DEFAULT_VIDEO_FRAME_WIDTH;
		}
		return 0;
	}

	int StreamingEngine::start(const std::string& streamkey)
	{
		if (m_signaler.start(streamkey) < 0)
			return -1;
		
		m_videoPacketizer.start();
		m_audioPacketizer.start();
		return 0;
	}


	void StreamingEngine::pushData(PixelBuffer& pf)
	{
		if (m_sendingThreads.size() == 0)
			return;
		std::chrono::time_point<std::chrono::system_clock> currentTime;
		currentTime = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = currentTime - m_firstTime;
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_seconds).count();
		pf.timestamp = millis;
		m_videoPacketizer.pushFrame(pf);
	}

	void StreamingEngine::pushData(GenericAudioBufferList& ab)
	{
		m_audioPacketizer.push(ab);
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
    
    void StreamingEngine::registerCallback(frame_callback cb, void* user)
    {
        m_callback = cb;
        m_frameUser = user;
    }

	void StreamingEngine::registerCallback(streaming_callback cb, void* user)
	{
		m_streamingCallback = cb;
		m_streamingUser = user;
	}

	void StreamingEngine::setCommonInfo()
	{
		m_serviceInfo.type = ST_VIDEO_STREAMING;
        m_serviceInfo.key = m_ssrc;
	}

	void StreamingEngine::setVideoInfo(const std::vector<VideoActiveSource>& videoSources)
	{
		VideoStreamInfo* vsi = &m_serviceInfo.videoStreamInfo;
        if (videoSources.size() > 0)
        {
            vsi->noSources = videoSources.size();
            vsi->sources = new VideoSourceInfo[vsi->noSources];
            
            std::vector<VideoActiveSource>::const_iterator it;
            int i = 0;
            for (it = videoSources.begin(); it != videoSources.end(); ++it)
            {
                vsi->sources[i].source = it->id;
                vsi->sources[i].order = it->order;
                vsi->sources[i].format = it->pixel_format;
                vsi->sources[i].width = it->rect.right - it->rect.left;
                vsi->sources[i].height = it->rect.top - it->rect.bottom;
                vsi->sources[i].stride = it->stride;
                i++;
            }
        }
	}

	void StreamingEngine::setAudioInfo(const std::vector<AudioActiveSource>& audioSources)
	{
		AudioStreamInfo* asi = &m_serviceInfo.audioStreamInfo;
		if (audioSources.size() > 0)
		{
			asi->noSources = audioSources.size();
			asi->sources = new AudioSourceInfo[asi->noSources];
			std::vector<AudioActiveSource>::const_iterator it;
			int i = 0;
			for (it = audioSources.begin(); it != audioSources.end(); ++it)
			{
				asi->sources[i].source = it->id;
				asi->sources[i].type = AUDIO_TYPE_MIXED;
				asi->sources[i].format = AUDIO_FORMAT_FLOAT;
				asi->sources[i].numberChannels = it->numberChannels;
				asi->sources[i].samplePerChannels = it->samplePerChannels;
				asi->sources[i].sampleRate = it->sampleRate;
				i++;
			}
		}
	}
    
    void StreamingEngine::setStreamInfo(const std::vector<VideoActiveSource>& videoSources)
    {
        setCommonInfo();
        setVideoInfo(videoSources);        
    }

    void StreamingEngine::setStreamInfo(const std::vector<AudioActiveSource>& audioSources)
    {
    	setCommonInfo();
    	setAudioInfo(audioSources);
    }

	void StreamingEngine::setStreamInfo(const std::vector<VideoActiveSource>& videoSources, const std::vector<AudioActiveSource>& audioSources)
	{
		setCommonInfo();
		setVideoInfo(videoSources);
		setAudioInfo(audioSources);
	}

	void StreamingEngine::printServiceInfo()
	{
		LOGD("Stream Info: Number of capture video sources: %d\n", m_serviceInfo.videoStreamInfo.noSources);
		for (int i = 0; i < m_serviceInfo.videoStreamInfo.noSources; i++)
		{
			LOGD("Source: %d Width: %d Height: %d Stride: %d Order: %d Format: %d\n", m_serviceInfo.videoStreamInfo.sources[i].source, 
				m_serviceInfo.videoStreamInfo.sources[i].width, m_serviceInfo.videoStreamInfo.sources[i].height,
				m_serviceInfo.videoStreamInfo.sources[i].stride, m_serviceInfo.videoStreamInfo.sources[i].order,
				m_serviceInfo.videoStreamInfo.sources[i].format);
		}
		LOGD("Stream Info: Number of capture audio sources: %d\n", m_serviceInfo.audioStreamInfo.noSources);
		for (int i = 0; i < m_serviceInfo.audioStreamInfo.noSources; i++)
		{
			LOGD("Source %d channels: %d sample rate: %d\n", m_serviceInfo.audioStreamInfo.sources[i].source, 
	    				m_serviceInfo.audioStreamInfo.sources[i].numberChannels,
	    				m_serviceInfo.audioStreamInfo.sources[i].sampleRate);	
		}
	}


	void StreamingEngine::onNewSubscriber(void* object, IceStream* stream)
	{
		StreamingEngine* engine = (StreamingEngine*)object;
		if (!engine->isRunning())
		{
			engine->setIsRunning(true);
			engine->createMainThread();
			engine->m_firstTime = std::chrono::system_clock::now();
		}
		engine->createSendingThread(stream);
	}

	void StreamingEngine::createSendingThread(IceStream* stream)
	{
		StreamingSendThread* thread = new StreamingSendThread(stream);
		m_sendingThreads.push_back(thread);
		thread->create();
	}

	void StreamingEngine::createMainThread()
	{
		m_mainThread = new Thread(runMainThreadFunction, this);
		m_mainThread->create();
	}

	void* StreamingEngine::runMainThreadFunction(void* object)
	{
		StreamingEngine* engine = (StreamingEngine*)object;
		//if (engine->getRole() == ROLE_BROADCASTER)
			engine->send();
		//else
		//	engine->receive();
		return NULL;
	}

	void StreamingEngine::send()
	{
		while (!m_exitMainThread)
		{
			if (m_sendPool.size() > 0)
			{
				SharedDynamicBufferRef segment = *m_sendPool.pop();
				for(unsigned i = 0; i < m_sendingThreads.size(); ++i) {
					m_sendingThreads[i]->pushSegment(segment);
				}
			}
			usleep(1000);
		}
	}

	void StreamingEngine::onReceiveSegment(void* object, uint8_t* data, uint32_t len)
	{
		StreamingEngine* engine = (StreamingEngine*)object;
		if (engine->getRole() == ROLE_VIEWER)
		{
			/*if (!engine->isRunning())
			{
				engine->setIsRunning(true);
				engine->createMainThread();
			}*/
			engine->receive(data, len);
		}
	}

	StreamingRole StreamingEngine::getRole()
	{
		return m_configuration.role;
	}

	void StreamingEngine::receive(uint8_t* data, uint32_t len)
	{
		m_depacketizer.pushSegment(data, len);
	}

	void StreamingEngine::receive()
	{
		/*while (!m_exitMainThread)
		{
			
			usleep(50000);
		}*/
	}

	int StreamingEngine::updateStreamInfo(const ServiceInfo& info)
	{
		if (p_audioRingBuffer == NULL)
		{
			LOGD("Can not update stream info\n");
			return -1;
		}

		LOGD("Update stream info\n");
		m_serviceInfo = info;
		printServiceInfo();
		p_audioRingBuffer->allocate(8, 10 * 512);
		m_depacketizer.init(&m_serviceInfo, p_videoFrameBuffer, p_audioRingBuffer, &m_sendPool);
		m_depacketizer.attachCallback(m_callback, m_frameUser);

		m_streamingCallback(m_streamingUser);

		m_signaler.sendPeerRegister();
		return 0;
	}

	void StreamingEngine::attachBuffer(AudioRingBuffer* pbuf)
	{
		p_audioRingBuffer = pbuf;
	}

	void StreamingEngine::attachBuffer(VideoFrameBuffer* pbuf)
	{
		p_videoFrameBuffer = pbuf;
	}
}

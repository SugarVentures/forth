#include "depacketizer.h"
#include <ctime>
#include <chrono>

namespace oppvs {
	Depacketizer::Depacketizer(): p_recvPool(NULL), m_user(NULL)
	{
		m_timestamp = 0;
		p_segmentPool = NULL;
	}

	Depacketizer::~Depacketizer()
	{
		m_videoDecoder.release();
		for(unsigned i = 0; i < m_readers.size(); ++i) {
			IncomingStreamingMessageController* msg = m_readers.back();
			delete msg->thread;
			m_readers.pop_back();
		}
	}

	void Depacketizer::init(ServiceInfo* info, VideoFrameBuffer* pvideoBuf, AudioRingBuffer* paudioBuf, tsqueue<SharedDynamicBufferRef>* psendPool)
	{
		if (info->videoStreamInfo.noSources > 0)
			m_videoDecoder.init(info->videoStreamInfo);

		if (info->audioStreamInfo.noSources > 0)
			m_audioDecoder.init(info->audioStreamInfo);
		
		for(unsigned i = 0; i < info->videoStreamInfo.noSources; ++i) {
			IncomingStreamingMessageController* msg = new IncomingStreamingMessageController();
			msg->sourceid = info->videoStreamInfo.sources[i].source;
			msg->thread = new DepacketizerThread(this);
			msg->thread->create();
			m_readers.push_back(msg);
		}
		for(unsigned i = 0; i < info->audioStreamInfo.noSources; ++i) {
			IncomingStreamingMessageController* msg = new IncomingStreamingMessageController();
			msg->sourceid = info->audioStreamInfo.sources[i].source;
			msg->thread = new DepacketizerThread(this);
			msg->thread->create();
			m_readers.push_back(msg);
		}

		p_videoFrameBuffer = pvideoBuf;
		p_audioRingBuffer = paudioBuf;
		p_serviceInfo = info;
		p_segmentPool = psendPool;
	}

	void Depacketizer::pushSegment(uint8_t* data, uint32_t len)
	{
		uint32_t timestamp = 0;
		uint8_t sourceid = 0;
		uint16_t type = 0;

		if (len < RTP_HEADER_SIZE)
			return;

		memcpy(&timestamp, data, 4);
		timestamp = ntohl(timestamp);
		memcpy(&sourceid, data + 4, 1);
		memcpy(&type, data + 5, 2);
		type = ntohs(type);
		//LOGD("timestamp %u len: %u source: %d type: %d\n", timestamp, len, sourceid, type);
		
		IncomingStreamingMessageController* controller = getController(sourceid);
		if (controller == nullptr)
		{
			assert("No corresponding controller to handle the received segment");
			return;
		}

		SegmentReader* reader = &controller->reader;
		if (reader == nullptr)
		{
			assert("No corresponding reader to handle the received segment");
			return;	
		}
	
		int ret = -1;
		switch (type)
		{
		 	case VP8_PAYLOAD_TYPE:
		 		ret = reader->addVP8Bytes(data, len);
		 		break;
		 	case OPUS_PAYLOAD_TYPE:
		 		ret = reader->addOpusBytes(data, len);
		 		break;
		 	default:
		 		return;
		}

		if (ret < 0)
			return;
		else if (ret == 1)
		{
			SharedIncomingStreamingFrame frame = SharedIncomingStreamingFrame(new IncomingStreamingFrame());
			frame->sourceid = sourceid;
			frame->type = type;
			frame->data = reader->getBuffer();
			frame->timestamp = timestamp;

			DepacketizerThread* thread = controller->thread;
			if (thread == nullptr)
			{
				assert("No corresponding thread to handle the frame");
				return;
			}
			
			thread->pushFrame(frame);
		}

		//Push to send pool
		if (p_segmentPool)
		{
			SharedDynamicBufferRef segment = SharedDynamicBufferRef(new DynamicBuffer());
			segment->setData(data, len);
			p_segmentPool->push(segment);
		}
	}

	int Depacketizer::pullFrame(PixelBuffer& pf, SharedDynamicBufferRef frame)
	{
		if (m_videoDecoder.decode(pf, frame->size(), frame->data()) < 0)
			return -1;

		return 0;
	}

	int Depacketizer::pullFrame(SharedDynamicBufferRef frame, uint8_t source, uint32_t timestamp)
	{
		float* out = new float[AUDIO_MAX_ENCODING_PACKET_SIZE];
		uint32_t len = m_audioDecoder.decode(frame->data(), frame->size(), source, out);
		
		//uint64_t start = timestamp;
		if (p_audioRingBuffer != NULL)
		{
			int err = p_audioRingBuffer->store(&len, out, m_timestamp);
			if (err)
				printf("Error when pushing to audio buffer %d\n", err);
			m_timestamp += len;
		}
		delete [] out;
		return 0;
	}

	void Depacketizer::pullFrame(SharedIncomingStreamingFrame frame)
	{
		if (frame.get() == NULL)
			return;
        
        //printf("Timestamp: %u\n", frame->timestamp);
        //printHash((char*)frame->data->data(), frame->data->size());

		switch (frame->type)
		{
			case VP8_PAYLOAD_TYPE:
				for (int i = 0; i < p_serviceInfo->videoStreamInfo.noSources; i++)
				{
					if (p_serviceInfo->videoStreamInfo.sources[i].source == frame->sourceid)
					{
						PixelBuffer pf;
						pf.width[0] = p_serviceInfo->videoStreamInfo.sources[i].width;
						pf.height[0] = p_serviceInfo->videoStreamInfo.sources[i].height;
						pf.stride[0] = p_serviceInfo->videoStreamInfo.sources[i].stride;
						pf.order = p_serviceInfo->videoStreamInfo.sources[i].order;
						pf.source = p_serviceInfo->videoStreamInfo.sources[i].source;
						pf.format = (PixelFormat)p_serviceInfo->videoStreamInfo.sources[i].format;
						
						if (pullFrame(pf, frame->data) < 0)
						{
							LOGD("Invalid video frame\n");
						}
						else
						{
                            pf.user = m_user;
							m_callback(pf);
						}
						break;
					}
				}
				break;
			case OPUS_PAYLOAD_TYPE:
				if (pullFrame(frame->data, frame->sourceid, frame->timestamp) < 0)
				{
					printf("Invalid audio frame\n");
				}
				break;
			default:
				printf("Unkown Error\n");
		}
	}

	void Depacketizer::attachCallback(frame_callback cb)
	{
		m_callback = cb;
	}
    
    void Depacketizer::attachCallback(frame_callback cb, void* user)
    {
        m_callback = cb;
        m_user = user;
    }

    IncomingStreamingMessageController* Depacketizer::getController(uint8_t sourceid)
	{
		for(unsigned i = 0; i < m_readers.size(); ++i) {
			if (m_readers[i]->sourceid == sourceid)
				return m_readers[i];
		}
		return nullptr;
	}

	DepacketizerThread::DepacketizerThread(Depacketizer* dep) : Thread(run, this), m_exitThread(false), p_depacketizer(dep)
	{
		m_lastPTS = 0;
	}

	DepacketizerThread::~DepacketizerThread()
	{
		m_exitThread = true;
		waitUntilEnding();
	}

	void DepacketizerThread::pushFrame(SharedIncomingStreamingFrame frame)
	{
		frame->delay = frame->timestamp - m_lastPTS;
        //printf("Push frame %d\n", frame->timestamp);
		m_queue.push(frame);
		m_lastPTS = frame->timestamp;
	}

	void* DepacketizerThread::run(void* object)
	{
		DepacketizerThread* thread = (DepacketizerThread*)object;
		thread->processFrame();
		return NULL;
	}

	void DepacketizerThread::processFrame()
	{
		while (!m_exitThread)
		{
			if (m_queue.size() > 0 && p_depacketizer != NULL)
			{
				SharedIncomingStreamingFrame frame = *m_queue.pop();
				if (p_depacketizer != NULL)
					p_depacketizer->pullFrame(frame);

				if (frame->type == VP8_PAYLOAD_TYPE)
				{
					/*std::chrono::time_point<std::chrono::system_clock> currenttime;
					currenttime = std::chrono::system_clock::now();
					auto duration = currenttime.time_since_epoch();
					auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
					printf("%lld\n", millis);*/
					//printf("Delay: %d\n", frame->delay);
					//m_idleTime = frame->delay * 1000;
					m_idleTime = 5000;
					usleep(m_idleTime);
					continue;
				}
			}
			usleep(10000);
			
		}
	}

	uint32_t DepacketizerThread::getLastPTS()
	{
		return m_lastPTS;
	}


} // oppvs
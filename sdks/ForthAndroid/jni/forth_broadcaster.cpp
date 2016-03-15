#include "forth_broadcaster.h"

#include "android_video_engine.hpp"
#include "streaming_engine.hpp"

namespace oppvs {
	StreamingEngine* streamingEngine;
	struct ForthBroadcastObject
	{
		StreamingEngine streamingEngine;
		AndroidVideoEngine* videoEngine;
		PixelBuffer pixelBuffer;
	};

	void frameCallback(PixelBuffer& pf)
	{
		if (pf.user)
		{
			
		}
	}

	ForthBroadcaster::ForthBroadcaster()
	{
		m_object = new ForthBroadcastObject();
		m_object->videoEngine = new AndroidVideoEngine(frameCallback, NULL);
		m_isSetPixelBuffer = false;
	}

	ForthBroadcaster::~ForthBroadcaster()
	{
		if (m_object)
			delete m_object->videoEngine;
		delete m_object;
	}

	void ForthBroadcaster::Initialize(JNIEnv* env, jobject context)
	{
		AndroidVideoEngine::initialize(env, context);
	}

	int ForthBroadcaster::startCaptureSession(int deviceIndex, int type)
	{
		LOGD("Start capture session");
		if (m_object)
		{
			if (m_object->videoEngine)
			{
				m_object->videoEngine->startCaptureSession();
			}
		}
		return 0;
	}

	void ForthBroadcaster::setupCaptureSession(jobject wrapperObject)
	{
		LOGD("Setup capture session");
		if (m_object)
		{
			if (m_object->videoEngine)
			{
				window_rect_t sourceRect;
				VideoActiveSource* activeSource = m_object->videoEngine->addSource(VST_WINDOW, "0", 24, sourceRect, sourceRect, (void*)wrapperObject, 0);
				m_object->videoEngine->setupCaptureSession(activeSource);
			}
		}	
	}

	void ForthBroadcaster::runCaptureSession(jobject intent)
	{
		LOGD("Run capture session");
		if (m_object)
		{
			if (m_object->videoEngine)
			{
				m_object->videoEngine->runCaptureSession(intent);
			}
		}	
	}

	void ForthBroadcaster::pushVideoFrame(void* data, jint width, jint height, jint stride)
	{
		if (m_object)
		{
			if (m_object->videoEngine)
			{
				VideoActiveSource* activeSource = m_object->videoEngine->getSource();
				if (activeSource)
				{
					if (!m_isSetPixelBuffer)
					{
						//Update active source information based on callback data from capturing engine
						activeSource->rect.left = 0;
						activeSource->rect.right = width;
						activeSource->rect.bottom = 0;
						activeSource->rect.top = height;
						activeSource->stride = stride;
						m_isSetPixelBuffer = true;

						//Update pixel buffer
						m_object->pixelBuffer.width[0] = width;
						m_object->pixelBuffer.height[0] = height;
						m_object->pixelBuffer.stride[0] = stride;
						m_object->pixelBuffer.nbytes = stride * height;

					}
				}
				m_object->pixelBuffer.plane[0] = (uint8_t*)data;
				if (m_object->streamingEngine.isRunning())
				{
					m_object->streamingEngine.pushData(m_object->pixelBuffer);
				}
			}
		}
	}

	void ForthBroadcaster::startStreaming(const std::string& streamKey, const std::string& serverAddress)
	{
		LOGD("Start Streaming");
		if (m_object)
		{
			if (m_object->videoEngine)
			{
				StreamingEngine& engine = m_object->streamingEngine;
				engine.setStreamInfo(m_object->videoEngine->getVideoActiveSources());
				if (engine.init(ROLE_BROADCASTER, serverAddress, serverAddress, TURN_SERVER_USER, TURN_SERVER_PASS, serverAddress, SIGN_SERVER_PORT) < 0)
				{
					LOGD("Failed to init streaming engine");
					return;
				}

				if (engine.start(streamKey) < 0)
				{
					LOGD("Failed to start streaming engine");
					return;
				}
			}
		}
	}
} // oppvs
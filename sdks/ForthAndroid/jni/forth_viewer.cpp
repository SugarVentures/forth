#include "forth_viewer.h"

#include "streaming_engine.hpp"

namespace oppvs
{	
	StreamingEngine* streamingEngine;
	struct ForthObject
	{
		StreamingEngine streamingEngine;
		AudioRingBuffer audioRingBuffer;
	};

	void frameCallback(PixelBuffer& pf)
	{
		if (pf.user)
		{		
			ForthViewer* viewer = (ForthViewer*)pf.user;
			viewer->m_renderer->renderFrame(pf.plane[0], pf.width[0], pf.height[0], true);
			viewer->m_callback();
		}
	}

	void streamingCallback(void* user)
	{

	}

	ForthViewer::ForthViewer()
	{
		m_object = new ForthObject();
		m_renderer = NULL;
	}

	ForthViewer::~ForthViewer()
	{
		delete m_object;
	}

	void ForthViewer::attachCallback(java_callback cb)
	{
		m_callback = cb;
	}

	int ForthViewer::startStreaming(const std::string& streamKey, const std::string& serverAddress)
	{
		LOGD("Start streamming\n");
		
		m_object->streamingEngine.registerCallback(frameCallback, this);
		m_object->streamingEngine.registerCallback(streamingCallback, NULL);
		m_object->streamingEngine.attachBuffer(&m_object->audioRingBuffer);

		if (m_object->streamingEngine.init(ROLE_VIEWER, serverAddress, serverAddress, TURN_SERVER_USER, TURN_SERVER_PASS, serverAddress, SIGN_SERVER_PORT) < 0)
		{
			LOGD("Failed to init streaming engine\n");
			return -1;
		}

		if (m_object->streamingEngine.start(streamKey) < 0)
		{
			LOGD("Failed to start streaming engine\n");
			return -1;
		}
		return 0;
	}

	void ForthViewer::attachRenderer(ForthRenderer* render)
	{
		m_renderer = render;
	}
} //oppvs


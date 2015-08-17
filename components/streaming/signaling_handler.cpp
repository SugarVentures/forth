#include "signaling_handler.hpp"

namespace oppvs {
	SignalingHandler::SignalingHandler()
	{
		p_thread = new Thread(SignalingHandler::run, (void*)this);
		p_manager = NULL;
	}

	SignalingHandler::~SignalingHandler()
	{
		if (p_manager != NULL)
			p_manager->signalForStop();

		p_thread->waitUntilEnding();		
		delete p_thread;
	}

	void* SignalingHandler::run(void* object)
	{
		SignalingHandler* handler = (SignalingHandler*)object;
		if (object != NULL)
			handler->waitingSignal();
		return NULL;
	}

	int SignalingHandler::init(const SocketAddress& server, StreamingRole role)
	{
		p_manager = new SignalingManager(server);
		if (p_manager->init() < 0)
			return -1;

		m_role = role;
		return 0;
	}

	void SignalingHandler::waitingSignal()
	{
		p_manager->waitResponse();
	}

	int SignalingHandler::start(const std::string& streamKey)
	{
		switch (m_role)
		{
			case ROLE_BROADCASTER:
				if (p_manager->sendStreamRegister(streamKey) < 0)
					return -1;
				p_thread->create();
				break;
			case ROLE_VIEWER:
				break;
		}
		return 0;
	}
} // oppvs
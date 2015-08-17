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
		p_manager->registerCallback(SignalingHandler::callbackOnIceResponse, (void*)this);
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
				p_manager->setStreamKey(streamKey);
				break;
		}
		return 0;
	}

	void SignalingHandler::callbackCandidateGatheringDone(void* object, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates)
	{
		std::cout << "Local credential: " << username << " " << password << std::endl;
		for (int i = 0; i < candidates.size(); i++)
		{
	        std::cout << "Local Candidate: " << candidates[i].component << " "
				  << candidates[i].foundation << " "
				  << candidates[i].priority << " "
				  << candidates[i].ip << " "
				  << candidates[i].protocol << " "
				  << candidates[i].port << " "
				  << candidates[i].type << std::endl;
		}
		SignalingHandler* handler = (SignalingHandler*)object;
		handler->sendStreamRequest(username, password, candidates);
	}

	void SignalingHandler::callbackOnIceResponse(void* object, std::string& username, std::string& password, std::vector<oppvs::IceCandidate>& candidates)
	{
		for (int i = 0; i < candidates.size(); i++)
		{
	        std::cout << "Remote Candidate: " << candidates[i].component << " "
				  << candidates[i].foundation << " "
				  << candidates[i].priority << " "
				  << candidates[i].ip << " "
				  << candidates[i].protocol << " "
				  << candidates[i].port << " "
				  << candidates[i].type << std::endl;
		}
	}

	int SignalingHandler::sendStreamRequest(std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates)
	{
		if (m_role == ROLE_VIEWER)
			return p_manager->sendStreamRequest(username, password, candidates);
		else
			return 0;
	}

} // oppvs
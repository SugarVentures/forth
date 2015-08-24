#include "signaling_handler.hpp"

namespace oppvs {
	SignalingHandler::SignalingHandler()
	{
		p_thread = new Thread(SignalingHandler::run, (void*)this);
		p_manager = NULL;
		p_streamInfo = NULL;
	}

	SignalingHandler::~SignalingHandler()
	{
		for(unsigned i = 0; i < m_connectors.size(); ++i) {
			delete m_connectors.back();
			m_connectors.pop_back();
		}

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

	int SignalingHandler::init(const IceServerInfo& stunServer, const IceServerInfo& turnServer, 
			const SocketAddress& signalingServerAddress, StreamingRole role, VideoStreamInfo* pinfo)
	{
		p_manager = new SignalingManager(signalingServerAddress);
		if (p_manager->init() < 0)
			return -1;

		m_stunServer = stunServer;
		m_turnServer = turnServer;
		p_manager->attachCallbackEvent(SignalingHandler::callbackOnIceResponse, (void*)this);
		m_role = role;
		p_streamInfo = pinfo;
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
				if (p_streamInfo == NULL)
					return -1;
				if (p_manager->sendStreamRegister(streamKey, *p_streamInfo) < 0)
					return -1;
				p_thread->create();
				break;
			case ROLE_VIEWER:
				p_manager->setStreamKey(streamKey);
				{
					IceManager* icemgr = new IceManager();
					icemgr->init(m_stunServer, m_turnServer, 0);
					icemgr->attachCallbackEvent(SignalingHandler::callbackCandidateGatheringDone, (void*)this);
					icemgr->attachCallbackEvent(SignalingHandler::callbackOnReceiveImpl, (void*)this);
					IceStream* stream = icemgr->createStream();
					stream->requestLocalCandidates();
					m_connectors.push_back(icemgr);
				}
				break;
		}

		return 0;
	}

	void SignalingHandler::callbackCandidateGatheringDone(void* object, void* icemgr, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates)
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
		if (object == NULL || icemgr == NULL)
		{
			std::cout << "Unknown error in callbackCandidateGatheringDone" << std::endl;
			return;
		}

		SignalingHandler* handler = (SignalingHandler*)object;
		switch (handler->getRole())
		{
			case ROLE_VIEWER:
				handler->sendStreamRequest(username, password, candidates);
				break;
			case ROLE_BROADCASTER:
				((IceManager*)icemgr)->establishPeerConnection();
				break;
		}
		
	}

	void SignalingHandler::callbackOnIceResponse(void* object, std::string& username, std::string& password, std::vector<IceCandidate>& candidates)
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
		SignalingHandler* handler = (SignalingHandler*)object;
		handler->prepareConnection(username, password, candidates);
	}

	void SignalingHandler::callbackNewSubscriberImpl(void* object, IceStream* stream)
	{
		SignalingHandler* handler = (SignalingHandler*)object;
		handler->callbackNewSubscriberEvent(handler->callbackNewSubscriberObject, stream);
	}

	void SignalingHandler::callbackOnReceiveImpl(void* object, uint8_t* data, uint32_t len)
	{
		SignalingHandler* handler = (SignalingHandler*)object;
		handler->callbackOnReceiveEvent(handler->callbackOnReceiveObject, data, len);
	}

	int SignalingHandler::sendStreamRequest(std::string username, std::string password, std::vector<IceCandidate>& candidates)
	{
		return p_manager->sendStreamRequest(username, password, candidates);
	}

	void SignalingHandler::prepareConnection(std::string& username, std::string& password, std::vector<IceCandidate>& candidates)
	{
		IceManager* icemgr = new IceManager();
		icemgr->init(m_stunServer, m_turnServer, 1);
		icemgr->setPeerInfo(username, password, candidates);
		icemgr->attachCallbackEvent(SignalingHandler::callbackCandidateGatheringDone, (void*)this);
		icemgr->attachCallbackEvent(SignalingHandler::callbackNewSubscriberImpl, (void*)this);
		icemgr->attachCallbackEvent(SignalingHandler::callbackOnReceiveImpl, (void*)this);
		IceStream* stream = icemgr->createStream();
		stream->requestLocalCandidates();
		m_connectors.push_back(icemgr);	
	}

	StreamingRole SignalingHandler::getRole()
	{
		return m_role;
	}

	void SignalingHandler::attachCallback(callbackNewSubscriber cb, void* object)
	{
		callbackNewSubscriberEvent = cb;
		callbackNewSubscriberObject = object;
	}

	void SignalingHandler::attachCallback(callbackOnReceive cb, void* object)
	{
		callbackOnReceiveEvent = cb;
		callbackOnReceiveObject = object;
	}

} // oppvs
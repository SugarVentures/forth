#include "publish_channel.hpp"

namespace oppvs
{
	void PublishChannel::callbackCandidateGatheringDoneImpl(void* object, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates)
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
	}

	void PublishChannel::callbackOnIceResponse(void* object, std::string& username, std::string& password, std::vector<oppvs::IceCandidate>& candidates)
	{
		std::cout << "call back on ice response" << std::endl;
		oppvs::IceManager* iceManager = (oppvs::IceManager*)object;
		oppvs::IceStream* stream = iceManager->getStreamByID(1);
		std::cout << "Username " << username << " password " << password << std::endl;
		stream->setRemoteCredentials(username, password);
		stream->setRemoteCandidates(candidates);

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


	PublishChannel::PublishChannel(void* owner, callbackNewSubscriber event) : Channel(), m_interrupt(false)
	{
		m_event = event;
		m_owner = owner;
		m_thread = new Thread(PublishChannel::run, (void*)this);

	}

	PublishChannel::~PublishChannel()
	{
		m_interrupt = 1;
		delete m_thread;
	}

	void* PublishChannel::run(void* object)
	{
		PublishChannel* publisher = (PublishChannel*)object;
		publisher->waitingSubscribers();
		return NULL;
	}

	int PublishChannel::start()
	{
		oppvs::IceServerInfo stunServer("192.168.0.106", oppvs::DEFAULT_STUN_PORT, "", "");
		oppvs::IceServerInfo turnServer("192.168.0.106", oppvs::DEFAULT_STUN_PORT, "turn", "password");

		oppvs::SocketAddress signalingServerAddress;
		signalingServerAddress.setIP(oppvs::IPAddress("127.0.0.1"));
		signalingServerAddress.setPort(33333);

		mp_signalingManager = new SignalingManager(signalingServerAddress);
		mp_signalingManager->init();

		mp_iceManager = new IceManager((void*)mp_signalingManager);
		mp_iceManager->init(stunServer, turnServer);

		mp_signalingManager->registerCallback(PublishChannel::callbackOnIceResponse, (void*)mp_iceManager);
		mp_iceManager->registerCallback(PublishChannel::callbackCandidateGatheringDoneImpl);
		mp_iceManager->registerChannelCallback(m_event, m_owner);

		IceStream* stream = mp_iceManager->createStream();
		stream->requestLocalCandidates();

		mp_signalingManager->sendStreamRegister("1234");
		

		m_thread->create();
		return 0;
	}

	void PublishChannel::waitingSubscribers()
	{
		mp_signalingManager->waitResponse();
	}

	
}

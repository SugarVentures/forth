#include "subscribe_channel.hpp"

namespace oppvs
{
	SubscribeChannel::SubscribeChannel(std::string server, uint16_t port, const ServiceInfo& info)
	{
		IPAddress ipAddr(server);
		m_remoteAddress.setIP(ipAddr);
		m_remoteAddress.setPort(port);
		setServiceInfo(info);
	}

	int SubscribeChannel::registerInterest(uint8_t* info, int len, int* rcvLen)
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

		mp_signalingManager->registerCallback(SubscribeChannel::callbackOnIceResponse, (void*)mp_iceManager);
		mp_iceManager->registerCallback(SubscribeChannel::callbackCandidateGatheringDoneImpl);

		mp_signalingManager->setStreamKey("1234");
		IceStream* stream = mp_iceManager->createStream();
		stream->requestLocalCandidates();

		stream->registerCallback(m_cbOnReceive, m_object);
		return 0;
	}

	const SocketAddress& SubscribeChannel::getRemoteAddress() const
	{
		return m_remoteAddress;
	}

	void SubscribeChannel::callbackCandidateGatheringDoneImpl(void* object, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates)
	{
		std::cout << "Local credential: " << username << " " << password << std::endl;
		SignalingManager *sigManager = (oppvs::SignalingManager*)object;
		sigManager->sendStreamRequest(username, password, candidates);
		
		for (int i = 0; i < candidates.size(); i++)
		{
	        std::cout << "Candidate: " << candidates[i].component << " "
				  << candidates[i].foundation << " "
				  << candidates[i].priority << " "
				  << candidates[i].ip << " "
				  << candidates[i].protocol << " "
				  << candidates[i].port << " "
				  << candidates[i].type << std::endl;
		}
	}

	void SubscribeChannel::callbackOnIceResponse(void* object, std::string& username, std::string& password, std::vector<oppvs::IceCandidate>& candidates)
	{
		std::cout << "call back on ice response" << std::endl;
		oppvs::IceManager* iceManager = (oppvs::IceManager*)object;
		oppvs::IceStream* stream = iceManager->getStreamByID(1);
		std::cout << "Username " << username << " password " << password << std::endl;
		stream->setRemoteCredentials(username, password);
		stream->setRemoteCandidates(candidates);

		//gchar msg[] = "hello world!";
	    //stream->send(sizeof(msg), msg, 1);
		for (int i = 0; i < candidates.size(); i++)
		{
	        std::cout << "Candidate: " << candidates[i].component << " "
				  << candidates[i].foundation << " "
				  << candidates[i].priority << " "
				  << candidates[i].ip << " "
				  << candidates[i].protocol << " "
				  << candidates[i].port << " "
				  << candidates[i].type << std::endl;
		}
		
	}

	void SubscribeChannel::registerCallback(callbackOnReceive cb, void* object)
	{
		m_cbOnReceive = cb;
		m_object = object;
	}
}

#ifndef OPPVS_SUBSCRIBE_CHANNEL
#define OPPVS_SUBSCRIBE_CHANNEL

#include "channel.hpp"

namespace oppvs
{
	class SubscribeChannel : public Channel
	{
	public:
		SubscribeChannel(std::string server, uint16_t port, const ServiceInfo& info);
		int registerInterest(uint8_t *info, int len, int* rcvLen);
		const SocketAddress& getRemoteAddress() const;

		void registerCallback(callbackOnReceive cb, void* object);

		static void callbackCandidateGatheringDoneImpl(void* object, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates);
		static void callbackOnIceResponse(void* object, std::string& username, std::string& password, std::vector<oppvs::IceCandidate>& candidates);
	private:
		SocketAddress m_remoteAddress;
		ClientSocket m_client;

		callbackOnReceive m_cbOnReceive;
		void* m_object;
	};
}

#endif
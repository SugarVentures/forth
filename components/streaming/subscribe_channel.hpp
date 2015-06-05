
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
	private:
		SocketAddress m_remoteAddress;
		ClientSocket m_client;
	};
}

#endif
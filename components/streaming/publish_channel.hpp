
#ifndef OPPVS_PUBLISH_CHANNEL
#define OPPVS_PUBLISH_CHANNEL

#include "channel.hpp"

namespace oppvs
{
	class PublishChannel : public Channel
	{
	public:
		PublishChannel() : m_interrupt(false) {}
		int start();
		void waitingSubscribers();
	private:
		ServerSocket m_server;
		bool m_interrupt;
	};
}

#endif
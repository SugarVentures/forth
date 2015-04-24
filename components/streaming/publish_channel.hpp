
#ifndef OPPVS_PUBLISH_CHANNEL
#define OPPVS_PUBLISH_CHANNEL

#include "channel.hpp"

namespace oppvs
{
	typedef void (*on_new_subscriber_event)(void* owner, const SocketAddress& remoteaddr, SocketAddress& localaddr);

	class PublishChannel : public Channel
	{
	public:
		PublishChannel(void* owner, PixelBuffer* pf, on_new_subscriber_event event);
		int start();
		void waitingSubscribers();
	private:
		ServerSocket m_server;
		bool m_interrupt;
		on_new_subscriber_event m_event;
		void* m_owner;
		PixelBuffer* m_pixelBuffer;
	};
}

#endif
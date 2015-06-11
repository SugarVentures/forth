
#ifndef OPPVS_PUBLISH_CHANNEL
#define OPPVS_PUBLISH_CHANNEL

#include "channel.hpp"

namespace oppvs
{
	typedef void (*on_new_subscriber_event)(void* owner, const SocketAddress& remoteaddr, SocketAddress& localaddr);

	class PublishChannel : public Channel
	{
	public:
		PublishChannel(void* owner, on_new_subscriber_event event);
		virtual ~PublishChannel();
		int start();
		void waitingSubscribers();

		static void* run(void* object);
	private:
		ServerSocket m_server;
		bool m_interrupt;
		on_new_subscriber_event m_event;
		void* m_owner;
		Thread* m_thread;
	};
}

#endif
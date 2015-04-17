#include "publish_channel.hpp"

namespace oppvs
{
	PublishChannel::PublishChannel(void* owner, on_new_subscriber_event event) : m_interrupt(false)
	{
		m_event = event;
		m_owner = owner;
	}

	int PublishChannel::start()
	{
		if (m_server.Create(AF_INET, SOCK_STREAM, 0) < 0)
		{
			printf("Open socket error %s\n", strerror(errno));
			return -1;
		}
		if (m_server.Bind(m_localAddress) < 0)
		{
			printf("Binding error %s\n", strerror(errno));
			return -1;
		}

		if (m_server.Listen() < 0)
		{
			printf("Cannot start listening\n");
			return -1;
		}
		m_localAddress = m_server.getLocalAddress();
		printf("Start listening at port: %d\n", m_localAddress.getPort());
		return 0;
	}

	void PublishChannel::waitingSubscribers()
	{
		SocketAddress remoteAddr;
		printf("Waiting subscribers\n");
		while (!m_interrupt)
		{
			int sockfd = m_server.Accept(remoteAddr);
			if (sockfd >= 0)
			{
				printf("New subscriber at address %s %d\n", remoteAddr.getIP().toString().c_str(), remoteAddr.getPort());
				m_event(m_owner, remoteAddr);
				m_server.increaseNumSubscribers();
			}
			usleep(OPPVS_IDLE_TIME);
		}
	}
}
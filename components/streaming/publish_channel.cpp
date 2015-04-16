#include "publish_channel.hpp"

namespace oppvs
{
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

		m_localAddress = m_server.getLocalAddress();
		if (m_server.Listen() < 0)
		{
			printf("Cannot start listening\n");
			return -1;
		}

		printf("Start listening at port: %d\n", m_localAddress.getPort());
		return 0;
	}

	void PublishChannel::waitingSubscribers()
	{
		SocketAddress remoteAddr;
		while (!m_interrupt)
		{
			int sockfd = m_server.Accept(remoteAddr);
			if (sockfd >= 0)
			{
				printf("New subscriber \n");
				m_server.increaseNumSubscribers();
			}
			usleep(OPPVS_IDLE_TIME);
		}
	}
}
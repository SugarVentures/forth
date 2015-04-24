#include "publish_channel.hpp"

namespace oppvs
{
	PublishChannel::PublishChannel(void* owner, PixelBuffer* pf, on_new_subscriber_event event) : m_interrupt(false),
	m_pixelBuffer(pf)
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
		SocketAddress peerAddr;
		ChannelMessage msg;
		msg.width = m_pixelBuffer->width[0];
		msg.height = m_pixelBuffer->height[0];
		msg.stride = m_pixelBuffer->stride[0];
		msg.flip = 1;
		printf("Waiting subscribers\n");
		while (!m_interrupt)
		{
			int sockfd = m_server.Accept(remoteAddr);
			if (sockfd >= 0)
			{
				m_event(m_owner, remoteAddr, peerAddr);
				peerAddr.toSocketAddr((struct sockaddr_in*)&msg.destination);
				if (m_server.Send(sockfd, (void*)&msg, sizeof(msg)) < 0)
				{
					m_server.Close(sockfd);
					continue;
				}
				
				m_server.Close(sockfd);
			}
			usleep(OPPVS_IDLE_TIME);
		}
	}
}

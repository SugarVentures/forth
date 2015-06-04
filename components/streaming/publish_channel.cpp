#include "publish_channel.hpp"

namespace oppvs
{
	PublishChannel::PublishChannel(void* owner, PixelBuffer* pf, on_new_subscriber_event event) : m_interrupt(false),
	m_pixelBuffer(pf)
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
		m_thread->create();
		return 0;
	}

	void PublishChannel::waitingSubscribers()
	{
		SocketAddress remoteAddr;
		SocketAddress peerAddr;
		ChannelMessage msg;
		msg.videoWidth = m_pixelBuffer->width[0];
		msg.videoHeight = m_pixelBuffer->height[0];

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
		m_server.Close();
	}
}

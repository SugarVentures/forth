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

	int SubscribeChannel::registerInterest(PixelBuffer* pf)
	{
		int sid = m_client.Create(AF_INET, SOCK_STREAM, 0);
		if (sid < 0)
	    {
	        printf("Create signaling socket failed\n");
	        return -1;
	    }
	    if (m_client.Bind(m_localAddress) < 0)
		{
			printf("Binding error %s\n", strerror(errno));
			return -1;
		}
		if (m_client.Connect(m_remoteAddress) < 0)
		{
			printf("Register failed %s\n", strerror(errno));
			return -1;
		}
		m_localAddress = m_client.getLocalAddress();
		ChannelMessage msg;
		if (m_client.Receive((void*)&msg, sizeof(msg)) < 0)
		{
			printf("Cannot receive the info from publisher\n");
		}
		else
		{
			m_remoteAddress = SocketAddress(msg.destination);
			pf->width[0] = msg.videoWidth;
			pf->height[0] = msg.videoHeight;
			pf->nbytes = 0;
		}
		m_client.Close();
		return 0;
	}

	const SocketAddress& SubscribeChannel::getRemoteAddress() const
	{
		return m_remoteAddress;
	}
}
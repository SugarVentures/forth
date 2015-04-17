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

	int SubscribeChannel::registerInterest()
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
		return 0;
	}
}
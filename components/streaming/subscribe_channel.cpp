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

	int SubscribeChannel::registerInterest(uint8_t* info, int len, int* rcvLen)
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
		*rcvLen = m_client.Receive((void*)info, len);
		if (*rcvLen < sizeof(sockaddr))
		{
			printf("Cannot receive the info from publisher\n");
			m_client.Close();
			return -1;
		}
		else
		{
			sockaddr destination;
			memcpy(&destination, info, sizeof(destination));
			m_remoteAddress = SocketAddress(destination);
			printf("destination address: %s\n", m_remoteAddress.toString().c_str());
		}
		m_client.Close();
		return 0;
	}

	const SocketAddress& SubscribeChannel::getRemoteAddress() const
	{
		return m_remoteAddress;
	}
}
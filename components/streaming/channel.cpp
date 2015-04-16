#include "channel.hpp"

namespace oppvs
{
	Channel::Channel()
	{
		IPAddress ipAddr;
		m_localAddress.setIP(ipAddr);
		m_localAddress.setPort(0);
	}

	uint16_t Channel::getPort()
	{
		return m_localAddress.getPort();
	}
}
#include "channel.hpp"

namespace oppvs
{
	Channel::Channel()
	{
		IPAddress ipAddr;
		m_localAddress.setIP(ipAddr);
		m_localAddress.setPort(0);
	}

	const ServiceInfo& Channel::getServiceInfo() const {
		return m_service;
	}
	
	void Channel::setServiceInfo(uint8_t type, uint32_t key)
	{
		m_service.type = type;
		m_service.key = key;
	}

	void Channel::setServiceInfo(const ServiceInfo& s)
	{
		memcpy(&m_service, &s, sizeof(s));
	}

	uint32_t Channel::getServiceKey() const
	{
		return m_service.key;
	}

	const SocketAddress& Channel::getLocalAddress() const
	{
		return m_localAddress;
	}


}
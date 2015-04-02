
#include "ipaddress.hpp"

namespace oppvs
{
	std::string IPAddress::toString() const
	{
		if (m_addressFamily != AF_INET && m_addressFamily != AF_INET6)
			return std::string();
		return inet_ntoa(mu_address.ip4);
	}

	bool IPAddress::getIPFromSockAddr(const struct sockaddr& addr)
	{
		return true;
	}
}
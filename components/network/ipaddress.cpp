
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

	IPAddress::IPAddress(const std::string& hostname) : m_addressFamily(AF_INET)
	{
		struct in_addr in;
		inet_pton(AF_INET, hostname.c_str(), &in);
		memset(&mu_address, 0, sizeof(mu_address));
	    mu_address.ip4 = in;
	}

	int IPAddress::getAddressFamily()
	{
		return m_addressFamily;
	}
}
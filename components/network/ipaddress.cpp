
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

	int IPAddress::getAddressFamily() const
	{
		return m_addressFamily;
	}

	bool IPAddress::isZero() const
	{
		const static uint8_t ZERO_ARRAY[16] = {};
		if (m_addressFamily == AF_INET)
		{
			
			return !memcmp(&mu_address.ip4, ZERO_ARRAY, sizeof(mu_address.ip4));
		}
		if (m_addressFamily == AF_INET6)
		{
			return !memcmp(&mu_address.ip6, ZERO_ARRAY, sizeof(mu_address.ip6));
		}
		printf("Not ipv4 or ipv6\n");
		return 0;
	}

	int IPAddress::getRawBytes(uint8_t* data, uint16_t length) const
	{	
		if (!data)
			return -1;

		if (m_addressFamily != AF_INET && m_addressFamily != AF_INET6)
			return -1;

		if (m_addressFamily == AF_INET)
		{
			uint32_t ip = mu_address.ip4.s_addr;
			if (m_byteOrder == NETWORK_BYTE_ORDER)
			{
				ip = htonl(ip);
			}
			memcpy(data, &ip, sizeof(ip));
			return sizeof(ip);
		}
		else
		{
			memcpy(data, &mu_address.ip6.s6_addr, sizeof(mu_address.ip6.s6_addr));
			return sizeof(mu_address.ip6.s6_addr);
		}

	}	
}
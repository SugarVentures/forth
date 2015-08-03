#include "stun_socket_address.hpp"

namespace oppvs
{
	void StunSocketAddress::applyXorMap(const StunTransactionId& transid)
	{
		const size_t iplen = (m_ip.getAddressFamily() == AF_INET) ? STUN_IPV4_LENGTH : STUN_IPV6_LENGTH;
		uint8_t* pPort;
		uint8_t* pIp;
		in6_addr* ip6;
		in_addr* ip4;

		pPort = (uint8_t*)&m_port;
		if (m_ip.getAddressFamily() == AF_INET)
		{			
			ip4 = m_ip.getIPv4Pointer();
			pIp = (uint8_t*)ip4;
		}	
		else
		{
			ip6 = m_ip.getIPv6Pointer();
			pIp = (uint8_t*)ip6;
		}

		pPort[0] = pPort[0] ^ transid.id[0];
	    pPort[1] = pPort[1] ^ transid.id[1];
	    
	    for (size_t i = 0; i < iplen; i++)
	    {
	        pIp[i] = pIp[i] ^ transid.id[i];
	    }

	    
	}

	bool StunSocketAddress::isZeroAddress() const
	{
		return m_ip.isZero();
	}
}
/*
	Common Interface for IP address
*/

#ifndef OPPVS_IPADDRESS_HPP
#define OPPVS_IPADDRESS_HPP



//POSIX System
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>

namespace oppvs
{
	enum ByteOrdering {
		NETWORK_BYTE_ORDER,
		HOST_BYTE_ORDER
	};

	class IPAddress {
		public:
			IPAddress() {
				//default with IPv4 and local address
				m_addressFamily = AF_INET;
				m_byteOrder = NETWORK_BYTE_ORDER;
				memset(&mu_address, 0, sizeof(mu_address));
				mu_address.ip4.s_addr = htonl(INADDR_ANY);
			}

			IPAddress(const std::string& hostname);
			IPAddress(int family): m_addressFamily(family) {}

			virtual ~IPAddress() {}

			explicit IPAddress(const in_addr& ip4) : m_addressFamily(AF_INET) {
	    		memset(&mu_address, 0, sizeof(mu_address));
	    		mu_address.ip4 = ip4;
	  		}

	  		explicit IPAddress(const in6_addr& ip6) : m_addressFamily(AF_INET6) {
	  			memset(&mu_address, 0, sizeof(mu_address));
	  			mu_address.ip6 = ip6;
	  		}

	  		std::string toString() const;
	  		in_addr getIPv4() const {
	  			return mu_address.ip4;
	  		}

	  		in_addr* getIPv4Pointer() {
	  			return &mu_address.ip4;
	  		}

	  		in6_addr getIPv6() const {
	  			return mu_address.ip6;
	  		}

	  		in6_addr* getIPv6Pointer() {
	  			return &mu_address.ip6;
	  		}

	  		int getAddressFamily() const;
	  		int getRawBytes(uint8_t* data, uint16_t length) const;

	  		bool getIPFromSockAddr(const struct sockaddr& addr);

	  		bool isZero() const;
	  		
		private:
			int m_addressFamily;
			union {
				in_addr ip4;
				in6_addr ip6;
			} mu_address;

			ByteOrdering m_byteOrder;
	};
}

#endif
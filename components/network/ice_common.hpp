#ifndef OPPVS_ICE_COMMON_HPP
#define OPPVS_ICE_COMMON_HPP

#include "datatypes.hpp"
#include "stun_header.hpp"
#include <string>

namespace oppvs {
	struct IceServerInfo
	{
		std::string serverAddress;
		uint16_t port;
		std::string username;
		std::string password;

		IceServerInfo()
		{
			serverAddress = "";
			port = DEFAULT_STUN_PORT;
			username = "";
			password = "";
		}

		IceServerInfo(std::string addr, uint16_t p, std::string un = "", std::string pwd = "")
		{
			serverAddress = addr;
			port = p;
			username = un;
			password = pwd;
		}
	};
} // oppvs

#endif // OPPVS_ICE_COMMON_HPP

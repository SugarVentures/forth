#ifndef OPPVS_ICE_COMMON_HPP
#define OPPVS_ICE_COMMON_HPP

#include "datatypes.hpp"
#include "stun_header.hpp"
#include <string>

namespace oppvs {

	static const char* IceStateName[] = {"disconnected", "gathering", "connecting",
                                    "connected", "ready", "failed"};

	static const char* IceCandidateTypeName[] = {"host", "srflx", "prflx", "relay"};
	static const char* IceCandidateProtocol[] = {"udp", "tcp active", "tcp passive", "tcp so"};

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

	struct IceCandidate
	{
		uint16_t component;
	    std::string foundation;
	    std::string ip;
	    uint16_t port;
	    uint32_t priority;
	    std::string protocol;
	    std::string rel_addr;
	    uint16_t rel_port;
	    std::string type;

	    IceCandidate()
	    {
	    	component = 0;
	    	foundation = "";
	    	ip = "";
	    	port = 0;
	    	priority = 0;
	    	protocol = "";
	    	rel_addr = "";
	    	rel_port = 0;
	    	type = "";
	    }
	};
} // oppvs

#endif // OPPVS_ICE_COMMON_HPP

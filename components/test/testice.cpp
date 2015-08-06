#include "ice_common.hpp"
#include <iostream>
#include <cstdint>
#include <cstring>

#include "nice/nice.h"

int main(int argc, char* argv[])
{
	std::string address, username, password;
	

	if (argc > 3)
	{
		address = argv[0];
		username = argv[1];
		password = argv[2];
	}
	else
	{
		address = std::string("127.0.0.1");
		username = "";
		password = "";
	}

	oppvs::IceServerInfo turnServer(address, oppvs::DEFAULT_STUN_PORT, username, password);

	std::cout<< "Server: " << turnServer.serverAddress << " port: " << turnServer.port << " user: " << turnServer.username << " " << turnServer.password << std::endl;

	return 0;
}
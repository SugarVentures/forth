#include <iostream>
#include <errno.h>
#include "crc.hpp"
#include <iomanip>
#include <string>
#include <signal.h>

#include "stun_server.hpp"

bool interrupt;

void signalhandler(int param)
{
 	interrupt = 1;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Please enter primary server address" << std::endl;
		return -1;
	}

	char* primaryAddress = argv[1];
	signal(SIGINT, signalhandler);

	oppvs::StunServerConfiguration config;
	config.addressPrimaryAdvertised.setIP(oppvs::IPAddress(primaryAddress));
	config.addressPrimaryAdvertised.setPort(oppvs::DEFAULT_STUN_PORT);

	config.addressAlternateAdvertised.setIP(oppvs::IPAddress("127.0.0.1"));
	config.addressAlternateAdvertised.setPort(oppvs::DEFAULT_STUN_PORT + 1);

	config.enabledPP = true;
	config.addressPP = config.addressPrimaryAdvertised;
	config.addressPP.setPort(oppvs::DEFAULT_STUN_PORT);

	config.enabledPA = true;
	config.addressPA = config.addressPrimaryAdvertised;
	config.addressPA.setPort(oppvs::DEFAULT_STUN_PORT + 1);

	config.enabledAP = true;
	config.addressAP = config.addressAlternateAdvertised;
	config.addressAP.setPort(oppvs::DEFAULT_STUN_PORT);

	config.enabledAA = true;
	config.addressAA = config.addressAlternateAdvertised;
	config.addressAA.setPort(oppvs::DEFAULT_STUN_PORT + 1);

	config.enabledMultithreadMode = false;
	
	oppvs::StunServer server;
	std::cout << "Start server" << std::endl;
	server.init(config);
	server.start();

	while (!interrupt)
	{
		usleep(100);
	}
	return 0;
}
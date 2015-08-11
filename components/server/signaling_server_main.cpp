#include <iostream>
#include <errno.h>
#include "crc.hpp"
#include <iomanip>
#include <string>
#include <signal.h>

#include "signaling_server.hpp"

bool interrupt;

void signalhandler(int param)
{
 	interrupt = 1;
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Please enter primary server address: [local address] [public address]" << std::endl;
		return -1;
	}

	char* localAddress = argv[1];
	char* publicAddress = argv[2];

	signal(SIGINT, signalhandler);

	oppvs::SignalingServerConfiguration config;
	config.addressListen.setIP(oppvs::IPAddress(localAddress));
	config.addressListen.setPort(oppvs::DEFAULT_SIGNALING_PORT);

	config.addressAdvertised.setIP(oppvs::IPAddress(publicAddress));
	config.addressAdvertised.setPort(oppvs::DEFAULT_SIGNALING_PORT);
	
	oppvs::SignalingServer server;
	std::cout << "Start server" << std::endl;
	server.init(config);
	server.start();

	while (!interrupt)
	{
		usleep(100);
	}
	return 0;
}
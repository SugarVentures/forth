#include <iostream>

#include "stun_socket_address.hpp"
#include "stun_socket.hpp"
#include "stun_server.hpp"
#include "stun_client.hpp"

#include <errno.h>
#include "data_stream.hpp"
#include "crc.hpp"
#include <iomanip>
#include <string>
#include <signal.h>

bool interrupt;

void signalhandler(int param)
{
 	interrupt = 1;
}

int main(int argc, char* argv[])
{
	int opt;
	interrupt = 0;
	
	uint32_t num = 38912;

	uint32_t b0,b1,b2,b3;
	uint32_t res;

	b0 = (num & 0x000000ff) << 24u;
	b1 = (num & 0x0000ff00) << 8u;
	b2 = (num & 0x00ff0000) >> 8u;
	b3 = (num & 0xff000000) >> 24u;

	res = b0 | b1 | b2 | b3;
	std::cout << res << std::endl;


	if (argc > 1)
		opt = atoi(argv[1]);
	else
	{
		std::cout << "Enter 0 for server, 1 for client\n";
		return -1;
	}

	std::cout << "Test stun functions\n";

	oppvs::StunSocketAddress socketAddress;
	oppvs::StunTransactionId transactionId = {};
	oppvs::SharedDynamicBufferRef buffer;

	//socketAddress.applyXorMap(transactionId);

	if (opt == 0)
	{
		signal(SIGINT, signalhandler);
		char* hostname = argv[2];

		oppvs::StunServerConfiguration config;
		config.addressPrimaryAdvertised.setIP(oppvs::IPAddress(hostname));
		config.addressPrimaryAdvertised.setPort(oppvs::DEFAULT_STUN_PORT);

		config.addressAlternateAdvertised.setIP(oppvs::IPAddress("192.168.0.107"));
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
		server.init(config);
		server.start();

		while (!interrupt)
		{
			usleep(100);
		}


	}
	else
	{
		char* hostname = argv[2];
		int port = atoi(argv[3]);
		
		std::cout << "Client starts" << std::endl;
		std::cout << "Server address: " << hostname << ":" << port << std::endl;
		oppvs::StunClient client;
		oppvs::StunClientConfiguration config;
		config.localAddress.setIP(oppvs::IPAddress("192.168.0.103"));
		config.serverAddress.setIP(oppvs::IPAddress(hostname));
		config.serverAddress.setPort(port);
		if (client.init(config) < 0)
			return -1;
		
		client.sendBindingRequest();
		client.waitResponse();

		return 0;

	}
	return 0;
}

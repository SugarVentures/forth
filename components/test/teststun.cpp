#include <iostream>

#include "stun_socket_address.hpp"
#include "stun_socket.hpp"
#include "stun_server.hpp"
#include <errno.h>

int main(int argc, char* argv[])
{
	int opt;
	if (argc > 1)
		opt = atoi(argv[1]);
	else
	{
		std::cout << "Enter 0 for server, 1 for client\n";
		return -1;
	}

	std::cout << "Test stun functions\n";
	oppvs::StunSocketAddress socketAddress;
	oppvs::StunTransactionId transactionId;

	//socketAddress.applyXorMap(transactionId);

	if (opt == 0)
	{
		oppvs::StunServerConfiguration config;
		config.enabledPP = true;
		config.enabledPA = true;
		config.enabledAP = true;
		config.enabledAA = true;

		config.addressPrimaryAdvertised.setIP(oppvs::IPAddress("127.0.0.1"));
		config.addressPrimaryAdvertised.setPort(oppvs::DEFAULT_STUN_PORT);
		oppvs::StunServer server;
		server.init(config);
		server.start();

		while (1)
		{
			usleep(100);
		}
	}
	else
	{
		int port = atoi(argv[2]);
		std::cout << "Client starts" << std::endl;
		oppvs::StunSocket clientSocket;
		clientSocket.initUDP(socketAddress, oppvs::RolePP);
		oppvs::SocketAddress destination;
		destination.setPort(port);
		destination.setIP(oppvs::IPAddress("127.0.0.1"));
		char testmsg[10] = "hello";

		int socket = clientSocket.getSocketHandle();
		if (clientSocket.Send(socket, testmsg, sizeof(testmsg), destination) < 0)
		{
			std::cout << "Connect error " << strerror(errno);
		}
		else
		{
			std::cout << "sent to " << destination.toString() << std::endl;
		}
	}
	return 0;
}
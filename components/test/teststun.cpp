#include <iostream>

#include "stun_socket_address.hpp"
#include "stun_socket.hpp"
#include "stun_server.hpp"
#include <errno.h>
#include "data_stream.hpp"
#include "crc.hpp"
#include <iomanip>
#include <string>

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
	oppvs::StunTransactionId transactionId = {};
	oppvs::SharedDynamicBufferRef buffer;

	//socketAddress.applyXorMap(transactionId);

	if (opt == 0)
	{
		oppvs::StunServerConfiguration config;
		config.addressPrimaryAdvertised.setIP(oppvs::IPAddress("127.0.0.1"));
		config.addressPrimaryAdvertised.setPort(oppvs::DEFAULT_STUN_PORT);

		config.addressAlternateAdvertised.setIP(oppvs::IPAddress("192.168.0.103"));
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
		fd_set set;
    	timeval tv = {};

		std::cout << "Client starts" << std::endl;
		
		oppvs::StunSocket clientSocket;
		clientSocket.initUDP(socketAddress, oppvs::RolePP);
		oppvs::StunSocketAddress destination;
		destination.setPort(port);
		destination.setIP(oppvs::IPAddress("127.0.0.1"));

		oppvs::StunMessageBuilder builder;
		if (builder.addBindingRequestHeader() < 0)
		{
			std::cout << "Failed to add binding request header" << std::endl;
			return -1;
		}

        if (IsValidTransactionId(transactionId))
			builder.addTransactionID(transactionId);
		else
			builder.addRandomTransactionID(&transactionId);



		builder.getResult(buffer);

		std::cout << "Current size: " << buffer->size() << std::endl;

		if (clientSocket.enablePacketInfoOption(true) < 0)
			std::cout << "Cannot enable packet info option\n";

		int socket = clientSocket.getSocketHandle();

		if (clientSocket.Send(socket, buffer->data(), buffer->size(), destination) < 0)
		{
			std::cout << "Send error " << strerror(errno);
		}
		else
		{
			std::cout << "sent to " << destination.toString() << std::endl;
		}
		while (1)
		{
			FD_ZERO(&set);
	        FD_SET(socket, &set);
	        tv.tv_usec = 500000; // half-second
	        tv.tv_sec = 5;
	        int ret = select(socket + 1, &set, NULL, NULL, &tv);
	        oppvs::SocketAddress remote, local;
	        char testmsg[oppvs::MAX_STUN_MESSAGE_SIZE];

        	if (ret > 0)
        	{

				if (clientSocket.ReceiveMsg(socket, testmsg, sizeof(testmsg), MSG_DONTWAIT, remote, local) >= 0)
				{

					std::cout << testmsg[0] << std::endl;
				}
				else
				{
					std::cout << "Receive error " << strerror(errno);
				}
			}
			
		}
	}
	return 0;
}

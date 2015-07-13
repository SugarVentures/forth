#include <iostream>

#include "stun_socket_address.hpp"
#include "stun_socket.hpp"
#include "stun_server.hpp"

int main()
{
	std::cout << "Test stun functions\n";
	oppvs::StunSocketAddress socketAddress;
	oppvs::StunTransactionId transactionId;

	//socketAddress.applyXorMap(transactionId);

	oppvs::StunServerConfiguration config;
	config.enabledPP = true;
	config.enabledPA = true;
	config.enabledAP = true;
	config.enabledAA = true;

	config.addressPrimaryAdvertised.setIP(oppvs::IPAddress("127.0.0.1"));
	oppvs::StunServer server;
	server.init(config);

	return 0;
}
#include <iostream>

#include "stun_socket_address.hpp"
#include "stun_socket.hpp"


int main()
{
	std::cout << "Test stun functions\n";
	oppvs::StunSocketAddress socketAddress;
	oppvs::StunTransactionId transactionId;

	//socketAddress.applyXorMap(transactionId);

	oppvs::StunSocket socket;
	int ret = socket.initUDP(socketAddress, oppvs::RolePP);
	printf("Init UDP: %d\n", ret);
	return 0;
}
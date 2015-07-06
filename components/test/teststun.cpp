#include <iostream>

#include "stun_socket_address.hpp"

int main()
{
	std::cout << "Test stun functions\n";
	oppvs::StunSocketAddress socketAddress;
	oppvs::StunTransactionId transactionId;

	socketAddress.applyXorMap(transactionId);
	return 0;
}
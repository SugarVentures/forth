#ifndef OPPVS_STUN_MESSAGE_HANDLER_HPP
#define OPPVS_STUN_MESSAGE_HANDLER_HPP

#include "stun_socket.hpp"

namespace oppvs
{
	struct StunTransportAddress
	{
		StunSocketAddress addr;
		bool fValid; // set to false if not valid (basic mode and most TCP/SSL scenarios)
	};

	struct StunTransportAddressSet
	{
		StunTransportAddress set[4]; // one for each socket role RolePP, RolePA, RoleAP, and RoleAA
	};

	class StunMessageHandler
	{

	};
}

#endif
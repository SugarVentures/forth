#ifndef OPPVS_STUN_SERVER_HPP
#define OPPVS_STUN_SERVER_HPP

#include "stun_socket_address.hpp"

namespace oppvs
{
	struct StunServerConfiguration
	{
		bool enabledPP;	//Primary IP, Primary Port
		bool enabledPA;	//Primary IP, Alternate Port
		bool enabledAP; //Alternate IP, Primary Port
		bool enabledAA; //Alternate IP, Alternate Port

		bool enabledMultithreadMode;	//1 thread - 1 socket

		bool enabledTCP;
		uint32_t maxConnections;

		StunSocketAddress addressPP;
		StunSocketAddress addressPA;
		StunSocketAddress addressAP;
		StunSocketAddress addressAA;

		StunSocketAddress addressPrimaryAdvertised;
		StunSocketAddress addressAlternateAdvertised;

		bool enabledDosProtection;

		StunServerConfiguration() :
		{
			enabledPP = false;
			enabledPA = false;
			enabledAP = false;
			enabledAA = false;
			enabledMultithreadMode = false;
			enabledTCP = false;
			maxConnections = 0;
			enabledDosProtection = false;
		}
	};

	class StunServer
	{
	public:
		int init();
		int shutdown();

		int start();
		int stop();
	private:
		StunSocketAddress m_sockets[4];

		StunServer();
		~StunServer();
	};
}

#endif
#ifndef OPPVS_FORWARDING_SERVER_H
#define OPPVS_FORWARDING_SERVER_H

#include "physical_socket.hpp"
#include "forwarding_server_thread.h"

namespace oppvs {
	struct ForwardingServerConfiguration
	{
		SocketAddress addressListen;
	};

	class ForwardingServer
	{
	public:
		ForwardingServer();
		~ForwardingServer();
		
		void init(const ForwardingServerConfiguration& config);
		int start();
		int stop();
		int shutdown();

	private:
		std::vector<ForwardingServerThread*> m_threads;

		int addSocket(const SocketAddress& addressListen, PhysicalSocket*& psock);
	};
} // oppvs


#endif // OPPVS_FORWARDING_SERVER_H

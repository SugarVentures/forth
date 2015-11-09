#ifndef OPPVS_FORWARDING_SERVER_H
#define OPPVS_FORWARDING_SERVER_H

#include "physical_socket.hpp"

namespace oppvs {
	class ForwardingServer
	{
	public:
		ForwardingServer() = default;
		~ForwardingServer();
		
		void init();
		int start();
		int stop();
		int shutdown();

	private:
		PhysicalSocket m_socket;

		int addSocket(const SocketAddress& addressList);
	};
} // oppvs


#endif // OPPVS_FORWARDING_SERVER_H

#ifndef OPPVS_SIGNALING_MANAGER_HPP
#define OPPVS_SIGNALING_MANAGER_HPP

#include "socket_address.hpp"
#include "physical_socket.hpp"
#include "signaling_common.hpp"
#include "signaling_message_builder.hpp"

namespace oppvs {
	class SignalingManager
	{
	public:
		SignalingManager(const SocketAddress& address);
		~SignalingManager();

		int init();
		int sendRequest();
	private:
		PhysicalSocket m_socket;
		SocketAddress m_serverAddress;
		SignalingMessageBuilder m_messageBuilder;
	};
} // oppvs

#endif // OPPVS_SIGNALING_MANAGER_HPP

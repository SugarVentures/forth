#ifndef OPPVS_SIGNALING_MANAGER_HPP
#define OPPVS_SIGNALING_MANAGER_HPP

#include "socket_address.hpp"
#include "physical_socket.hpp"
#include "signaling_common.hpp"
#include "signaling_message_builder.hpp"
#include "ice_common.hpp"

#include <vector>

namespace oppvs {
	class SignalingManager
	{
	public:
		SignalingManager(const SocketAddress& address);
		~SignalingManager();

		int init();
		int sendRequest(std::string username, std::string password, std::vector<IceCandidate>& candidates);
	private:
		PhysicalSocket m_socket;
		SocketAddress m_serverAddress;
		SignalingMessageBuilder m_messageBuilder;
	};
} // oppvs

#endif // OPPVS_SIGNALING_MANAGER_HPP

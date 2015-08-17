#ifndef OPPVS_SIGNALING_SERVER_HPP
#define OPPVS_SIGNALING_SERVER_HPP

#include "socket_address.hpp"
#include "physical_socket.hpp"
#include "signaling_common.hpp"
#include "signaling_server_thread.hpp"

#include <vector>

namespace oppvs {

	struct SignalingServerConfiguration
	{
		SocketAddress addressListen;
		SocketAddress addressAdvertised;
	};

	class SignalingServer 
	{
	public:
		SignalingServer();
		~SignalingServer();

		void init(const SignalingServerConfiguration& config);
		int start();
		int stop();
		int shutdown();

		void setStreamKey(const std::string& streamKey);
	private:
		SignalingServerConfiguration m_config;
		PhysicalSocket m_socket;

		std::vector<SignalingServerThread*> m_threads;
		int addSocket(const SocketAddress& addressListen, const SocketAddress& addressAdvertised);

		std::string m_streamKey;
		int m_broadcaster;
		std::vector<IceCandidate> m_candidates;
	};
} // oppvs

#endif // OPPVS_SIGNALING_SERVER_HPP

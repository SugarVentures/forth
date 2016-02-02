#ifndef OPPVS_SIGNALING_SERVER_HPP
#define OPPVS_SIGNALING_SERVER_HPP

#include "socket_address.hpp"
#include "physical_socket.hpp"
#include "signaling_common.hpp"
#include "signaling_server_thread.hpp"

#include <vector>
#include <algorithm>

namespace oppvs {

	struct SignalingServerConfiguration
	{
		SocketAddress addressListen;
		SocketAddress addressAdvertised;
	};

	struct SignalingStreamInfo
	{
		std::string streamKey;
		int broadcaster;
		std::vector<int> peerList;

		std::vector<IceCandidate> iceCandidates;
		ServiceInfo serviceInfo;

		bool operator == (const SignalingStreamInfo& m) const {
			return (m.streamKey.compare(streamKey) == 0);
		}

		bool operator != (const SignalingStreamInfo& m) const {
			return (m.streamKey.compare(streamKey) != 0);
		}
	};

	struct findStreamOp {
		std::string streamKey;
		findStreamOp(std::string key) : streamKey(key) {}
		
		bool operator() (const SignalingStreamInfo& m) const {
			return (m.streamKey.compare(streamKey) == 0);
		}
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

		int updateStream(const std::string& streamKey, int sockfd, const ServiceInfo& info);
		int getStreamInfo(const std::string& streamKey, int* psockfd, ServiceInfo& info);
		int removeStream(int sockfd);
		int registerPeer(const std::string& streamKey, int sockfd);

	private:
		SignalingServerConfiguration m_config;
		PhysicalSocket m_socket;

		std::vector<SignalingServerThread*> m_threads;
		int addSocket(const SocketAddress& addressListen, const SocketAddress& addressAdvertised);

		std::vector<IceCandidate> m_candidates;
		std::vector<SignalingStreamInfo> m_streams;

		SignalingStreamInfo* findStream(const std::string& streamKey);

	};
} // oppvs

#endif // OPPVS_SIGNALING_SERVER_HPP

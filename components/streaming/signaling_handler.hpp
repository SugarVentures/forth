#ifndef OPPVS_SIGNALING_HANDLER_HPP
#define OPPVS_SIGNALING_HANDLER_HPP

#include "common.hpp"
#include "signaling_manager.hpp"
#include "thread.hpp"

namespace oppvs {
	class SignalingHandler {
	private:
		Thread*				p_thread;
		SignalingManager* 	p_manager;
		StreamingRole		m_role;
	public:
		SignalingHandler();
		~SignalingHandler();

		int init(const SocketAddress& server, StreamingRole role);
		int start(const std::string& streamKey);
		int sendStreamRequest(std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates);
		
		static void* run(void* object);
		static void callbackCandidateGatheringDone(void* object, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates);
		static void callbackOnIceResponse(void* object, std::string& username, std::string& password, std::vector<oppvs::IceCandidate>& candidates);
		
		void waitingSignal();

	};
} // oppvs

#endif // OPPVS_SIGNALING_HANDLER_HPP

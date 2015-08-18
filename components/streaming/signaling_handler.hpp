#ifndef OPPVS_SIGNALING_HANDLER_HPP
#define OPPVS_SIGNALING_HANDLER_HPP

#include "common.hpp"
#include "signaling_manager.hpp"
#include "thread.hpp"
#include "ice_manager.hpp"

namespace oppvs {

	class SignalingHandler {
	private:
		Thread*				p_thread;
		SignalingManager* 	p_manager;
		StreamingRole		m_role;
		std::vector<IceManager*> m_connectors;
		IceServerInfo		m_stunServer;
		IceServerInfo		m_turnServer;
	public:
		SignalingHandler();
		~SignalingHandler();

		int init(const IceServerInfo& stunServer, const IceServerInfo& turnServer, 
			const SocketAddress& signalingServerAddress, StreamingRole role);
		int start(const std::string& streamKey);
		int sendStreamRequest(std::string username, std::string password, std::vector<IceCandidate>& candidates);
		void prepareConnection(std::string& username, std::string& password, std::vector<IceCandidate>& candidates);
		
		static void* run(void* object);
		static void callbackCandidateGatheringDone(void* object, void* icemgr, std::string username, std::string password, std::vector<IceCandidate>& candidates);
		static void callbackOnIceResponse(void* object, std::string& username, std::string& password, std::vector<IceCandidate>& candidates);
		
		void waitingSignal();
		StreamingRole getRole();
	};
} // oppvs

#endif // OPPVS_SIGNALING_HANDLER_HPP

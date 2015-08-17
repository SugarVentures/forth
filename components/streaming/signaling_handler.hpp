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
		
		static void* run(void* object);
		void waitingSignal();
	};
} // oppvs

#endif // OPPVS_SIGNALING_HANDLER_HPP

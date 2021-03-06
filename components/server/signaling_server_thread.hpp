#ifndef OPPVS_SIGNALING_SERVER_THREAD_HPP
#define OPPVS_SIGNALING_SERVER_THREAD_HPP

#include "thread.hpp"
#include "dynamic_buffer.hpp"
#include "physical_socket.hpp"
#include "signaling_common.hpp"
#include "signaling_server_subthread.hpp"
#include "event_handler.hpp"

#include <vector>

namespace oppvs {


	class SignalingServerThread : public Thread
	{
	public:
		SignalingServerThread();
		~SignalingServerThread();

		int init(PhysicalSocket* socket);
		int run();
		int start();

		int signalForStop(bool postMessages);
		int waitForStopAndClose();
		void attachCallback(callbackStreamRegister cb);
		void attachCallback(callbackStreamRequest cb);
		void attachCallback(callbackDisconnect cb);
		void attachCallback(callbackPeerRegister cb);

	private:
		PhysicalSocket* m_sendSockets;
		std::vector<PhysicalSocket*> m_listenSockets;
		std::vector<SignalingServerSubThread*> m_threads;

		static void* threadExecuteFunction(void* param);

		bool m_exitThread;

		callbackStreamRegister m_cbStreamRegister;
		callbackStreamRequest m_cbStreamRequest;
		callbackDisconnect m_cbDisconnect;
		callbackPeerRegister m_cbPeerRegister;
	};
	
} // oppvs

#endif // OPPVS_SIGNALING_SERVER_THREAD_HPP

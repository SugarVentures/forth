#ifndef OPPVS_SIGNALING_SERVER_THREAD_HPP
#define OPPVS_SIGNALING_SERVER_THREAD_HPP

#include "thread.hpp"
#include "dynamic_buffer.hpp"
#include "physical_socket.hpp"
#include "signaling_common.hpp"
#include "signaling_server_subthread.hpp"

#include <vector>

namespace oppvs {


	class SignalingServerThread : public Thread
	{
	public:
		SignalingServerThread();
		~SignalingServerThread();

		int init(PhysicalSocket* sockets, std::string* streamkey, int* bsock);
		int run();
		int start();

		int signalForStop(bool postMessages);
		int waitForStopAndClose();

	private:
		PhysicalSocket* m_sendSockets;
		std::vector<PhysicalSocket*> m_listenSockets;
		std::vector<SignalingServerSubThread*> m_threads;

		static void* threadExecuteFunction(void* param);

		bool m_exitThread;

		std::string* m_streamKey;
		int* m_broadcaster;
	};
	
} // oppvs

#endif // OPPVS_SIGNALING_SERVER_THREAD_HPP

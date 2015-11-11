#ifndef OPPVS_FORWARDING_SERVER_THREAD_H
#define OPPVS_FORWARDING_SERVER_THREAD_H

#include "thread.hpp"
#include "dynamic_buffer.hpp"
#include "physical_socket.hpp"
#include "forwarding_server_subthread.h"

#include <vector>

namespace oppvs {


	class ForwardingServerThread : public Thread
	{
	public:
		ForwardingServerThread();
		~ForwardingServerThread();

		int init(PhysicalSocket* psocket);
		int run();
		int start();

		int signalForStop(bool postMessages);
		int waitForStopAndClose();

	private:
		PhysicalSocket* m_sendSockets;
		std::vector<PhysicalSocket*> m_listenSockets;
		std::vector<ForwardingServerSubThread*> m_threads;

		static void* threadExecuteFunction(void* param);

		bool m_exitThread;

	};
	
} // oppvs

#endif // OPPVS_FORWARDING_SERVER_THREAD_H

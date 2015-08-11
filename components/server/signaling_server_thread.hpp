#ifndef OPPVS_SIGNALING_SERVER_THREAD_HPP
#define OPPVS_SIGNALING_SERVER_THREAD_HPP

#include "thread.hpp"
#include "dynamic_buffer.hpp"
#include "physical_socket.hpp"
#include "signaling_common.hpp"

#include <vector>

namespace oppvs {
	class SignalingServerThread : public Thread
	{
	public:
		SignalingServerThread();
		~SignalingServerThread();

		int init(PhysicalSocket* sockets);
		int run();
		int start();

		int signalForStop(bool postMessages);
		int waitForStopAndClose();
	private:
		PhysicalSocket* m_sendSockets;
		std::vector<PhysicalSocket*> m_listenSockets;

		static void* threadExecuteFunction(void* param);
		bool m_exitThread;

		SharedDynamicBufferRef m_incomingBuffer;
		SharedDynamicBufferRef m_outgoingBuffer;

		void allocBuffers();
		void releaseBuffers();
	};
	
} // oppvs

#endif // OPPVS_SIGNALING_SERVER_THREAD_HPP

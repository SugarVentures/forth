#ifndef OPPVS_FORWARDING_SERVER_SUBTHREAD_H
#define OPPVS_FORWARDING_SERVER_SUBTHREAD_H

#include "thread.hpp"
#include "physical_socket.hpp"
#include "dynamic_buffer.hpp"

namespace oppvs {
	class ForwardingServerSubThread : public Thread
	{
	public:
		ForwardingServerSubThread();
		~ForwardingServerSubThread();
		
		void signalForStop();
		void waitForStopAndClose();

		int init(PhysicalSocket* socket, int sockfd, const SocketAddress &remote);
		void start();

	private:
		PhysicalSocket* m_socket;
		int m_sockfd;
		bool m_exitThread;
		SocketAddress m_remoteAddress;

		SharedDynamicBufferRef m_incomingBuffer;

		static void* threadExecuteFunction(void* param);
		void run();

		void allocBuffers();
		void releaseBuffers();
		void handleMessage();
	};
} // oppvs

#endif // OPPVS_FORWARDING_SERVER_SUBTHREAD_H

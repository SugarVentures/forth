#ifndef OPPVS_FORWARDING_SERVER_SUBTHREAD_H
#define OPPVS_FORWARDING_SERVER_SUBTHREAD_H

#include "thread.hpp"
#include "physical_socket.hpp"


namespace oppvs {
	class ForwardingServerSubThread : public Thread
	{
	public:
		ForwardingServerSubThread();
		~ForwardingServerSubThread();
		
		void signalForStop();
		void waitForStopAndClose();

		void run();
	private:
		PhysicalSocket* m_socket;
		int m_sockfd;
		bool m_exitThread;

		static void* threadExecuteFunction(void* param);
	};
} // oppvs

#endif // OPPVS_FORWARDING_SERVER_SUBTHREAD_H

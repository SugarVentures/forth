#ifndef OPPVS_SIGNALING_SERVER_SUBTHREAD_HPP
#define OPPVS_SIGNALING_SERVER_SUBTHREAD_HPP

#include "thread.hpp"
#include "physical_socket.hpp"
#include "dynamic_buffer.hpp"
#include "signaling_message_reader.hpp"
#include "signaling_message_builder.hpp"

namespace oppvs
{

	class SignalingServerSubThread : public Thread
	{
	public:
		SignalingServerSubThread();
		~SignalingServerSubThread();

		int init(PhysicalSocket* socket, int sockfd, const SocketAddress& remote);
		void start();
		void signalForStop();
		void waitForStopAndClose();
	private:
		PhysicalSocket* m_socket;
		int m_sockfd;
		bool m_exitThread;
		SocketAddress m_remote;
		static void* threadExecuteFunction(void* param);

		void run();

		SharedDynamicBufferRef m_incomingBuffer;
		SharedDynamicBufferRef m_outgoingBuffer;

		SharedDynamicBufferRef m_readerBuffer;
		SignalingMessageReader m_messageReader;

		SignalingMessageBuilder m_messageBuilder;

		void allocBuffers();
		void releaseBuffers();

		void handleMessage();
		void sendResponse();
		int buildIceRequest();

	};
}

#endif // OPPVS_SIGNALING_SERVER_SUBTHREAD_HPP

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
		
		void attachCallback(callbackStreamRegister cb);
		void attachCallback(callbackStreamRequest cb);
		void attachCallback(callbackDisconnect cb);
		void attachCallback(callbackPeerRegister cb);

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
		void sendResponse(int dest);
		int buildIceRequest();
		int buildIceResponse(const std::string& streamKey, const std::string& username, const std::string& password, const std::vector<IceCandidate>& candidates);
		int buildStreamResponse(const std::string& streamKey, const ServiceInfo& videoInfo);

		callbackStreamRegister m_cbStreamRegister;
		callbackStreamRequest m_cbStreamRequest;
		callbackDisconnect m_cbDisconnect;
		callbackPeerRegister m_cbPeerRegister;
	};
}

#endif // OPPVS_SIGNALING_SERVER_SUBTHREAD_HPP

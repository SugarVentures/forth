#ifndef OPPVS_SIGNALING_MANAGER_HPP
#define OPPVS_SIGNALING_MANAGER_HPP

#include "socket_address.hpp"
#include "physical_socket.hpp"
#include "signaling_common.hpp"
#include "signaling_message_builder.hpp"
#include "signaling_message_reader.hpp"
#include "ice_common.hpp"
#include "ice_manager.hpp"

#include <vector>

namespace oppvs {
	typedef void (*callbackOnReceiveIceRequest)(void* object);
	class SignalingManager
	{
	public:
		SignalingManager(const SocketAddress& address);
		~SignalingManager();

		int init();
		int sendIceResponse(std::string username, std::string password, std::vector<IceCandidate>& candidates);
		int sendStreamRegister(const std::string& streamKey);
		void waitResponse();
		void signalForStop();

		void registerCallback(callbackOnReceiveIceRequest cb, void* object);
		
	private:
		PhysicalSocket m_socket;
		SocketAddress m_serverAddress;
		SignalingMessageBuilder m_messageBuilder;

		SharedDynamicBufferRef m_incomingBuffer;
		SharedDynamicBufferRef m_readerBuffer;
		SignalingMessageReader m_messageReader;

		std::string m_streamKey;
		bool m_interrupt;

		callbackOnReceiveIceRequest cbOnReceiveIceRequest;
		void* m_object;

		int sendSignal();
		void release();
		void processResponse();
	};
} // oppvs

#endif // OPPVS_SIGNALING_MANAGER_HPP

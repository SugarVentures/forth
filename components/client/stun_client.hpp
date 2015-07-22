#ifndef OPPVS_STUN_CLIENT_HPP
#define OPPVS_STUN_CLIENT_HPP

#include "stun_socket.hpp"
#include "dynamic_buffer.hpp"
#include "stun_message_handler.hpp"

namespace oppvs {
	enum NatBehavior
	{
	    UnknownBehavior,
	    DirectMapping,                  // IP address and port are the same between client and server view (NO NAT)
	    EndpointIndependentMapping,     // same mapping regardless of IP:port original packet sent to (the kind of NAT we like)
	    AddressDependentMapping,        // mapping changes for local socket based on remote IP address only, but remote port can change (partially symmetric, not great)
	    AddressAndPortDependentMapping  // different port mapping if the ip address or port change (symmetric NAT, difficult to predict port mappings)
	};

	enum NatFiltering
	{
	    UnknownFiltering,
	    DirectConnectionFiltering,
	    EndpointIndependentFiltering,    // shouldn't be common unless connection is already direct (can receive on mapped address from anywhere regardless of where the original send went)
	    AddressDependentFiltering,       // IP-restricted NAT
	    AddressAndPortDependentFiltering // port-restricted NAT
	};

	struct StunClientConfiguration
	{
		StunSocketAddress serverAddress;
		bool ignoreTimeOut;
		uint32_t timeOut;	//in seconds
		uint32_t maxAttempts;
		StunSocketAddress localAddress;
	};

	class StunClient
	{
	public:
		StunClient();
		int init(StunClientConfiguration& config);
		int sendBindingRequest();
		void waitResponse();
		int processResponse(SharedDynamicBufferRef& pmsg, StunSocketAddress& remoteAddr, StunSocketAddress& localAddr);
	private:
		StunClientConfiguration m_configuration;
		StunSocket m_socket;
		StunTransactionId m_transactionId;
		oppvs::StunMessageBuilder m_messageBuilder;
		SharedDynamicBufferRef m_messageBuffer;
	};

} // oppvs

#endif // OPPVS_STUN_CLIENT_HPP

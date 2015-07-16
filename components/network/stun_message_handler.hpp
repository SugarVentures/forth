#ifndef OPPVS_STUN_MESSAGE_HANDLER_HPP
#define OPPVS_STUN_MESSAGE_HANDLER_HPP

#include "stun_socket.hpp"
#include "data_stream.hpp"

namespace oppvs
{
	class StunMessageHandler;

	struct StunIncomingMessage
	{
		SocketRole role;
		StunSocketAddress localAddress;
		StunSocketAddress remoteAddress;
		StunMessageHandler *handler;
		bool isConnectionOriented;     // true for TCP or TLS (where we can't send back to a different port)
	};

	struct StunOutgoingMessage
	{
		SocketRole role;
		StunSocketAddress destinationAddress;
	};

	struct StunTransportAddress
	{
		StunSocketAddress addr;
		bool isValid; // set to false if not valid (basic mode and most TCP/SSL scenarios)
	};

	struct StunTransportAddressSet
	{
		StunTransportAddress set[4]; // one for each socket role RolePP, RolePA, RoleAP, and RoleAA
	};

	class StunMessageHandler
	{
    private:
        DataStream m_dataStream;
        StunTransactionId m_transactionId;

        int addMappedAddressCommon(uint16_t type, const StunSocketAddress& address);
    public:
        StunMessageHandler();
        DataStream& getDataStream();

        //Message Header
        int addMessageType(StunMessageType msgType, StunMessageClass msgClass);
        int addTransactionID(const StunTransactionId& transactionid);	//4 bytes magic cookies & 12 bytes transaction id
        int addMessageLength();

        //Message Attributes
        int addAttributeHeader(uint16_t type, uint16_t size);
        int addMappedAddress(const StunSocketAddress& address);
	};

}

#endif

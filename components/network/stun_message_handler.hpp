#ifndef OPPVS_STUN_MESSAGE_HANDLER_HPP
#define OPPVS_STUN_MESSAGE_HANDLER_HPP

#include "stun_socket.hpp"
#include "data_stream.hpp"
#include "stun_auth.hpp"
#include "atomic_helper.hpp"

namespace oppvs
{
    class StunMessageBuilder;

	struct StunIncomingMessage
	{
		SocketRole role;
		StunSocketAddress localAddress;
		StunSocketAddress remoteAddress;
        StunMessageBuilder *handler;
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

    class StunMessageBuilder
	{
    private:
        DataStream m_dataStream;
        StunTransactionId m_transactionId;
        bool m_legacyMode;

        int addMappedAddressCommon(uint16_t type, const StunSocketAddress& address);
        int addMessageIntegrity(uint8_t* key, size_t keysize);
    public:
        StunMessageBuilder();
        DataStream& getDataStream();
        int getResult(SharedDynamicBufferRef& spBuffer); 
        void setLegacyMode(bool value);

        //Message Header
        int addMessageType(StunMessageType msgType, StunMessageClass msgClass);
        int addTransactionID(const StunTransactionId& transactionid);	//4 bytes magic cookies & 12 bytes transaction id
        int addRandomTransactionID(StunTransactionId* pTransactionid);
        int addMessageLength();

    	int addBindingRequestHeader();
    	int addBindingResponseHeader(bool success);

        //Message Attributes
        int addAttributeHeader(uint16_t type, uint16_t size);
        int addAttribute(uint16_t type, const void* data, uint16_t size);
        int addStringAttribute(uint16_t type, const char* data);

        int addMappedAddress(const StunSocketAddress& address);
        int addXorMappedAddress(const StunSocketAddress& address);

        int addResponseOriginAddress(const StunSocketAddress& other);
    	int addOtherAddress(const StunSocketAddress& other);

    	int addResponsePort(uint16_t port);
    	int addPaddingAttribute(uint16_t paddingSize);

    	int addChangeRequest(const StunChangeRequestAttribute& changeAttribute);
    	int addErrorCode(uint16_t errorNumber, const char* msg);
    	int addUnknownAttributes(const uint16_t* arrAttributeIds, size_t count);

    	int addFingerprintAttribute();

    	int addUserName(const char* pUserName);
    	int addRealm(const char* pRealm);
    	int addNonce(const char* pNonce);
       
    	int addMessageIntegrityShortTerm(const char* pPassword);
    	int addMessageIntegrityLongTerm(const char* pUserName, const char* pRealm, const char* pPassword);

	};

}

#endif

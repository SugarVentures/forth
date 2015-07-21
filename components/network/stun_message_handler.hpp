#ifndef OPPVS_STUN_MESSAGE_HANDLER_HPP
#define OPPVS_STUN_MESSAGE_HANDLER_HPP

#include "stun_socket.hpp"
#include "data_stream.hpp"
#include "stun_auth.hpp"
#include "atomic_helper.hpp"
#include "fasthash.h"

namespace oppvs
{
    class StunMessageBuilder;
    class StunMessageParser;
    int getXorMappedAddress(uint8_t* pData, size_t size, StunTransactionId &transid, StunSocketAddress* pAddr);
	int getMappedAddress(uint8_t* pData, size_t size, StunSocketAddress* pAddr);


	struct StunIncomingMessage
	{
		SocketRole role;
		StunSocketAddress localAddress;
		StunSocketAddress remoteAddress;
        StunMessageParser *handler;
		bool isConnectionOriented;     // true for TCP or TLS (where we can't send back to a different port)
	};

	struct StunOutgoingMessage
	{
		SocketRole role;
		StunSocketAddress destinationAddress;
		SharedDynamicBufferRef buffer;
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

	struct StunMessageIntegrity
	{
	    bool fSendWithIntegrity;
	    
	    bool fUseLongTerm;
	    char szUser[MAX_STUN_AUTH_STRING_SIZE+1]; // used for computing the message-integrity value
	    char szRealm[MAX_STUN_AUTH_STRING_SIZE+1]; // used for computing the message-integrity value
	    char szPassword[MAX_STUN_AUTH_STRING_SIZE+1]; // used for computing the message-integrity value
	};

	struct StunErrorCode
	{
	    uint16_t errorcode;
	    StunMessageClass msgclass;
	    uint16_t msgtype;
	    uint16_t attribUnknown; // for now, just send back one unknown attribute at a time
	    char szNonce[MAX_STUN_AUTH_STRING_SIZE+1];
	    char szRealm[MAX_STUN_AUTH_STRING_SIZE+1];
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

	class StunMessageParser
	{
	public:
		enum ReaderParseState
	    {
	        HeaderNotRead,
	        HeaderValidated,
	        BodyValidated,
	        ParseError
	    };

		StunMessageParser();
		void reset();

		void setAllowLegacyFormat(bool value);
    
    	ReaderParseState addBytes(const uint8_t* pData, uint32_t size);
    	uint16_t getNumberBytesNeeded();
    	ReaderParseState getState();

    	bool isMessageLegacyFormat();
    
	    bool hasFingerprintAttribute();
	    bool isFingerprintAttributeValid();
    
    	bool hasMessageIntegrityAttribute();
    	int validateMessageIntegrityShort(const char* pszPassword);
    	int validateMessageIntegrityLong(const char* pszUser, const char* pszRealm, const char* pszPassword);
    
	    int getAttributeByType(uint16_t attributeType, StunAttribute* pAttribute);
	    int getAttributeByIndex(int index, StunAttribute* pAttribute);
	    int getAttributeCount();

	    void getTransactionId(StunTransactionId* pTransId );
	    StunMessageClass getMessageClass();
	    uint16_t getMessageType();

	    int getResponsePort(uint16_t *pPort);
	    int getChangeRequest(StunChangeRequestAttribute* pChangeRequest);
	    int getPaddingAttributeSize(uint16_t* pSizePadding);
	    int getErrorCode(uint16_t* pErrorNumber);

	    SharedDynamicBufferRef getBuffer();
	    
	    
	    int getXorMappedAddress(StunSocketAddress* pAddress);
	    int getMappedAddress(StunSocketAddress* pAddress);
	    int getOtherAddress(StunSocketAddress* pAddress);
	    int getResponseOriginAddress(StunSocketAddress* pAddress);
	    
	    int getStringAttributeByType(uint16_t attributeType, char* pszValue, /*in-out*/ size_t size);
	    

	   	DataStream& getStream();

	private:
		DataStream m_dataStream;
		bool m_allowLegacyFormat;
		bool m_messageIsLegacyFormat;

		ReaderParseState m_state;

		static const size_t MAX_NUM_ATTRIBUTES = 30;

		typedef FastHash<uint16_t, StunAttribute, MAX_NUM_ATTRIBUTES, 53> AttributeHashTable; // 53 is a prime number for a reasonable table width
    
    	AttributeHashTable m_mapAttributes;
    
    	// special index values for message integrity attribute validation
    	int m_indexFingerprint;
    	int m_indexMessageIntegrity;
    	int m_countAttributes;
    

	    StunTransactionId m_transactionid;
	    uint16_t m_msgTypeNormalized;
	    StunMessageClass m_msgClass;
	    uint16_t m_msgLength;

	    int readHeader();
	    int readBody();

	    int getAddressHelper(uint16_t attribType, StunSocketAddress* pAddr);
	    
	    int validateMessageIntegrity(uint8_t* key, size_t keylength);
	};

	class StunRequestHandler
	{
	public:
    	static int processRequest(const StunIncomingMessage* msgIn, StunOutgoingMessage* msgOut, 
    		StunTransportAddressSet* pAddressSet);

    	void setMsgIn(const StunIncomingMessage* msgin);
    	void setMsgOut(StunOutgoingMessage* msgout);
    	void setTransportAddressSet(StunTransportAddressSet* addrset);
    	StunOutgoingMessage* getMsgOut();
	private:
	    
	    StunRequestHandler();

	    int processBindingRequest();
	    void buildErrorResponse();
	    int validateAuth();
	    int processRequestImpl();
	    
	    // input
	    StunTransportAddressSet* mp_addrSet;
	    const StunIncomingMessage* mp_msgIn;
	    StunOutgoingMessage* mp_msgOut;
	    
	    // member variables to remember along the way
	    StunMessageIntegrity m_integrity;
	    StunErrorCode m_error;
	    
	    bool m_isRequestHasResponsePort;
	    StunTransactionId m_transactionid;
	    bool m_legacyMode;
	    
	    bool hasAddress(SocketRole role);
	    bool isIPAddressZeroOrInvalid(SocketRole role);
	};

}

#endif

#include "stun_message_handler.hpp"
#include "crc.hpp"

#if defined( __APPLE__ )
 #include <CommonCrypto/CommonHMAC.h> 
 #define HMAC(algorithm, key, keyLength, data, dataLength, macOut, outLen) CCHmac(algorithm, key, keyLength, data, dataLength, macOut)
 #define HMAC_Init(hmac, key, len, algo) CCHmacInit(hmac, algo, key, len) 
 #define HMAC_Update CCHmacUpdate 
 #define HMAC_Final(hmac, hash, ptr) CCHmacFinal(hmac, hash) 
 #define HMAC_CTX_cleanup(c)
 #define HMAC_CTX_init(c)
 #define HMAC_CTX CCHmacContext

#define EVP_md5() kCCHmacAlgMD5 
 #define EVP_sha1() kCCHmacAlgSHA1
 #include <CommonCrypto/CommonDigest.h>
 #ifdef MD5_DIGEST_LENGTH

    #undef MD5_DIGEST_LENGTH

 #endif

 #define MD5_Init            CC_MD5_Init
 #define MD5_Update          CC_MD5_Update
 #define MD5_Final           CC_MD5_Final
 #define MD5_DIGEST_LENGTH   CC_MD5_DIGEST_LENGTH
 #define MD5_CTX             CC_MD5_CTX
 #define MD5				 CC_MD5

#else 
 #include <openssl/evp.h> 
 #include <openssl/hmac.h> 
 #include <openssl/md5.h>
#endif 
 
#include <fcntl.h>

namespace oppvs {
	static int g_sequence_number = 0xaaaaaaaa;

    StunMessageBuilder::StunMessageBuilder(): m_transactionId(), m_legacyMode(false)
    {

    }

    DataStream& StunMessageBuilder::getDataStream()
    {
        return m_dataStream;
    }

    void StunMessageBuilder::setLegacyMode(bool value)
    {
        m_legacyMode = value;
    }

    int StunMessageBuilder::addMessageType(StunMessageType msgType, StunMessageClass msgClass)
    {
    	uint16_t msgTypeField = 0;

    	m_dataStream.grow(200);

	    msgTypeField =  (msgType & 0x0f80) << 2;	//M0 - M3
	    msgTypeField |= (msgType & 0x0070) << 1;	//M4 - M6
	    msgTypeField |= (msgType & 0x000f);			//M7 - M11
	    msgTypeField |= (msgClass & 0x02) << 7;		//Bit 8th (l-r) of the first byte is the second bit of message class (C1)
	    msgTypeField |= (msgClass & 0x01) << 4;		//Bit 3th (l-r) of the second byte is the first bit of message class (C0)

	    if (m_dataStream.writeUInt16(htons(msgTypeField)) < 0)
	    	return -1;
	    if (m_dataStream.writeUInt16(0) < 0)
	    	return -1;

    	return 0;
    }

    int StunMessageBuilder::addTransactionID(const StunTransactionId& transactionid)
    {
    	m_transactionId = transactionid;
    	return m_dataStream.write((void*)transactionid.id, sizeof(transactionid.id));
    }

    int StunMessageBuilder::addRandomTransactionID(StunTransactionId* pTransId)
    {
    	StunTransactionId transid;
    	uint32_t stun_cookie_nbo = htonl(STUN_COOKIE);

    	uint32_t entropy=0;

	    // on x86, the rdtsc instruction is about as good as it gets for a random sequence number
	    // on linux, there's /dev/urandom


#ifdef _WIN32
	    // on windows, there's lots of simple stuff we can get at to give us a random number
	    // the rdtsc instruction is about as good as it gets
	    uint64_t clock = __rdtsc();
	    entropy ^= (uint32_t)(clock);
#else
	    // on linux, /dev/urandom should be sufficient
	    {
	    	int randomfile = open("/dev/urandom", O_RDONLY);
	    	if (randomfile >= 0)
	    	{
	    	    int readret = read(randomfile, &entropy, sizeof(entropy));
	            UNREFERENCED_VARIABLE(readret);
	    	    ASSERT(readret > 0);
	    		close(randomfile);
	    	}
	    }


	    if (entropy == 0)
	    {
	        entropy ^= getpid();
	        entropy ^= reinterpret_cast<uintptr_t>(this);
	        entropy ^= time(NULL);
	        entropy ^= AtomicIncrement(&g_sequence_number);
	    }

#endif


	    srand(entropy);


	    // the first four bytes of the transaction id is always the magic cookie
	    // followed by 12 bytes of the real transaction id
	    memcpy(transid.id, &stun_cookie_nbo, sizeof(stun_cookie_nbo));
	    for (int x = 4; x < (STUN_TRANSACTION_ID_LENGTH-4); x++)
	    {
	        transid.id[x] = (uint8_t)(rand() % 256);
	    }

	    if (pTransId)
	    {
	        *pTransId = transid;
	    }

	    return addTransactionID(transid);
    }

    int StunMessageBuilder::addMessageLength()
    {
    	uint16_t len = m_dataStream.size();
    	size_t currentPos = m_dataStream.getPosition();
    	if (len < STUN_HEADER_SIZE)
    	{
    		printf("Wrong size in stun message\n");
    		return -1;
    	}

    	len -= STUN_HEADER_SIZE;
    	m_dataStream.setAbsolutePosition(2);	//Message length is at 3 & 4th byte
    	m_dataStream.writeUInt16(htons(len));
    	m_dataStream.setAbsolutePosition(currentPos);
    	return 0;
    }

    int StunMessageBuilder::addAttributeHeader(uint16_t type, uint16_t size)
    {
    	int ret = 0;
    	ret = m_dataStream.writeUInt16(htons(type));
    	if (ret >= 0)
    		ret = m_dataStream.writeUInt16(htons(size));
    	return ret;
    }

    int StunMessageBuilder::addAttribute(uint16_t type, const void* data, uint16_t size)
    {
        uint8_t padBytes[4] = {0};
        size_t padding = 0;

        uint16_t sizeheader = size;

        if (data == NULL)
        {
            size = 0;
        }

        // attributes always start on a 4-byte boundary
        padding = (size % 4) ? (4 - (size % 4)) : 0;

        if (m_legacyMode)
        {
            // in legacy mode (RFC 3489), the header size of the attribute includes the padding
            // in RFC 5389, the attribute header is the exact size of the data, and extra padding bytes are implicitly assumed
            sizeheader += padding;
        }

        //zero length attributes as an indicator of something
        if (addAttributeHeader(type, sizeheader) < 0)
        	return -1;

        if (size > 0)
        {
        	if (m_dataStream.write(data, size) < 0)
            	return -1;
        }

        // pad with zeros to get the 4-byte alignment
        if (padding > 0)
        {
        	if (m_dataStream.write(padBytes, padding) < 0)
            	return -1;
        }
        return 0;
    }

    int StunMessageBuilder::addStringAttribute(uint16_t type, const char* data)
    {
    	if (!data)
    		return -1;
    	return addAttribute(type, data, strlen(data));
    }

    int StunMessageBuilder::addMappedAddressCommon(uint16_t attribute, const StunSocketAddress& address)
    {
    	uint16_t port;
    	size_t length;
    	uint8_t ip[STUN_IPV6_LENGTH];
    
    	uint8_t family = (address.getIP().getAddressFamily() == AF_INET) ? STUN_ATTRIBUTE_FIELD_IPV4 :STUN_ATTRIBUTE_FIELD_IPV6;
    	size_t attributeSize = (family == STUN_ATTRIBUTE_FIELD_IPV4) ? STUN_ATTRIBUTE_MAPPEDADDRESS_SIZE_IPV4 : STUN_ATTRIBUTE_MAPPEDADDRESS_SIZE_IPV6;

    	if (addAttributeHeader(attribute, attributeSize) < 0)
    		return -1;

    	port = address.getPort();
    	int ret = 0;
    	length = address.getIP().getRawBytes(ip, sizeof(ip));

    	if (length != STUN_IPV6_LENGTH && length != STUN_IPV4_LENGTH)
    		return -1;
    	ret = m_dataStream.writeUInt8(0);
    	if (ret < 0)
    		return -1;
    	ret = m_dataStream.writeUInt8(family);
    	if (ret < 0)
    		return -1;
    	ret = m_dataStream.writeUInt16(port);
    	if (ret < 0)
    		return -1;
    	ret = m_dataStream.write(ip, length);

    	return 0;
    }

    int StunMessageBuilder::addMappedAddress(const StunSocketAddress& address)
    {
    	return addMappedAddressCommon(STUN_ATTRIBUTE_MAPPEDADDRESS, address);
    }

    int StunMessageBuilder::addXorMappedAddress(const StunSocketAddress& address)
    {
    	StunSocketAddress addrxor(address);
    	uint16_t attributeID = m_legacyMode ? STUN_ATTRIBUTE_XORMAPPEDADDRESS_OPTIONAL : STUN_ATTRIBUTE_XORMAPPEDADDRESS;
		
		addrxor.applyXorMap(m_transactionId);
		return addMappedAddressCommon(attributeID, addrxor);
    }

    int StunMessageBuilder::addResponseOriginAddress(const StunSocketAddress& other)
    {
    	uint16_t attribid = m_legacyMode ? STUN_ATTRIBUTE_SOURCEADDRESS : STUN_ATTRIBUTE_RESPONSE_ORIGIN;
    	return addMappedAddressCommon(attribid, other);
    }
    
    int StunMessageBuilder::addOtherAddress(const StunSocketAddress& other)
    {
    	uint16_t attribid = m_legacyMode ? STUN_ATTRIBUTE_CHANGEDADDRESS : STUN_ATTRIBUTE_OTHER_ADDRESS;
    	return addMappedAddressCommon(attribid, other);
    }

	int StunMessageBuilder::addResponsePort(uint16_t port)
	{
		// convert to network byte order
    	port = htons(port);
    	return addAttribute(STUN_ATTRIBUTE_RESPONSE_PORT, &port, sizeof(port));
	}

	int StunMessageBuilder::addPaddingAttribute(uint16_t paddingSize)
	{
    	const uint16_t PADDING_BUFFER_SIZE = 128;
    	static char padding_bytes[PADDING_BUFFER_SIZE] = {};

	    // round up so we're a multiple of 4
	    if (paddingSize % 4)
	    {
	        paddingSize = paddingSize + 4 - (paddingSize % 4);
	    }


	    if (addAttributeHeader(STUN_ATTRIBUTE_PADDING, paddingSize) < 0)
	    	return -1;

	    while (paddingSize > 0)
	    {
	        uint16_t blocksize = (paddingSize >= PADDING_BUFFER_SIZE) ? PADDING_BUFFER_SIZE : paddingSize;
	        if (m_dataStream.write(padding_bytes, blocksize) < 0)
	        	return -1;
	        paddingSize -= blocksize;
	    }
	    return 0;
	}

	int StunMessageBuilder::addChangeRequest(const StunChangeRequestAttribute& changeAttribute)
	{
		uint32_t changeData = 0;

	    if (changeAttribute.fChangeIP)
	    {
	        changeData |= 0x04;
	    }

	    if (changeAttribute.fChangePort)
	    {
	        changeData |= 0x02;
	    }

	    changeData = htonl(changeData);
	    return addAttribute(STUN_ATTRIBUTE_CHANGEREQUEST, &changeData, sizeof(changeData));
	}

	int StunMessageBuilder::addErrorCode(uint16_t errorNumber, const char* msg)
	{
	    uint8_t padBytes[4] = {0};
	    size_t strsize = (msg == NULL) ? 0 : strlen(msg);
	    size_t size = strsize + 4;
	    size_t sizeheader = size;
	    size_t padding = 0;
	    uint8_t cl = 0;
	    uint8_t ernum = 0;

	    if (strsize >= 128 || errorNumber < 300 || errorNumber > 600)
	    {
	    	printf("addErrorCode: Invalid argument\n");
	    	return -1;
	    }
	    
	    padding = (size % 4) ? (4 - size % 4) : 0;

	    // fix for RFC 3489 clients - explicitly do the 4-byte padding alignment on the string with spaces instead of
	    // padding the message with zeros. Adjust the length field to always be a multiple of 4.
	    if ((size % 4) && m_legacyMode)
	    {
	        padding = 4 - (size % 4);
	    }
	    
	    if (m_legacyMode)
	    {
	        sizeheader += padding;
	    }

	    if (addAttributeHeader(STUN_ATTRIBUTE_ERRORCODE, sizeheader) < 0)
	    	return -1;

	    if (m_dataStream.writeUInt16(0) < 0)
	    	return -1;

	    cl = (uint8_t)(errorNumber / 100);
	    ernum = (uint8_t)(errorNumber % 100);

	    if (m_dataStream.writeUInt8(cl) < 0)
	    	return -1;

	    if (m_dataStream.writeUInt8(ernum) < 0)
	    	return -1;

	    if (strsize > 0)
	    {
	    	m_dataStream.write(msg, strsize);
	    }
	    
	    if (padding > 0)
	    {
	    	if (m_dataStream.write(padBytes, padding) < 0)
	        	return -1;
	    }
	    return 0;
	}

	int StunMessageBuilder::addUnknownAttributes(const uint16_t* arrAttributeIds, size_t count)
	{
	    uint16_t size = count * sizeof(uint16_t);
	    uint16_t unpaddedsize = size;
	    bool isPad = false;
	    
	    if (arrAttributeIds == NULL || count <= 0)
	    {
	    	printf("addUnknownAttributes: Invalid argument\n");
	    	return -1;
	    }
	    
	    // fix for RFC 3489. Since legacy clients can't understand implicit padding rules
	    // of rfc 5389, then we do what rfc 3489 suggests.  If there are an odd number of attributes
	    // that would make the length of the attribute not a multiple of 4, then repeat one
	    // attribute.
	    
	    isPad = m_legacyMode && (!!(count % 2));
	    
	    if (isPad)
	    {
	        size += sizeof(uint16_t);
	    }
	    
	    if (addAttributeHeader(STUN_ATTRIBUTE_UNKNOWNATTRIBUTES, size) < 0)
	    	return -1;
	    
	    if (m_dataStream.write(arrAttributeIds, unpaddedsize) < 0)
	    	return -1;
	    
	    if (isPad)
	    {
	        // repeat the last attribute in the array to get an even alignment of 4 bytes
	        if (m_dataStream.write(&arrAttributeIds[count-1], sizeof(arrAttributeIds[0])) < 0)
	        	return -1;
	    }
	    return 0;
	}

	int StunMessageBuilder::addFingerprintAttribute()
	{
	    uint32_t value;
	    SharedDynamicBufferRef spBuffer;
	    uint8_t* pData = NULL;
	    size_t length = 0;
	    int offset;

	    if (m_dataStream.writeUInt16(htons(STUN_ATTRIBUTE_FINGERPRINT)) < 0)
	    	return -1;

	    //field length is 4 bytes
	    if (m_dataStream.writeUInt16(htons(sizeof(uint32_t))) < 0)
	    	return -1;

	    if (m_dataStream.writeUInt32(0) < 0)
	    	return -1;

	    if (addMessageLength() < 0)
	    	return -1;

	    // now do a CRC-32 on everything but the last 8 bytes
	    spBuffer = m_dataStream.getBuffer();
	    pData = spBuffer->data();
	    length = spBuffer->size();

	    ASSERT(length > 8);
	    length = length - 8;

	    value = crc(pData, pData + length);
	    value = value ^ STUN_FINGERPRINT_XOR;

	    offset = -(int)(sizeof(value));

	    if (m_dataStream.setRelativePosition(offset) < 0)
	    	return -1;

	    if (m_dataStream.writeUInt32(htonl(value)) < 0)
	    	return -1;

		return 0;
	}

	int StunMessageBuilder::addUserName(const char* pUserName)
	{
		return addStringAttribute(STUN_ATTRIBUTE_USERNAME, pUserName);
	}

	int StunMessageBuilder::addRealm(const char* pRealm)
	{
		return addStringAttribute(STUN_ATTRIBUTE_REALM, pRealm);
	}

	int StunMessageBuilder::addNonce(const char* pNonce)
	{
		return addStringAttribute(STUN_ATTRIBUTE_NONCE, pNonce);
	}

	int StunMessageBuilder::addMessageIntegrity(uint8_t* key, size_t keysize)
	{
	    const size_t c_hmacsize = 20;
	    uint8_t hmacvaluedummy[c_hmacsize] = {}; // zero-init
	    unsigned int resultlength = c_hmacsize;
	    uint8_t* pDstBuf = NULL;
	    
	    SharedDynamicBufferRef spBuffer;
	    void* pData = NULL;
	    size_t length = 0;
	    unsigned char* pHashResult = NULL;
	    UNREFERENCED_VARIABLE(pHashResult);
	    
	    if (key==NULL || keysize <= 0)
	    {
	    	printf("addMessageIntegrity: Invalid argument\n");
	    	return -1;
	    }
	    
	    // add in a "zero-init" HMAC value.  This adds 24 bytes to the length
	    if (addAttribute(STUN_ATTRIBUTE_MESSAGEINTEGRITY, hmacvaluedummy, c_hmacsize) < 0)
	    	return -1;

	    if (addMessageLength() < 0)
	    	return -1;
	    // now do a SHA1 on everything but the last 24 bytes (4 bytes of the attribute header and 20 bytes for the dummy content)

	    spBuffer = m_dataStream.getBuffer();
	    pData = spBuffer->data();
	    length = spBuffer->size();

	    ASSERT(length > 24);
	    length = length - 24;
	    
	    
	    // now do a little pointer math so that HMAC can write exactly to where the hash bytes will appear
	    pDstBuf = ((uint8_t*)pData) + length + 4;
	    HMAC(EVP_sha1(), key, keysize, (uint8_t*)pData, length, pDstBuf, &resultlength);
	    
	    ASSERT(resultlength == 20);
	    //ASSERT(pHashResult != NULL);
	    return 0;
	}

	int StunMessageBuilder::addMessageIntegrityShortTerm(const char* pPassword)
	{
		return addMessageIntegrity((uint8_t*)pPassword, sizeof(pPassword));
	}

	int StunMessageBuilder::addMessageIntegrityLongTerm(const char* pUserName, const char* pRealm, const char* pPassword)
	{
	    const size_t MAX_KEY_SIZE = MAX_STUN_AUTH_STRING_SIZE * 3 + 2;
	    uint8_t key[MAX_KEY_SIZE + 1]; // long enough for 64-char strings and two semicolons and a null char for debugging
	    
	    uint8_t hash[MD5_DIGEST_LENGTH] = {};
	    uint8_t* pResult = NULL;
	    uint8_t* pDst = key;
	    
	    size_t lenUserName = pUserName ? strlen(pUserName) : 0;
	    size_t lenRealm = pRealm ? strlen(pRealm) : 0;
	    size_t lenPassword = pPassword ? strlen(pPassword) : 0;
	    size_t lenTotal = lenUserName + lenRealm + lenPassword + 2; // +2 for the two colons

	    UNREFERENCED_VARIABLE(pResult);
	    
	    if (lenTotal > MAX_KEY_SIZE)
	    {
	    	printf("Invalid argument\n");
	    }
	    
	    pDst = key;
	    
	    memcpy(pDst, pUserName, lenUserName);
	    pDst += lenUserName;
	    
	    *pDst = ':';
	    pDst++;
	    
	    memcpy(pDst, pRealm, lenRealm);
	    pDst += lenRealm;
	    
	    *pDst = ':';
	    pDst++;

	    memcpy(pDst, pPassword, lenPassword);
	    pDst += lenPassword;
	    *pDst ='\0'; // null terminate for debugging (this char doesn not get hashed
	    
	    ASSERT((pDst-key) == lenTotal);

	    pResult = MD5(key, lenTotal, hash);
	    ASSERT(pResult != NULL);
	    return addMessageIntegrity(hash, MD5_DIGEST_LENGTH);
	}

	int StunMessageBuilder::getResult(SharedDynamicBufferRef& spBuffer)
	{
		int ret = addMessageLength();
		if (ret >= 0)
			spBuffer = m_dataStream.getBuffer();

		return ret;
	}

	int StunMessageBuilder::addBindingRequestHeader()
	{		
    	return addMessageType(StunMsgTypeBinding, StunMsgClassRequest);
	}

	int StunMessageBuilder::addBindingResponseHeader(bool success)
	{
		return addMessageType(StunMsgTypeBinding, success ? StunMsgClassSuccessResponse : StunMsgClassFailureResponse);
	}

	/*
		Functions of StunMessageParser Class
	*/
	StunMessageParser::StunMessageParser()
	{
		reset();
	}

	void StunMessageParser::reset()
	{
		m_allowLegacyFormat = true;
    	m_messageIsLegacyFormat = false;
    	m_state = HeaderNotRead;
    	m_mapAttributes.Reset();
    
	    m_indexFingerprint = -1;
	    m_indexMessageIntegrity = -1;
	    m_countAttributes = 0;
	    
	    memset(&m_transactionid, '\0', sizeof(m_transactionid));
	    m_msgTypeNormalized = 0xffff;
	    m_msgClass = StunMsgClassInvalidMessageClass;
	    m_msgLength = 0;

	    m_dataStream.reset();
	}

	void StunMessageParser::setAllowLegacyFormat(bool value)
	{
		m_allowLegacyFormat = value;
	}
    
    int StunMessageParser::readHeader()
    {
    	bool isHeaderValid = false;
	    uint16_t msgType;
	    uint16_t msgLength;
	    uint32_t cookie;
	    StunTransactionId transID;

	    if (m_dataStream.setAbsolutePosition(0) < 0)
	    	return -1;
	    if (m_dataStream.readUInt16(&msgType) < 0)
	    	return -1;
	    if (m_dataStream.readUInt16(&msgLength) < 0)
	    	return -1;
	    if (m_dataStream.read(&transID.id, sizeof(transID.id)) < 0)
	    	return -1;

	    // convert from big endian to native type
	    msgType = ntohs(msgType);
	    msgLength = ntohs(msgLength);

	    memcpy(&cookie, transID.id, 4);
	    cookie = ntohl(cookie);

    	m_messageIsLegacyFormat = !(cookie == STUN_COOKIE);

    	isHeaderValid = ( (0==(msgType & 0xc000)) && ((msgLength%4)==0) );

    	// if we aren't in legacy format (the default), then the cookie field of the transaction id must be the STUN_COOKIE
    	isHeaderValid = (isHeaderValid && (m_allowLegacyFormat || !m_messageIsLegacyFormat));

    	if (isHeaderValid == false)
    		return -1;

    	m_msgTypeNormalized = (  (msgType & 0x000f) | ((msgType & 0x00e0)>>1) | ((msgType & 0x3E00)>>2)  );
    	m_msgLength = msgLength;

    	m_transactionid = transID;

    	if(m_msgLength > MAX_STUN_MESSAGE_SIZE)
    		return -1;


	    if (STUN_IS_REQUEST(msgType))
	    {
	        m_msgClass = StunMsgClassRequest;
	    }
	    else if (STUN_IS_INDICATION(msgType))
	    {
	        m_msgClass = StunMsgClassIndication;
	    }
	    else if (STUN_IS_SUCCESS_RESP(msgType))
	    {
	        m_msgClass = StunMsgClassSuccessResponse;
	    }
	    else if (STUN_IS_ERR_RESP(msgType))
	    {
	        m_msgClass = StunMsgClassFailureResponse;
	    }
	    else
	    {
	        // couldn't possibly happen, because msgClass is only two bits
	        m_msgClass = StunMsgClassInvalidMessageClass;
	        return -1;
	    }
	    return 0;
	    
    }

    int StunMessageParser::readBody()
    {
    	size_t currentSize = m_dataStream.size();
    	size_t bytesConsumed = STUN_HEADER_SIZE;
    	int hr = 0;

    	if (m_dataStream.setAbsolutePosition(STUN_HEADER_SIZE) < 0)
    		return -1;

	    while (hr == 0 && (bytesConsumed < currentSize))
	    {
	        uint16_t attributeType;
	        uint16_t attributeLength;
	        uint16_t attributeOffset;
	        int paddingLength=0;

	        hr = m_dataStream.readUInt16(&attributeType);

	        if (hr == 0)
	        {
	            hr = m_dataStream.readUInt16(&attributeLength);
	        }

	        if (hr == 0)
	        {
	            attributeOffset = m_dataStream.getPosition();
	            attributeType = ntohs(attributeType);
	            attributeLength = ntohs(attributeLength);

	            // todo - if an attribute has no size, it's length is not padded by 4 bytes, right?
	            if (attributeLength % 4)
	            {
	                paddingLength = 4 - attributeLength % 4;
	            }

	            hr = (attributeLength <= MAX_STUN_ATTRIBUTE_SIZE) ? 0 : -1;
	        }

	        if (hr == 0)
	        {
	            int result;
	            StunAttribute attrib;
	            attrib.attributeType = attributeType;
	            attrib.size = attributeLength;
	            attrib.offset = attributeOffset;

	            // if we have already read in more attributes than MAX_NUM_ATTRIBUTES, then Insert call will fail (this is how we gate too many attributes)
	            result = m_mapAttributes.Insert(attributeType, attrib);
	            hr = (result >= 0) ? 0 : -1;
	        }
	        
	        if (hr == 0)
	        {
	            
	            if (attributeType == STUN_ATTRIBUTE_FINGERPRINT)
	            {
	                m_indexFingerprint = m_countAttributes;
	            }
	            
	            if (attributeType == STUN_ATTRIBUTE_MESSAGEINTEGRITY)
	            {
	                m_indexMessageIntegrity = m_countAttributes;
	            }
	            
	            m_countAttributes++;
	        }
	        

	        
	        if (hr == 0)
	        {
	            hr = m_dataStream.setRelativePosition(attributeLength);
	        }

	        // consume the padding
	        if (hr == 0)
	        {
	            if (paddingLength > 0)
	            {
	                hr = m_dataStream.setRelativePosition(paddingLength);
	            }
	        }

	        if (hr == 0)
	        {
	            bytesConsumed += sizeof(attributeType) + sizeof(attributeLength) + attributeLength + paddingLength;
	        }
	    }

	    // I don't think we could consume more bytes than stream size, but it's a worthy check to still keep here
	    hr = (bytesConsumed == currentSize) ? 0 : -1;

	    return hr;
    }

	StunMessageParser::ReaderParseState StunMessageParser::addBytes(const uint8_t* pData, uint32_t size)
	{
	    size_t currentSize;
	    int hr = 0;

	    if (m_state == ParseError)
	    {
	        return ParseError;
	    }

	    if (size == 0)
	    {
	        return m_state;
	    }
	    
	    // seek to the end of the stream
	    m_dataStream.setAbsolutePosition(m_dataStream.size());

	    if (m_dataStream.write(pData, size) < 0)
	    {
	        return ParseError;
	    }

	    currentSize = m_dataStream.size();

	    if (m_state == HeaderNotRead)
	    {
	        if (currentSize >= STUN_HEADER_SIZE)
	        {
	            hr = readHeader();

	            m_state = hr == 0 ? HeaderValidated : ParseError;

	            if (hr == 0 && (m_msgLength==0))
	            {
	                m_state = BodyValidated;
	            }
	        }
	    }

	    if (m_state == HeaderValidated)
	    {
	        if (currentSize >= (m_msgLength + STUN_HEADER_SIZE))
	        {
	            if (currentSize == (m_msgLength + STUN_HEADER_SIZE))
	            {
	                hr = readBody();
	                m_state = hr == 0 ? BodyValidated : ParseError;
	            }
	            else
	            {
	                // TOO MANY BYTES FED IN
	                m_state = ParseError;
	            }
	        }
	    }

	    if (m_state == BodyValidated)
	    {
	        // What?  After validating the body, the caller still passes in way too many bytes?
	        if (currentSize > (m_msgLength + STUN_HEADER_SIZE))
	        {
	            m_state = ParseError;
	        }
	    }

	    return m_state;
	}

	uint16_t StunMessageParser::getNumberBytesNeeded()
	{
		size_t currentSize = m_dataStream.size();
	    switch (m_state)
	    {
		    case HeaderNotRead:
		        ASSERT(STUN_HEADER_SIZE > currentSize);
		        return STUN_HEADER_SIZE - currentSize;
		    case HeaderValidated:
		        ASSERT((m_msgLength + STUN_HEADER_SIZE) > currentSize);
		        return (m_msgLength + STUN_HEADER_SIZE) - currentSize;
		    default:
		    	return 0;
	    }
	    return 0;
	}

	StunMessageParser::ReaderParseState StunMessageParser::getState()
	{
		return m_state;
	}

	bool StunMessageParser::isMessageLegacyFormat()
	{
		return m_messageIsLegacyFormat;
	}

    bool StunMessageParser::hasFingerprintAttribute()
    {
    	StunAttribute *pAttrib = m_mapAttributes.Lookup(STUN_ATTRIBUTE_FINGERPRINT);
    	return (pAttrib != NULL);
    }

    bool StunMessageParser::isFingerprintAttributeValid()
    {    
	    StunAttribute* pAttrib = m_mapAttributes.Lookup(STUN_ATTRIBUTE_FINGERPRINT);
	    SharedDynamicBufferRef spBuffer;
	    size_t size = 0;
	    uint32_t computedValue = 1;
	    uint32_t readValue = 0;
	    uint8_t* ptr = NULL;


	    // the fingerprint attribute MUST be the last attribute in the stream.
	    // If it's not, then the code below will return false

	    if (pAttrib == NULL)
	    	return false;
	    
	    if (pAttrib->attributeType != STUN_ATTRIBUTE_FINGERPRINT)
	    	return false;

	   	if (pAttrib->size != 4)
	   		return false;

	    if (m_state != BodyValidated)
	    	return false;

	    spBuffer = m_dataStream.getBuffer();

	    size = m_dataStream.size();
	    if (size < STUN_HEADER_SIZE)
	    	return false;

	    ptr = spBuffer->data();
	    if (ptr == NULL)
	    	return false;
	    
	    computedValue = crc(ptr, ptr + size - 8);
	    computedValue = computedValue ^ STUN_FINGERPRINT_XOR;

	    readValue = *(uint32_t*)(ptr + pAttrib->offset);
	    readValue = ntohl(readValue);
	    int hr = (readValue == computedValue) ? 0 : -1;
	    return hr;
    }

	bool StunMessageParser::hasMessageIntegrityAttribute()
	{
		return (NULL != m_mapAttributes.Lookup(STUN_ATTRIBUTE_MESSAGEINTEGRITY));
	}

	int StunMessageParser::validateMessageIntegrity(uint8_t* key, size_t keylength)
	{
    
    	int lastAttributeIndex = m_countAttributes - 1;
    	bool fingerprintAdjustment = false;
    	bool noOtherAttributesAfterIntegrity = false;
    	const size_t c_hmacsize = 20;
    	uint8_t hmaccomputed[c_hmacsize] = {}; // zero-init
    	unsigned int hmaclength = c_hmacsize;
	    HMAC_CTX ctx = {};
	    uint32_t chunk32;
	    uint16_t chunk16;
	    size_t len, nChunks;
	    DataStream stream;
	    SharedDynamicBufferRef spBuffer;
	    StunAttribute* pAttribIntegrity = NULL;
    
	    int cmp = 0;
	    bool contextInit = false;
    
    	if (m_state != BodyValidated)
    		return -1;
   	
   		if (m_countAttributes == 0)	// if there's not attributes, there's definitely not a message integrity attribute
   			return -1;
    
    	if (m_indexMessageIntegrity == -1)
    		return -1;
    
    	// can a key be empty?
    	if (key == NULL || keylength == 0)
    	{
    		printf("validateMessageIntegrity: Invalid input parameter\n");
    		return -1;
    	}
    
    	pAttribIntegrity = m_mapAttributes.Lookup(STUN_ATTRIBUTE_MESSAGEINTEGRITY);

    	if (pAttribIntegrity == NULL)
    		return -1;
    
    	if (pAttribIntegrity->size != c_hmacsize)
    		return -1;
    
    	// first, check to make sure that no other attributes (other than fingerprint) follow the message integrity
    	noOtherAttributesAfterIntegrity = (m_indexMessageIntegrity == lastAttributeIndex) || 
    		((m_indexMessageIntegrity == (lastAttributeIndex-1)) && (m_indexFingerprint == lastAttributeIndex));
    
    	if (noOtherAttributesAfterIntegrity == false)
    		return -1;
    
    	fingerprintAdjustment = (m_indexMessageIntegrity == (lastAttributeIndex - 1));

    	getBuffer();
    	stream.attach(spBuffer, false);
    
	    // Here comes the fun part.  If there is a fingerprint attribute, we have to adjust the length header in computing the hash
	    HMAC_CTX_init(&ctx);
    	contextInit = true;
    	HMAC_Init(&ctx, key, keylength, EVP_sha1());
    
    	// message type
    	if (stream.readUInt16(&chunk16) < 0)
    		return -1;

    	HMAC_Update(&ctx, (unsigned char*)&chunk16, sizeof(chunk16));
    
    	// message length
    	if (stream.readUInt16(&chunk16) < 0)
    		return -1;

	    if (fingerprintAdjustment)
	    {
	        // subtract the length of the fingerprint off the length header
	        // fingerprint attribute is 8 bytes long including it's own header
	        // and to do this, we have to fix the network byte ordering issue
	        uint16_t lengthHeader = ntohs(chunk16);
	        uint16_t adjustedlengthHeader = lengthHeader - 8;
	        
	        chunk16 = htons(adjustedlengthHeader);
	    }

    	HMAC_Update(&ctx, (unsigned char*)&chunk16, sizeof(chunk16));
    
    	// now include everything up to the hash attribute itself.
	    len = pAttribIntegrity->offset;
	    len -= 4; // subtract the size of the attribute header
	    len -= 4; // subtract the size of the message header (not including the transaction id)
	    
	    // len should be the number of bytes from the start of the transaction ID up through to the start of the integrity attribute header
	    // the stun message has to be a multiple of 4 bytes, so we can read in 32 bit chunks
	    nChunks = len / 4;
	    ASSERT((len % 4) == 0);
	    for (size_t count = 0; count < nChunks; count++)
	    {
	        if (stream.readUInt32(&chunk32) < 0)
	        	return -1;
	        HMAC_Update(&ctx, (unsigned char*)&chunk32, sizeof(chunk32));
	    }
    
    	HMAC_Final(&ctx, hmaccomputed, &hmaclength);
    
	    // now compare the bytes
	    cmp = memcmp(hmaccomputed, spBuffer->data() + pAttribIntegrity->offset, c_hmacsize);
	    
	    int hr = (cmp == 0 ? 0 : -1);
    
	    if (contextInit)
	    {
	        HMAC_CTX_cleanup(&ctx);
	    }
        
    	return hr;
	}

	int StunMessageParser::validateMessageIntegrityShort(const char* pszPassword)
	{
		return validateMessageIntegrity((uint8_t*)pszPassword, strlen(pszPassword));
	}

	int StunMessageParser::validateMessageIntegrityLong(const char* pszUser, const char* pszRealm, const char* pszPassword)
	{
	    const size_t MAX_KEY_SIZE = MAX_STUN_AUTH_STRING_SIZE * 3 + 2;
	    uint8_t key[MAX_KEY_SIZE + 1]; // long enough for 64-char strings and two semicolons and a null char
	    uint8_t* pData = NULL;
	    uint8_t* pDst = key;
	    size_t totallength = 0;
	    
	    size_t passwordlength = pszPassword ? strlen(pszPassword) : 0;
	    size_t userLength = pszUser ? strlen(pszUser)  : 0;
	    size_t realmLength = pszRealm ? strlen(pszRealm) : 0;
	    
	    uint8_t hash[MD5_DIGEST_LENGTH] = {};
	    
	    if (m_state != BodyValidated)
	    	return -1;
	    
	   
	    totallength = userLength + realmLength + passwordlength + 2; // +2 for two semi-colons
	    
	    pData = getStream().getUnSafeDataPointer();
	    if (pData == NULL)
	    	return -1;
	    
	    if (userLength > 0)
	    {
	        memcpy(pDst, pszUser, userLength);
	        pDst += userLength;
	    }
	    *pDst = ':';
	    pDst++;
	    
	    
	    if (realmLength > 0)
	    {
	        memcpy(pDst, pszRealm, realmLength);
	        pDst += realmLength;
	    }
	    *pDst = ':';
	    pDst++;

	    if (passwordlength > 0)
	    {
	        memcpy(pDst, pszPassword, passwordlength);
	        pDst += passwordlength;
	    }
	    *pDst = '0'; // null terminate for debugging (does not get hashed)
	    
	    ASSERT((pDst-key) == totallength);
	    
	    if (MD5(key, totallength, hash) == NULL)
	    	return -1;
	    
	    return validateMessageIntegrity(hash, sizeof(hash));	    
	}

    int StunMessageParser::getAttributeByType(uint16_t attributeType, StunAttribute* pAttribute)
    {
    	StunAttribute* pFound = m_mapAttributes.Lookup(attributeType);
        
	    if (pFound == NULL)
	    {
	        return -1;
	    }
	    
	    if (pAttribute)
	    {
	        *pAttribute = *pFound;
	    }
	    return 0;
    }

    int StunMessageParser::getAttributeByIndex(int index, StunAttribute* pAttribute)
    {
    	StunAttribute* pFound = m_mapAttributes.LookupValueByIndex((size_t)index);
    
	    if (pFound == NULL)
	    {
	        return -1;
	    }
	    
	    if (pAttribute)
	    {
	        *pAttribute = *pFound;
	    }
	    return 0;
    	
    }

    int StunMessageParser::getAttributeCount()
    {
    	return (int)(m_mapAttributes.Size());
    }

    void StunMessageParser::getTransactionId(StunTransactionId* pTransId)
    {
    	if (pTransId)
	    {
	        *pTransId = m_transactionid;
	    }
    }

    StunMessageClass StunMessageParser::getMessageClass()
    {
    	return m_msgClass;
    }

    uint16_t StunMessageParser::getMessageType()
    {
    	return m_msgTypeNormalized;
    }

    int StunMessageParser::getResponsePort(uint16_t *pPort)
    {
    	StunAttribute* pAttrib = NULL;
	    uint16_t portNBO;
	    uint8_t *pData = NULL;

	    if (pPort == NULL)
	    {
	    	printf("getResponsePort: invalid input parameter\n");
	    	return -1;
	    }

	    pAttrib = m_mapAttributes.Lookup(STUN_ATTRIBUTE_RESPONSE_PORT);
	    if (pAttrib == NULL)
	    	return -1;
	   
	   	if (pAttrib->size != STUN_ATTRIBUTE_RESPONSE_PORT_SIZE)
	   		return -1;

	    pData = m_dataStream.getUnSafeDataPointer();
	    if (pData == NULL)
	    	return -1;

	    memcpy(&portNBO, pData + pAttrib->offset, STUN_ATTRIBUTE_RESPONSE_PORT_SIZE);
	    *pPort = ntohs(portNBO);
	    return 0;
    }

    int StunMessageParser::getChangeRequest(StunChangeRequestAttribute* pChangeRequest)
    {
	    uint8_t *pData = NULL;
	    StunAttribute *pAttrib;
	    uint32_t value = 0;

	    if (pChangeRequest == NULL)
	    {
	    	printf("getChangeRequest: Invalid parameter\n");
	    	return -1;
	    }
	    
	    pAttrib = m_mapAttributes.Lookup(STUN_ATTRIBUTE_CHANGEREQUEST);
	    if (pAttrib == NULL)
	    	return -1;
	    
	    if (pAttrib->size != STUN_ATTRIBUTE_CHANGEREQUEST_SIZE)
	    	return -1;

	    pData = m_dataStream.getUnSafeDataPointer();
	    if (pData == NULL)
	    	return -1;

	    memcpy(&value, pData + pAttrib->offset, STUN_ATTRIBUTE_CHANGEREQUEST_SIZE);

	    value = ntohl(value);

	    pChangeRequest->fChangeIP = !!(value & 0x0004);
	    pChangeRequest->fChangePort = !!(value & 0x0002);

	    return 0;
    }

    int StunMessageParser::getPaddingAttributeSize(uint16_t* pSizePadding)
    {
	    StunAttribute *pAttrib;

		if (pSizePadding == NULL)
		{
			return -1;
		}

	    *pSizePadding = 0;
	    
	    pAttrib = m_mapAttributes.Lookup(STUN_ATTRIBUTE_PADDING);

	    if (pAttrib == NULL)
	    	return -1;

	    *pSizePadding = pAttrib->size;
	    return 0;
    }

    int StunMessageParser::getErrorCode(uint16_t* pErrorNumber)
    {
	    uint8_t* ptr = NULL;
	    uint8_t cl = 0;
	    uint8_t num = 0;

	    StunAttribute* pAttrib;

	    if (pErrorNumber == NULL)
	    	return -1;

	    pAttrib = m_mapAttributes.Lookup(STUN_ATTRIBUTE_ERRORCODE);
	    if (pAttrib == NULL)
	    	return -1;

	    // first 21 bits of error-code attribute must be zero.
	    // followed by 3 bits of "class" and 8 bits for the error number modulo 100
	    ptr = m_dataStream.getUnSafeDataPointer() + pAttrib->offset + 2;

	    cl = *ptr++;
	    cl = cl & 0x07;
	    num = *ptr;
	    *pErrorNumber = cl * 100 + num;
	    return 0;
    }

    SharedDynamicBufferRef StunMessageParser::getBuffer()
    {
    	return m_dataStream.getBuffer();
    }
    
    int StunMessageParser::getAddressHelper(uint16_t attribType, StunSocketAddress* pAddr)
    {
	    StunAttribute* pAttrib = m_mapAttributes.Lookup(attribType);
	    uint8_t *pAddrStart = NULL;

    	if (pAttrib == NULL)
    		return -1;
    
    	pAddrStart = m_dataStream.getUnSafeDataPointer() + pAttrib->offset;
    	return ::oppvs::getMappedAddress(pAddrStart, pAttrib->size, pAddr);
    }

    int StunMessageParser::getXorMappedAddress(StunSocketAddress* pAddress)
    {
	    int hr = getAddressHelper(STUN_ATTRIBUTE_XORMAPPEDADDRESS, pAddress);
	    
	    if (hr == -1)
	    {
	        // this is the vovida compat address attribute
	        hr = getAddressHelper(STUN_ATTRIBUTE_XORMAPPEDADDRESS_OPTIONAL, pAddress);
	    }
	    
	    if (hr == 0)
	    {
	        pAddress->applyXorMap(m_transactionid);
	    }

	    return hr;
    }

    int StunMessageParser::getMappedAddress(StunSocketAddress* pAddress)
    {
    	return getAddressHelper(STUN_ATTRIBUTE_MAPPEDADDRESS, pAddress);
    }

    int StunMessageParser::getOtherAddress(StunSocketAddress* pAddress)
    {
    	int hr = 0;
    
	    hr = getAddressHelper(STUN_ATTRIBUTE_OTHER_ADDRESS, pAddress);
	    
	    if (hr == -1)
	    {
	        // look for the legacy changed address attribute that a legacy (RFC 3489) server would send
	        hr = getAddressHelper(STUN_ATTRIBUTE_CHANGEDADDRESS, pAddress);
	    }
	    
	    return hr;
    }

    int StunMessageParser::getResponseOriginAddress(StunSocketAddress* pAddress)
    {
    	int hr = 0;
    
    	hr = getAddressHelper(STUN_ATTRIBUTE_RESPONSE_ORIGIN, pAddress);
    
	    if (hr == -1)
	    {
	        // look for the legacy address attribute that a legacy (RFC 3489) server would send
	        hr = getAddressHelper(STUN_ATTRIBUTE_SOURCEADDRESS, pAddress);
	    }
	    
	    return hr;
    }
    
    int StunMessageParser::getStringAttributeByType(uint16_t attributeType, char* pszValue, /*in-out*/ size_t size)
    {
	    StunAttribute* pAttrib = m_mapAttributes.Lookup(attributeType);
	    
	    if (pszValue == NULL || pAttrib == NULL)
	    {
	    	printf("Invalid argument \n");
	    	return -1;
	    }
	    
	    // size needs to be 1 greater than attrib.size so we can properly copy over a null char at the end
	    if (pAttrib->size >= size)
	    {
	    	printf("Invalid argument \n");
	    	return -1;
	    }
	    
	    memcpy(pszValue, m_dataStream.getUnSafeDataPointer() + pAttrib->offset, pAttrib->size);
	    pszValue[pAttrib->size] = '\0';
	    
		return 0;
    }
    

   	DataStream& StunMessageParser::getStream()
   	{
   		return m_dataStream;
   	}

   	/*
   		Functions of StunRequestHandler
   	*/

   	StunRequestHandler::StunRequestHandler() : mp_addrSet(NULL), mp_msgIn(NULL), mp_msgOut(NULL), 
   		m_integrity(), m_error(), m_isRequestHasResponsePort(false), m_transactionid(), m_legacyMode(false)
   	{

   	}

   	void StunRequestHandler::setMsgIn(const StunIncomingMessage* msgin)
   	{
   		mp_msgIn = msgin;
   	}
    	
    void StunRequestHandler::setMsgOut(StunOutgoingMessage* msgout)
    {
    	mp_msgOut = msgout;
    }

    StunOutgoingMessage* StunRequestHandler::getMsgOut()
    {
    	return mp_msgOut;
    }
    	
    void StunRequestHandler::setTransportAddressSet(StunTransportAddressSet* addrset)
    {
    	mp_addrSet = addrset;
    }
    

   	int StunRequestHandler::processRequest(const StunIncomingMessage* msgIn, StunOutgoingMessage* msgOut, 
    		StunTransportAddressSet* pAddressSet)
   	{
   		if (msgIn == NULL || msgOut == NULL)
   		{
   			printf("processRequest: Invalid argument\n");
   			return -1;
   		}

   		if (msgIn->handler == NULL || !IsValidSocketRole(msgIn->role))
   		{
   			printf("processRequest: Invalid argument\n");
   			return -1;
   		}

   		if (msgOut->buffer == NULL)
   		{
   			printf("processRequest: Invalid argument\n");
   			return -1;
   		}

   		if (msgOut->buffer->capacity() < MAX_STUN_MESSAGE_SIZE)
   		{
   			printf("processRequest: Invalid argument\n");
   			return -1;
   		}

   		if (pAddressSet == NULL) {
   			printf("processRequest: Invalid argument\n");
   			return -1;	
   		}

   		if (msgIn->handler->getState() != StunMessageParser::BodyValidated)
   		{
   			printf("processRequest: Unepxected error\n");
   			return -1;
   		}

   		msgOut->role = msgIn->role;
   		msgOut->destinationAddress = msgIn->remoteAddress;
   		StunRequestHandler handler;
   		handler.setMsgIn(msgIn);
   		handler.setMsgOut(msgOut);
   		handler.setTransportAddressSet(pAddressSet);

   		handler.processRequestImpl();
   		return 0;
   	}

   	int StunRequestHandler::processBindingRequest()
   	{
   		StunMessageParser& reader = *(mp_msgIn->handler);
    	printf("processBindingRequest starts\n");
	    bool requestHasPaddingAttribute = false;
	    SocketRole socketOutput = mp_msgIn->role; // initialize to be from the socket we received from
	    StunChangeRequestAttribute changerequest = {};
	    bool sendOtherAddress = false;
	    bool sendOriginAddress = false;
	    SocketRole socketOther;
	    StunSocketAddress addrOrigin;
	    StunSocketAddress addrOther;
	    StunMessageBuilder builder;
	    uint16_t paddingSize = 0;
	    int hrResult;

    
    	mp_msgOut->buffer->setSize(0);
    	builder.getDataStream().attach(mp_msgOut->buffer, true);

	    // if the client request smells like RFC 3478, then send the resposne back in the same way
	    builder.setLegacyMode(m_legacyMode);

	    // check for an alternate response port
	    // check for padding attribute (todo - figure out how to inject padding into the response)
	    // check for a change request and validate we can do it. If so, set _socketOutput. If not, fill out _error and return.
	    // determine if we have an "other" address to notify the caller about


	    // did the request come with a padding request
	    if (reader.getPaddingAttributeSize(&paddingSize) == 0)
	    {
	        // todo - figure out how we're going to get the MTU size of the outgoing interface
	        requestHasPaddingAttribute = true;
	    }
    
	    // as per 5780, section 6.1, If the Request contained a PADDING attribute...
	    // "If the Request also contains the RESPONSE-PORT attribute the server MUST return an error response of type 400."
	    if (m_isRequestHasResponsePort && requestHasPaddingAttribute)
	    {
	        m_error.errorcode = STUN_ERROR_BADREQUEST;
	        return -1;
	    }
    
	    // handle change request logic and figure out what "other-address" attribute is going to be
	    // Some clients (like jstun) will send a change-request attribute with neither the IP or PORT flag set
	    // So ignore this block of code in that case (because the fConnectionOriented check below could fail)
	    hrResult = reader.getChangeRequest(&changerequest);
	    if (hrResult == 0 && (changerequest.fChangeIP || changerequest.fChangePort))
	    {
	        if (changerequest.fChangeIP)
	        {
	            socketOutput = SocketRoleSwapIP(socketOutput);
	        }
	        if(changerequest.fChangePort)
	        {
	            socketOutput = SocketRoleSwapPort(socketOutput);
	        }

	        // IsValidSocketRole just validates the enum, not whether or not we can send on it
	        ASSERT(IsValidSocketRole(socketOutput));

	        // now, make sure we have the ability to send from another socket
	        // For TCP/TLS, we can't send back from another port
	        if ((hasAddress(socketOutput) == false) || mp_msgIn->isConnectionOriented)
	        {
	            // send back an error. We're being asked to respond using another address that we don't have a socket for
	            m_error.errorcode = STUN_ERROR_BADREQUEST;
	            return -1;
	        }
	    }    
    
    	sendOtherAddress = hasAddress(RolePP) && hasAddress(RolePA) && hasAddress(RoleAP) && hasAddress(RoleAA);

	    if (sendOtherAddress)
	    {
	        socketOther = SocketRoleSwapIP(SocketRoleSwapPort(mp_msgIn->role));
	        // so if our ip address is "0.0.0.0", disable this attribute
	        sendOtherAddress = (isIPAddressZeroOrInvalid(socketOther) == false);
	        
	        // so if the local address of the other socket isn't known (e.g. ip == "0.0.0.0"), disable this attribute
	        if (sendOtherAddress)
	        {
	            addrOther = mp_addrSet->set[socketOther].addr;
	        }
	    }

	    
	    // What's our address origin?
	    addrOrigin = mp_addrSet->set[socketOutput].addr;
	    if (addrOrigin.getIP().isZero())
	    {
	        // Since we're sending back from the IP address we received on, we can just use the address the message came in on
	        // Otherwise, we don't actually know it
	        if (socketOutput == mp_msgIn->role)
	        {
	            addrOrigin = mp_msgIn->localAddress;
	        }
	    }
	    sendOriginAddress = (false == addrOrigin.getIP().isZero());

	    // Success - we're all clear to build the response
	    mp_msgOut->role = socketOutput;
	    
	    builder.addMessageType(StunMsgTypeBinding, StunMsgClassSuccessResponse);
	    builder.addTransactionID(m_transactionid);
    
	    // paranoia - just to be consistent with Vovida, send the attributes back in the same order it does
	    // I suspect there are clients out there that might be hardcoded to the ordering
	    
	    // MAPPED-ADDRESS
	    // SOURCE-ADDRESS (RESPONSE-ORIGIN)
	    // CHANGED-ADDRESS (OTHER-ADDRESS)
	    // XOR-MAPPED-ADDRESS (XOR-MAPPED_ADDRESS-OPTIONAL)
	    builder.addMappedAddress(mp_msgIn->remoteAddress);

	    if (sendOriginAddress)
	    {
	        builder.addResponseOriginAddress(addrOrigin); // pass true to send back SOURCE_ADDRESS, otherwise, pass false to send back RESPONSE-ORIGIN
	    }

	    if (sendOtherAddress)
	    {
	        builder.addOtherAddress(addrOther); // pass true to send back CHANGED-ADDRESS, otherwise, pass false to send back OTHER-ADDRESS
	    }

	    // send back the XOR-MAPPED-ADDRESS (encoded as an optional message for legacy clients)
	    builder.addXorMappedAddress(mp_msgIn->remoteAddress);
	    
	    
	    // finally - if we're supposed to have a message integrity attribute as a result of authorization, add it at the very end
	    if (m_integrity.fSendWithIntegrity)
	    {
	        if (m_integrity.fUseLongTerm == false)
	        {
	            builder.addMessageIntegrityShortTerm(m_integrity.szPassword);
	        }
	        else
	        {
	            builder.addMessageIntegrityLongTerm(m_integrity.szUser, m_integrity.szRealm, m_integrity.szPassword);
	        }
	    }

	    builder.addMessageLength();

   		return 0;
   	}
    
    void StunRequestHandler::buildErrorResponse()
    {

    }

    int StunRequestHandler::validateAuth()
    {
    	return 0;
    }

    int StunRequestHandler::processRequestImpl()
    {
    	StunMessageParser& parser = *(mp_msgIn->handler);
    	if (parser.getMessageClass() != StunMsgClassRequest)
    		return -1;

    	m_error.msgclass = StunMsgClassFailureResponse;
    	m_error.msgtype = parser.getMessageType();

    	parser.getTransactionId(&m_transactionid);
    	m_legacyMode = parser.isMessageLegacyFormat();
    	
    	uint16_t responseport;
    	parser.getResponsePort(&responseport);
    	if (responseport != 0)
    	{
    		m_isRequestHasResponsePort = true;
    		if (mp_msgIn->isConnectionOriented)
    		{
    			// special case for TCP - we can't do a response port for connection oriented sockets
	            // so just flag this request as an error
	            // todo - consider relaxing this check since the calling code is going to ignore the response address anyway for TCP
            	m_error.errorcode = STUN_ERROR_BADREQUEST;
	        }
	        else
	        {
	            mp_msgOut->destinationAddress.setPort(responseport);
	        }
    	}

    	if (m_error.errorcode == 0)
    	{
    		if (parser.getMessageType() !=  StunMsgTypeBinding)
    		{
    			m_error.errorcode = STUN_ERROR_BADREQUEST;
    		}
    	}

    	int hrResult = 0;
    	if (m_error.errorcode == 0)
	    {
	        hrResult = validateAuth(); // returns S_OK if _pAuth is NULL
	        
	        // if auth didn't succeed, but didn't set an error code, then setup a generic error response
	        if (hrResult == -1 && (m_error.errorcode == 0))
	        {
	            m_error.errorcode = STUN_ERROR_BADREQUEST;
	        }
	    }
	    
	    
	    if (m_error.errorcode == 0)
	    {
	        hrResult = processBindingRequest();
	        if (hrResult == -1 && (m_error.errorcode == 0))
	        {
	            m_error.errorcode = STUN_ERROR_BADREQUEST;
	        }
	    }
	    
	    if (m_error.errorcode != 0)
	    {
	        buildErrorResponse();
	    }

    	return 0;
    }

    bool StunRequestHandler::hasAddress(SocketRole role)
    {
    	return (mp_addrSet && IsValidSocketRole(role) && mp_addrSet->set[role].isValid);
	    
    }

    bool StunRequestHandler::isIPAddressZeroOrInvalid(SocketRole role)
    {
    	 bool isValid = hasAddress(role) && (mp_addrSet->set[role].addr.getIP().isZero() == false);
    	return !isValid;
    }

   	/*
   		utility functions
   	*/
   	int getMappedAddress(uint8_t* pData, size_t size, StunSocketAddress* pAddr)
	{
	    uint16_t port;
	    uint8_t attributeid;
	    uint8_t ip6[STUN_IPV6_LENGTH];
	    uint32_t ip4;

	    SharedDynamicBufferRef spBuffer(new DynamicBuffer(pData, size));
	    DataStream stream(spBuffer);

	    if (pAddr == NULL)
	    	return -1;
	    
	    if (stream.setAbsolutePosition(1) < 0)
	    	return -1;

	    if (stream.readUInt8(&attributeid) < 0)
	    	return -1;

	    if (stream.readUInt16(&port) < 0)
	    	return -1;

	    port = ntohs(port);

	    if (attributeid == STUN_ATTRIBUTE_FIELD_IPV4)
	    {
	        if (stream.readUInt32(&ip4) < 0)
	        	return -1;

	        ip4 = ntohl(ip4);
	        *pAddr = StunSocketAddress(ip4, port);
	    }
	    else
	    {
	        sockaddr_in6 addr6 = {};
	        if (stream.read(ip6, STUN_IPV6_LENGTH) < 0)
	        	return -1;
	        addr6.sin6_family = AF_INET6;
	        addr6.sin6_port = htons(port);
	        memcpy(&addr6.sin6_addr, ip6, STUN_IPV6_LENGTH);
	        *pAddr = StunSocketAddress(addr6);
	    }

		return 0;
	}

	int getXorMappedAddress(uint8_t* pData, size_t size, StunTransactionId &transid, StunSocketAddress* pAddr)
	{
	    if (getMappedAddress(pData, size, pAddr) < 0)
	    	return -1;

	    pAddr->applyXorMap(transid);

	    return 0;
	}
}


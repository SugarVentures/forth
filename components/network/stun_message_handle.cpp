#include "stun_message_handler.hpp"
#include "crc.hpp"

#if defined( __APPLE__ )
 #include <CommonCrypto/CommonHMAC.h> 
 #define HMAC(algorithm, key, keyLength, data, dataLength, macOut, outLen) CCHmac(algorithm, key, keyLength, data, dataLength, macOut)
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
    	int seq;

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
	        entropy ^= AtomicIncrement(&seq);
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
	    length = length-24;
	    
	    
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
}


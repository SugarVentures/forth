#include "stun_message_handler.hpp"

namespace oppvs {
    StunMessageHandler::StunMessageHandler(): m_transactionId()
    {

    }

    DataStream& StunMessageHandler::getDataStream()
    {
        return m_dataStream;
    }

    int StunMessageHandler::addMessageType(StunMessageType msgType, StunMessageClass msgClass)
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

    int StunMessageHandler::addTransactionID(const StunTransactionId& transactionid)
    {
    	m_transactionId = transactionid;
    	return m_dataStream.write((void*)transactionid.id, sizeof(transactionid.id));
    }

    int StunMessageHandler::addMessageLength()
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

    int StunMessageHandler::addAttributeHeader(uint16_t type, uint16_t size)
    {
    	int ret = 0;
    	ret = m_dataStream.writeUInt16(htons(type));
    	if (ret >= 0)
    		ret = m_dataStream.writeUInt16(htons(size));
    	return ret;
    }

    int StunMessageHandler::addMappedAddressCommon(uint16_t attribute, const StunSocketAddress& address)
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

    int StunMessageHandler::addMappedAddress(const StunSocketAddress& address)
    {
    	return addMappedAddressCommon(STUN_ATTRIBUTE_MAPPEDADDRESS, address);
    }
}

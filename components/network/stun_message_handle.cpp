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
}

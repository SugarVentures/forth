#ifndef OPPVS_SIGNALING_MESSAGE_READER_HPP
#define OPPVS_SIGNALING_MESSAGE_READER_HPP

#include "data_stream.hpp"
#include "signaling_common.hpp"

namespace oppvs {
	class SignalingMessageReader
	{
	public:
		SignalingMessageReader();
		~SignalingMessageReader();
		int addBytes(const uint8_t* pData, uint32_t size);
		void reset();
		DataStream& getStream();
	private:
		DataStream m_dataStream;
		SignalingMessageType m_messageType;

		int readHeader();
		int readBody();
		int readStringAttribute(uint16_t type, std::string& s);
		int readUInt16Attribute(uint16_t type, uint16_t* attr);
	};
} // oppvs

#endif // OPPVS_SIGNALING_MESSAGE_READER_HPP

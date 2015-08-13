#ifndef OPPVS_SIGNALING_MESSAGE_READER_HPP
#define OPPVS_SIGNALING_MESSAGE_READER_HPP

#include "data_stream.hpp"
#include "signaling_common.hpp"
#include "ice_common.hpp"
#include <vector>

namespace oppvs {
	class SignalingMessageReader
	{
	public:
		SignalingMessageReader();
		~SignalingMessageReader();
		int addBytes(const uint8_t* pData, uint32_t size);
		void reset();
		DataStream& getStream();
		std::vector<IceCandidate>& getIceCandidates();
	private:
		DataStream m_dataStream;
		SignalingMessageType m_messageType;
		std::vector<IceCandidate> m_candidates;

		int readHeader();
		int readBody();
		int readStringAttribute(uint16_t type, std::string& s);
		int readUInt16Attribute(uint16_t type, uint16_t* attr);
		int readUInt32Attribute(uint16_t type, uint32_t* attr);
		int readAttributeLength(uint16_t type, uint16_t* length);
	};
} // oppvs

#endif // OPPVS_SIGNALING_MESSAGE_READER_HPP

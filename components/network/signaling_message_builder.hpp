#ifndef OPPVS_SIGNALING_MESSAGE_BUILDER_HPP
#define OPPVS_SIGNALING_MESSAGE_BUILDER_HPP

#include "data_stream.hpp"
#include "signaling_common.hpp"
#include "ice_common.hpp"
#include "arpa/inet.h"
#include <vector>

namespace oppvs {
	class SignalingMessageBuilder
	{
	public:
		SignalingMessageBuilder();
		~SignalingMessageBuilder();

		void reset();
	
		DataStream& getDataStream();
		int getResult(SharedDynamicBufferRef& buffer);

		//Header part
		int addMessageType(SignalingMessageType type);
		int addStreamKey(const std::string& streamKey);
		int addMessageLength();

		//Body part
		int addIceUsername(const std::string& username);
		int addIcePassword(const std::string& password);
		int addIceCandidates(const std::vector<IceCandidate>& candidates);

		int addVideoSources(const VideoStreamInfo&);

	private:
		DataStream m_dataStream;

		int addStringAttribute(uint16_t type, std::string s);
		int addAttributeHeader(uint16_t type, uint16_t size);
		int addAttribute(uint16_t type, const void* data, uint16_t size);
	};
} // oppvs

#endif // OPPVS_SIGNALING_MESSAGE_BUILDER_HPP

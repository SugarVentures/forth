#ifndef OPPVS_SIGNALING_MESSAGE_BUILDER_HPP
#define OPPVS_SIGNALING_MESSAGE_BUILDER_HPP

#include "data_stream.hpp"
#include "signaling_common.hpp"

namespace oppvs {
	class SignalingMessageBuilder
	{
	public:
		SignalingMessageBuilder();
		~SignalingMessageBuilder();
	
		DataStream& getDataStream();
		int getResult(SharedDynamicBufferRef& buffer);

		//Header part
		int addMessageType(SignalingMessageType type);
		int addStreamKey(uint32_t streamKey);
		int addMessageLength();
	private:
		DataStream m_dataStream;
	};
} // oppvs

#endif // OPPVS_SIGNALING_MESSAGE_BUILDER_HPP

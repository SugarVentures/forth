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

		int addMessageType(SignalingMessageType type);
	private:
		uint32_t m_streamKey;
		DataStream m_dataStream;
	};
} // oppvs

#endif // OPPVS_SIGNALING_MESSAGE_BUILDER_HPP

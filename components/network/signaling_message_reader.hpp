#ifndef OPPVS_SIGNALING_MESSAGE_READER_HPP
#define OPPVS_SIGNALING_MESSAGE_READER_HPP

#include "data_stream.hpp"

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
	};
} // oppvs

#endif // OPPVS_SIGNALING_MESSAGE_READER_HPP

#include "data_stream.hpp"

namespace oppvs
{
	DataStream::DataStream() : p_buffer(NULL)
	{

	}

	uint8_t* DataStream::getUnSafeDataPointer()
	{
		if (!p_buffer)
			return NULL;
		return p_buffer->data();
	}

	SharedDynamicBufferRef DataStream::getBuffer()
	{
		return sp_buffer;
	}
}
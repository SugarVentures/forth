/*
	Data Stream
*/

#ifndef OPPVS_DATA_STREAM_HPP
#define OPPVS_DATA_STREAM_HPP

#include "dynamic_buffer.hpp"

namespace oppvs
{
	class DataStream
	{

	private:
		DynamicBuffer* p_buffer;	//Direct pointer to the buffer
		SharedDynamicBufferRef	sp_buffer;	//Thread safe pointer to the buffer
	public:
		DataStream();

		uint8_t* getUnSafeDataPointer();
		SharedDynamicBufferRef getBuffer();
	};
}

#endif
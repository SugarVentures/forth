/*
	Data Stream
*/

#ifndef OPPVS_DATA_STREAM_HPP
#define OPPVS_DATA_STREAM_HPP

#include "dynamic_buffer.hpp"
#include <mutex>
#include <cstring>

namespace oppvs
{
	class DataStream
	{

	private:
		DynamicBuffer* p_buffer;	//Direct pointer to the buffer
		SharedDynamicBufferRef	sp_buffer;	//Thread safe pointer to the buffer

		size_t m_currentPos;
		bool m_noGrow;	//Disable the buffer growing
		std::mutex m_io_mutex;

	public:
		DataStream();
		DataStream(SharedDynamicBufferRef&);

		void attach(SharedDynamicBufferRef& buf, bool allowWritting);
		void grow(size_t size);
		void reset();
		size_t size();
		size_t capacity();
		void disableGrowing(bool);
		size_t getPosition();
		int setAbsolutePosition(size_t pos);
		int setRelativePosition(int offset);

		int write(const void* data, size_t length);
		int read(void* data, size_t length);

		int writeUInt8(uint8_t value) { return write(&value, sizeof(value)); }
		int writeUInt16(uint16_t value) { return write(&value, sizeof(value)); }
		int writeUInt32(uint32_t value) { return write(&value, sizeof(value)); }

		int readUInt8(uint8_t* pvalue) { return read(pvalue, sizeof(*pvalue)); }
		int readUInt16(uint16_t* pvalue) { return read(pvalue, sizeof(*pvalue)); }
		int readUInt32(uint32_t* pvalue) { return read(pvalue, sizeof(*pvalue)); }

		uint8_t* getUnSafeDataPointer();
		SharedDynamicBufferRef getBuffer();
	};
}

#endif
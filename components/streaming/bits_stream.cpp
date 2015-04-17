#include "bits_stream.hpp"

namespace oppvs
{
	BitsStream::BitsStream() : m_data(NULL), m_length(0)
	{

	}

	BitsStream::BitsStream(uint8_t* data, uint32_t length) : m_data(NULL), m_length(0)
	{
		if (data == NULL)
			return;
		m_data = new uint8_t[length];
		m_length = length;
	}

	BitsStream::~BitsStream()
	{
		if (m_data != NULL)
			delete [] m_data;
		m_length = 0;
		m_data = NULL;
	}

}
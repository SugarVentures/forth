#include "signaling_message_reader.hpp"

namespace oppvs {
	SignalingMessageReader::SignalingMessageReader()
	{
		reset();	
	}

	SignalingMessageReader::~SignalingMessageReader()
	{
			
	}

	void SignalingMessageReader::reset()
	{
		m_dataStream.reset();
	}

	DataStream& SignalingMessageReader::getStream()
	{
		return m_dataStream;
	}

	int SignalingMessageReader::addBytes(const uint8_t* pData, uint32_t size)
	{
		// seek to the end of the stream
	    m_dataStream.setAbsolutePosition(m_dataStream.size());

	    if (m_dataStream.write(pData, size) < 0)
	    {
	        return -1;
	    }

	    size_t currentSize = m_dataStream.size();
	    std::cout << "Read " << currentSize << " bytes" << std::endl;
		return 0;
	}
} // oppvs
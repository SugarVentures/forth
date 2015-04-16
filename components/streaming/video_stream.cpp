#include "video_stream.hpp"

namespace oppvs
{
	VideoStream::VideoStream(uint16_t width, uint16_t height) : m_width(width), m_height(height)
	{
		
	}

	int VideoStream::addSource()
	{
		printf("Add source\n");
		return 0;
	}
}
/*
	Video Stream
*/

#ifndef OPPVS_VIDEO_STREAM_HPP
#define OPPVS_VIDEO_STREAM_HPP

#include "datatypes.hpp"

namespace oppvs
{
	class VideoStream
	{
	public:
		VideoStream() : m_width(0), m_height(0), m_stride(0) {}
		VideoStream(uint16_t width, uint16_t height);
		virtual ~VideoStream() {}
		int addSource();

		uint16_t getWidth() { return m_width; }
		void setWidth(uint16_t value) { m_width = value; }

		uint16_t getHeight() { return m_height; }
		void setHeight(uint16_t value) { m_height = value; }

		uint16_t getStride() { return m_stride; }
		void setStride(uint16_t value) { m_stride = value; }
	private:
		uint16_t m_width;	//The width of the video frame
		uint16_t m_height;	//The height of the video frame
		uint16_t m_stride;	//The bytes per row of the video frame

	};
}

#endif
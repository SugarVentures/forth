#ifndef OPPVS_VIDEO_ENCODING_HPP
#define OPPVS_VIDEO_ENCODING_HPP

#include "datatypes.hpp"
#include "libyuv.h"

namespace oppvs
{
	class VideoFrameEncoding
	{
	public:
		VideoFrameEncoding() {}
		virtual ~VideoFrameEncoding() {}

		int convertBGRAToI420(PixelBuffer& pf, uint8_t** data, uint32_t* length);
		int convertI420ToBGRA(uint8_t* data, uint32_t* length, PixelBuffer& pf);
	private:

	};
}

#endif
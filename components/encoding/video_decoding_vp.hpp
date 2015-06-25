#ifndef OPPVS_VIDEO_DECODING_VP_HPP
#define OPPVS_VIDEO_DECODING_VP_HPP

#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"
#include "error.hpp"
#include "datatypes.hpp"
#include "libyuv.h"

namespace oppvs
{
	struct DecodingController
	{
		uint8_t source;
		bool state;
		vpx_codec_ctx_t codec;
	};

	class VPVideoDecoder
	{
	public:
		int init();
		int init(VideoStreamInfo&);
		int decode(PixelBuffer& pf, uint32_t length, uint8_t* frame);	//Frame in YUV12 or I420 format

		int release();
	private:
		vpx_codec_ctx_t m_codec;
		uint8_t m_numSources;
		DecodingController m_controllers[OPPVS_MAX_CAPTURE_SOURCES];

		int updateImage(PixelBuffer& pf, vpx_image_t *img);
	};
}

#endif
#ifndef OPPVS_VIDEO_DECODING_VP_HPP
#define OPPVS_VIDEO_DECODING_VP_HPP

#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"
#include "error.hpp"
#include "datatypes.hpp"
#include "libyuv.h"

namespace oppvs
{

	class VPVideoDecoding
	{
	public:
		int init(int width, int height);
		int decode(PixelBuffer& pf, uint32_t length, uint8_t* frame);	//Frame in YUV12 or I420 format

		int release();
	private:
		vpx_codec_ctx_t m_codec;
		vpx_codec_dec_cfg_t m_configuration;

		int updateImage(PixelBuffer& pf, vpx_image_t *img);
	};
}

#endif
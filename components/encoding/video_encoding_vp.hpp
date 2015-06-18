#ifndef OPPVS_VIDEO_ENCODING_VP_HPP
#define OPPVS_VIDEO_ENCODING_VP_HPP

#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"
#include "error.hpp"
#include "datatypes.hpp"

namespace oppvs
{

	class VPVideoEncoding
	{
	public:
		int init(int width, int height);
		int encode(uint8_t* frame, uint32_t length, uint8_t** encoded_frame);	//Frame in YUV12 or I420 format
		int release();
	private:
		vpx_codec_ctx_t m_codec;
		vpx_codec_enc_cfg_t m_configuration;
		vpx_image_t m_image;
	};
}

#endif
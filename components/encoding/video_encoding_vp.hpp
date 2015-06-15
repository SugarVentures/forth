#ifndef OPPVS_VIDEO_ENCODING_VP_HPP
#define OPPVS_VIDEO_ENCODING_VP_HPP

#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"

namespace oppvs
{
	class VPVideoEncoding
	{
	public:
		int init();
		
	private:
		vpx_codec_ctx_t m_codec;
		vpx_codec_enc_cfg_t m_configuration;
	};
}

#endif
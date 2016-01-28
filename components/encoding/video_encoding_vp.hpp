#ifndef OPPVS_VIDEO_ENCODING_VP_HPP
#define OPPVS_VIDEO_ENCODING_VP_HPP

#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"
#include "error.hpp"
#include "datatypes.hpp"
#include "libyuv.h"


namespace oppvs
{
	struct EncodingController
	{
		uint8_t source;
		bool state;
		vpx_codec_ctx_t codec;
		vpx_codec_enc_cfg_t config;
		vpx_image_t image;
		uint32_t frameIndex;
		int picID;
	};

	class VPVideoEncoder
	{
	public:
		int init(VideoStreamInfo&);
		int init(int width, int height);
		int encode(PixelBuffer& pf, uint32_t *length, uint8_t** encoded_frame, int* picID, bool *isKey, uint32_t ts);	//Frame in YUV12 or I420 format

		int release();
	private:
		vpx_codec_ctx_t m_codec;
		vpx_codec_enc_cfg_t m_configuration;
		vpx_image_t m_image;

		uint8_t m_numSources;
		EncodingController m_controllers[OPPVS_MAX_CAPTURE_SOURCES];

		int updateImage(PixelBuffer& pf, vpx_image_t *img);
	};
}

#endif
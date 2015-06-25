#include "video_encoding_vp.hpp"
#include <stdio.h>
#include <string.h>

namespace oppvs
{
	int VPVideoEncoder::init(int width, int height)
	{
		vpx_codec_err_t res;
		vpx_codec_iface_t *(*const codec_interface)() = &vpx_codec_vp8_cx;

		if (!vpx_img_alloc(&m_image, VPX_IMG_FMT_I420, width, height, 1)) {
    		printf("Failed to allocate image.\n");
    		return ERRS_ENCODING_INIT_FAILED;
 		}

 		printf("Using %s\n", vpx_codec_iface_name(codec_interface()));

	 	res = vpx_codec_enc_config_default(codec_interface(), &m_configuration, 0);
	 	if (res)
	 	{
	 		printf("Failed to get default codec config.\n");
	 		return ERRS_ENCODING_INIT_FAILED;
	 	}

	 	m_configuration.g_w = width;
	 	m_configuration.g_h = height;
	 	m_configuration.rc_min_quantizer = 4;
	 	m_configuration.rc_max_quantizer = 56;
	 	m_configuration.g_timebase.num = 1;
	 	m_configuration.g_timebase.den = 30;
	 	m_configuration.rc_target_bitrate = 500;
	 	m_configuration.g_threads = 4;

	 	if (vpx_codec_enc_init(&m_codec, codec_interface(), &m_configuration, 0))
	 	{
	 		printf("Failed to initialize encoder\n");
	 		return ERRS_ENCODING_INIT_FAILED;
	 	}
	 	
		return ERRS_ENCODING_OK;
	}

	int VPVideoEncoder::init(VideoStreamInfo& info)
	{
		vpx_codec_err_t res;
		vpx_codec_iface_t *(*const codec_interface)() = &vpx_codec_vp8_cx;
		m_numSources = info.noSources;

		for (int i = 0; i < m_numSources; i++)
		{
			if (!vpx_img_alloc(&m_controllers[i].image, VPX_IMG_FMT_I420, info.sources[i].width, info.sources[i].height, 1)) {
	    		printf("Failed to allocate image.\n");
	    		m_controllers[i].state = false;
	    		continue;
 			}
 			res = vpx_codec_enc_config_default(codec_interface(), &m_controllers[i].config, 0);
 			if (res)
 			{
 				printf("Failed to get default codec config\n");
 				m_controllers[i].state = false;
 				continue;
 			}
 			m_controllers[i].config.g_w = info.sources[i].width;
 			m_controllers[i].config.g_h = info.sources[i].height;
 			m_controllers[i].config.rc_min_quantizer = 4;
			m_controllers[i].config.rc_max_quantizer = 56;
			m_controllers[i].config.g_timebase.num = 1;
			m_controllers[i].config.g_timebase.den = 30;
			m_controllers[i].config.rc_target_bitrate = 500;
			m_controllers[i].config.g_threads = 2;


 			if (vpx_codec_enc_init(&m_controllers[i].codec, codec_interface(), &m_controllers[i].config, 0))
		 	{
		 		printf("Failed to initialize encoder\n");
		 		m_controllers[i].state = false;
		 		continue;
		 	}
		 	m_controllers[i].source = info.sources[i].source;
		 	m_controllers[i].state = true;
		 	m_controllers[i].frameIndex = 0;
		 	m_controllers[i].picID = rand() % 128;
		}

		return ERRS_ENCODING_OK;
	}

	int VPVideoEncoder::encode(PixelBuffer& pf, uint32_t* length, uint8_t** encoded_frame, int* picID, bool* isKey)
	{
		uint32_t frameIndex = 0;
	 	int flags = 0;
	 	vpx_codec_iter_t iter = NULL;
	  	const vpx_codec_cx_pkt_t *pkt = NULL;
		vpx_codec_ctx_t *codec = NULL;
		vpx_image_t *image = NULL;
		for (int i = 0; i < m_numSources; i++)
		{
			if (m_controllers[i].source == pf.source && m_controllers[i].state)
			{
				codec = &m_controllers[i].codec;
				image = &m_controllers[i].image;
				frameIndex = m_controllers[i].frameIndex++;
				*picID = m_controllers[i].picID;
				m_controllers[i].picID = (m_controllers[i].picID + 1) % 128;
			}
		}

		/*codec = &m_codec;
		image = &m_image;*/

		if (!codec)
		{
			printf("No codec availble for this frame\n");
			return -1;
		}
		if (updateImage(pf, image) < 0)
		{
			printf("Cannot convert to I420\n");
			return -1;	  	
		}

	  	const vpx_codec_err_t res = vpx_codec_encode(codec, image, frameIndex, 1,
	                                               flags, VPX_DL_REALTIME);
	  
	  	if (res != VPX_CODEC_OK)
	    	printf("Failed to encode frame\n");

	    while ((pkt = vpx_codec_get_cx_data(codec, &iter)) != NULL) {
	    	switch (pkt->kind) {
				case VPX_CODEC_CX_FRAME_PKT:
					*length = pkt->data.frame.sz;
					*encoded_frame = static_cast<uint8_t*>(pkt->data.frame.buf);
					*isKey = pkt->data.frame.flags & VPX_FRAME_IS_KEY;
					//printf("Out length: %d\n", *length);
					//printHashCode(*encoded_frame, *length);
					break;
				default:
					break;
			}
	    }
	    
		return 0;
	}

	int VPVideoEncoder::updateImage(PixelBuffer& pf, vpx_image_t *img)
	{
		uint16_t frame_width = pf.width[0];
		uint16_t frame_height = pf.height[0];

		int result = libyuv::ARGBToI420((const uint8_t*)pf.plane[0], pf.stride[0],
		      img->planes[0], img->stride[0],
		      img->planes[1], img->stride[1],
		      img->planes[2], img->stride[2],
		      frame_width, frame_height);

		img->w = img->d_w = frame_width;
		img->h = img->d_h = frame_height;

		img->x_chroma_shift = img->y_chroma_shift = 1;
		img->planes[3] = NULL;

		//printf("%d %d %d %d\n", m_image.w, m_image.d_w, m_image.stride[1], m_image.stride[2]);

		return result;
	}

	int VPVideoEncoder::release()
	{
		for (int i = 0; i < m_numSources; i++)
		{
			vpx_img_free(&m_controllers[i].image);
			vpx_codec_destroy(&m_controllers[i].codec);
		}

		vpx_img_free(&m_image);
 	
 		if (vpx_codec_destroy(&m_codec))
    		printf("Failed to destroy codec.\n");

		return 0;
	}
}
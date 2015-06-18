#include "video_encoding_vp.hpp"
#include <stdio.h>

namespace oppvs
{
	int VPVideoEncoding::init(int width, int height)
	{
		vpx_codec_err_t res;
		vpx_codec_iface_t *(*const codec_interface)() = &vpx_codec_vp9_cx;

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

	 	if (vpx_codec_enc_init(&m_codec, codec_interface(), &m_configuration, 0))
	 	{
	 		printf("Failed to initialize encoder\n");
	 		return ERRS_ENCODING_INIT_FAILED;
	 	}
	 	
		return ERRS_ENCODING_OK;
	}

	int VPVideoEncoding::encode(uint8_t* frame, uint32_t length, uint8_t** encoded_frame)
	{
		int frame_index = 1;
	 	int flags = 0;
	 	vpx_codec_iter_t iter = NULL;
	  	const vpx_codec_cx_pkt_t *pkt = NULL;

	  	memcpy(m_image.planes[0], frame, length);
	  	const vpx_codec_err_t res = vpx_codec_encode(&m_codec, &m_image, frame_index, 1,
	                                               flags, VPX_DL_GOOD_QUALITY);
	  
	  	if (res != VPX_CODEC_OK)
	    	printf("Failed to encode frame\n");

	    int got_pkts = 0;
	    while ((pkt = vpx_codec_get_cx_data(&m_codec, &iter)) != NULL) {
	    	got_pkts = 1;
	    	printf("Has packet\n");
	    }
		return 0;
	}

	int VPVideoEncoding::release()
	{
		vpx_img_free(&m_image);
 	
 		if (vpx_codec_destroy(&m_codec))
    		printf("Failed to destroy codec.\n");

		return 0;
	}
}
#include "video_encoding_vp.hpp"
#include <stdio.h>
#include <string.h>

namespace oppvs
{
	int VPVideoEncoding::init(int width, int height)
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

	 	if (vpx_codec_enc_init(&m_codec, codec_interface(), &m_configuration, 0))
	 	{
	 		printf("Failed to initialize encoder\n");
	 		return ERRS_ENCODING_INIT_FAILED;
	 	}
	 	
		return ERRS_ENCODING_OK;
	}

	int VPVideoEncoding::encode(PixelBuffer& pf, uint32_t* length, uint8_t** encoded_frame)
	{
		int frame_index = 1;
	 	int flags = 0;
	 	vpx_codec_iter_t iter = NULL;
	  	const vpx_codec_cx_pkt_t *pkt = NULL;

		if (updateImage(pf) < 0)
			return -1;	  	


	  	const vpx_codec_err_t res = vpx_codec_encode(&m_codec, &m_image, frame_index, 1,
	                                               flags, VPX_DL_GOOD_QUALITY);
	  
	  	if (res != VPX_CODEC_OK)
	    	printf("Failed to encode frame\n");

	    while ((pkt = vpx_codec_get_cx_data(&m_codec, &iter)) != NULL) {
	    	switch (pkt->kind) {
				case VPX_CODEC_CX_FRAME_PKT:
					*length = pkt->data.frame.sz;
					*encoded_frame = static_cast<uint8_t*>(pkt->data.frame.buf);
					printf("encoded\n");
					break;
				default:
					break;
			}
	    }
		return 0;
	}

	int VPVideoEncoding::updateImage(PixelBuffer& pf)
	{
		uint16_t frame_width = pf.width[0];
		uint16_t frame_height = pf.height[0];

		int result = libyuv::ARGBToI420((const uint8_t*)pf.plane[0], frame_width*4,
		      m_image.planes[0], m_image.stride[0],
		      m_image.planes[1], m_image.stride[1],
		      m_image.planes[2], m_image.stride[2],
		      frame_width, frame_height);

		m_image.w = m_image.d_w = frame_width;
		m_image.h = m_image.d_h = frame_height;

		m_image.x_chroma_shift = m_image.y_chroma_shift = 1;
		m_image.planes[3] = NULL;

		//printf("%d %d %d %d\n", m_image.w, m_image.d_w, m_image.stride[1], m_image.stride[2]);

		return result;
	}

	int VPVideoEncoding::release()
	{
		vpx_img_free(&m_image);
 	
 		if (vpx_codec_destroy(&m_codec))
    		printf("Failed to destroy codec.\n");

		return 0;
	}
}
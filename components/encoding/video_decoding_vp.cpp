#include "video_decoding_vp.hpp"
#include <stdio.h>
#include <string.h>

namespace oppvs
{
	int VPVideoDecoding::init()
	{
		vpx_codec_iface_t *(*const codec_interface)() = &vpx_codec_vp8_dx;

 		if (vpx_codec_dec_init(&m_codec, codec_interface(), NULL, 0))
 			printf("Init failed\n");
	 	
		return ERRS_DECODING_OK;
	}

	int VPVideoDecoding::decode(PixelBuffer& pf, uint32_t length, uint8_t* frame)
	{
		vpx_codec_iter_t iter = NULL;
		vpx_image_t *img = NULL;
		int frame_cnt = 0;
		int error = 0;
		if (vpx_codec_decode(&m_codec, frame, (unsigned int)length, NULL, 0))
		{
    		printf("Failed to decode frame.\n");
    		return -1;
    	}
	    while ((img = vpx_codec_get_frame(&m_codec, &iter)) != NULL) {
	    	int corrupted = 0;
	    	vpx_codec_control(&m_codec, VP8D_GET_FRAME_CORRUPTED, &corrupted);
			if (corrupted) {
				printf("corrupted\n");
				error = -1;
				continue;
			}

			//printf("%d %d %d %d\n", img->w, img->d_w, img->stride[1], img->stride[2]);
	      	if (updateImage(pf, img) < 0)
	      	{
	      		return -1;
	      	}
	     	++frame_cnt;
	    }
	    return error;
	}

	int VPVideoDecoding::updateImage(PixelBuffer& pf, vpx_image_t* img)
	{

		pf.plane[0] = new uint8[pf.stride[0]*pf.height[0]];
		int result = libyuv::I420ToARGB(img->planes[0], img->stride[0],
	               img->planes[1], img->stride[1],
	               img->planes[2], img->stride[2],
	               pf.plane[0], pf.stride[0],
	               pf.width[0], pf.height[0]);
		pf.nbytes = pf.stride[0] * pf.height[0];
		return result;
	}

	int VPVideoDecoding::release()
	{
 	
 		if (vpx_codec_destroy(&m_codec))
    		printf("Failed to destroy codec.\n");

		return 0;
	}
}
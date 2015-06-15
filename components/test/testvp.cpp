
#include <stdio.h>
#include <iostream>
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"


int main(int argc, char* argv[])
{
	vpx_codec_ctx_t codec;
	vpx_codec_enc_cfg_t cfg;
	vpx_codec_err_t res;

	vpx_codec_iface_t *(*const codec_interface)() = &vpx_codec_vp9_cx;

	vpx_image_t raw;
	int frame_width = 1280;
	int frame_height = 780;

	if (!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, frame_width,
                                             frame_height, 1)) {
    	std::cout << "Failed to allocate image.\n";
 	}

 	printf("Using %s\n", vpx_codec_iface_name(codec_interface()));

 	res = vpx_codec_enc_config_default(codec_interface(), &cfg, 0);
 	if (res)
 	{
 		std::cout << "Failed to get default codec config.\n";
 	}

 	cfg.g_w = frame_width;
 	cfg.g_h = frame_height;

 	if (vpx_codec_enc_init(&codec, codec_interface(), &cfg, 0))
 	{
 		std::cout << "Failed to initialize encoder\n";
 	}
 	else
 	{
 		std::cout << "Init encoder Successfully\n";
 	}
 	//Encode
 	int frame_index = 1;
 	int flags;
 	vpx_codec_iter_t iter = NULL;
  	const vpx_codec_cx_pkt_t *pkt = NULL;
  	const vpx_codec_err_t codec_res = vpx_codec_encode(&codec, &raw, frame_index, 1,
                                               flags, VPX_DL_GOOD_QUALITY);
  
  	if (codec_res != VPX_CODEC_OK)
    	std::cout << "Failed to encode frame\n";

    int got_pkts = 0;
    while ((pkt = vpx_codec_get_cx_data(&codec, &iter)) != NULL) {
    	got_pkts = 1;
    	std::cout << "Has packet\n";
    }

 	//Release
 	vpx_img_free(&raw);
 	
 	if (vpx_codec_destroy(&codec))
    	std::cout << "Failed to destroy codec.";


	return 0;
}
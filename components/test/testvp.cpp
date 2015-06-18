
#include <stdio.h>
#include <iostream>
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"
#include "vpx/vpx_codec.h"

#include "datatypes.hpp"
#include "libyuv.h"

int main(int argc, char* argv[])
{
	vpx_codec_ctx_t codec;
	vpx_codec_enc_cfg_t cfg;
	vpx_codec_err_t res;

	vpx_codec_iface_t *(*const codec_interface)() = &vpx_codec_vp9_cx;

	vpx_image_t raw;
	int frame_width = 180;
	int frame_height = 180;

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
  uint8_t rgb24pp[frame_width][frame_height][4];
  for (int rowCt = 0; rowCt < frame_width; ++rowCt)
    for (int colCt = 0; colCt < frame_height; ++colCt)
    {
      rgb24pp[rowCt][colCt][0] = 0x80;  // red, I think
      rgb24pp[rowCt][colCt][1] = 0xC0;  // green (?)
      rgb24pp[rowCt][colCt][2] = 0xFF;  // blue, I hope
      rgb24pp[rowCt][colCt][3] = 1;
    }

  const uint32_t size_i420 = frame_width * frame_height * 3 / 2;
  const uint32_t y_length = frame_width * frame_height;
  const uint32_t uv_stride = frame_width / 2;
  const uint32_t uv_length = uv_stride * (frame_height / 2);
  
  uint8_t* const dst_y = new uint8_t[size_i420];
  uint8_t* const dst_u = dst_y + y_length;
  uint8_t* const dst_v = dst_u + uv_length;

  printf("I420 frame: %u %u %u %u\n", size_i420, y_length, uv_stride, uv_length);

  /*int result = libyuv::ARGBToI420((const uint8_t*)rgb24pp, frame_width*4,
          dst_y, frame_width,
          dst_u, uv_stride,
          dst_v, uv_stride,
          frame_width, frame_height);*/
  

  raw.w = raw.d_w = frame_width;
  raw.h = raw.d_h = frame_height;

  raw.x_chroma_shift = raw.y_chroma_shift = 0;
  raw.planes[0] = dst_y;
  raw.planes[1] = dst_u;
  raw.planes[2] = dst_v;
  raw.planes[3] = NULL;

  raw.stride[0] = frame_width;
  raw.stride[1] = uv_stride;
  raw.stride[2] = uv_stride;
  raw.stride[3] = 0;
  raw.bps = 24;

 	int frame_index = 1;
 	int flags = 0;
 	vpx_codec_iter_t iter = NULL;
  	const vpx_codec_cx_pkt_t *pkt = NULL;
  
  vpx_codec_err_t testres = validate_img(&codec, &raw);
  	/*const vpx_codec_err_t codec_res = vpx_codec_encode(&codec, &raw, frame_index, 1,
                                               flags, 0);
  

  	if (codec_res != VPX_CODEC_OK)
    	std::cout << "Failed to encode frame\n";
    std::cout <<  vpx_codec_err_to_string(codec_res) << "\n";
    int got_pkts = 0;
    while ((pkt = vpx_codec_get_cx_data(&codec, &iter)) != NULL) {
    	got_pkts = 1;
    	std::cout << "Has packet\n";
    }*/

 	//Release
 	vpx_img_free(&raw);
 	
 	if (vpx_codec_destroy(&codec))
    	std::cout << "Failed to destroy codec.";

    
	return 0;
}
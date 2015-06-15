#include "video_encoding.hpp"

namespace oppvs
{
	int VideoFrameEncoding::convertBGRAToI420(PixelBuffer& pf, uint8_t** data, uint32_t* length)
	{
		const uint32_t size_i420 = pf.width[0] * pf.height[0] * 3 / 2;
		const uint32_t y_length = pf.width[0] * pf.height[0];
		const uint32_t uv_stride = pf.width[0] / 2;
		const uint32_t uv_length = uv_stride * (pf.height[0] / 2);
	
		uint8_t* const dst_y = new uint8[size_i420];
	    uint8_t* const dst_u = dst_y + y_length;
	    uint8_t* const dst_v = dst_u + uv_length;

	    //printf("I420 frame: %u %u %u %u\n", size_i420, y_length, uv_stride, uv_length);

	    int result = libyuv::ARGBToI420(pf.plane[0], pf.stride[0],
	            dst_y, pf.width[0],
	            dst_u, uv_stride,
	            dst_v, uv_stride,
	            pf.width[0], pf.height[0]);
	    
	    *data = dst_y;
	    *length = size_i420;

		return result;
	}

	int VideoFrameEncoding::convertI420ToBGRA(uint8_t* data, PixelBuffer& pf)
	{
		const uint32_t y_length = pf.width[0] * pf.height[0];
		const uint32_t uv_stride = pf.width[0] / 2;
		const uint32_t uv_length = uv_stride * (pf.height[0] / 2);
	
		uint8_t* const dst_y = data;
	    uint8_t* const dst_u = dst_y + y_length;
	    uint8_t* const dst_v = dst_u + uv_length;

	    //printf("I420 frame: %u %u %u\n", y_length, uv_stride, uv_length);

	    pf.plane[0] = new uint8[pf.nbytes];
	    int result = libyuv::I420ToARGB(dst_y, pf.width[0],
	               dst_u, uv_stride,
	               dst_v, uv_stride,
	               pf.plane[0], pf.stride[0],
	               pf.width[0], pf.height[0]);


		return result;
	}

}
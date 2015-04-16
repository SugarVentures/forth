/*
	Define data types
*/

#ifndef OPPVS_DATA_TYPES_HPP
#define OPPVS_DATA_TYPES_HPP


#include <inttypes.h> 
//#include "brg_types.h"

#include <stdio.h>
#include <errno.h>

#define OPPVS_IDLE_TIME	500
#define OPPVS_DEFAULT_SERVER_LISTEN_PORT 33432
#define OPPVS_NETWORK_PACKET_LENGTH 1024

namespace oppvs {
	typedef enum PixelFormat {
		PF_UYVY422 = 1,		/* Cb Y0 Cr Y1 */
		PF_YUYV422 = 2,		/* Y0 Cb Y1 Cr */
		PF_YUV422P = 3,		/* YUV422 Planar */
		PF_YUV420P = 4,		/* YUV420 Planar */
		PF_YUV420BP = 5,    /* YUV420 Bi Planar */
		PF_YUVJ420P = 6,	/* YUV420 Planar Full Range (JPEG), J comes from the JPEG. (values 0-255 used) */
		PF_YUVJ420BP = 7,   /* YUV420 Bi-Planer Full Range (JPEG), J comes fro the JPEG. (values: luma = [16,235], chroma=[16,240]) */
		PF_ARGB32 = 8,		/* ARGB 8:8:8:8 32bpp, ARGBARGBARGB... */
		PF_BGRA32 = 9,		/* BGRA 8:8:8:8 32bpp, BGRABGRABGRA... */
		PF_RGBA32 = 10,		/* RGBA 8:8:8:8 32bpp. */
		PF_RGB24 = 11,		/* RGB 8:8:8 24bit */
		PF_JPEG_OPENDML = 12,/* JPEG with Open-DML extensions */
		PF_H264 = 13,		/* H264 */
		PF_MJPEG = 14
	} pixel_format_t;

	struct ControllerLinker {
		void* render;
		void* streamer;
		void* origin;
	};

	class PixelBuffer {
	public:
		PixelBuffer()
		{
			reset();
			user = NULL;
		}
		~PixelBuffer() {}

		void reset() {
			pixels = NULL;
			nbytes = 0;
			stride[0] = 0;
		    stride[1] = 0;
	    	stride[2] = 0;
	    	plane[0] = NULL;
	    	plane[1] = NULL;
	    	plane[2] = NULL;
	    	width[0] = 0;
	    	width[1] = 0;
	    	width[2] = 0;
	    	height[0] = 0;
	    	height[1] = 0;
	    	height[2] = 0;
	    	offset[0] = 0;
	    	offset[1] = 0;
	    	offset[2] = 0;
	    	
		}
		
		//int setup(uint16_t width, uint16_t height, pixel_format_t format);

		uint8_t* pixels;	/* When data is one continuous block of member you can use this, otherwise it points to the same location as plane[0]. */
    	uint8_t* plane[3];  /* Pointers to the pixel data; when we're a planar format all members are set, if packets only plane[0] */
    	uint16_t stride[3];
    	uint16_t width[3];
    	uint16_t height[3];
    	size_t offset[3];	/* When the data is planar but packed, these contains the byte offsets from the first byte / plane. e.g. you can use this with YUV420P. */ 
    	size_t nbytes;
    	pixel_format_t format;
    	uint8_t flip;	//For different capture sources, the frame can be fliped when rendering
    	void* user;
	};

	typedef void (*frame_callback)(PixelBuffer& pf);	//Pointer to function that handles pixel buffer
}

#endif

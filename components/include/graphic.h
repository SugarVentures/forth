#ifndef OPPVS_GRAPHIC_H
#define OPPVS_GRAPHIC_H


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


#endif // OPPVS_GRAPHIC_H

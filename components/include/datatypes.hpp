/*
	Define data types
*/

#ifndef OPPVS_DATA_TYPES_HPP
#define OPPVS_DATA_TYPES_HPP


#include <inttypes.h> 
//#include "brg_types.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#if defined( __APPLE__ )
#include <string>
#else
#include <cstring>
#endif

#define OPPVS_IDLE_TIME	500
#define OPPVS_DEFAULT_SERVER_LISTEN_PORT 33432
#define OPPVS_NETWORK_PACKET_LENGTH 1024
#define OPPVS_MAX_CAPTURE_SOURCES 5
#define OPPVS_NETWORK_MAX_WAIT_TIME 5

/* 120ms at 48000 */
#define MAX_FRAME_SIZE (960*6)

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
	    	order = 0;
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
    	uint16_t originx;
    	uint16_t originy;
    	uint8_t source;
    	uint8_t order;
    	void* user;

    	uint32_t timestamp;
	};

	//Based on AudioBuffer Struct of Apple Core Audio
	struct GenericAudioBuffer
	{
		uint32_t numberChannels;	//Number of interleaved channels in the buffer
		uint32_t dataLength;		//Number of bytes pointed at by data;
		void* data;

		GenericAudioBuffer()
		{
			numberChannels = 0;
			dataLength = 0;
			data = NULL;
		}

	};

	class GenericAudioBufferList {
	public:
		uint32_t nBuffers;	//1 channel - 1 buffer
		GenericAudioBuffer* buffers;
		uint32_t nFrames;
		uint8_t source;
		double sampleTime;
		void* user;

		GenericAudioBufferList(): nBuffers(0), buffers(NULL)
		{
			reset();
		}

		~GenericAudioBufferList()
		{
			reset();
		}

		void reset() {
			if (nBuffers > 0 && buffers != NULL)
			{
				for (unsigned i = 0; i < nBuffers; ++i) {
					if (buffers[i].data)
					{
						//data is pointed to the local audio buffer, then no release here
						buffers[i].data = NULL;
					}
				}
				delete [] buffers;
			}
			buffers = NULL;
			nBuffers = 0;
		}
	};

	typedef void (*frame_callback)(PixelBuffer& pf);	//Pointer to function that handles pixel buffer
	typedef void (*audio_callback)(GenericAudioBufferList& ab);	//Pointer to function that handles audio buffer

	const int DEFAULT_VIDEO_FRAME_WIDTH = 1280;
	const int DEFAULT_VIDEO_FRAME_HEIGHT = 	720;
	const int ST_VIDEO_STREAMING = 1;

	enum MEDIA_CODEC
	{
		MEDIA_CODEC_VIDEO_FRAME_VP8 = 1,
		MEDIA_CODEC_AUDIO_OPUS = 2
	};

	struct VideoSourceInfo
	{
		uint8_t source;
		uint8_t order;
		uint16_t width;
		uint16_t height;
		uint16_t stride;
		uint16_t originx;
		uint16_t originy;

		inline int size()
		{
			return sizeof(uint8_t)
			+ sizeof(uint8_t)
			+ sizeof(uint16_t)
			+ sizeof(uint16_t)
			+ sizeof(uint16_t);
		}
	};

	struct VideoStreamInfo
	{
		uint8_t noSources;
		VideoSourceInfo *sources;

		VideoStreamInfo()
		{
			noSources = 0;
		}
	};

	const uint8_t AUDIO_TYPE_VOIP = 1;	//Voice signals
	const uint8_t AUDIO_TYPE_MIXED = 2;	//Mixed music/voice

	const uint8_t AUDIO_FORMAT_FLOAT = 1;
	const uint8_t AUDIO_FORMAT_INT = 2;

	const uint16_t AUDIO_MAX_ENCODING_PACKET_SIZE = 4000;
	const uint16_t AUDIO_ENCODING_FRAMES = 960;

	struct AudioSourceInfo
	{
		uint8_t source;
		uint8_t type;
		uint8_t format;	//Float or Int
		uint16_t numberChannels;
		uint32_t samplePerChannels;
		uint32_t sampleRate;
	};

	struct AudioStreamInfo
	{
		uint8_t noSources;
		AudioSourceInfo *sources;

		AudioStreamInfo()
		{
			noSources = 0;
		}
	};

	struct ServiceInfo
	{
		uint8_t type;
		uint32_t key;					//In case of video streaming, key = ssrc
		uint64_t startTimeStamp;		//Timestamp to start stream in global time
		VideoStreamInfo videoStreamInfo;
		AudioStreamInfo audioStreamInfo;
	};

	struct OpusHeader
    {
        int version;
        int channels; /* Number of channels: 1..255 */
        int preskip;
        uint32_t input_sample_rate;
        int gain; /* in dB S7.8 should be zero whenever possible */
        int channel_mapping;
        /* The rest is only used if channel_mapping != 0 */
        int nb_streams;
        int nb_coupled;
        unsigned char stream_map[255];
    };

    const int VP8_PAYLOAD_HEADER_SIZE = 3;
	const int VP8_COMMON_HEADER_SIZE = 3;
	const int RTP_HEADER_SIZE = 7;
	const int VP8_MAX_HEADER_SIZE = VP8_COMMON_HEADER_SIZE + VP8_PAYLOAD_HEADER_SIZE + RTP_HEADER_SIZE;

	const uint16_t OPUS_PAYLOAD_TYPE = 120;
	const uint16_t VP8_PAYLOAD_TYPE = 103;

	const int OPUS_PAYLOAD_HEADER_SIZE = 5;
	const int OPUS_MAX_HEADER_SIZE = OPUS_PAYLOAD_HEADER_SIZE + RTP_HEADER_SIZE;

	const int DEFAULT_FORWARDING_SERVER_PORT = 31313;

	struct AvailableDuration
	{
		uint32_t start;
		uint32_t end;
	};
}


#endif

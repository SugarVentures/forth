/*
	Error code
*/

#ifndef OPPVS_ERROR_HPP
#define OPPVS_ERROR_HPP

extern "C"
{
	#include "crypto_kernel.h"
	#include "sha1.h"
}

namespace oppvs {
	typedef enum ERRS_VIDEO_CAPTURE {
		ERR_VIDEO_CAPTURE_NONE,
		ERR_VIDEO_CAPTURE_SESSION_INIT_FAILED,
		ERR_VIDEO_CAPTURE_INPUT_DEVICE_FAILED,
		ERR_VIDEO_CAPTURE_OUTPUT_DEVICE_FAILED
	} error_video_capture_t;

	enum ERRS_ENCODING {
		ERRS_ENCODING_OK,
		ERRS_ENCODING_INIT_FAILED
	};

	enum ERRS_DECODING {
		ERRS_DECODING_OK,
		ERRS_DECODING_INIT_FAILED
	};

	static void printHashCode(const void* msg, int len)
	{
		sha1_ctx_t ctx;
		uint32_t hashcode[5];
		sha1_init(&ctx);

		sha1_update(&ctx, (uint8_t*)msg, len);
		sha1_final(&ctx, hashcode);
			
		printf("len: computed hash value:  %d, %u %u %u %u\n", len, 
			 hashcode[0], hashcode[1], hashcode[2], hashcode[3]);
	}
}

#endif
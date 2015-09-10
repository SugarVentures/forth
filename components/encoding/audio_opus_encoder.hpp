#ifndef OPPVS_AUDIO_OPUS_ENCODER_HPP
#define OPPVS_AUDIO_OPUS_ENCODER_HPP

//Opus headers
#include "opus_multistream.h"
#include "opus.h"

#include "resample/speex_resampler.h"
#include "datatypes.hpp"

#include <vector>

namespace oppvs {
	
	class AudioOpusEncoder
	{	
	public:
		AudioOpusEncoder();
		~AudioOpusEncoder();

		int init(const AudioStreamInfo&);
		void cleanup();
		int encode(const void* input, uint8_t source, uint8_t* output);
	private:
		struct Resampler {
			SpeexResamplerState *resampler;
			void *realReadData;
			int channels;
			int bufPos;
			int bufSize;
			float *bufs;
			int done;
		};

		struct AudioEncodingController
		{
			OpusEncoder *enc;
			uint8_t source;
			uint8_t format;	//Float or Int
			int channels;
			uint32_t rate;
			uint32_t samples;
			int skip;
			Resampler* resampler;
		};


		std::vector<AudioEncodingController*> m_controllers;
		int m_bitrate;	//bps
		int m_frameSize;

		OpusEncoder* initOpus(AudioEncodingController* controller, int application);
		int getCodingMode(uint8_t);
		void destroyOpus(OpusEncoder* enc);
		AudioEncodingController* getController(uint8_t source);

		//Resample functions
		int setupResampler(AudioEncodingController* controller, int complexity, long outFreq);
		long readResampler(Resampler* rs, const void* input, float* buffer, int samples);
		void clearResampler(Resampler* rs);
	};
} // oppvs

#endif // OPPVS_AUDIO_OPUS_ENCODER_HPP

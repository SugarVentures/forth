#ifndef OPPVS_AUDIO_OPUS_ENCODER_HPP
#define OPPVS_AUDIO_OPUS_ENCODER_HPP

//Opus headers
#include "opus_multistream.h"
#include "opus.h"

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
		int encode(const void* input, int source, uint8_t* output);
	private:
		struct AudioEncodingController
		{
			OpusEncoder *enc;
			uint8_t source;
		};

		std::vector<AudioEncodingController*> m_controllers;

		OpusEncoder* initOpus(int sampleRate, int channels, int application);
		int getCodingMode(uint8_t);
		void destroyOpus(OpusEncoder* enc);
	};
} // oppsvs

#endif // OPPVS_AUDIO_OPUS_ENCODER_HPP

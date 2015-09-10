#ifndef OPPVS_AUDIO_OPUS_DECODER_HPP
#define OPPVS_AUDIO_OPUS_DECODER_HPP

#include "opus.h"

#include "datatypes.hpp"
#include <vector>

namespace oppvs {
	class AudioOpusDecoder
	{
	public:
		AudioOpusDecoder();
		~AudioOpusDecoder();
		
		int init(const AudioStreamInfo&);
		void cleanup();
	private:
		struct AudioDecodingController
		{
			OpusDecoder *dec;
			uint8_t source;
			uint8_t format;	//Float or Int
			int channels;
			uint32_t rate;
			uint32_t samples;
			int skip;
		};

		std::vector<AudioDecodingController*> m_controllers;
		OpusDecoder* initOpus(AudioDecodingController* controller);
		void destroyOpus(OpusDecoder* dec);
	};
} // oppvs

#endif // OPPVS_AUDIO_OPUS_DECODER_HPP

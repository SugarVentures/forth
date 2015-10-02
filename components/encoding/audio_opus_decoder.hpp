#ifndef OPPVS_AUDIO_OPUS_DECODER_HPP
#define OPPVS_AUDIO_OPUS_DECODER_HPP

#include "opus.h"
#include "opus_multistream.h"

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
		int decode(const void* input, int len, uint8_t source, float* output);
	private:
		struct AudioDecodingController
		{
			OpusMSDecoder *dec;
            OpusHeader header;
			uint8_t source;
			uint8_t format;	//Float or Int
			int channels;
			uint32_t rate;
			uint32_t samples;
            int skip;
		};

		std::vector<AudioDecodingController*> m_controllers;
		OpusMSDecoder* initOpus(AudioDecodingController* controller);
		void destroyOpus(OpusMSDecoder* dec);

		AudioDecodingController* getController(uint8_t source);

	};
} // oppvs

#endif // OPPVS_AUDIO_OPUS_DECODER_HPP

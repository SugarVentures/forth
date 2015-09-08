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

	private:
		struct AudioEncodingController
		{
			OpusEncoder *enc;
			uint8_t source;
		};

		std::vector<AudioEncodingController*> m_controllers;
	};
} // oppsvs

#endif // OPPVS_AUDIO_OPUS_ENCODER_HPP

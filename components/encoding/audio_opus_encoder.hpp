#ifndef OPPVS_AUDIO_OPUS_ENCODER_HPP
#define OPPVS_AUDIO_OPUS_ENCODER_HPP

//Opus headers
#include "opus_multistream.h"
#include "opus.h"

//Local headers
#include "datatypes.hpp"

//Stardard headers
#include <vector>

namespace oppvs {

	const int AUDIO_ENCODING_FRAME_SIZE = 960;
    
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
    
    constexpr static const uint8_t opusCoupledStreams[8] = {0, 1, 1, 2, 2, 2, 2, 3};
    constexpr static const uint8_t opusEncodeChannelMap[8][8] = {
        { 0 },
        { 0, 1 },
        { 0, 1, 2 },
        { 0, 1, 2, 3 },
        { 0, 1, 3, 4, 2 },
        { 0, 1, 4, 5, 2, 3 },
        { 0, 1, 5, 6, 2, 4, 3 },
        { 0, 1, 6, 7, 4, 5, 2, 3 },
    };
	
	class AudioOpusEncoder
	{
	public:
		AudioOpusEncoder();
		~AudioOpusEncoder();

		int init(const AudioStreamInfo&);
		void cleanup();
		int encode(const void* input, uint8_t source, float* output);
	private:
		struct AudioEncodingController
		{
			OpusMSEncoder *enc;
			uint8_t source;
			uint8_t format;	//Float or Int
			int channels;
			uint32_t rate;
			uint32_t samples;
			int skip;
		};

		std::vector<AudioEncodingController*> m_controllers;
		int m_bitrate;	//bps
		int m_frameSize;

		OpusMSEncoder* initOpus(AudioEncodingController* controller, int application);
		int getCodingMode(uint8_t);
		void destroyOpus(OpusMSEncoder* enc);
		AudioEncodingController* getController(uint8_t source);

	};
} // oppvs

#endif // OPPVS_AUDIO_OPUS_ENCODER_HPP

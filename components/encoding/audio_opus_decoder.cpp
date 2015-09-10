#include "audio_opus_decoder.hpp"

namespace oppvs {
	AudioOpusDecoder::AudioOpusDecoder()
	{

	}

	AudioOpusDecoder::~AudioOpusDecoder()
	{
		cleanup();
	}

	int AudioOpusDecoder::init(const AudioStreamInfo& info)
	{
		if (info.noSources == 0 || info.sources == NULL)
		{
			printf("Invalid input parameters\n");
			return -1;
		}
		for (unsigned i = 0; i < info.noSources; ++i) {
			AudioDecodingController* controller = new AudioDecodingController();
			
			controller->source = info.sources[i].source;
			controller->format = info.sources[i].format;
			controller->channels = info.sources[i].numberChannels;
			controller->rate = info.sources[i].sampleRate;
			controller->samples = info.sources[i].samplePerChannels;

			OpusDecoder *dec = initOpus(controller);
			if (dec == NULL)
			{
				delete controller;
				return -1;
			}
			controller->dec = dec;
			m_controllers.push_back(controller);
		}
		return 0;
	}

	void AudioOpusDecoder::cleanup()
	{
		for (unsigned i = 0; i < m_controllers.size(); ++i) {
			destroyOpus(m_controllers[i]->dec);
			m_controllers[i]->dec = NULL;
			//clearResampler(m_controllers[i]->resampler);
			delete m_controllers[i];
			m_controllers[i] = NULL;
		}
		m_controllers.clear();
	}

	OpusDecoder* AudioOpusDecoder::initOpus(AudioDecodingController* controller)
	{
		int error = 0;
		OpusDecoder *dec = NULL;
		int codingRate;
		uint32_t sampleRate = controller->rate;
		if (sampleRate > 24000)
			codingRate = 48000;
		else if (sampleRate > 16000)
			codingRate = 24000;
		else if (sampleRate > 12000)
			codingRate = 16000;
		else if (sampleRate > 8000)
			codingRate = 12000;
		else
			codingRate = 80000;
		dec = opus_decoder_create(codingRate, controller->channels, &error);
		if (error != OPUS_OK || dec == NULL)
		{
			printf("Failed to init Opus Decoder: %s\n", opus_strerror(error));
			return NULL;
		}
		return dec;
	}

	void AudioOpusDecoder::destroyOpus(OpusDecoder* dec)
	{
		if (dec)
			opus_decoder_destroy(dec);
	}
} // oppvs
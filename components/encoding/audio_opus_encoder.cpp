#include "audio_opus_encoder.hpp"

namespace oppvs {
	AudioOpusEncoder::AudioOpusEncoder()
	{

	}

	AudioOpusEncoder::~AudioOpusEncoder()
	{
		cleanup();
	}

	int AudioOpusEncoder::init(const AudioStreamInfo& info)
	{
		int application;
		if (info.noSources == 0 || info.sources == NULL)
		{
			printf("Invalid input parameters\n");
			return -1;
		}
		for (unsigned i = 0; i < info.noSources; ++i) {
			application = getCodingMode(info.sources[i].type);
			if (application == -1)
			{
				return -1;
			}
			OpusEncoder *enc = initOpus(info.sources[i].sampleRate, info.sources[i].numberChannels, application);
			if (enc == NULL)
			{
				return -1;
			}
			AudioEncodingController* controller = new AudioEncodingController();
			controller->enc = enc;
			controller->source = info.sources[i].source;
			m_controllers.push_back(controller);
		}
		return 0;
	}

	void AudioOpusEncoder::cleanup()
	{
		for (unsigned i = 0; i < m_controllers.size(); ++i) {
			destroyOpus(m_controllers[i]->enc);
			m_controllers[i]->enc = NULL;
			delete m_controllers[i];
			m_controllers[i] = NULL;
		}
		m_controllers.clear();
	}

	int AudioOpusEncoder::encode(const void* input, int source, uint8_t* output)
	{
		return 0;
	}

	OpusEncoder* AudioOpusEncoder::initOpus(int sampleRate, int channels, int application)
	{
		int error = 0;
		OpusEncoder *enc = NULL;
		int codingRate;
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

		enc = opus_encoder_create(codingRate, channels, application, &error);
		if (error != OPUS_OK || enc == NULL)
		{
			printf("Failed to init Opus Encoder: %d\n", error);
			return NULL;
		}
		return enc;
	}

	int AudioOpusEncoder::getCodingMode(uint8_t type)
	{
		switch (type)
		{
			case AUDIO_TYPE_VOIP:
				return OPUS_APPLICATION_VOIP;
			case AUDIO_TYPE_MIXED:
				return OPUS_APPLICATION_AUDIO;
		}
		return -1;
	}

	void AudioOpusEncoder::destroyOpus(OpusEncoder* enc)
	{
		if (enc)
		{
			opus_encoder_destroy(enc);
		}
	}
} // oppvs
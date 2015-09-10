#include "audio_opus_encoder.hpp"


namespace oppvs {
	AudioOpusEncoder::AudioOpusEncoder(): m_bitrate(512000), m_frameSize(960)
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
			AudioEncodingController* controller = new AudioEncodingController();
			
			controller->source = info.sources[i].source;
			controller->format = info.sources[i].format;
			controller->channels = info.sources[i].numberChannels;
			controller->rate = info.sources[i].sampleRate;
			controller->samples = info.sources[i].samplePerChannels;

			OpusEncoder *enc = initOpus(controller, application);
			if (enc == NULL)
			{
				delete controller;
				return -1;
			}
			controller->enc = enc;
			m_controllers.push_back(controller);
		}
		return 0;
	}

	void AudioOpusEncoder::cleanup()
	{
		for (unsigned i = 0; i < m_controllers.size(); ++i) {
			destroyOpus(m_controllers[i]->enc);
			m_controllers[i]->enc = NULL;
			clearResampler(m_controllers[i]->resampler);
			delete m_controllers[i];
			m_controllers[i] = NULL;
		}
		m_controllers.clear();
	}

	int AudioOpusEncoder::encode(const void* input, uint8_t source, uint8_t* output)
	{
		AudioEncodingController* controller = getController(source);
		if (controller == NULL)
			return -1;
		float* buffer = new float[960 * 2];
		long noSamples = readResampler(controller->resampler, input, buffer, 960);
		int nBytes = opus_encode_float(controller->enc, buffer, 960, output, AUDIO_MAX_ENCODING_PACKET_SIZE);
		if (nBytes < 0)
		{
			printf("Encode failed: %s\n", opus_strerror(nBytes));
			return -1;
		}
		printf("Encoded bytes: %d\n", nBytes);
		return 0;
	}

	OpusEncoder* AudioOpusEncoder::initOpus(AudioEncodingController* controller, int application)
	{
		int error = 0;
		int complexity = 10;	//Complexity must be 0 - 10
		uint32_t sampleRate = controller->rate;
		int channels = controller->channels;
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

		/*Scale the resampler complexity, but only for 48000 output because the near-cutoff behavior matters a lot more at lower rates.*/
		if (sampleRate != codingRate)
			setupResampler(controller, codingRate == 48000 ? (complexity + 1)/2:5, codingRate);

		enc = opus_encoder_create(codingRate, channels, application, &error);
		if (error != OPUS_OK || enc == NULL)
		{
			printf("Failed to init Opus Encoder: %s\n", opus_strerror(error));
			return NULL;
		}
		//error = opus_encoder_ctl(enc, OPUS_SET_BITRATE(m_bitrate));
		/*if (error < 0)
		{
			printf("Failed to set bitrate: %s\n", opus_strerror(error));
		}*/
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

	AudioOpusEncoder::AudioEncodingController* AudioOpusEncoder::getController(uint8_t source)
	{
		for (unsigned i = 0; i < m_controllers.size(); ++i) {
			if (m_controllers[i]->source == source)
				return m_controllers[i];
		}
		return NULL;
	}

	int AudioOpusEncoder::setupResampler(AudioEncodingController* controller, int complexity, long outFreq)
	{
		Resampler *rs = new Resampler;
		int err;
		
		rs->bufSize = 5760*2; /* Have at least two output frames worth, just in case of ugly ratios */
		rs->bufPos = 0;

		rs->channels = controller->channels;
		rs->done = 0;

		rs->resampler = speex_resampler_init(rs->channels, controller->rate, outFreq, complexity, &err);
		if (err != 0)
			printf("resampler error: %s\n", speex_resampler_strerror(err));
		controller->skip += speex_resampler_get_output_latency(rs->resampler);

		rs->bufs = new float[rs->bufSize * controller->channels];

		if (controller->samples)
		    controller->samples = (int)((float)controller->samples * ((float)outFreq/(float)controller->rate));

		controller->rate = outFreq;
		controller->resampler = rs;
		return 0;
	}

	long AudioOpusEncoder::readResampler(Resampler* rs, const void* input, float* buffer, int samples)
	{
		int outSamples = 0;
		float *pcmBuf;
		int *inBuf;

		pcmBuf = rs->bufs;
		inBuf = &rs->bufPos;
		unsigned int inLen = 512*2;
		unsigned int outLen = samples;
		speex_resampler_process_interleaved_float(rs->resampler, (const float*)input, &inLen, buffer, &outLen);
		
		return outLen;
	}

	void AudioOpusEncoder::clearResampler(Resampler* rs)
	{
		speex_resampler_destroy(rs->resampler);
		delete [] rs->bufs;
		delete rs;
		rs = NULL;
	}
} // oppvs
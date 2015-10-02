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

			OpusMSEncoder *enc = initOpus(controller, application);
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
			delete m_controllers[i];
			m_controllers[i] = NULL;
		}
		m_controllers.clear();
	}

	int AudioOpusEncoder::encode(const void* input, int inLen, uint8_t source, uint8_t* output)
	{
		AudioEncodingController* controller = getController(source);
		if (controller == NULL)
			return -1;
		
        int maxDataBytes = AUDIO_MAX_ENCODE_PACKET_SIZE;
        output = new uint8_t[maxDataBytes];
        int ret = opus_multistream_encode_float(controller->enc, (const float*)input, m_frameSize, output, maxDataBytes);
        if (ret < 0)
        {
            delete [] output;
        }
		return ret;
	}

	OpusMSEncoder* AudioOpusEncoder::initOpus(AudioEncodingController* controller, int application)
	{
		int error = 0;
		int complexity = 10;	//Complexity must be 0 - 10
		uint32_t sampleRate = controller->rate;
		int channels = controller->channels;
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
			codingRate = 8000;
        
        m_frameSize = AUDIO_ENCODING_FRAME_SIZE / (48000/codingRate);
        controller->header.channels = channels;
        controller->header.input_sample_rate = sampleRate;
        
        OpusMSEncoder* enc;
        const uint8_t *mapping;
        mapping = opusEncodeChannelMap[channels - 1];
        int coupleStreamCount = opusCoupledStreams[channels - 1];
        int streamCount = channels - coupleStreamCount;
        
        controller->header.nb_streams = streamCount;
        
        //In opus, the mapping for channels > 8 is undefined
        if (channels > 8)
        {
            printf("Channel layout undefined for %d channels\n", channels);
            return NULL;
        }
        
        enc = opus_multistream_encoder_create(codingRate, channels, streamCount, coupleStreamCount, mapping, application, &error);
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

	void AudioOpusEncoder::destroyOpus(OpusMSEncoder* enc)
	{
		if (enc)
		{
            opus_multistream_encoder_destroy(enc);
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


} // oppvs
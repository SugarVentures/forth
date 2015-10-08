#include "audio_packet_handler.h"

namespace oppvs {
	AudioPacketizer::AudioPacketizer(): m_isRunning(true), p_audioBuffer(NULL)
	{
		p_thread = new Thread(AudioPacketizer::run, this);

	}

	AudioPacketizer::~AudioPacketizer()
	{
		delete p_thread;
	}

	int AudioPacketizer::init(const AudioStreamInfo& info)
	{
		if (info.noSources != 1)
		{
			printf("Only support 1 audio source now\n");
			return -1;
		}
		for (int i = 0; i < info.noSources; i++)
		{
			int size = 0;
			if (info.sources[i].format == AUDIO_FORMAT_FLOAT)
				size = 4;
			else
				size = 2;

			m_size = size * info.sources[i].numberChannels;
			p_audioBuffer = new AudioRingBuffer();
			p_audioBuffer->allocate(m_size, 10 * 512);

			m_source = info.sources[i].source;
		}
		//Init encoder
		if (m_encoder.init(info) < 0)
			return -1;
		return 0;
	}

	void* AudioPacketizer::run(void* object)
	{
		AudioPacketizer* handler = (AudioPacketizer*)object;
		while (handler->isRunning())
		{
			handler->pull();
			usleep(10000);
		}
		return NULL;
	}

	void AudioPacketizer::start()
	{
		p_thread->create();
	}

	void AudioPacketizer::push(const GenericAudioBufferList& ab)
	{
		if (p_audioBuffer == NULL)
			return;

		uint32_t noFrames = ab.nFrames;
		RingBufferError err = p_audioBuffer->store(&noFrames, ab.buffers[0].data, ab.sampleTime);
		if (err)
			printf("Can not push audio samples, error: %d\n", err);
		else
			printf("Push audio to ring buffer\n");
	}

	void AudioPacketizer::pull()
	{
		if (p_audioBuffer == NULL)
			return;
		uint16_t noFrames = AUDIO_ENCODING_FRAMES;
		if (p_audioBuffer->getNumberFrames() > AUDIO_ENCODING_FRAMES)
		{
			int err = p_audioBuffer->fetch(noFrames, m_inBuffer, p_audioBuffer->getStartTime());
			//printf("Fetch audio from ring buffer err: %d frames: %d\n", err, noFrames);
			int inLen = noFrames * m_size;
			uint8_t* out = NULL;
			int outLen = m_encoder.encode(m_inBuffer, inLen, m_source, out);
			printf("Encode out len: %d source: %d\n", outLen, m_source);
			delete [] out;
		}
	}

	bool AudioPacketizer::isRunning()
	{
		return m_isRunning;
	}
} // oppvs	
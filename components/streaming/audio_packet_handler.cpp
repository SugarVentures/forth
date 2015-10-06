#include "audio_packet_handler.h"

namespace oppvs {
	AudioPacketizer::AudioPacketizer(): m_isRunning(true)
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

			size = size * info.sources[i].numberChannels;
			p_audioBuffer = new AudioRingBuffer();
			p_audioBuffer->allocate(size, 10 * 512);
		}
		return 0;
	}

	void* AudioPacketizer::run(void* object)
	{
		return NULL;
	}

	void AudioPacketizer::start()
	{
		p_thread->create();
	}

	void AudioPacketizer::push(const GenericAudioBufferList& ab)
	{
		uint32_t noFrames = ab.nFrames;
		RingBufferError err = p_audioBuffer->store(&noFrames, ab.buffers[0].data, ab.sampleTime);
		if (err)
			printf("Can not push audio samples, error: %d\n", err);
	}

	void AudioPacketizer::pull()
	{

	}
} // oppvs	
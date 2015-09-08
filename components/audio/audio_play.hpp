#ifndef OPPVS_AUDIO_PLAY_HPP
#define OPPVS_AUDIO_PLAY_HPP

#include "audio_device.hpp"

namespace oppvs
{
	class AudioPlay
	{
	public:
		AudioPlay() {}
		AudioPlay(const AudioDevice& device, uint64_t isr, uint32_t inc) : m_device(device), m_inputSampleRate(isr), m_inputNumChannels(inc) {}
		~AudioPlay() {}

		virtual int init() { return 0; }
	protected:
		AudioDevice m_device;
		uint64_t m_inputSampleRate;
		uint32_t m_inputNumChannels;
	};
}

#endif // OPPVS_AUDIO_PLAY_HPP

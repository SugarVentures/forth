#ifndef OPPVS_AUDIO_ENGINE_HPP
#define OPPVS_AUDIO_ENGINE_HPP

#include "audio_device.hpp"
#include <vector>

namespace oppvs {
	class AudioEngine
	{
	public:
		AudioEngine() {}

		virtual ~AudioEngine() {}
		virtual void getListAudioDevices(std::vector<AudioDevice>& result) {}
	protected:
		std::vector<AudioDevice> m_listAudioDevices;
	private:

	};
} // oppvs

#endif // OPPVS_AUDIO_ENGINE_HPP

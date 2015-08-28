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

		void printAudioDeviceList()
		{
			std::vector<AudioDevice>::iterator it;
			for (it = m_listAudioDevices.begin(); it != m_listAudioDevices.end(); ++it)
			{
				printf("Source id %d Name: %s Channels: %d\n", it->getDeviceID(), it->getDeviceName().c_str(), it->getNumberChannels());
			}
		}
	protected:
		std::vector<AudioDevice> m_listAudioDevices;
		void resetAudioDeviceList()
		{
			m_listAudioDevices.clear();
		}


	private:

	};
} // oppvs

#endif // OPPVS_AUDIO_ENGINE_HPP

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
		virtual int addNewCapture(uint32_t deviceid) { return 0; }

		void printAudioDeviceList() const
		{
			std::vector<AudioDevice>::const_iterator it;
			for (it = m_listAudioDevices.begin(); it != m_listAudioDevices.end(); ++it)
			{
				printf("Source id %d Name: %s Channels: %d Rate: %u\n", it->getDeviceID(), it->getDeviceName().c_str(), 
					it->getNumberChannels(), it->getSampleRate());
			}
		}

		int getDeviceByID(uint32_t deviceid, AudioDevice& device) const
		{
			std::vector<AudioDevice>::const_iterator it;
			for (it = m_listAudioDevices.begin(); it != m_listAudioDevices.end(); ++it)
			{
				if (it->getDeviceID() == deviceid)
				{
					device = *it;
					return 0;
				}
			}
			return -1;
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

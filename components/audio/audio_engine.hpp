#ifndef OPPVS_AUDIO_ENGINE_HPP
#define OPPVS_AUDIO_ENGINE_HPP

#include "audio_device.hpp"
#include "audio_capture.hpp"
#include <vector>


namespace oppvs {
	struct AudioActiveSource
	{
		uint8_t id;
		uint32_t deviceId;
		uint64_t sampleRate;
		AudioCapture* capture;
	};

	class AudioEngine
	{
	public:
		AudioEngine() {}

		virtual ~AudioEngine() {}
		virtual void getListAudioDevices(std::vector<AudioDevice>& result, bool input) {}
		virtual int addNewCapture(uint32_t deviceid, void* user) { return 0; }

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

		const std::vector<AudioActiveSource>& getAudioActiveSources() const
		{
			return m_activeSources;
		}

		audio_callback callbackAudio;
	protected:
		std::vector<AudioDevice> m_listAudioDevices;
		std::vector<AudioActiveSource> m_activeSources;

		void resetAudioDeviceList()
		{
			m_listAudioDevices.clear();
		}

		AudioActiveSource* addSource(uint32_t deviceid, uint64_t sampleRate)
		{
			if (m_activeSources.size() >= MAX_ACTIVE_SOURCES)
			{
				return NULL;
			}
			std::vector<AudioActiveSource>::iterator it = std::find_if(m_activeSources.begin(), m_activeSources.end(), findSource(deviceid));
			if (it != m_activeSources.end())
				return NULL;
			AudioActiveSource src;
			src.id = m_activeSourceIndex;
			src.deviceId = deviceid;
			src.sampleRate = sampleRate;
			m_activeSources.push_back(src);
			return &m_activeSources.back();
		}

		int removeSource(uint32_t deviceid)
		{
			std::vector<AudioActiveSource>::iterator it = std::find_if(m_activeSources.begin(), m_activeSources.end(), findSource(deviceid));
			if (it == m_activeSources.end())
				return -1;
			m_activeSources.erase(it);
			return 0;
		}
	private:
		const static uint8_t MAX_ACTIVE_SOURCES = 1;

		uint8_t m_activeSourceIndex = 0;

		struct findSource {
			uint32_t id;
			findSource(uint32_t id) : id(id) {}
			
			bool operator() (const AudioActiveSource& m) const {
				return (m.deviceId == id) ;
			}
		};
	};
    
} // oppvs

#endif // OPPVS_AUDIO_ENGINE_HPP

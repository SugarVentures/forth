#ifndef OPPVS_AUDIO_DEVICE_HPP
#define OPPVS_AUDIO_DEVICE_HPP

#include "datatypes.hpp"
#include <string>

namespace oppvs {
	class AudioDevice
	{
	public:
		AudioDevice();
		AudioDevice(uint32_t id, const std::string& deviceName, const std::string& deviceManu,
			uint32_t safetyeOffset, uint32_t bufferSizeFrames, int noChannels);
		~AudioDevice();

		uint32_t getDeviceID();
		const std::string& getDeviceName();
		const std::string& getManufacturer();
		int getNumberChannels();
	private:
		uint32_t m_deviceID;
		std::string m_deviceName;
		std::string m_manufacturer;
		uint32_t m_safetyOffset;
		uint32_t m_bufferSizeFrames;
		int m_numberChannels;
	};
} // oppvs

#endif // OPPVS_AUDIO_DEVICE_HPP

#ifndef OPPVS_AUDIO_DEVICE_HPP
#define OPPVS_AUDIO_DEVICE_HPP

#include "datatypes.hpp"
#include <string>

namespace oppvs {
	class AudioDevice
	{
	public:
		AudioDevice();
		~AudioDevice();

		uint32_t getDeviceID();
		const std::string& getDeviceName();
		const std::string& getManufacturer();
	private:
		uint32_t m_deviceID;
		std::string m_deviceName;
		std::string m_manufacturer;
	};
} // oppvs

#endif // OPPVS_AUDIO_DEVICE_HPP

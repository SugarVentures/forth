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
			uint32_t safetyeOffset, uint32_t bufferSizeFrames, int noChannels, uint32_t sampleRate);
		AudioDevice(uint32_t id);
		AudioDevice(uint32_t id, const std::string& deviceName, const std::string& deviceType, 
			const std::string& deviceOrientation, int noChannels, uint32_t sampleRate);
		~AudioDevice();

		uint32_t getDeviceID() const;
		const std::string& getDeviceName() const;
		const std::string& getManufacturer() const;
		const std::string& getType() const;
		const std::string& getOrientation() const;
		int getNumberChannels() const;
		uint32_t getSampleRate() const;
	private:
		uint32_t m_deviceID;
		std::string m_deviceName;
		std::string m_manufacturer;
		std::string m_deviceType;
		std::string m_orientation;
		uint32_t m_safetyOffset;
		uint32_t m_bufferSizeFrames;
		int m_numberChannels;
		uint32_t m_sampleRate;
	};
} // oppvs

#endif // OPPVS_AUDIO_DEVICE_HPP

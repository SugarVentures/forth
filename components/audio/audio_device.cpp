#include "audio_device.hpp"

namespace  oppvs {
	AudioDevice::AudioDevice()
	{

	}

	AudioDevice::AudioDevice(uint32_t id, const std::string& deviceName, const std::string& deviceManu,
		uint32_t safetyOffset, uint32_t bufferSizeFrames, int noChannels, uint32_t sampleRate) : 
		m_deviceID(id), m_deviceName(deviceName), m_manufacturer(deviceManu),
		m_safetyOffset(safetyOffset), m_bufferSizeFrames(bufferSizeFrames), 
		m_numberChannels(noChannels), m_sampleRate(sampleRate)
	{
	}

	AudioDevice::~AudioDevice()
	{

	}

	uint32_t AudioDevice::getDeviceID()
	{
		return m_deviceID;
	}

	const std::string& AudioDevice::getDeviceName()
	{
		return m_deviceName;
	}

	const std::string& AudioDevice::getManufacturer()
	{
		return m_manufacturer;
	}

	int AudioDevice::getNumberChannels()
	{
		return m_numberChannels;
	}

	uint32_t AudioDevice::getSampleRate()
	{
		return m_sampleRate;
	}
} //  oppvs
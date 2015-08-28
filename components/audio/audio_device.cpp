#include "audio_device.hpp"

namespace  oppvs {
	AudioDevice::AudioDevice()
	{

	}

	AudioDevice::AudioDevice(uint32_t id, const std::string& deviceName, const std::string& deviceManu) : 
		m_deviceID(id), m_deviceName(deviceName), m_manufacturer(deviceManu)
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
} //  oppvs
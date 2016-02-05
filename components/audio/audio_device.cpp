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

	AudioDevice::AudioDevice(uint32_t id): m_deviceID(id)
	{

	}

	AudioDevice::AudioDevice(uint32_t id, const std::string& deviceName, const std::string& deviceType, 
			const std::string& deviceOrientation, int noChannels, uint32_t sampleRate) :
		m_deviceID(id), m_deviceName(deviceName), m_deviceType(deviceType),
		m_orientation(deviceOrientation), m_numberChannels(noChannels),
		m_sampleRate(sampleRate)
	{

	}

	AudioDevice::~AudioDevice()
	{

	}

	uint32_t AudioDevice::getDeviceID() const
	{
		return m_deviceID;
	}

	const std::string& AudioDevice::getDeviceName() const
	{
		return m_deviceName;
	}

	const std::string& AudioDevice::getManufacturer() const
	{
		return m_manufacturer;
	}

	const std::string& AudioDevice::getType() const
	{
		return m_deviceType;
	}

	const std::string& AudioDevice::getOrientation() const
	{
		return m_orientation;
	}

	int AudioDevice::getNumberChannels() const
	{
		return m_numberChannels;
	}

	uint32_t AudioDevice::getSampleRate() const
	{
		return m_sampleRate;
	}
} //  oppvs
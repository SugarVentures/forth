#include "mac_audio_engine.hpp"

namespace oppvs {
	MacAudioEngine::MacAudioEngine()
	{

	}

	MacAudioEngine::~MacAudioEngine()
	{

	}

	void MacAudioEngine::getListAudioDevices(std::vector<AudioDevice>& result)
	{
		resetAudioDeviceList();
		AudioObjectPropertyAddress propertyAddress = {
	        kAudioHardwarePropertyDevices,
	        kAudioObjectPropertyScopeGlobal,
	        kAudioObjectPropertyElementMaster
    	};

    	UInt32 dataSize = 0;
	    OSStatus status = AudioHardwareServiceGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &dataSize);
	    if (status != kAudioHardwareNoError) {
	        printf("AudioObjectGetPropertyDataSize (kAudioHardwarePropertyDevices) failed: %i\n", status);
	        return;
	    }

	    UInt32 noDevices = (UInt32)(dataSize / sizeof(AudioDeviceID));
    	printf("Number of audio devices: %d\n", noDevices);
	    AudioDeviceID *audioDevices = new AudioDeviceID[dataSize];
	    
	    status = AudioHardwareServiceGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &dataSize, audioDevices);
	    if(status != kAudioHardwareNoError) {
	        printf("AudioHardwareServiceGetPropertyData (kAudioHardwarePropertyDevices) failed: %i\n", status);
	        delete [] audioDevices;
	        audioDevices = NULL;
	        return;
	    }

	    for(unsigned i = 0; i < noDevices; ++i) {
	    	if (audioDevices[i] == kAudioDeviceUnknown)
	    		continue;

	    	AudioObjectPropertyScope scope = kAudioDevicePropertyScopeInput;

	    	//Get safety offset
	    	UInt32 proSize = sizeof(Float32);
	    	UInt32 safetyOffset;
	    	AudioObjectPropertyAddress address = { 
	    		kAudioDevicePropertySafetyOffset,
	    		scope,
	    		0
	    	};

	    	status = AudioHardwareServiceGetPropertyData(audioDevices[i], &address, 0, NULL, &proSize, &safetyOffset);
	    	if (status != kAudioHardwareNoError) {
	    		printf("AudioHardwareServiceGetPropertyData (kAudioDevicePropertySafetyOffset) failed: %i\n", status);
	    		continue;
	    	}
	    	//Get buffer frame size
	    	proSize = sizeof(UInt32);
	    	UInt32 bufferSizeFrames;
	    	address.mSelector = kAudioDevicePropertyBufferFrameSize;
	    	status = AudioHardwareServiceGetPropertyData(audioDevices[i], &address, 0, NULL, &proSize, &bufferSizeFrames);
	    	if (status != kAudioHardwareNoError) {
	    		printf("AudioHardwareServiceGetPropertyData (kAudioDevicePropertyBufferFrameSize) failed: %i\n", status);
	    		continue;
	    	}

	    	//Get device UID
	    	CFStringRef deviceUID = NULL;
	        proSize = sizeof(deviceUID);
	        address.mSelector = kAudioDevicePropertyDeviceUID;
	        status = AudioHardwareServiceGetPropertyData(audioDevices[i], &address, 0, NULL, &proSize, &deviceUID);
	        if (status != kAudioHardwareNoError) {
	        	printf("AudioHardwareServiceGetPropertyData (kAudioDevicePropertyDeviceUID) failed: %i\n", status);
	        	continue;
	        }

	        printf("Device ID: %s size: %d\n", CFStringGetCStringPtr(deviceUID, kCFStringEncodingMacRoman), proSize);
	        CFStringRef deviceName = NULL;
	        proSize = sizeof(deviceName);
	        address.mSelector = kAudioDevicePropertyDeviceNameCFString;
			status = AudioHardwareServiceGetPropertyData(audioDevices[i], &address, 0, NULL, &proSize, &deviceName);
	        if (status != kAudioHardwareNoError) {
	        	printf("AudioHardwareServiceGetPropertyData (kAudioDevicePropertyDeviceName) failed: %i\n", status);
	        	continue;
	        }
	        printf("Device Name: %s size: %d\n", CFStringGetCStringPtr(deviceName, kCFStringEncodingMacRoman), proSize);
	        std::string strDeviceName = std::string(CFStringGetCStringPtr(deviceName, kCFStringEncodingMacRoman));

	        CFStringRef deviceManufacturer = NULL;
	        proSize = sizeof(deviceManufacturer);
	        address.mSelector = kAudioDevicePropertyDeviceManufacturerCFString;
	        status = AudioHardwareServiceGetPropertyData(audioDevices[i], &address, 0, NULL, &proSize, &deviceManufacturer);
	        if(kAudioHardwareNoError != status) {
	            printf("AudioHardwareServiceGetPropertyData (kAudioDevicePropertyDeviceManufacturerCFString) failed: %i\n", status);
	            continue;
	        }
	        std::string strDeviceManu = std::string(CFStringGetCStringPtr(deviceManufacturer, kCFStringEncodingMacRoman));
	        printf("Device Manu: %s size: %d\n", CFStringGetCStringPtr(deviceManufacturer, kCFStringEncodingMacRoman), proSize);
	        AudioDevice aDevice(audioDevices[i], strDeviceName, strDeviceManu);
	        m_listAudioDevices.push_back(aDevice);
	    }

	}
} // oppvs
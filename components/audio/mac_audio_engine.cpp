#include "mac_audio_engine.hpp"

namespace oppvs {
	MacAudioEngine::MacAudioEngine()
	{
		shutdown();
	}

	MacAudioEngine::~MacAudioEngine()
	{
		shutdown();
	}

	void MacAudioEngine::getListAudioDevices(std::vector<AudioDevice>& result, bool input)
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

	        CFStringRef deviceName = NULL;
	        proSize = sizeof(deviceName);
	        address.mSelector = kAudioDevicePropertyDeviceNameCFString;
			status = AudioHardwareServiceGetPropertyData(audioDevices[i], &address, 0, NULL, &proSize, &deviceName);
	        if (status != kAudioHardwareNoError) {
	        	printf("AudioHardwareServiceGetPropertyData (kAudioDevicePropertyDeviceName) failed: %i\n", status);
	        	continue;
	        }

	        std::string strDeviceName = std::string(CFStringGetCStringPtr(deviceName, kCFStringEncodingMacRoman));

	        CFStringRef deviceManufacturer = NULL;
	        proSize = sizeof(deviceManufacturer);
	        address.mSelector = kAudioDevicePropertyDeviceManufacturerCFString;
	        status = AudioHardwareServiceGetPropertyData(audioDevices[i], &address, 0, NULL, &proSize, &deviceManufacturer);
	        if (kAudioHardwareNoError != status) {
	            printf("AudioHardwareServiceGetPropertyData (kAudioDevicePropertyDeviceManufacturerCFString) failed: %i\n", status);
	            continue;
	        }
	        std::string strDeviceManu = std::string(CFStringGetCStringPtr(deviceManufacturer, kCFStringEncodingMacRoman));

	        //Get channel
	        int totalChannels = 0;
	        proSize = 0;
	        address.mSelector = kAudioDevicePropertyStreamConfiguration;
            address.mScope = input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
            
	        status = AudioHardwareServiceGetPropertyDataSize(audioDevices[i], &address, 0, NULL, &proSize);
	        if (kAudioHardwareNoError != status) {
	            printf("AudioObjectGetPropertyDataSize (kAudioDevicePropertyStreamConfiguration) failed: %i\n", status);
	            continue;
	        }
            
	        AudioBufferList *bufferList = new AudioBufferList[proSize];
	        status = AudioHardwareServiceGetPropertyData(audioDevices[i], &address, 0, NULL, &proSize, bufferList);
	        if (kAudioHardwareNoError != status || 0 == bufferList->mNumberBuffers) {
	            if (kAudioHardwareNoError != status)
	                printf("AudioHardwareServiceGetPropertyData (kAudioDevicePropertyStreamConfiguration) failed: %i\n", status);
	            delete [] bufferList;
	            bufferList = NULL;
	        }
	        else
	        {        	
	        	UInt32 numBuffers = bufferList->mNumberBuffers;
	        	
	        	for (int j = 0; j < numBuffers; ++j)
	        	{
	        		totalChannels += bufferList->mBuffers[j].mNumberChannels;
	        	}

	        	delete [] bufferList;
	        }
            
            if (totalChannels <= 0)
                continue;

	        //Get sample rate
	        Float64 sampleRate = 0.0;
	        proSize = sizeof(Float64);
	        address.mSelector = kAudioDevicePropertyNominalSampleRate;
	        status = AudioHardwareServiceGetPropertyData(audioDevices[i], &address, 0, NULL, &proSize, &sampleRate);
	        if (kAudioHardwareNoError != status) {
	            printf("AudioObjectGetPropertyDataSize (kAudioDevicePropertyNominalSampleRate) failed: %i\n", status);
	            continue;
	        }

	        AudioDevice aDevice(audioDevices[i], strDeviceName, strDeviceManu, safetyOffset, bufferSizeFrames, totalChannels, sampleRate);
	        m_listAudioDevices.push_back(aDevice);
	    }

	    result = m_listAudioDevices;
	}

	int MacAudioEngine::init()
	{
		
		return 0;
	}

	int MacAudioEngine::addNewCapture(uint32_t deviceid, void* user)
	{
		AudioDevice device;
		if (getDeviceByID(deviceid, device) < 0)
		{
			printf("The device is not found\n");
			return -1;
		}

		MacAudioCapture* capture = new MacAudioCapture(device);
		capture->callbackAudio = callbackAudio;
		capture->user = user;
		if (capture->init() < 0)
		{
			printf("Can not init the audio capture device\n");
			delete capture;
			return -1;
		}
		if (capture->start() < 0)
		{
			printf("Can not start the audio capture\n");
			delete capture;
			return -1;
		}

		AudioActiveSource* psrc = addSource(deviceid, device.getSampleRate());
		if (psrc == NULL)
		{
			printf("Can not add the audio device\n");
			return -1;
		}

		psrc->capture = capture;
		return 0;
	}	

	int MacAudioEngine::shutdown()
	{
		for (unsigned i = 0; i < m_activeSources.size(); ++i) {
			AudioActiveSource source = m_activeSources.back();
			MacAudioCapture* capture = (MacAudioCapture*)source.capture;
			delete capture;
			m_activeSources.pop_back();
		}
		return 0;
	}	
} // oppvs
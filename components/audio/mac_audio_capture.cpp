#include "mac_audio_capture.hpp"

namespace oppvs {
	int MacAudioCapture::init()
	{
		if (createAudioOutputUnit() < 0)
			return -1;
		enableIO();

		if (setInputDevice(m_device.getDeviceID()) < 0)
			return -1;
		return 0;
	}

	int MacAudioCapture::createAudioOutputUnit()
	{
		//Create an AudioOutputUnit
		AudioComponent component;
	    AudioComponentDescription description;
	    
	    description.componentType = kAudioUnitType_Output;
	    description.componentSubType = kAudioUnitSubType_HALOutput;
	    
	    description.componentManufacturer = kAudioUnitManufacturer_Apple;
	    description.componentFlags = 0;
	    description.componentFlagsMask = 0;
	    
	    component = AudioComponentFindNext(NULL, &description);
	    if (component == NULL)
	    {
	        printf("Can not find audio component\n");
	        return -1;
	    }
	    AudioComponentInstanceNew(component, &m_auHAL);
		return 0;	
	}

	void MacAudioCapture::enableIO()
	{
		UInt32 enableIO;
	    enableIO = 1;
	    AudioUnitSetProperty(m_auHAL, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &enableIO, sizeof(enableIO));
	    enableIO = 0;
	    AudioUnitSetProperty(m_auHAL, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enableIO, sizeof(enableIO));
	}

	int MacAudioCapture::setInputDevice(AudioDeviceID deviceid)
	{
		UInt32 size;
		OSStatus err = noErr;
		size = sizeof(AudioDeviceID);
		err = AudioUnitSetProperty(m_auHAL, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &deviceid, sizeof(deviceid));
		if (err)
			return -1;
		return 0;
	}

	OSStatus MacAudioCapture::AudioInputProc(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData)
	{
		return noErr;
	}

	void MacAudioCapture::setupCallback()
	{
		AURenderCallbackStruct input;
		input.inputProc = AudioInputProc;
		input.inputProcRefCon = 0;
	}
} // oppvs
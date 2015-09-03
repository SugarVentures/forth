#include "mac_audio_capture.hpp"

namespace oppvs {
	int MacAudioCapture::init()
	{
		if (createAudioOutputUnit(&m_auHAL) < 0)
			return -1;
		enableIO(m_auHAL);
	}

	int MacAudioCapture::createAudioOutputUnit(AudioComponentInstance* pinstance)
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
	    AudioComponentInstanceNew(component, pinstance);
		return 0;	
	}

	void MacAudioCapture::enableIO(AudioComponentInstance& instance)
	{
		UInt32 enableIO;
	    enableIO = 1;
	    AudioUnitSetProperty(instance, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &enableIO, sizeof(enableIO));
	    enableIO = 0;
	    AudioUnitSetProperty(instance, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enableIO, sizeof(enableIO));
	}
} // oppvs
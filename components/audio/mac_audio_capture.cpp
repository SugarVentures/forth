#include "mac_audio_capture.hpp"
#include "mac_audio_tool.hpp"

namespace oppvs {
	MacAudioCapture::~MacAudioCapture()
	{
		stop();
		destroyAudioBufferList(m_audioBuffer);
		if (m_auHAL)
			AudioComponentInstanceDispose(m_auHAL);
	}

	int MacAudioCapture::init()
	{
		if (createAudioOutputUnit() < 0)
			return -1;
		enableIO();

		if (setInputDevice(m_device.getDeviceID()) < 0)
			return -1;
		setupCallback();
		OSStatus err = noErr;
	    err = AudioUnitInitialize(m_auHAL);
	    if (err)
	    	return -1;

		if (setupBuffer() < 0)
			return -1;
		return 0;
	}

	int MacAudioCapture::start()
	{
		OSStatus err = noErr;
	    err = AudioOutputUnitStart(m_auHAL);
	   	if (err)
	   		return -1;
		return 0;
	}

	int MacAudioCapture::stop()
	{
		if (m_auHAL != NULL)
		{
			OSStatus err = AudioOutputUnitStop(m_auHAL);
			if (err)
				return -1;
		}
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
		MacAudioCapture* capture = (MacAudioCapture*)inRefCon;
		OSStatus err = AudioUnitRender(capture->m_auHAL, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, capture->m_audioBuffer);
		if (err)
		{
			printf("AudioUnitRender failed with error %i\n", err);
			return err;
		}
		//Convert to GenericAudioBufferList
		convertABLToGenericABL(capture->m_audioBuffer, &capture->m_callbackBuffer);
		capture->m_callbackBuffer.nFrames = inNumberFrames;
		capture->m_callbackBuffer.sampleTime = inTimeStamp->mSampleTime;
		capture->callbackAudio(capture->m_callbackBuffer);
		return noErr;
	}

	void MacAudioCapture::setupCallback()
	{
		AURenderCallbackStruct input;
		input.inputProc = AudioInputProc;
		input.inputProcRefCon = this;

    	AudioUnitSetProperty(m_auHAL, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &input, sizeof(input));
	}

	int MacAudioCapture::setupBuffer()
	{
		CAStreamBasicDescription deviceFormat;
		UInt32 bufferSizeFrames, propsize;		
    	UInt32 size = sizeof(CAStreamBasicDescription);

    	//Get the size of the IO buffers
    	propsize = sizeof(bufferSizeFrames);
    	OSStatus err = AudioUnitGetProperty(m_auHAL, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &bufferSizeFrames, &propsize);
		if (err)
			return -1;
 
	     //Get the input device format
	    err = AudioUnitGetProperty (m_auHAL, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 1, &deviceFormat, &size);
	    if (err)
	    	return -1;
	    printf("Input Device Format: \n");
	    deviceFormat.Print();
	    //Get the output format
	    err = AudioUnitGetProperty (m_auHAL, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &m_outputFormat, &size);
	 	if (err)
	 		return -1;

	    //set the desired format to the device's sample rate
	    m_outputFormat.mSampleRate =  deviceFormat.mSampleRate;
	 
	     //set format to output scope
	    AudioUnitSetProperty(m_auHAL, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &m_outputFormat, sizeof(CAStreamBasicDescription));
	    
	    printf("Output Format: \n");
	    m_outputFormat.Print();
	    printf("Bits Per Channel: %d Bytes Per Frame: %d Frame Per Packet: %d\n", m_outputFormat.mBitsPerChannel, m_outputFormat.mBytesPerFrame, 
	    	m_outputFormat.mFramesPerPacket);

		m_audioBuffer = allocateAudioBufferListWithNumChannels(m_outputFormat.mChannelsPerFrame, bufferSizeFrames * m_outputFormat.mBytesPerFrame);
		if (m_audioBuffer == NULL)
		{
			printf("Cannot allocate buffer for audio capture\n");
			return -1;
		}
		return 0;
	}

	
} // oppvs
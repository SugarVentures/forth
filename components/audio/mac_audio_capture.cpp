#include "mac_audio_capture.hpp"
#include "mac_audio_tool.hpp"

namespace oppvs {
    MacAudioCapture::MacAudioCapture(const AudioDevice& device) : AudioCapture(device), m_bufferList(NULL),
    m_sampleRatio(0.0), m_bufferSize(0), m_bufferData(NULL)
    {
        m_convertSampleRate = 48000;
    }
    
	MacAudioCapture::~MacAudioCapture()
	{
		stop();
		if (m_auHAL)
			AudioComponentInstanceDispose(m_auHAL);
        destroyAudioBufferList(m_bufferList);
	}

	int MacAudioCapture::init()
	{
		m_callbackBuffer.user = user;
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

		m_streamFormat = m_deviceFormat;

        m_streamFormat.mSampleRate = m_convertSampleRate;
        m_streamFormat.mFormatID = kAudioFormatLinearPCM;
        m_streamFormat.mBitsPerChannel = 32;
        m_streamFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        m_streamFormat.mFramesPerPacket = 1;
        m_streamFormat.mChannelsPerFrame = 2;
        m_streamFormat.mBytesPerFrame = sizeof(Float32) * m_streamFormat.mChannelsPerFrame;
        m_streamFormat.mBytesPerPacket = m_streamFormat.mFramesPerPacket * m_streamFormat.mBytesPerFrame;
        
		if (m_resampler.init(m_deviceFormat, m_streamFormat) < 0)
            return -1;
        err = configureOutputFile(m_streamFormat);
        if (err)
            printf("Cannot create output file\n");

        writeCookie(m_resampler.getConverter(), fOutputAudioFile);
        
        m_deviceFormat.Print();
        printf("Output capture format: %d %d %d %d %d\n", m_deviceFormat.mBitsPerChannel, m_deviceFormat.mBytesPerFrame, m_deviceFormat.mChannelsPerFrame, m_deviceFormat.mBytesPerPacket, m_deviceFormat.mFramesPerPacket);
        
        m_streamFormat.Print();
        printf("Output file format: %d %d %d %d %d\n", m_streamFormat.mBitsPerChannel, m_streamFormat.mBytesPerFrame, m_streamFormat.mChannelsPerFrame, m_streamFormat.mBytesPerPacket, m_streamFormat.mFramesPerPacket);
        
        //Setup Stream Buffer
        m_streamBufferSize = 32768;
        m_streamBuffer = new char[m_streamBufferSize];
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
        writeCookie(m_resampler.getConverter(), fOutputAudioFile);
		AudioFileClose(fOutputAudioFile);
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

    //Callback to feed audio input buffer
	OSStatus MacAudioCapture::AudioInputProc(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData)
	{
        //printf("Audio Capture: %f %d %lld\n", inTimeStamp->mSampleTime, inNumberFrames, inTimeStamp->mHostTime);
        //No record data then returns
        if (inNumberFrames == 0)
            return noErr;
     
        MacAudioCapture* capture = (MacAudioCapture*)inRefCon;
        OSStatus err = noErr;
        
		err = AudioUnitRender(capture->m_auHAL, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, capture->m_bufferList);
        checkResult(err, "AudioUnitRender failed");
        
        //Setup buffer for converter
        AudioBufferList convertBufferList;
        convertBufferList.mNumberBuffers = 1;
        convertBufferList.mBuffers[0].mNumberChannels = capture->m_deviceFormat.mChannelsPerFrame;
        convertBufferList.mBuffers[0].mDataByteSize = capture->m_streamBufferSize;
        convertBufferList.mBuffers[0].mData = capture->m_streamBuffer;
        memset(convertBufferList.mBuffers[0].mData, 0, capture->m_streamBufferSize);
        
        //Convert data
        UInt32 ioOutputDataPackets = 0;
        capture->m_pos = 0;
        err = capture->m_resampler.convert(MacAudioCapture::EncoderDataProc, capture, &ioOutputDataPackets, &convertBufferList, inNumberFrames, capture->m_streamFormat.mBytesPerPacket);
        checkResult(err, "AudioConverter");
        
        if (ioOutputDataPackets == 0)
        {
            //EOF condition;
            return noErr;
        }
        
        capture->m_callbackBuffer.nFrames = ioOutputDataPackets;
        capture->m_callbackBuffer.sampleTime = inTimeStamp->mSampleTime;
        
        convertABLToGenericABL(&convertBufferList, &capture->m_callbackBuffer);
        capture->callbackAudio(capture->m_callbackBuffer);
        
		return noErr;
	}

	OSStatus MacAudioCapture::EncoderDataProc(AudioConverterRef			inAudioConverter,
										UInt32*							ioNumberDataPackets,
										AudioBufferList*				ioData,
										AudioStreamPacketDescription**	outDataPacketDescription,
										void*							inUserData)
	{
		OSStatus err = noErr;
		MacAudioCapture* capture = (MacAudioCapture*)inUserData;
        //printf("Encoder no data packets before: %d\n", *ioNumberDataPackets);
        UInt32 numPacketsPerRead = capture->m_bufferList->mBuffers[0].mDataByteSize / capture->m_deviceFormat.mBytesPerPacket;
        
        if (capture->m_pos >= capture->m_bufferList->mBuffers[0].mDataByteSize)
        {
            *ioNumberDataPackets = 0;
            ioData->mBuffers[0].mData = NULL;
            ioData->mBuffers[0].mDataByteSize = 0;
            return err;
        }
        
        if (*ioNumberDataPackets > numPacketsPerRead)
            *ioNumberDataPackets = numPacketsPerRead;
        
        //printf("Encoder no data packets after: %d\n", *ioNumberDataPackets);
        
        ioData->mBuffers[0].mData = (uint8_t*)capture->m_bufferList->mBuffers[0].mData + capture->m_pos;
        ioData->mBuffers[0].mDataByteSize = *ioNumberDataPackets * capture->m_deviceFormat.mBytesPerPacket;
        
        capture->m_pos += *ioNumberDataPackets * capture->m_deviceFormat.mBytesPerPacket;

		return err;
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

        //Get the output format
	    err = AudioUnitGetProperty (m_auHAL, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &m_deviceFormat, &size);
	 	if (err)
	 		return -1;

        m_deviceFormat = deviceFormat;
        
        //set format to output scope
	    AudioUnitSetProperty(m_auHAL, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &m_deviceFormat, sizeof(CAStreamBasicDescription));
	    
        m_bufferList = allocateAudioBufferListWithNumChannels(m_deviceFormat.mChannelsPerFrame, bufferSizeFrames * m_deviceFormat.mBytesPerFrame);
        
        if (m_bufferList == NULL)
            return -1;
        
		UInt32 numFrames = 1500;
		UInt32 dataSize = sizeof(numFrames);
		 
		AudioUnitSetProperty(m_auHAL, kAudioUnitProperty_MaximumFramesPerSlice,
		    kAudioUnitScope_Global, 0, &numFrames, sizeof(numFrames));
		 
		// the AUConverter will scale up
		AudioUnitGetProperty(m_auHAL, kAudioUnitProperty_MaximumFramesPerSlice,
		    kAudioUnitScope_Global, 0, &numFrames, &dataSize);

        
		return 0;
	}

	OSStatus MacAudioCapture::configureOutputFile(CAStreamBasicDescription& sformat)
	{
		OSStatus err = noErr;
		CFURLRef destinationURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
		                                                        CFSTR("/Users/caominhtrang/Desktop/testaudio.wav"),
		                                                        kCFURLPOSIXPathStyle, 
		                                                        false);
	    
	    err = AudioFileCreateWithURL(destinationURL, kAudioFileCAFType, &sformat, kAudioFileFlags_EraseFile, &fOutputAudioFile);

        checkResult(err, "Can not create audio file");
		return err;
	}
    
    void MacAudioCapture::writeCookie (AudioConverterRef converter, AudioFileID outfile)
    {
        // grab the cookie from the converter and write it to the file
        UInt32 cookieSize = 0;
        OSStatus err = AudioConverterGetPropertyInfo(converter, kAudioConverterCompressionMagicCookie, &cookieSize, NULL);
        // if there is an error here, then the format doesn't have a cookie, so on we go
        if (!err && cookieSize) {
            char* cookie = new char [cookieSize];
            
            err = AudioConverterGetProperty(converter, kAudioConverterCompressionMagicCookie, &cookieSize, cookie);
            checkResult(err, "Get Cookie From AudioConverter");
            
            /*err =*/ AudioFileSetProperty (outfile, kAudioFilePropertyMagicCookieData, cookieSize, cookie);
            // even though some formats have cookies, some files don't take them, so we ignore the error
            delete [] cookie;
        }
    }


	
} // oppvs
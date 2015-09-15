#include "mac_audio_capture.hpp"
#include "mac_audio_tool.hpp"

namespace oppvs {
    MacAudioCapture::MacAudioCapture(const AudioDevice& device) : AudioCapture(device), m_audioBuffer(NULL),
    m_sampleRatio(0.0), m_bufferSize(0), m_bufferData(NULL)
    {
        
    }
    
	MacAudioCapture::~MacAudioCapture()
	{
		stop();
		destroyAudioBufferList(m_audioBuffer);
		if (m_auHAL)
			AudioComponentInstanceDispose(m_auHAL);
        delete m_ringBuffer;
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

		m_streamFormat = m_deviceFormat;

		err = ConfigureOutputFile(m_streamFormat);
		if (err)
			printf("Cannot create output file\n");

        m_streamFormat.mSampleRate = 48000;
        
		m_resampler.init(m_deviceFormat, m_streamFormat);
    
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

	OSStatus MacAudioCapture::AudioInputProc(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData)
	{
		MacAudioCapture* capture = (MacAudioCapture*)inRefCon;
		printf("sample time: %f frames: %d\n", inTimeStamp->mSampleTime, inNumberFrames);
        //No record data then returns
        if (inNumberFrames == 0)
            return noErr;
        
		OSStatus err = AudioUnitRender(capture->m_auHAL, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, capture->m_bufferList);
		if (err)
		{
			printf("AudioUnitRender failed with error %i\n", err);
			return err;
		}
        
        err = capture->m_ringBuffer->Store(capture->m_bufferList, inNumberFrames, inTimeStamp->mSampleTime);
        if (err)
        {
            printf("Cannot store recording audio data to ring buffer %d\n", err);
            return err;
        }
        
        if (capture->m_rPos == 0)
            capture->m_rPos = inTimeStamp->mSampleTime;
        printf("Pos in inputproc: %f\n", capture->m_rPos);
        
        UInt32 ioNumberDataPackets = 512;
        UInt32 wroteDataPackets = 0;
        capture->m_pos = 0;
        while (ioNumberDataPackets)
        {
            wroteDataPackets = inNumberFrames;
            if (capture->m_resampler.convert(MacAudioCapture::EncoderDataProc, capture, &wroteDataPackets, capture->m_resampler.getBuffer()) == 0)
            {
               // printf("data size: %d no packets: %d frames: %d\n", capture->m_resampler.getBuffer()->mBuffers[0].mDataByteSize, ioNumberDataPackets, capture->m_callbackBuffer.nFrames);
                //ioNumberDataPackets = 512;
                UInt32 noWriteBytes = ioNumberDataPackets * 4;
                err = AudioFileWriteBytes(capture->fOutputAudioFile, false, capture->m_totalPos, &noWriteBytes, capture->m_resampler.getBuffer()->mBuffers[0].mData);
                //err = AudioFileWriteBytes(capture->fOutputAudioFile, false, capture->m_totalPos, &noWriteBytes, capture->m_bufferList->mBuffers[0].mData);
                if (err)
                {
                    printf("Faile to write to file\n");
                }
                else
                {
                    printf("Wrote %d bytes\n", noWriteBytes);
                    capture->m_totalPos += noWriteBytes;
                    //err = AudioFileWriteBytes(capture->fOutputAudioFile, false, capture->m_totalPos, &noWriteBytes, capture->m_resampler.getBuffer()->mBuffers[1].mData);
                    //err = AudioFileWriteBytes(capture->fOutputAudioFile, false, capture->m_totalPos, &noWriteBytes, capture->m_bufferList->mBuffers[1].mData);
                    //if (!err)
                    //    capture->m_totalPos += noWriteBytes;
                }
                capture->m_callbackBuffer.nFrames = capture->m_resampler.getBuffer()->mBuffers[0].mDataByteSize / 4;
                capture->m_callbackBuffer.sampleTime = inTimeStamp->mSampleTime;
                
                convertABLToGenericABL(capture->m_resampler.getBuffer(), &capture->m_callbackBuffer);
                //capture->callbackAudio(capture->m_callbackBuffer);
                ioNumberDataPackets -= wroteDataPackets;
            }
            else
                break;
        }
        
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
        printf("Encoder no data packets: %d\n", *ioNumberDataPackets);

		if (*ioNumberDataPackets > 512)
			*ioNumberDataPackets = 512;
        

        printf("m_pos: %d \n", capture->m_pos);
		if (capture->m_pos >= 2048)
		{
			ioData->mBuffers[0].mData = NULL;
			ioData->mBuffers[1].mData = NULL;
			ioData->mBuffers[0].mDataByteSize = 0;
			ioData->mBuffers[1].mDataByteSize = 0;
            *ioNumberDataPackets = 0;
            err = noErr;
		}
		else	
		{
            ioData->mNumberBuffers = 2;
            ioData->mBuffers[0].mNumberChannels = 1;
            ioData->mBuffers[1].mNumberChannels = 1;
            
            printf("Pos to fetch: %f\n", capture->m_rPos);
            err = capture->m_ringBuffer->Fetch(capture->m_tempBuffer, *ioNumberDataPackets, capture->m_rPos);
            if (err)
            {
                printf("Cannot get audio data from ring buffer for converter %d\n", err);
                return err;
            }
            
            ioData->mBuffers[0].mData = (uint8_t*)capture->m_tempBuffer->mBuffers[0].mData + capture->m_pos;
            ioData->mBuffers[1].mData = (uint8_t*)capture->m_tempBuffer->mBuffers[1].mData + capture->m_pos;
            ioData->mBuffers[0].mDataByteSize = (*ioNumberDataPackets) * 4;
            ioData->mBuffers[1].mDataByteSize = (*ioNumberDataPackets) * 4;
            capture->m_pos += (*ioNumberDataPackets) * 4;
            capture->m_rPos += *ioNumberDataPackets;
		}
		

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

	    //set the desired format to the device's sample rate
	    m_deviceFormat.mSampleRate =  deviceFormat.mSampleRate;
	 
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

        //Setup ring buffer
        m_ringBuffer = new CARingBuffer();
        m_ringBuffer->Allocate(2, m_deviceFormat.mBytesPerFrame, bufferSizeFrames * 20);
        m_rPos = 0;
        
        m_tempBuffer = allocateAudioBufferListWithNumChannels(m_bufferList->mNumberBuffers, m_bufferList->mBuffers[0].mDataByteSize);

		return 0;
	}

	OSStatus MacAudioCapture::ConfigureOutputFile(CAStreamBasicDescription& sformat)
	{
		OSStatus err = noErr;
        CAStreamBasicDescription format;
		format.mSampleRate = 48000;
		format.mFormatID = kAudioFormatLinearPCM;
		format.mBitsPerChannel = 32;
		format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
		format.mFramesPerPacket = 1;
        format.mChannelsPerFrame = 2;
		format.mBytesPerFrame = 8;
		format.mBytesPerPacket = 8;

		m_totalPos = 0;
		CFURLRef destinationURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
		                                                        CFSTR("/Users/caominhtrang/Desktop/testaudio.wav"),
		                                                        kCFURLPOSIXPathStyle, 
		                                                        false);
	    
	    err = AudioFileCreateWithURL(destinationURL, kAudioFileWAVEType, &format, kAudioFileFlags_EraseFile, &fOutputAudioFile);

		if(err != noErr)
		{
			char formatID[5];
			*(UInt32 *)formatID = CFSwapInt32HostToBig(err);
			formatID[4] = '\0';
			fprintf(stderr, "ExtAudioFileSetProperty FAILED! '%-4.4s'\n", formatID);
			return err;
		}


		return err;
	}

	
} // oppvs
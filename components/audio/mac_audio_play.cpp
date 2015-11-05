#include "mac_audio_play.hpp"
#include "mac_audio_tool.hpp"

namespace oppvs {
	MacAudioPlay::MacAudioPlay()
	{

	}

	MacAudioPlay::MacAudioPlay(const AudioDevice& device, uint64_t isr, uint32_t inc) : AudioPlay(device, isr, inc)
	{
		m_firstInputTime = -1;
		m_firstOutputTime = -1;
	}

	MacAudioPlay::~MacAudioPlay()
	{
		cleanup();
	}

	int MacAudioPlay::init()
	{
		OSStatus err = noErr;
		err = setupGraph(m_device.getDeviceID());
		if (err)
			return -1;

		printf("Init audio player successfully\n");
		err = setupBuffer();
		if (err)
			return -1;
        
		err = AUGraphConnectNodeInput(m_graph, m_varispeedNode, 0, m_outputNode, 0);
        checkResult(err, "Can not connect varispeed to output node");
		if (err)
			return -1;
        
        AUGraphUpdate(m_graph, NULL);
		err = AUGraphInitialize(m_graph);
		if (err)
			return -1;


		return 0;
	}

	void MacAudioPlay::cleanup()
	{
		stop();
        if (m_converter)
            AudioConverterDispose(m_converter);

		AUGraphClose(m_graph);
		DisposeAUGraph(m_graph);
        
        delete [] m_encoderBuffer;
	}

	bool MacAudioPlay::isRunning()
	{
		OSStatus err = noErr;
		Boolean graphRunning = false;
		if (m_graph)
		{
			err = AUGraphIsRunning(m_graph, &graphRunning);
			checkErr(err);
			m_firstInputTime = -1;
		}

		return graphRunning;
	}

	int MacAudioPlay::start()
	{
		OSStatus err = noErr;
		if (!isRunning()) {
			//Start pulling for audio data
			err = AUGraphStart(m_graph);
			checkErr(err);
			m_firstInputTime = -1;
			m_firstOutputTime = -1;
		}
		return 0;
	}

	int MacAudioPlay::stop()
	{
		OSStatus err = noErr;
		if (isRunning())
		{
			err = AUGraphStop(m_graph);
			checkErr(err);
			m_firstInputTime = -1;
			m_firstOutputTime = -1;
		}
		return 0;
	}

	OSStatus MacAudioPlay::setupGraph(AudioDeviceID deviceid)
	{
		OSStatus err = noErr;
		AURenderCallbackStruct output;

		//Make a New Graph
		err = NewAUGraph(&m_graph);
		checkErr(err);
		//Open the Graph, AudioUnits are opened but not initialized    
	    err = AUGraphOpen(m_graph);
		checkErr(err);
		
		err = makeGraph();
					
		err = setOutputDevice(deviceid);
		checkErr(err);
		
		//Tell the output unit not to reset timestamps 
		//Otherwise sample rate changes will cause sync los
		UInt32 startAtZero = 0;
		err = AudioUnitSetProperty(m_outputUnit, 
								  kAudioOutputUnitProperty_StartTimestampsAtZero, 
								  kAudioUnitScope_Global,
								  0,
								  &startAtZero, 
								  sizeof(startAtZero));


		output.inputProc = OutputProc;
		output.inputProcRefCon = this;
		
		err = AudioUnitSetProperty(m_varispeedUnit,
								  kAudioUnitProperty_SetRenderCallback, 
								  kAudioUnitScope_Input,
								  0,
								  &output, 
								  sizeof(output));
		checkErr(err);
		return err;
	}

	OSStatus MacAudioPlay::makeGraph()
	{
		OSStatus err = noErr;
		AudioComponentDescription varispeedDesc, outDesc, converterDesc;
		
		//Q:Why do we need a varispeed unit?
		//A:If the input device and the output device are running at different sample rates
		//we will need to move the data coming to the graph slower/faster to avoid a pitch change.
		varispeedDesc.componentType = kAudioUnitType_FormatConverter;
		varispeedDesc.componentSubType = kAudioUnitSubType_Varispeed;
		varispeedDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
		varispeedDesc.componentFlags = 0;        
		varispeedDesc.componentFlagsMask = 0;     
	  
		outDesc.componentType = kAudioUnitType_Output;
		outDesc.componentSubType = kAudioUnitSubType_DefaultOutput;
		outDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
		outDesc.componentFlags = 0;
		outDesc.componentFlagsMask = 0;
		
        converterDesc.componentType = kAudioUnitType_FormatConverter;
        converterDesc.componentSubType = kAudioUnitSubType_AUConverter;
        converterDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
        converterDesc.componentFlags = 0;
        converterDesc.componentFlagsMask = 0;
        
		//////////////////////////
		///MAKE NODES
		//This creates a node in the graph that is an AudioUnit, using
		//the supplied ComponentDescription to find and open that unit
		err = AUGraphAddNode(m_graph, &varispeedDesc, &m_varispeedNode);
		checkErr(err);
		err = AUGraphAddNode(m_graph, &outDesc, &m_outputNode);
		checkErr(err);
        
		//Get Audio Units from AUGraph node
		err = AUGraphNodeInfo(m_graph, m_varispeedNode, NULL, &m_varispeedUnit);   
		checkErr(err);
		err = AUGraphNodeInfo(m_graph, m_outputNode, NULL, &m_outputUnit);   
		checkErr(err);
		
		// don't connect nodes until the varispeed unit has input and output formats set

		return err;

	}

	OSStatus MacAudioPlay::setOutputDevice(AudioDeviceID deviceid)
	{
		UInt32 size = sizeof(AudioDeviceID);;
	    OSStatus err = noErr;
#ifndef FORTH_IOS
	    AudioObjectPropertyAddress theAddress = { kAudioHardwarePropertyDefaultOutputDevice,
	                                              kAudioObjectPropertyScopeGlobal,
	                                              kAudioObjectPropertyElementMaster };
		
		if (deviceid == kAudioDeviceUnknown) //Retrieve the default output device
		{
			err = AudioHardwareServiceGetPropertyData(kAudioObjectSystemObject, &theAddress, 0, NULL, &size, &deviceid);
	        checkErr(err);
		}

		//Set the Current Device to the Default Output Unit.
	    err = AudioUnitSetProperty(m_outputUnit,
								  kAudioOutputUnitProperty_CurrentDevice, 
								  kAudioUnitScope_Global, 
								  0, 
								  &deviceid, 
								  sizeof(deviceid));
#endif
		return err;
	}

	OSStatus MacAudioPlay::OutputProc(void *inRefCon,
							 AudioUnitRenderActionFlags *ioActionFlags,
							 const AudioTimeStamp *TimeStamp,
							 UInt32 inBusNumber,
							 UInt32 inNumberFrames,
							 AudioBufferList * ioData)
	{
		OSStatus err = noErr;
		MacAudioPlay* player = (MacAudioPlay*)inRefCon;
        double rate = 48000.0/44100.0;
		//printf("AudioPlay: sample time: %f Frames: %d at %f\n", TimeStamp->mSampleTime, inNumberFrames, CFAbsoluteTimeGetCurrent());
        
		if (player->getFirstInputTime() < 0.)
		{
			//No input
			makeBufferSilent(ioData);
			return err;
		}
        
		err = AudioUnitSetParameter(player->m_varispeedUnit, kVarispeedParam_PlaybackRate, kAudioUnitScope_Global, 0, rate, 0);
		checkErr(err);
		
		if (player->m_ringBuffer)
		{
            
			if (player->m_firstOutputTime < 0.0)
			{
				player->m_firstOutputTime = TimeStamp->mSampleTime;
                double delta = player->m_firstInputTime - player->m_firstOutputTime;
				//printf("in time: %f out time: %f delta: %f\n", player->m_firstInputTime, player->m_firstOutputTime, delta);

				//player->m_offset = 1175.0; //Fix for now
                player->m_offset = 0.0;
				if (delta < 0.0)		
					player->m_offset -= delta;
				else
					player->m_offset = -delta + player->m_offset;
				makeBufferSilent(ioData);
				return noErr;
			}

            //If the number of packets in buffer is too small, then don't fetch data
            if (player->m_ringBuffer->getNumberFrames() < 3 * inNumberFrames)
            {
                makeBufferSilent(ioData);
                return noErr;
            }
            UInt32 ioNumberDataPackets = 0;
            player->m_currentSampleTime = TimeStamp->mSampleTime;
            err = player->m_resampler.convert(MacAudioPlay::EncoderDataProc, player, &ioNumberDataPackets, ioData, inNumberFrames, player->m_variFormat.mBytesPerPacket, true);

            //printf("after convert io pkt %d %d\n", ioNumberDataPackets, ioData->mBuffers[0].mDataByteSize);
            if (err || ioNumberDataPackets == 0)
            {
                printf("Error in convert\n");
                makeBufferSilent(ioData);
                return noErr;
            }
		}
		return err;
	}

	OSStatus MacAudioPlay::setupBuffer()
	{
		OSStatus err = noErr;
		CAStreamBasicDescription outputFormat, variFormat, convertFormat;

		UInt32 propertySize = sizeof(CAStreamBasicDescription);
		err = AudioUnitGetProperty(m_outputUnit, kAudioUnitProperty_StreamFormat, 
			kAudioUnitScope_Output, 0, &outputFormat, &propertySize);
		checkErr(err);
        
        err = AudioUnitGetProperty(m_varispeedUnit, kAudioUnitProperty_StreamFormat,
                                   kAudioUnitScope_Input, 0, &variFormat, &propertySize);
        checkResult(err, "AudioUnitGetProperty: m_varispeedUnit");
    
        convertFormat.mFormatID = kAudioFormatLinearPCM;
        convertFormat.mSampleRate = m_inputSampleRate;
        convertFormat.mBitsPerChannel = 32;
        convertFormat.mChannelsPerFrame = 2;
        convertFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        convertFormat.mFramesPerPacket = 1;
        convertFormat.mBytesPerFrame = sizeof(Float32) * convertFormat.mChannelsPerFrame;
        convertFormat.mBytesPerPacket = convertFormat.mFramesPerPacket * convertFormat.mBytesPerFrame;
        
        //Get the lower number of channels
        variFormat.mChannelsPerFrame = (outputFormat.mChannelsPerFrame < m_inputNumChannels) ? outputFormat.mChannelsPerFrame : m_inputNumChannels;
        variFormat.mSampleRate = 44100.0;
        
		err = AudioUnitSetProperty(m_varispeedUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &variFormat, propertySize);
        checkResult(err, "AudioUnitSetProperty: m_varispeedUnit");

		//Set the correct sample rate for the output device, but keep the channel count same
		variFormat.mSampleRate = outputFormat.mSampleRate = m_inputSampleRate;
        
        //Set format for the ouput of Varispeed Node
        err = AudioUnitSetProperty(m_varispeedUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &variFormat, propertySize);
        checkErr(err);

		//Set format for the input of Output Node
		err = AudioUnitSetProperty(m_outputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &outputFormat, propertySize);
		checkErr(err);
        
        //Setup buffer for converter
        uint32_t maxEncoderBuffer = 32768; //32KB
        m_encoderBuffer = new char[maxEncoderBuffer];
        
        if (m_resampler.init(convertFormat, variFormat) < 0)
            return -1;
        m_convertFormat = convertFormat;
        m_variFormat = variFormat;
        
        UInt32 numFrames = 1500;
        UInt32 dataSize = sizeof(numFrames);
        
        AudioUnitSetProperty(m_varispeedUnit, kAudioUnitProperty_MaximumFramesPerSlice,
                             kAudioUnitScope_Global, 0, &numFrames, sizeof(numFrames));
        
        // the AUConverter will scale up
        AudioUnitGetProperty(m_varispeedUnit, kAudioUnitProperty_MaximumFramesPerSlice,
                             kAudioUnitScope_Global, 0, &numFrames, &dataSize);
        
        printFormat(m_convertFormat);
        printFormat(m_variFormat);
        
        err = configureOutputFile(m_convertFormat);
        if (err)
            printf("Cannot create output file\n");
        
		return err;
	}

	void MacAudioPlay::attachBuffer(AudioRingBuffer* pb)
	{
		m_ringBuffer = pb;
	}

	double MacAudioPlay::getFirstInputTime()
	{
		return m_firstInputTime;
	}

	void MacAudioPlay::setFirstInputTime(double itime)
	{
		m_firstInputTime = itime;
	}
    
    OSStatus MacAudioPlay::EncoderDataProc(AudioConverterRef			inAudioConverter,
                                              UInt32*							ioNumberDataPackets,
                                              AudioBufferList*				ioData,
                                              AudioStreamPacketDescription**	outDataPacketDescription,
                                              void*							inUserData)
    {
        OSStatus err = noErr;
        MacAudioPlay* player = (MacAudioPlay*)inUserData;
        
        ioData->mBuffers[0].mData = NULL;
        ioData->mBuffers[0].mDataByteSize = 0;
        
        //uint64_t timeStamp = player->m_currentSampleTime - player->m_offset;
        uint64_t timeStamp = player->m_ringBuffer->getStartTime();
        RingBufferError rbError = player->m_ringBuffer->fetch(*ioNumberDataPackets, player->m_encoderBuffer, timeStamp);
        
        if (rbError)
            return err;
        
        if (*ioNumberDataPackets > 0)
        {
            ioData->mBuffers[0].mData = player->m_encoderBuffer;
            ioData->mBuffers[0].mDataByteSize = *ioNumberDataPackets * player->m_convertFormat.mBytesPerPacket;
        }
        return err;
    }
    
    OSStatus MacAudioPlay::configureOutputFile(CAStreamBasicDescription& sformat)
    {
        OSStatus err = noErr;
        m_totalPos = 0;
        CFURLRef destinationURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                                                CFSTR("/Users/caominhtrang/Desktop/testaudioout.wav"),
                                                                kCFURLPOSIXPathStyle,
                                                                false);
        
        err = AudioFileCreateWithURL(destinationURL, kAudioFileCAFType, &sformat, kAudioFileFlags_EraseFile, &fOutputAudioFile);
        
        checkResult(err, "Can not create audio file");
        return err;
    }
    
    void MacAudioPlay::writeCookie (AudioConverterRef converter, AudioFileID outfile)
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
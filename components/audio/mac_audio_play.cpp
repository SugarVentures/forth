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
        
        //err = AUGraphConnectNodeInput(m_graph, m_converterNode, 0, m_varispeedNode, 0);
        //checkResult(err, "Can not connect converter to varispeed node");
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
        destroyAudioBufferList(m_inBuffer);
        destroyAudioBufferList(m_outBuffer);
		AUGraphClose(m_graph);
		DisposeAUGraph(m_graph);
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
        //err = AUGraphAddNode(m_graph, &converterDesc, &m_converterNode);
        //checkErr(err);
        
		err = AUGraphAddNode(m_graph, &varispeedDesc, &m_varispeedNode);
		checkErr(err);
		err = AUGraphAddNode(m_graph, &outDesc, &m_outputNode);
		checkErr(err);
        
		//Get Audio Units from AUGraph node
		err = AUGraphNodeInfo(m_graph, m_varispeedNode, NULL, &m_varispeedUnit);   
		checkErr(err);
		err = AUGraphNodeInfo(m_graph, m_outputNode, NULL, &m_outputUnit);   
		checkErr(err);
		
        //err = AUGraphNodeInfo(m_graph, m_converterNode, NULL, &m_converterUnit);
		// don't connect nodes until the varispeed unit has input and output formats set

		return err;

	}

	OSStatus MacAudioPlay::setOutputDevice(AudioDeviceID deviceid)
	{
		UInt32 size = sizeof(AudioDeviceID);;
	    OSStatus err = noErr;
	    	    
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
		double rate = 44100.0 / 48000.0;
        //double rate = 512.0 / 557.0;

		printf("AudioPlay: sample time: %f Frames: %d\n", TimeStamp->mSampleTime, inNumberFrames);;
		if (player->getFirstInputTime() < 0.)
		{
			//No input
			makeBufferSilent(ioData);
			return err;
		}

		err = AudioUnitSetParameter(player->m_varispeedUnit, kVarispeedParam_PlaybackRate, kAudioUnitScope_Global, 0, rate, 0);
		checkErr(err);
		
		if (player->m_buffer)
		{
			if (player->m_firstOutputTime < 0.0)
			{
				player->m_firstOutputTime = TimeStamp->mSampleTime;
				printf("in time: %f out time: %f\n", player->m_firstInputTime, player->m_firstOutputTime);
				double delta = player->m_firstInputTime - player->m_firstOutputTime;
				player->m_offset = 1175.0 - 1350; //Fix for now
				if (delta < 0.0)		
					player->m_offset -= delta;
				else
					player->m_offset = -delta + player->m_offset;
				makeBufferSilent(ioData);				
				return noErr;
			}

			printf("Delta: %f\n", TimeStamp->mSampleTime - player->m_offset);
			err = player->m_buffer->Fetch(player->m_inBuffer, inNumberFrames, SInt64(TimeStamp->mSampleTime - player->m_offset));
			/*uint8_t* data = (uint8_t*)ioData->mBuffers[0].mData;
            for (int i = 0; i < ioData->mBuffers[0].mDataByteSize; i++)
            {
                printf("%d", data[i]);
            }
            printf("\n");*/
			if (err != kCARingBufferError_OK)
			{
				printf("Error in fetch\n");
			}
            else
            {
                printf("%d\n", player->m_inBuffer->mNumberBuffers);
                //Convert
                printf("data byte size; %d %d\n", ioData->mBuffers[0].mDataByteSize, player->m_inBuffer->mBuffers[0].mDataByteSize);
                AudioConverterReset(player->m_converter);
                AudioConverterConvertComplexBuffer(player->m_converter, inNumberFrames, player->m_inBuffer, ioData);
                printf("data byte size after convert %d\n", ioData->mBuffers[0].mDataByteSize);
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
        
        //err = AudioUnitGetProperty(m_converterUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &convertFormat, &propertySize);
        //checkResult(err, "AudioUnitGetProperty: m_converterUnit");
    
        convertFormat.mFormatID = kAudioFormatLinearPCM;
        convertFormat.mSampleRate = m_inputSampleRate;
        convertFormat.mBitsPerChannel = 32;
        convertFormat.mChannelsPerFrame = 2;
        convertFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        convertFormat.mFramesPerPacket = 1;
        convertFormat.mBytesPerFrame = 8;
        convertFormat.mBytesPerPacket = 8;
        
        //Set format for the input of Converter Node
        //err = AudioUnitSetProperty(m_converterUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &convertFormat, propertySize);
        //checkResult(err, "AudioUnitSetProperty: failed to set format");
        //printFormat(convertFormat);
        
        //Get the lower number of channels
        variFormat.mChannelsPerFrame = (outputFormat.mChannelsPerFrame < m_inputNumChannels) ? outputFormat.mChannelsPerFrame : m_inputNumChannels;
        variFormat.mSampleRate = m_inputSampleRate;
        //Set format for the output of Converter Node
        //err = AudioUnitSetProperty(m_converterUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &variFormat, propertySize);
        //checkResult(err, "AudioUnitSetProperty: failed to set format");
        //printFormat(variFormat);
		//Set format for the input of Varispeed Node
        
        //Setup converter
        err = AudioConverterNew(&convertFormat, &variFormat, &m_converter);
        checkResult(err, "Failed to create converter");
        UInt32 size = sizeof(convertFormat);
        err = AudioConverterGetProperty(m_converter, kAudioConverterCurrentInputStreamDescription, &size, &convertFormat);
        if (err)
        {
            return -1;
        }
        
        size = sizeof(variFormat);
        err = AudioConverterGetProperty(m_converter, kAudioConverterCurrentOutputStreamDescription, &size, &variFormat);
        if (err)
        {
            return -1;
        }
        printFormat(convertFormat);
        printFormat(variFormat);
        
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
        //Get the size of the IO buffers
        UInt32 bufferSizeFrames = 0;
        propertySize = sizeof(bufferSizeFrames);
        err = AudioUnitGetProperty(m_outputUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Input, 0, &bufferSizeFrames, &propertySize);
        checkResult(err, "Failed to get buffer size frames");
        
        printf("buffer size frames: %d\n", bufferSizeFrames);
        m_inBuffer = allocateAudioBufferListWithNumChannels(convertFormat.mChannelsPerFrame, bufferSizeFrames * convertFormat.mBytesPerFrame);
        m_outBuffer = allocateDeinterleaveAudioBufferListWithNumChannels(variFormat.mChannelsPerFrame, bufferSizeFrames * variFormat.mBytesPerFrame);
        
        convertFormat.Print();
        variFormat.Print();
		return err;
	}

	void MacAudioPlay::attachBuffer(CARingBuffer* pb)
	{
		m_buffer = pb;
	}

	double MacAudioPlay::getFirstInputTime()
	{
		return m_firstInputTime;
	}

	void MacAudioPlay::setFirstInputTime(double itime)
	{
		m_firstInputTime = itime;
	}
} // oppvs
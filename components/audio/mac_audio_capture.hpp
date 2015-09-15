#ifndef OPPVS_MAC_AUDIO_CAPTURE_HPP
#define OPPVS_MAC_AUDIO_CAPTURE_HPP

#include "audio_capture.hpp"

#include <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreFoundation/CoreFoundation.h>
#include "mac_utility/CAStreamBasicDescription.h"
#include "mac_utility/CARingBuffer.h"
#include "mac_audio_resampler.hpp"

namespace oppvs {
	class MacAudioCapture : public AudioCapture
	{
	public:
        MacAudioCapture(const AudioDevice&);
		~MacAudioCapture();
		int init();
		int start();
		int stop();
	private:
		AudioComponentInstance m_auHAL;
		AudioBufferList* m_audioBuffer;
		UInt32 m_pos;
		CAStreamBasicDescription m_deviceFormat;    //Format of recording device
        CAStreamBasicDescription m_streamFormat;
        
		MacAudioResampler m_resampler;
        
        AudioBufferList* m_bufferList;   //Use to render audio data in recording thread
        Float64 m_sampleRatio;          //The ratio between the device and the stream sample rate
        UInt32 m_bufferSize;
        char*  m_bufferData;
        
        CARingBuffer* m_ringBuffer;     //For transferring data from the recording thread
        Float64 m_rPos;                 //Temporary pos to get data from ring buffer
        AudioBufferList* m_tempBuffer;  //Use to feed data for converter
        

		UInt32 m_totalPos;
		int createAudioOutputUnit();
		void enableIO();
		int setInputDevice(AudioDeviceID deviceid);
		void setupCallback();
		int setupBuffer();

		static OSStatus AudioInputProc(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData);

		static OSStatus EncoderDataProc(AudioConverterRef				inAudioConverter, 
										UInt32*							ioNumberDataPackets,
										AudioBufferList*				ioData,
										AudioStreamPacketDescription**	outDataPacketDescription,
										void*							inUserData);

		AudioFileID fOutputAudioFile;
		OSStatus ConfigureOutputFile(CAStreamBasicDescription& format);
	};

} // oppvs

#endif // OPPVS_MAC_AUDIO_CAPTURE_HPP

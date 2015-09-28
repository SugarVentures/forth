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
        
		UInt32 m_pos;
		CAStreamBasicDescription m_deviceFormat;    //Format of recording device
        CAStreamBasicDescription m_streamFormat;
        
		MacAudioResampler m_resampler;
        
        AudioBufferList* m_bufferList;   //Use to render audio data in recording thread
        Float64 m_sampleRatio;          //The ratio between the device and the stream sample rate
        UInt32 m_bufferSize;
        char*  m_bufferData;
                
        Float32 m_convertSampleRate;

		UInt32 m_totalPos;
        
        UInt32 m_streamBufferSize;
        char* m_streamBuffer;
        
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
		OSStatus configureOutputFile(CAStreamBasicDescription& format);
        void writeCookie (AudioConverterRef converter, AudioFileID outfile);
        
	};

} // oppvs

#endif // OPPVS_MAC_AUDIO_CAPTURE_HPP

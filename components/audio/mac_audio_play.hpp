#ifndef OPPVS_MAC_AUDIO_PLAY_HPP
#define OPPVS_MAC_AUDIO_PLAY_HPP

#include "audio_util.h"

#import <AudioToolbox/AudioToolbox.h>
#ifndef FORTH_IOS
#import <CoreAudio/CoreAudio.h>
#else
#import <CoreAudio/CoreAudioTypes.h>
#endif
#include "mac_utility/CAStreamBasicDescription.h"
#include "mac_utility/CARingBuffer.h"
#include "audio_play.hpp"
#include "mac_audio_resampler.hpp"

#include "audio_ring_buffer.h"

namespace oppvs {
	#define checkErr( err) \
	if(err) {\
		OSStatus error = static_cast<OSStatus>(err);\
		printf("MacAudioPlay Error: %ld ->  %s:  %d\n",  (long)error,\
				   __FILE__, \
				   __LINE__\
				   );\
		fflush(stdout);\
		return err; \
	}

	class MacAudioPlay : public AudioPlay
	{
	public:
		MacAudioPlay();
		MacAudioPlay(const AudioDevice&, uint64_t, uint32_t);
		~MacAudioPlay();
		int init();
		int start();
		int stop();
		void cleanup();
		void attachBuffer(AudioRingBuffer*);

		double getFirstInputTime();
		void setFirstInputTime(double);

	private:
		//AudioUnits and Graph
		AUGraph m_graph;
		AUNode m_varispeedNode;
		AudioUnit m_varispeedUnit;
		AUNode m_outputNode;
		AudioUnit m_outputUnit;
        
		CARingBuffer *m_buffer;
		double m_firstInputTime;
		double m_firstOutputTime;
		double m_offset;
        
        AudioConverterRef m_converter;  //Used to convert interleave to deinterleave
        
        MacAudioResampler m_resampler;
		CAStreamBasicDescription m_inputFormat;
        UInt32 m_pos;
        double m_currentSampleTime;
        
        AudioRingBuffer* m_ringBuffer;
        char* m_encoderBuffer;
        CAStreamBasicDescription m_convertFormat;
        CAStreamBasicDescription m_variFormat;

		OSStatus setupGraph(AudioDeviceID deviceid);
		OSStatus makeGraph();
		OSStatus setOutputDevice(AudioDeviceID deviceid);
		OSStatus setupBuffer();
		bool isRunning();

		static OSStatus OutputProc(void *inRefCon,
							 AudioUnitRenderActionFlags *ioActionFlags,
							 const AudioTimeStamp *TimeStamp,
							 UInt32 inBusNumber,
							 UInt32 inNumberFrames,
							 AudioBufferList * ioData);
        
        static OSStatus EncoderDataProc(AudioConverterRef				inAudioConverter,
                                        UInt32*							ioNumberDataPackets,
                                        AudioBufferList*				ioData,
                                        AudioStreamPacketDescription**	outDataPacketDescription,
                                        void*							inUserData);
	};

} // oppvs

#endif // OPPVS_MAC_AUDIO_PLAY_HPP

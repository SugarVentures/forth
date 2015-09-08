#ifndef OPPVS_MAC_AUDIO_PLAY_HPP
#define OPPVS_MAC_AUDIO_PLAY_HPP

#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudio/CoreAudio.h>
//#import <CoreFoundation/CoreFoundation.h>
#include "mac_utility/CAStreamBasicDescription.h"
#include "audio_play.hpp"

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
	private:
		//AudioUnits and Graph
		AUGraph m_graph;
		AUNode m_varispeedNode;
		AudioUnit m_varispeedUnit;
		AUNode m_outputNode;
		AudioUnit m_outputUnit;



		OSStatus setupGraph(AudioDeviceID deviceid);
		OSStatus makeGraph();
		OSStatus setOutputDevice(AudioDeviceID deviceid);
		OSStatus setupBuffer();

		static OSStatus OutputProc(void *inRefCon,
							 AudioUnitRenderActionFlags *ioActionFlags,
							 const AudioTimeStamp *TimeStamp,
							 UInt32 inBusNumber,
							 UInt32 inNumberFrames,
							 AudioBufferList * ioData);
	};
} // oppvs

#endif // OPPVS_MAC_AUDIO_PLAY_HPP

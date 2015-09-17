#ifndef OPPVS_MAC_AUDIO_RESAMPLER_HPP
#define OPPVS_MAC_AUDIO_RESAMPLER_HPP

#import <AudioToolbox/AudioToolbox.h>
#import <CoreFoundation/CoreFoundation.h>
#include "mac_utility/CAStreamBasicDescription.h"

namespace oppvs {
	class MacAudioResampler
	{
	public:
		MacAudioResampler();
		~MacAudioResampler();
		
		int init(CAStreamBasicDescription& inputFormat, CAStreamBasicDescription& outputFormat);
		void cleanup();
		OSStatus convert(AudioConverterComplexInputDataProc proc, void* userData, UInt32 *ioOutputDataPackets, AudioBufferList* abl, UInt32 inNumFrames, UInt32 bytesPerFrame);
        AudioConverterRef getConverter();
	private:
		AudioConverterRef m_converter;

	};
} // oppvs
	
#endif // OPPVS_MAC_AUDIO_RESAMPLER_HPP

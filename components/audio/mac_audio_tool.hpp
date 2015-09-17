#ifndef OPPVS_MAC_AUDIO_TOOL_HPP
#define OPPVS_MAC_AUDIO_TOOL_HPP

#import <AudioToolbox/AudioToolbox.h>
#include "datatypes.hpp"

namespace oppvs {
    static const OSStatus caConverterEOFDErr = 0x656F6664;
    
	AudioBufferList* allocateAudioBufferListWithNumChannels(UInt32 numChannels, UInt32 size, bool noAllocData = false);
	void destroyAudioBufferList(AudioBufferList* list, bool noAllocData = false);
	void convertGenericABLToABL(GenericAudioBufferList* gbl, AudioBufferList*& abl);
	void convertABLToGenericABL(AudioBufferList* abl, GenericAudioBufferList* gbl);
	void makeBufferSilent(AudioBufferList* ioData);
    
    void checkResult(OSStatus result, const char *operation);
} // oppvs


#endif // OPPVS_MAC_AUDIO_TOOL_HPP

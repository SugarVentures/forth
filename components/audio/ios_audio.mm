#import <AVFoundation/AVFoundation.h>
#include "ios_audio_interface.h"
#include <string>

@interface IosAudio : NSObject
{

}

@end

@implementation IosAudio {
	
}

+ (BOOL)prepareAudioSession
{
	NSError* error = nil;
	BOOL result = YES;
	AVAudioSession* session = [AVAudioSession sharedInstance];
    
    result = [session setCategory:AVAudioSessionCategoryPlayAndRecord error:&error];
    if (!result)
    {
        NSLog(@"setCategory failed");
    }
 
    result = [session setActive:YES error:&error];
    if (!result)
    {
        NSLog(@"setActive failed");
    }
    return result;
}

+ (void) getListAudioDevices: (std::vector<oppvs::AudioDevice>&) result isInput: (bool) input
{
    if ([IosAudio prepareAudioSession])
    {
        AVAudioSession* session = [AVAudioSession sharedInstance];
	    NSArray *availInputs = [session availableInputs];
	    int count = (int)[availInputs count];
        int index = 0;
	    for (int k = 0; k < count; k++) {
	        AVAudioSessionPortDescription *portDesc = [availInputs objectAtIndex:k];
            std::string strDeviceName = [portDesc.UID UTF8String];
            std::string strDeviceType = [portDesc.portType UTF8String];
            uint32_t sampleRate = [session sampleRate];
            int noChannel = (int)[session inputNumberOfChannels];
            
            AVAudioSessionDataSourceDescription* frontDataSource = nil;
		    for (AVAudioSessionDataSourceDescription* source in portDesc.dataSources)
		    {
                std::string strOrientation = [source.orientation UTF8String];
                oppvs::AudioDevice aDevice(index++, strDeviceName, strDeviceType, strOrientation, noChannel, sampleRate);
                result.push_back(aDevice);
		    } // end data source iteration
            
	   }
	}
}

void getListAudioDevices(std::vector<oppvs::AudioDevice>& result, bool input)
{
    [IosAudio getListAudioDevices: result isInput: input];
}

@end
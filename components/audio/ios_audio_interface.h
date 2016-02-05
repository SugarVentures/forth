#ifndef OPPVS_IOS_AUDIO_INTERFACE_H
#define OPPVS_IOS_AUDIO_INTERFACE_H

#include "audio_device.hpp"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

void getListAudioDevices(std::vector<oppvs::AudioDevice>& result, bool input);

#ifdef __cplusplus
}
#endif


#endif // OPPVS_IOS_AUDIO_INTERFACE_H

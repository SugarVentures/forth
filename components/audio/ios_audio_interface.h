#ifndef OPPVS_IOS_AUDIO_INTERFACE_H
#define OPPVS_IOS_AUDIO_INTERFACE_H

#include "audio_device.hpp"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

void prepareAudioSession();
void getListAudioDevices(std::vector<oppvs::AudioDevice>& result, bool input);
void setAudioCaptureDevice(const std::string& type, const std::string& orientation);

#ifdef __cplusplus
}
#endif


#endif // OPPVS_IOS_AUDIO_INTERFACE_H

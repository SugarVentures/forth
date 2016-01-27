
#pragma once

#include "ios_video_capture.hpp"

#ifdef __cplusplus
	extern "C" {
#endif

void* oppvs_vc_info_alloc();	//Allocate AV foundation VC object
int oppvs_get_list_video_sources(void* cap, std::vector<oppvs::VideoCaptureDevice>& result);

void* oppvs_vc_av_alloc();
int oppvs_setup_capture_session(void* cap, oppvs::VideoActiveSource* source);
int oppvs_start_video_recording(void* cap);
void oppvs_stop_video_recording(void* cap);
void oppvs_av_set_callback(void* cap, oppvs::frame_callback fc, void* user);   /* Set the callback function which will receive the frames. */
void oppvs_update_configuration(void* cap, const oppvs::VideoActiveSource& source);

#ifdef __cplusplus
}
#endif
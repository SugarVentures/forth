/*
	Interfaces to wrapper Objective C (AVFoundation Framework) and C++
*/

#pragma once

#include "../interface/video_capture.hpp"
#include "../error/error.hpp"

#ifdef __cplusplus
	extern "C" {
#endif

void* oppvs_vc_info_alloc();	//Allocate AV foundation VC object
int oppvs_get_list_video_sources(void* cap, std::vector<oppvs::VideoCaptureDevice>& result);
int oppvs_get_list_windows(void* cap, std::vector<oppvs::VideoScreenSource>& result);
int oppvs_get_device_id(void* cap, std::string& title);

void* oppvs_vc_av_alloc();
int oppvs_setup_capture_session(void* cap, oppvs::VideoActiveSource& source);
void oppvs_start_video_recording(void* cap);
void oppvs_stop_video_recording(void* cap);
void oppvs_av_set_callback(void* cap, oppvs::frame_callback fc, void* user);   /* Set the callback function which will receive the frames. */

void oppvs_set_window_id(void* cap, int wid);

#ifdef __cplusplus
}
#endif


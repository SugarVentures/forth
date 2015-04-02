/*
	Implement video capturing on OSX platform
*/

#include "../include/datatypes.hpp"
#include "mac_video_capture.hpp"
#include "../error/error.hpp"
#include "mac_video_capture_interface.h"

#include <stdio.h>
#include <vector>

namespace oppvs {

	MacVideoEngine::MacVideoEngine() {

	}
	
	MacVideoEngine::MacVideoEngine(frame_callback cbf, void* user) : VideoEngine(cbf, user){
		printf("Init video engine \n");
		info = oppvs_vc_info_alloc();
		cap = oppvs_vc_av_alloc();
		oppvs_av_set_callback(cap, cbf, user);

	}

	MacVideoEngine::~MacVideoEngine() {
		
	}

	void MacVideoEngine::getListCaptureDevices(std::vector<VideoCaptureDevice>& result) {
		int numDevices = oppvs_get_list_video_sources(info, result);
	}

	void MacVideoEngine::getListVideoSource(std::vector<VideoScreenSource>& result) {
		int numWindows = oppvs_get_list_windows(info, result);

	}

	void MacVideoEngine::setupCaptureSessions() {
		error_video_capture_t error;		
		error = oppvs_setup_capture_sessions(cap, active_sources);
	}

	void MacVideoEngine::startRecording() {
		oppvs_start_video_recording(cap);
	}

	void MacVideoEngine::stopRecording() {
		oppvs_stop_video_recording(cap);
	}

	int MacVideoEngine::getDeviceID(std::string& title) {
		return oppvs_get_device_id(info, title);
	}
	
}

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
		//cap = oppvs_vc_av_alloc();
		//oppvs_av_set_callback(cap, cbf, user);

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
		//error_video_capture_t error;		
		//error = oppvs_setup_capture_sessions(cap, active_sources);
		for (std::vector<VideoActiveSource>::const_iterator i = active_sources.begin(); i != active_sources.end(); ++i)
	    {
	        printf("Source: %d type: %d\n", i->video_source_id, i->video_source_type);
	        MacVideoCapture* videocap = new MacVideoCapture(callback_frame, i->user, *i);
	        videocap->setup();
	        videoCaptures.push_back(videocap);
	    }
	}

	void MacVideoEngine::startRecording() {
		for (std::vector<VideoCapture*>::const_iterator i = videoCaptures.begin(); i != videoCaptures.end(); ++i)
	    {
	        VideoCapture* vc = *i;
	        vc->start();
	    }
	}

	void MacVideoEngine::stopRecording() {
		//oppvs_stop_video_recording(cap);
		for (std::vector<VideoCapture*>::const_iterator i = videoCaptures.begin(); i != videoCaptures.end(); ++i)
	    {
	        VideoCapture* vc = *i;
	        vc->stop();
	    }
	}

	int MacVideoEngine::getDeviceID(std::string& title) {
		return oppvs_get_device_id(info, title);
	}
	

	MacVideoCapture::MacVideoCapture(frame_callback cbf, void* user, const VideoActiveSource& source): 
		VideoCapture(cbf, user, source)
	{
		printf("Init video capture\n");
		m_cap = oppvs_vc_av_alloc();
		oppvs_av_set_callback(m_cap, cbf, user);
	}

	MacVideoCapture::~MacVideoCapture()
	{

	}

	void MacVideoCapture::start()
	{
		oppvs_start_video_recording(m_cap);
	}

	void MacVideoCapture::stop()
	{
		oppvs_stop_video_recording(m_cap);
	}

	void MacVideoCapture::setup()
	{
		oppvs_setup_capture_session(m_cap, m_source);
	}
}

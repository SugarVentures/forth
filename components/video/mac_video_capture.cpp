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
	}

	MacVideoEngine::~MacVideoEngine() {
		
	}

	void MacVideoEngine::getListCaptureDevices(std::vector<VideoCaptureDevice>& result) {
		oppvs_get_list_video_sources(info, result);
	}

	void MacVideoEngine::getListVideoSource(std::vector<VideoScreenSource>& result) {
		oppvs_get_list_windows(info, result);
	}

	void MacVideoEngine::getListMonitors(std::vector<Monitor>& result)
	{
		oppvs_get_list_monitors(info, result);
	}

	void MacVideoEngine::setupCaptureSessions() {
		for (std::vector<VideoActiveSource>::iterator i = active_sources.begin(); i != active_sources.end(); ++i)
	    {
	        MacVideoCapture* videocap = new MacVideoCapture(callback_frame, i->user, &(*i));
	        videocap->setup();
	        videoCaptures.push_back(videocap);
	    }
	}

	void MacVideoEngine::setupCaptureSession(VideoActiveSource* source)
	{
		MacVideoCapture* videocap = new MacVideoCapture(callback_frame, source->user, source);
	    videocap->setup();
	    source->capture = (VideoCapture*)videocap;
	}

	void MacVideoEngine::startCaptureSession(VideoActiveSource& source)
	{
		source.capture->start();
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

	void MacVideoEngine::updateConfiguration(const VideoActiveSource& source) {
		for (std::vector<VideoCapture*>::const_iterator i = videoCaptures.begin(); i != videoCaptures.end(); ++i)
	    {
	        VideoCapture* vc = *i;
	        if (*vc->getSource() == source)
	        	vc->updateConfiguration(source);
	    }
	}
	

	MacVideoCapture::MacVideoCapture(frame_callback cbf, void* user, VideoActiveSource* source): 
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

	void MacVideoCapture::updateConfiguration(const VideoActiveSource& source)
	{
		if (*m_source != source)
			return;
		*m_source = source;
		//oppvs_update_configuration(m_cap, *m_source);
	}

	
}

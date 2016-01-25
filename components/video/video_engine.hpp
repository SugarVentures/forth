/*
	video_capture.h
	Common Video Capture Interface
*/

#ifndef OPPVS_VIDEO_ENGINE_HPP
#define OPPVS_VIDEO_ENGINE_HPP

#include <string>
#include <vector>
#include <algorithm>

#include "datatypes.hpp"
#include "video_capture.hpp"

namespace oppvs {
	

	class VideoEngine {
	public:
		VideoEngine() {
			
		}
		VideoEngine(frame_callback cbf, void* user) : callback_frame(cbf), callback_user(user) {
			
		}
		virtual ~VideoEngine() {}
		//Get list of video capture devices: webcam, external devices
		virtual void getListCaptureDevices(std::vector<VideoCaptureDevice>& result) {}
		//Get list of available screens
		virtual void getListVideoSources(std::vector<VideoScreenSource>& result) {}
		//Get list monitors
		virtual void getListMonitors(std::vector<Monitor>& result) {}
		//Get device id from the title
		virtual int getDeviceID(std::string& title) { return 0; }
		//Open a capture device, in case of Webcam, device_id is the index of the device, 
		//in case of screen capturing, device_id is the window id
		//device_id = 0 => capture main full screen
		virtual void setupCaptureSessions() {}

		virtual void setupCaptureSession(VideoActiveSource* source) {}
		virtual void startCaptureSession(VideoActiveSource& source) {}

		virtual void startRecording() {}
		virtual void stopRecording() {}
		virtual void updateConfiguration(const VideoActiveSource& source) {}

		void* info;	//Get information of capture devices, screens
		void* cap;	//Pointer to capture module
		std::vector<VideoScreenSource> screen_sources;
		std::vector<VideoCaptureDevice> capture_devices;

		uint8_t active_source_index = 0;

		VideoActiveSource* addSource(video_source_type_t type, std::string sid, uint8_t fps, window_rect_t rect, 
			window_rect_t renderRect, void* user, int index) {
			if (active_sources.size() >= MAX_ACTIVE_SOURCES)
			{
				return NULL;
			}

			VideoActiveSource src;
			src.video_source_type = type;
			src.video_source_id = sid;
			src.fps = fps;
			src.rect = rect;
			src.renderRect = renderRect;
			src.user = user;
			src.order = index;
			src.id = active_source_index++;
			active_sources.push_back(src);
			return &active_sources.back();
		}

		int removeSource(video_source_type_t type, std::string sid) {
			std::vector<VideoActiveSource>::iterator it;
			it = std::find_if(active_sources.begin(), active_sources.end(), findSource(sid, type));
			if (it == active_sources.end())
				return -1;
			active_sources.erase(it);
			return 0;
		}

		void removeAllSource()
		{
			active_sources.clear();
		}

		void printListActiveSources() {
			std::vector<VideoActiveSource>::const_iterator it;
			for (it = active_sources.begin(); it != active_sources.end(); ++it)
			{
				printf("Source id %s Type %d \n", it->video_source_id.c_str(), it->video_source_type);
			}
		}

		const std::vector<VideoActiveSource>& getVideoActiveSources() const
		{
			return active_sources;
		}

	private:
		const static uint8_t MAX_ACTIVE_SOURCES = 5;
		

	protected:
		std::vector<VideoActiveSource> active_sources;	//Sources used to record
		std::vector<VideoCapture*> 	videoCaptures;
		frame_callback callback_frame;
		void* callback_user;
	};

}

#endif
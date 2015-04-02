/*
	video_capture.h
	Common Video Capture Interface
*/

#ifndef OPPVS_VIDEO_CAPTURE_HPP
#define OPPVS_VIDEO_CAPTURE_HPP

#include <string>
#include <vector>
#include <algorithm>

#include "../include/datatypes.hpp"

namespace oppvs {
	typedef struct WindowRect {
		uint32_t left;
		uint32_t right;
		uint32_t top;
		uint32_t bottom;

		WindowRect() : left(0), right(0), top(0), bottom(0) {};
		WindowRect(uint32_t l, uint32_t r, uint32_t t, uint32_t b) :
			left(l), right(r), top(t), bottom(b) {}
	} window_rect_t;

	typedef enum VideoSourceType {
		VST_WINDOW,
		VST_WEBCAM,
		VST_OTHER
	} video_source_type_t;

	enum OPPVSPixelFormat {
		YUY2,
		RGB24
	};

	struct VideoSourceCapability {
		uint8_t id;
		uint16_t width;
		uint16_t height;
		uint16_t fps;	//Frame per Second
		std::string description;

	};

	class VideoCaptureDevice {
	public:
		std::string device_name;
		std::string device_id;
		std::vector<VideoSourceCapability> capabilities;
	private:

	};

	class VideoScreenSource {
	public:
		uint16_t id;
		std::string title;
		uint16_t app_id;
		std::string app_name;
	};

	struct VideoActiveSource {	
		uint16_t video_source_id;
		uint8_t capability;
		uint8_t pixel_format;
		video_source_type_t video_source_type;
		window_rect_t rect;

		bool operator == (const VideoActiveSource& m) const {
			return ((m.video_source_id == video_source_id) && (m.video_source_type == video_source_type));
		}

	};

	struct findSource {
		uint16_t id;
		video_source_type_t type;
		findSource(uint16_t id, video_source_type_t type) : id(id), type(type) {}
		
		bool operator() (const VideoActiveSource& m) const {
			return ((m.video_source_id == id) && (m.video_source_type == type));
		}
	};

	class VideoEngine {
	public:
		VideoEngine() {
			
		}
		VideoEngine(frame_callback cbf, void* user) : callback_frame(cbf), callback_user(user) {
			printf("Init video Engine with call back \n");
		}
		virtual ~VideoEngine() {}
		//Get list of video capture devices: webcam, external devices
		virtual void getListCaptureDevices(std::vector<VideoCaptureDevice>& result) {}
		//Get list of available screens
		virtual void getListVideoSources(std::vector<VideoScreenSource>& result) {}
		//Get device id from the title
		virtual int getDeviceID(std::string& title) { return 0; }
		//Open a capture device, in case of Webcam, device_id is the index of the device, 
		//in case of screen capturing, device_id is the window id
		//device_id = 0 => capture main full screen
		virtual void setupCaptureSessions() {}

		virtual void startRecording() {}
		virtual void stopRecording() {}
		void* info;	//Get information of capture devices, screens
		void* cap;	//Pointer to capture module
		std::vector<VideoScreenSource> screen_sources;
		std::vector<VideoCaptureDevice> capture_devices;

		int addSource(video_source_type_t type, uint16_t sid, uint8_t capability, window_rect_t rect) {
			if (active_sources.size() >= MAX_ACTIVE_SOURCES)
			{
				return -1;
			}

			VideoActiveSource src;
			src.video_source_type = type;
			src.video_source_id = sid;
			src.capability = capability;
			src.rect = rect;
			active_sources.push_back(src);
			return 0;
		}

		int removeSource(video_source_type_t type, uint16_t sid) {
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
				printf("Source id %d Type %d \n", it->video_source_id, it->video_source_type);
			}
		}

		const std::vector<VideoActiveSource>& getVideoActiveSources() const
		{
			return active_sources;
		}

		//For test
		int windowid;
	private:
		const static uint8_t MAX_ACTIVE_SOURCES = 5;
		frame_callback callback_frame;
		void* callback_user;
		uint8_t numVideoSources;	//Number of selected capturing sources
	protected:
		std::vector<VideoActiveSource> active_sources;	//Sources used to record
	};

}

#endif
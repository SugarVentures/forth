#ifndef OPPVS_VIDEO_CAPTURE_HPP
#define OPPVS_VIDEO_CAPTURE_HPP

#include "datatypes.hpp"
#include <vector>

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
		VST_CUSTOM,
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
		uint8_t fps;	//Frame per Second
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

	class VideoCapture;
	
	struct VideoActiveSource {	
		std::string video_source_id;
		uint8_t fps;
		uint8_t pixel_format;
		video_source_type_t video_source_type;
		window_rect_t rect;
		window_rect_t renderRect;
		uint8_t id;
		uint8_t order;
		uint16_t stride;
		void* user;
		VideoCapture* capture;

		bool operator == (const VideoActiveSource& m) const {
			return ((m.video_source_id.compare(video_source_id) == 0) && (m.video_source_type == video_source_type));
		}

		bool operator != (const VideoActiveSource& m) const {
			return ((m.video_source_id.compare(video_source_id) != 0) || (m.video_source_type != video_source_type));
		}

	};

	struct findSource {
		std::string id;
		video_source_type_t type;
		findSource(std::string id, video_source_type_t type) : id(id), type(type) {}
		
		bool operator() (const VideoActiveSource& m) const {
			return ((m.video_source_id.compare(id) == 0) && (m.video_source_type == type));
		}
	};

	struct Monitor
	{
		uint32_t id;
	};
	
	class VideoCapture {
	public:
		VideoCapture() {}
		VideoCapture(frame_callback cbf, void* user, VideoActiveSource* source) : 
			callback_frame(cbf), callback_user(user), m_source(source) {}

		virtual ~VideoCapture() {}
		virtual void setup() {}
		virtual void start() {}
		virtual void stop()	{}
		virtual void updateConfiguration(const VideoActiveSource& source) {}

		const VideoActiveSource* getSource() const { return m_source; }
	protected:
		frame_callback callback_frame;
		void* callback_user;
		void* m_cap;
		VideoActiveSource* m_source;
	};
} // oppvs

#endif // OPPVS_VIDEO_CAPTURE_HPP

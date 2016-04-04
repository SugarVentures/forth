#ifndef OPPVS_VIDEO_FRAME_BUFFER_H
#define OPPVS_VIDEO_FRAME_BUFFER_H

#include "datatypes.hpp"
#include <vector>

namespace oppvs {

	struct VideoFrameItem
	{
		void* data;
		uint64_t timestamp;
	};

	class VideoFrameBuffer
	{
	public:
		VideoFrameBuffer();
		~VideoFrameBuffer();

		int push(void* data, uint64_t timestamp);

	private:
		const int MAX_INDEX = 500;

		uint64_t m_startTimeStamp;
		uint64_t m_endTimeStamp;

		std::vector<VideoFrameItem> mBuffer;
		int m_hash[500];
	};
} // oppvs

#endif // OPPVS_VIDEO_FRAME_BUFFER_H

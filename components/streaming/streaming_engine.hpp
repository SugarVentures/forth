/*
	Streaming engine
*/

#ifndef OPPVS_STREAMING_ENGINE_HPP
#define OPPVS_STREAMING_ENGINE_HPP

#include "datatypes.hpp"
#include "thread.hpp"
#include "publish_channel.hpp"

namespace oppvs
{

	class StreamingEngine
	{
	public:
		StreamingEngine() : m_ssrc(0) {}
		virtual ~StreamingEngine() {}

		void setSSRC(uint32_t value) { m_ssrc = value; }
		uint32_t getSSRC() { return m_ssrc;}
		int initUploadStream();
		int initPublishChannel();
	private:
		uint32_t m_ssrc;
		PublishChannel* m_publisher;
	};

}
#endif
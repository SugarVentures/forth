/*
	Streaming engine
*/

#ifndef OPPVS_STREAMING_ENGINE_HPP
#define OPPVS_STREAMING_ENGINE_HPP


#include "datatypes.hpp"
#include "thread.hpp"
#include "publish_channel.hpp"
#include "subscribe_channel.hpp"

extern "C"
{
	#include "random_unique32.h"
}

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
		int initSubscribeChannel(const std::string& publisher, uint16_t port, const ServiceInfo& service);

		void pushData(const PixelBuffer& pf);
		void pullData(PixelBuffer& pf);
		
	private:
		uint32_t m_ssrc;
		PublishChannel* m_publisher;
		SubscribeChannel* m_subscribe;
		uint32_t generateSSRC();
	};

}
#endif
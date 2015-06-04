/*
	Channel for communication	
*/

#ifndef OPPVS_CHANNEL_HPP
#define OPPVS_CHANNEL_HPP

#include "datatypes.hpp"
#include "network.hpp"
#include "thread.hpp"

namespace oppvs
{
	enum ServiceType
	{
		ST_VIDEO_STREAMING
	};



	struct ChannelMessage
	{
		uint16_t videoWidth;
		uint16_t videoHeight;
		sockaddr destination;
		uint8_t noSource;
	};

	struct VideoSourceInfo
	{
		uint8_t source;
		uint8_t order;
		uint16_t width;
		uint16_t height;
		uint16_t stride;
		uint16_t originx;
		uint16_t originy;
	};

	struct VideoStreamInfo
	{
		uint16_t videoWidth;
		uint16_t videoHeight;
		uint8_t noSources;
		VideoSourceInfo *sources;
	};

	struct ServiceInfo
	{
		uint8_t type;
		uint32_t key;	//In case of video streaming, key = ssrc
		VideoStreamInfo videoStreamInfo;
	};

	class Channel
	{
	public:
		Channel();
		virtual ~Channel() {}

		virtual int init() { return 0; }
		const ServiceInfo& getServiceInfo() const;
		void setServiceInfo(uint8_t type, uint32_t key);
		void setServiceInfo(const ServiceInfo& s);
		uint32_t getServiceKey() const;
		const SocketAddress& getLocalAddress() const;

	protected:
		SocketAddress m_localAddress;
		ServiceInfo m_service;	
	};
}

#endif
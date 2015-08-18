/*
	Streaming engine
*/

#ifndef OPPVS_STREAMING_ENGINE_HPP
#define OPPVS_STREAMING_ENGINE_HPP


#include "datatypes.hpp"
#include "thread.hpp"
#include "publish_channel.hpp"
#include "subscribe_channel.hpp"
#include "network_stream.hpp"
#include "bits_stream.hpp"
#include "message_handling.hpp"
#include "cache_buffer.hpp"

#include "video_capture.hpp"
#include "video_decoding_vp.hpp"

#include "signaling_handler.hpp"


#include <vector>

extern "C"
{
	#include "random_unique32.h"
}

namespace oppvs
{

	struct StreamingConfiguration
	{
		IceServerInfo stunServer;
		IceServerInfo turnServer;
		SocketAddress signalingServerAddress;
		StreamingRole role;
	};

	class StreamingEngine
	{
	public:
		StreamingEngine();
		~StreamingEngine();

		int init(StreamingRole role, const std::string& stun, const std::string& turn, 
			const std::string& username, const std::string& password, const std::string& signaling, uint16_t port);
		int start(const std::string& streamkey);

		void setup();		

		void setSSRC(uint32_t value) { m_ssrc = value; }
		uint32_t getSSRC() { return m_ssrc;}
		int initUploadStream(IceStream* stream);
		int initDownloadStream();
		int initPublishChannel();
		int initSubscribeChannel(const std::string& publisher, uint16_t port, const ServiceInfo& service);

		void setStreamInfo(PixelBuffer& pf);
		void setStreamInfo(const std::vector<VideoActiveSource>& sources);
		void setStreamInfo(uint8_t *info, int len);
		std::string getStreamInfo() const;

		void pushData(PixelBuffer& pf);
		void pullData(uint8_t source);
		void pullData();

		bool isRunning();
		void setIsRunning(bool value);
		void registerCallback(frame_callback cb);
		
		void updateQueue();


	private:
		uint32_t m_ssrc;
		PublishChannel* m_publisher;
		SubscribeChannel* m_subscribe;		
		std::vector<NetworkStream*> m_subscribers;
		NetworkStream *m_broadcaster;
		NetworkStream *m_receiver;
		bool m_isRunning;

		uint32_t generateSSRC();

		pthread_mutex_t m_mutex;
		Thread* m_sendThread;
		Thread* m_receiveThread;
		Thread* m_renderThread;
		frame_callback m_callback;

		ConQueue<RawData*> m_sendingQueue;	//Shared buffer of all upload streams

		BitsStream *m_bitsstream;
		void initBitsStream();

		ServiceInfo m_serviceInfo;	//Store information of stream: video capture sources info
		void printServiceInfo();

		MessageHandling m_messageHandler;
		MessageParsing m_messageParser;

		CacheBuffer *m_cacheBuffer;
		VPVideoEncoder m_encoder;
		VPVideoDecoder m_decoder;

		StreamingConfiguration 	m_configuration;
		SignalingHandler 		m_signaler;
	};

}
#endif
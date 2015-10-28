/*
	Streaming engine
*/

#ifndef OPPVS_STREAMING_ENGINE_HPP
#define OPPVS_STREAMING_ENGINE_HPP


#include "datatypes.hpp"
#include "thread.hpp"
#include "video_capture.hpp"
#include "audio_engine.hpp"

#include "signaling_handler.hpp"
#include "video_packetizer.h"
#include "audio_packet_handler.h"
#include "depacketizer.h"

#include "streaming_send_thread.hpp"
#include "video_frame_buffer.h"

#include <vector>
#include <chrono>

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

	const std::string STUN_SERVER_ADDRESS("192.168.0.101");
	const std::string TURN_SERVER_ADDRESS("192.168.0.101");
	const std::string TURN_SERVER_USER("turn");
	const std::string TURN_SERVER_PASS("password");
	const std::string SIGN_SERVER_ADDRESS("192.168.0.101");
	const static int SIGN_SERVER_PORT = 33333;

	typedef void (*streaming_callback)(void* user);


	class StreamingEngine
	{
	public:
		StreamingEngine();
		~StreamingEngine();

		int init(StreamingRole role, const std::string& stun, const std::string& turn, 
			const std::string& username, const std::string& password, const std::string& signaling, uint16_t port);
		int start(const std::string& streamkey);

		void setSSRC(uint32_t value) { m_ssrc = value; }
		uint32_t getSSRC() { return m_ssrc;}

		void setStreamInfo(const std::vector<VideoActiveSource>& videoSources, const std::vector<AudioActiveSource>& audioSources);

		bool isRunning();
		void setIsRunning(bool value);
		void registerCallback(frame_callback cb);
		void registerCallback(streaming_callback cb, void* user);
		void attachBuffer(AudioRingBuffer* pbuf);
		void attachBuffer(VideoFrameBuffer* pbuf);

		void createSendingThread(IceStream* stream);
		void createMainThread();
		void send();
		void receive(uint8_t* data, uint32_t len);
		void receive();

		void pushData(PixelBuffer& pf);
		void pushData(GenericAudioBufferList& ab);

		int updateStreamInfo(const ServiceInfo& info);
	private:
		uint32_t m_ssrc;
		bool m_isRunning;

		uint32_t generateSSRC();

		frame_callback m_callback;
		streaming_callback m_streamingCallback;
		void* m_streamingUser;

		ServiceInfo m_serviceInfo;	//Store information of stream: video capture sources info
		void printServiceInfo();

		StreamingConfiguration 				m_configuration;
		SignalingHandler 					m_signaler;
		VideoPacketizer 					m_videoPacketizer;
		std::vector<StreamingSendThread*> 	m_sendingThreads;
		Thread*								m_mainThread;	//Thread to distribute segments to all sending threads
		bool								m_exitMainThread;
		tsqueue<SharedDynamicBufferRef> 	m_sendPool;
		Depacketizer						m_depacketizer;
		tsqueue<IncomingStreamingFrame*>	m_recvPool;

		//Audio
		AudioPacketizer						m_audioPacketizer;
		AudioRingBuffer*					p_audioRingBuffer;

		//Video Picture
		VideoFrameBuffer*					p_videoFrameBuffer;

		static void* runMainThreadFunction(void* object);
		static void onNewSubscriber(void* object, IceStream* stream);
		static void onReceiveSegment(void* object, uint8_t* data, uint32_t len);

		std::chrono::time_point<std::chrono::system_clock> m_firstTime;
		StreamingRole getRole();
	};

}
#endif
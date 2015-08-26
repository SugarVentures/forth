/*
	Streaming engine
*/

#ifndef OPPVS_STREAMING_ENGINE_HPP
#define OPPVS_STREAMING_ENGINE_HPP


#include "datatypes.hpp"
#include "thread.hpp"
#include "video_capture.hpp"
#include "video_decoding_vp.hpp"

#include "signaling_handler.hpp"
#include "packet_handler.hpp"

#include "streaming_send_thread.hpp"

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

		void setSSRC(uint32_t value) { m_ssrc = value; }
		uint32_t getSSRC() { return m_ssrc;}

		void setStreamInfo(PixelBuffer& pf);
		void setStreamInfo(const std::vector<VideoActiveSource>& sources);

		bool isRunning();
		void setIsRunning(bool value);
		void registerCallback(frame_callback cb);

		void createSendingThread(IceStream* stream);
		void createMainThread();
		void send();
		void receive(uint8_t* data, uint32_t len);
		void receive();

		void pushData(PixelBuffer& pf);

		int updateStreamInfo(const VideoStreamInfo& info);
	private:
		uint32_t m_ssrc;
		bool m_isRunning;

		uint32_t generateSSRC();

		frame_callback m_callback;

		ServiceInfo m_serviceInfo;	//Store information of stream: video capture sources info
		void printServiceInfo();

		StreamingConfiguration 				m_configuration;
		SignalingHandler 					m_signaler;
		Packetizer 							m_packetizer;
		std::vector<StreamingSendThread*> 	m_sendingThreads;
		Thread*								m_mainThread;	//Thread to distribute segments to all sending threads
		bool								m_exitMainThread;
		tsqueue<SharedDynamicBufferRef> 	m_sendPool;
		Depacketizer						m_depacketizer;
		tsqueue<SharedDynamicBufferRef>		m_recvPool;

		static void* runMainThreadFunction(void* object);
		static void onNewSubscriber(void* object, IceStream* stream);
		static void onReceiveSegment(void* object, uint8_t* data, uint32_t len);

		StreamingRole getRole();
	};

}
#endif
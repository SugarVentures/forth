#include "signaling_server.hpp"
#include <errno.h>

namespace oppvs {

	SignalingServer::SignalingServer()
	{

	}

	SignalingServer::~SignalingServer()
	{
		shutdown();
	}

	void SignalingServer::init(const SignalingServerConfiguration& config)
	{
		shutdown();

		if (addSocket(config.addressListen, config.addressAdvertised) < 0)
			return;

		SignalingServerThread* thread = new SignalingServerThread();
		m_threads.push_back(thread);
		thread->init(&m_socket, &m_streamKey, &m_broadcaster);
		thread->attachCallback([this](const std::string& sk, int sockfd, const VideoStreamInfo& info) { return updateStream(sk, sockfd, info); });
		thread->attachCallback([this](const std::string& sk, int* psockfd, VideoStreamInfo& info) { return getStreamInfo(sk, psockfd, info); });
		thread->attachCallback([this](int sockfd) { return removeStream(sockfd); });
	}

	int SignalingServer::addSocket(const SocketAddress& addressListen, const SocketAddress& addressAdvertised)
	{
		if (m_socket.Create(AF_INET, SOCK_STREAM, 0) < 0)
		{
			printf("Open socket error %s\n", strerror(errno));
			return -1;
		}
		if (m_socket.Bind(addressListen) < 0)
		{
			printf("Binding error %s\n", strerror(errno));
			return -1;
		}

		printf("Add socket for signaling: %s\n", m_socket.getLocalAddress().toString().c_str());
		return 0;		
	}

	int SignalingServer::start()
	{
		for (int i = 0; i < m_threads.size(); i++)
		{
			SignalingServerThread* thread = m_threads[i];
			thread->start();
		}
		return 0;
	}

	int SignalingServer::stop()
	{
		for(unsigned int i = 0; i < m_threads.size(); ++i) {
			SignalingServerThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->signalForStop(true);
			}
		}

		for(unsigned int i = 0; i < m_threads.size(); ++i) {
			SignalingServerThread* thread = m_threads[i];
			if (thread != NULL)
			{
				thread->waitForStopAndClose();
			}
		}
		return 0;
	}

	int SignalingServer::shutdown()
	{
		stop();
		m_socket.Close();
		for (int i = 0; i < m_threads.size(); i++)
		{
			SignalingServerThread* thread = m_threads[i];
			delete thread;
			m_threads[i] = NULL;
		}
		m_threads.clear();

		return 0;
	}

	void SignalingServer::setStreamKey(const std::string& streamKey)
	{
		printf("Set stream key %s\n", streamKey.c_str());
		m_streamKey = streamKey;
	}

	int SignalingServer::updateStream(const std::string& streamkey, int sockfd, const VideoStreamInfo& videoInfo)
	{
		printf("Update stream %s %d\n", streamkey.c_str(), sockfd);
		SignalingStreamInfo* pstream = findStream(streamkey);
		if (!pstream)
		{
			printf("Not found stream. Add new\n");
			SignalingStreamInfo stream;
			stream.streamKey = streamkey;
			stream.sockFD = sockfd;
			stream.videoStreamInfo = videoInfo;
			m_streams.push_back(stream);
		}
		else
		{
			printf("Found the stream. Update %d %d\n", pstream->videoStreamInfo.sources[0].width, pstream->videoStreamInfo.sources[0].height);
			pstream->sockFD = sockfd;
			pstream->videoStreamInfo = videoInfo;
		}
		return 0;
	}


	SignalingStreamInfo* SignalingServer::findStream(const std::string& streamkey)
	{
		std::vector<SignalingStreamInfo>::iterator it;
		it = std::find_if(m_streams.begin(), m_streams.end(), findStreamOp(streamkey));
		if (it == m_streams.end())
			return NULL;
		return &(*it);
	}

	int SignalingServer::getStreamInfo(const std::string& streamKey, int* psockfd, VideoStreamInfo& info)
	{
		SignalingStreamInfo* pstream = findStream(streamKey);
		if (!pstream)
			return -1;
		*psockfd = pstream->sockFD;
		info = pstream->videoStreamInfo;
		return 0;
	}

	int SignalingServer::removeStream(int sockfd)
	{
		if (sockfd < 0)
			return -1;
		std::vector<SignalingStreamInfo>::iterator it = m_streams.begin();
		while (it != m_streams.end())
		{
			SignalingStreamInfo stream = *it;
			if (stream.sockFD == sockfd)
			{
				printf("Delete stream: %s\n", stream.streamKey.c_str());
				it = m_streams.erase(it);
				break;
			}
			else
				++it;
		}
		return 0;
	}
} // oppvs
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
		thread->init(&m_socket);
		thread->attachCallback([this](const std::string& sk, int sockfd, const ServiceInfo& info) { return updateStream(sk, sockfd, info); });
		thread->attachCallback([this](const std::string& sk, int* psockfd, ServiceInfo& info) { return getStreamInfo(sk, psockfd, info); });
		thread->attachCallback([this](int sockfd) { return removeStream(sockfd); });
		thread->attachCallback([this](const std::string& sk, int sockfd, const AvailableDuration& dur) { return registerPeer(sk, sockfd); });
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

	int SignalingServer::updateStream(const std::string& streamkey, int sockfd, const ServiceInfo& info)
	{
		printf("Update stream %s %d\n", streamkey.c_str(), sockfd);
		SignalingStreamInfo* pstream = findStream(streamkey);
		if (!pstream)
		{
			printf("Not found stream. Add new\n");
			SignalingStreamInfo stream;
			stream.streamKey = streamkey;
			stream.broadcaster = sockfd;
			stream.peerList.push_back(sockfd);
			stream.serviceInfo = info;
			m_streams.push_back(stream);
		}
		else
		{
			printf("Found the stream. Update.\n" );
			pstream->broadcaster = sockfd;
			pstream->serviceInfo = info;
			pstream->peerList.clear();
			pstream->peerList.push_back(sockfd);
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

	int SignalingServer::getStreamInfo(const std::string& streamKey, int* psockfd, ServiceInfo& info)
	{
		SignalingStreamInfo* pstream = findStream(streamKey);
		if (!pstream)
			return -1;
		printf("size of peer lists %lld\n", pstream->peerList.size());

		*psockfd = pstream->peerList[pstream->peerList.size() - 1];
		info = pstream->serviceInfo;
		return 0;
	}

	int SignalingServer::removeStream(int sockfd)
	{
		if (sockfd < 0)
			return -1;
		//Need to rewrite
		std::vector<SignalingStreamInfo>::iterator it = m_streams.begin();
		while (it != m_streams.end())
		{
			SignalingStreamInfo stream = *it;
			if (stream.broadcaster == sockfd)
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

	int SignalingServer::registerPeer(const std::string& streamKey, int sockfd)
	{
		printf("Register peer\n");
		SignalingStreamInfo* pstream = findStream(streamKey);
		if (!pstream)
			return -1;
		else
		{
			std::vector<int>::iterator found = std::find(pstream->peerList.begin(), pstream->peerList.end(), sockfd);
			if (found != pstream->peerList.end())
			{
				printf("Found sockfd. Do nothing\n");
			}
			else
			{
				printf("Not found sockfd. Add sock\n");
				pstream->peerList.push_back(sockfd);
			}	
		}
		return 0;
	}
} // oppvs
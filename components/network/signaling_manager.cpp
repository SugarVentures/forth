#include "signaling_manager.hpp"
#include <errno.h>
#include <iostream>
#include "logs.h"

namespace oppvs {
	SignalingManager::SignalingManager(const SocketAddress& address): m_serverAddress(address), m_interrupt(false)
	{

	}

	SignalingManager::~SignalingManager()
	{

	}

	int SignalingManager::init()
	{
		if (m_socket.Create(AF_INET, SOCK_STREAM, 0) < 0)
		{
			LOGD("Open socket error %s\n", strerror(errno));
			return -1;
		}
		SocketAddress localAddress;
		if (m_socket.Bind(localAddress) < 0)
		{
			LOGD("Binding error %s\n", strerror(errno));
			return -1;
		}

		if (m_socket.Connect(m_serverAddress) < 0)
		{
			LOGD("Cannot connect to signaling server: %s\n", strerror(errno));
			return -1;
		}
		//m_socket.setReceiveTimeOut(OPPVS_NETWORK_MAX_WAIT_TIME);
		LOGD("Address for signaling: %s\n", m_socket.getLocalAddress().toString().c_str());
		//Init buffer for response
		m_incomingBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_incomingBuffer->setSize(MAX_SIGNALING_MESSAGE_SIZE);

		m_readerBuffer = SharedDynamicBufferRef(new DynamicBuffer());

		return 0;
	}


	void SignalingManager::attachCallbackEvent(callbackOnReceiveIceResponse cb, void* object)
	{
		cbOnReceiveIceResponse = cb;
		m_object = object;
	}

	int SignalingManager::sendIceResponse(std::string username, std::string password, std::vector<IceCandidate>& candidates)
	{
		m_messageBuilder.reset();

		if (m_messageBuilder.addMessageType(SignalingIceResponse) < 0)
			return -1;

		if (m_messageBuilder.addStreamKey(m_streamKey) < 0)
			return -1;

		if (m_messageBuilder.addIceUsername(username) < 0)
			return -1;

		if (m_messageBuilder.addIcePassword(password) < 0)
			return -1;

		if (m_messageBuilder.addIceCandidates(candidates) < 0)
			return -1;

		return sendSignal();
	}

	int SignalingManager::sendStreamRegister(const std::string& streamKey, const ServiceInfo& info)
	{
		if (streamKey == "")
			return -1;
		m_streamKey = streamKey;
		m_messageBuilder.reset();
		if (m_messageBuilder.addMessageType(SignalingStreamRegister) < 0)
			return -1;
		if (m_messageBuilder.addStreamKey(streamKey) < 0)
			return -1;

        int vret = m_messageBuilder.addVideoSources(info.videoStreamInfo);
        int aret = m_messageBuilder.addAudioSources(info.audioStreamInfo);
        
		if (vret < 0 && aret < 0)
			return -1;
		
		return sendSignal();
	}

	int SignalingManager::sendStreamRequest(const std::string& username, const std::string& password, 
			const std::vector<IceCandidate>& candidates)
	{
		if (m_streamKey == "" || username == "" || password == "" || candidates.size() == 0)
			return -1;
		m_messageBuilder.reset();
		if (m_messageBuilder.addMessageType(SignalingStreamRequest) < 0)
			return -1;
		if (m_messageBuilder.addStreamKey(m_streamKey) < 0)
			return -1;

		if (m_messageBuilder.addIceUsername(username) < 0)
			return -1;

		if (m_messageBuilder.addIcePassword(password) < 0)
			return -1;

		if (m_messageBuilder.addIceCandidates(candidates) < 0)
			return -1;

		return sendSignal();
	}

	int SignalingManager::sendPeerRegister()
	{
		m_messageBuilder.reset();
		if (m_messageBuilder.addMessageType(SignalingPeerRegister) < 0)
			return -1;
		if (m_messageBuilder.addStreamKey(m_streamKey) < 0)
			return -1;

		AvailableDuration dur;
		dur.start = 0;
		dur.end = 0;

		if (m_messageBuilder.addDuration(dur) < 0)
			return -1;
		return sendSignal();
	}

	int SignalingManager::sendSignal()
	{
		SharedDynamicBufferRef buffer;
		if (m_messageBuilder.getResult(buffer) < 0)
		{
			LOGD("Can not build the message to send\n");
			return -1;
		}

		if (m_socket.Send(m_socket.getSocketHandle(), buffer->data(), buffer->size(), m_serverAddress) < 0)
		{
			LOGD("Send error %s",strerror(errno));
			return -1;
		}
		else
		{
			LOGD("sent done %ld bytes \n", buffer->size());
		}
		return 0;
	}

	void SignalingManager::waitResponse()
	{
    	int sock = m_socket.getSocketHandle();

    	m_incomingBuffer->setSize(0);

		while (!m_interrupt)
		{
			
    		int ret = m_socket.Receive(sock, m_incomingBuffer->data(), m_incomingBuffer->capacity());
			if (ret > 0)
			{
				m_incomingBuffer->setSize(ret);
				LOGD("Receive %d bytes at %s from %s\n", ret, m_socket.getLocalAddress().toString().c_str(), m_socket.getRemoteAddress().toString().c_str());
				processResponse();
			}
			else
			{
				if (ret == 0)
					break;
				LOGD("Receive error %s\n", strerror(errno));
			}
		}
	}

	void SignalingManager::release()
	{
		m_incomingBuffer.reset();
		m_messageReader.reset();
	}

	void SignalingManager::signalForStop()
	{
		m_interrupt = true;
		m_socket.Close();
	}

	void SignalingManager::processResponse()
	{
		m_messageReader.reset();
		m_readerBuffer->setSize(0);
		m_messageReader.getStream().attach(m_readerBuffer, true);

		m_messageReader.addBytes(m_incomingBuffer->data(), m_incomingBuffer->size());
		switch (m_messageReader.getMessageType())
		{
			case SignalingIceRequest:
				LOGD("Receive Ice Request\n");
				m_interrupt = true;
				break;
			case SignalingIceResponse:
			{
				LOGD("Receive Ice Response\n");
				std::vector<IceCandidate>& candidates = m_messageReader.getIceCandidates();
				cbOnReceiveIceResponse(m_object, m_messageReader.getUsername(), m_messageReader.getPassword(), candidates);
				break;
			}
			case SignalingStreamResponse:
				LOGD("Receive stream response\n");
				m_cbStreamResponse(m_messageReader.getServiceInfo());
				break;
			default:
				break;
		}
	}

	void SignalingManager::setStreamKey(const std::string& streamkey)
	{
		m_streamKey = streamkey;
	}

	void SignalingManager::attachCallback(callbackStreamResponse cb)
	{
		m_cbStreamResponse = cb;
	}
} // oppvs
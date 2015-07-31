#include "stun_server_thread.hpp"
#include "common.hpp"

namespace oppvs
{
	StunServerThread::StunServerThread() : Thread(threadExecuteFunction, this), m_rotation(0), m_stas(), m_exitThread(false)
	{
		m_sendSockets = NULL;
		m_listenSockets.clear();
	}

	StunServerThread::~StunServerThread()
	{
		signalForStop(true);
		waitForStopAndClose();
	}

	int StunServerThread::init(StunSocket* sockets, StunTransportAddressSet* stas, SocketRole role)
	{
		bool singleSocket = IsValidSocketRole(role);
		if (!sockets)
		{
			printf("Invalid stun sockets\n");
			return -1;
		}
		if (!stas)
		{
			printf("Invalid transport address set\n");
			return -1;
		}

		if (singleSocket)
		{
			if (sockets[role].isValid() == false)
			{
				printf("Invalid stun socket role\n");
				return -1;
			}
		}

		m_sendSockets = sockets;
		m_stas = *stas;
		if (singleSocket)
		{
			m_listenSockets.push_back(&sockets[role]);
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				if (sockets[i].isValid())
				{
					m_listenSockets.push_back(&sockets[i]);
				}
			}
		}

		m_exitThread = false;
		allocBuffers();
		return 0;
	}

	int StunServerThread::start()
	{
		create();
		return 0;
	}

	void* StunServerThread::threadExecuteFunction(void* pthis)
	{
		StunServerThread* thread = (StunServerThread*)pthis;
		thread->run();
		return NULL;
	}

	StunSocket* StunServerThread::waitForSocketData()
	{
		fd_set set = {};
		int highestSocketValue = 0;
		int ret;
		StunSocket* pReadySocket = NULL;
		size_t noSockets = m_listenSockets.size();

		m_rotation = (m_rotation + 1) % noSockets;

		FD_ZERO(&set);

		for (size_t index = 0; index < noSockets; index++)
	    {
	        ASSERT(m_listenSockets[index] != NULL);
	        int sock = m_listenSockets[index]->getSocketHandle();
	        ASSERT(sock != -1);
	        FD_SET(sock, &set);
	        highestSocketValue = (sock > highestSocketValue) ? sock : highestSocketValue;
	    }

	    // wait indefinitely for a socket
    	ret = select(highestSocketValue + 1, &set, NULL, NULL, NULL);
    	if (ret < 0)
    	{
    		printf("Select error\n");
    		return NULL;
    	}
    	
	    for (size_t index = 0; index < noSockets; index++)
	    {
	        int indexconverted = (index + m_rotation) % noSockets;
	        int sock = m_listenSockets[indexconverted]->getSocketHandle();
	        
	        ASSERT(sock != -1);
	        
	        if (FD_ISSET(sock, &set))
	        {
	            pReadySocket = m_listenSockets[indexconverted];
	            break;
	        }
	    }
	    
	    ASSERT(pReadySocket != NULL);
	    
	    return pReadySocket;
	}

	int StunServerThread::run()
	{
		size_t noSockets = m_listenSockets.size();
		bool isMultiSockets = (noSockets > 1);
		int flags = isMultiSockets ? MSG_DONTWAIT : 0;
		StunSocket* psocket = m_listenSockets[0];
		int noSendSockets = 0;
		noSendSockets += (int)(m_stas.set[RolePP].isValid);
		noSendSockets += (int)(m_stas.set[RolePA].isValid);
		noSendSockets += (int)(m_stas.set[RoleAP].isValid);
		noSendSockets += (int)(m_stas.set[RoleAA].isValid);

		printf("Starting listener: %lu send %d recv\n", noSockets, noSendSockets);

		while (!m_exitThread)
		{
			if (isMultiSockets)
			{
				psocket = waitForSocketData();
			}
			ASSERT(psocket != NULL);
			
			int rcvlen = psocket->ReceiveMsg(psocket->getSocketHandle(), 
				m_incomingBuffer->data(), m_incomingBuffer->size(), flags, m_incomingMessage.remoteAddress, m_incomingMessage.localAddress);

			if (rcvlen < 0)
				continue;

			m_incomingMessage.localAddress.setPort(psocket->getLocalAddress().getPort());
			m_incomingMessage.role = psocket->getRole();
			m_incomingBuffer->setSize(rcvlen);

			handleMessage();
			
		}
		return 0;
	}

	int StunServerThread::allocBuffers()
	{
		m_incomingBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_incomingBuffer->setSize(MAX_STUN_MESSAGE_SIZE);
		m_outgoingBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_outgoingBuffer->setSize(MAX_STUN_MESSAGE_SIZE);
		m_readerBuffer = SharedDynamicBufferRef(new DynamicBuffer());
		m_readerBuffer->setSize(MAX_STUN_MESSAGE_SIZE);

		m_messageParser.reset();
		m_messageParser.getStream().attach(m_readerBuffer, true);

		m_incomingMessage.isConnectionOriented = false;
		m_incomingMessage.handler = &m_messageParser;
		m_outgoingMessage.buffer = m_outgoingBuffer;

		return 0;
	}

	int StunServerThread::releaseBuffers()
	{
		m_incomingBuffer->reset();
		m_outgoingBuffer->reset();
		m_readerBuffer->reset();
		m_messageParser.reset();
		m_incomingMessage.handler = NULL;
		m_outgoingMessage.buffer->reset();
		return 0;
	}

	int StunServerThread::handleMessage()
	{
		printf("Receive data %d from %s at %s\n", m_incomingBuffer->size(), m_incomingMessage.remoteAddress.toString().c_str(), 
			m_incomingMessage.localAddress.toString().c_str());

		m_messageParser.reset();
		m_readerBuffer->setSize(0);
		m_messageParser.getStream().attach(m_readerBuffer, true);

		m_messageParser.addBytes(m_incomingBuffer->data(), m_incomingBuffer->size());
		if (m_messageParser.getState() != StunMessageParser::BodyValidated)
		{
			printf("Error in parsing message \n");
			return -1;
		}

		StunRequestHandler::processRequest(&m_incomingMessage, &m_outgoingMessage, &m_stas);

		ASSERT(m_stas.set[m_outgoingMessage.role].isValid);
		ASSERT(m_sendSockets[m_outgoingMessage.role].isValid());

		int sock = m_sendSockets[m_outgoingMessage.role].getSocketHandle();
		StunSocket* psocket = &m_sendSockets[m_outgoingMessage.role];
		ASSERT(sock != -1);
		
		if (psocket->Send(sock, m_outgoingBuffer->data(), m_outgoingBuffer->size(), m_outgoingMessage.destinationAddress) >= 0)
			printf("Sent response to %s\n", m_outgoingMessage.destinationAddress.toString().c_str());
		return 0;
	}

	int StunServerThread::signalForStop(bool postMessages)
	{

		m_exitThread = true;

	    // have the socket send a message to itself
	    // if another thread is sharing the same socket, this may wake that thread up to
	    // but all the threads should be started and shutdown together
	    if (postMessages)
	    {
	        for (size_t index = 0; index < m_listenSockets.size(); index++)
	        {
	            char data = 'x';
	            
	            ASSERT(m_listenSockets[index] != NULL);
	            
	            SocketAddress addr = m_listenSockets[index]->getLocalAddress();
	            // If no specific adapter was binded to, IP will be 0.0.0.0
	            // Linux evidently treats 0.0.0.0 IP as loopback (and works)
	            // On Windows you can't send to 0.0.0.0. sendto will fail - switch to sending to localhost
	            if (addr.getIP().isZero())
	            {
	                StunSocketAddress addrLocal;
	                addrLocal.setIP(IPAddress());
	                addrLocal.setPort(addr.getPort());
	                addr = addrLocal;

	            }
	            m_listenSockets[index]->Send(m_listenSockets[index]->getSocketHandle(), &data, 1, addr);
	        }
	    }
		return 0;    	
	}

	int StunServerThread::waitForStopAndClose()
	{
		waitUntilEnding();
		releaseBuffers();
		m_listenSockets.clear();
		m_sendSockets = NULL;
		return 0;
	}
}

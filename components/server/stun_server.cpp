#include "stun_server.hpp"
#include "common.hpp"

namespace oppvs
{
	StunServer::StunServer() : m_sockets()
	{

	}

	StunServer::~StunServer()
	{
		shutdown();
	}

	int StunServer::addSocket(StunTransportAddressSet* pStas, SocketRole role, 
			const StunSocketAddress& addressListen, const StunSocketAddress& addressAdvertised)
	{
		ASSERT(IsValidSocketRole(role));
		if (m_sockets[role].initUDP(addressListen, role) < 0)
		{
			printf("Failed to init udp for primary address \n");
			return -1;
		}
		m_sockets[role].enablePacketInfoOption(true);
		printf("Success to init %d\n", role);
		pStas->set[role].isValid = true;
		if (addressAdvertised.isZeroAddress() == false)
		{
			pStas->set[role].addr = addressAdvertised;
			pStas->set[role].addr.setPort(addressListen.getPort());
		}
		else
		{
			pStas->set[role].addr = addressListen;
		}


		return 0;
	}

	int StunServer::init(const StunServerConfiguration& config)
	{
		int noSockets = 0;
		StunTransportAddressSet stas = {};
		shutdown();
		if (config.enabledPP)
		{
			addSocket(&stas, RolePP, config.addressPP, config.addressPrimaryAdvertised);
			noSockets++;
		}

		if (config.enabledPA)
		{
			addSocket(&stas, RolePA, config.addressPA, config.addressPrimaryAdvertised);
			noSockets++;	
		}

		if (config.enabledAP)
		{
			addSocket(&stas, RoleAP, config.addressAP, config.addressAlternateAdvertised);
			noSockets++;	
		}

		if (config.enabledAA)
		{
			addSocket(&stas, RoleAA, config.addressAA, config.addressAlternateAdvertised);
			noSockets++;	
		}

		if (noSockets == 0)
			return -1;

		if (config.enabledMultithreadMode)
		{

		}
		else
		{
			StunServerThread* thread = new StunServerThread();
			m_threads.push_back(thread);
			thread->init(m_sockets, &stas, (SocketRole)-1);
		}
		return 0;
	}

	int StunServer::shutdown()
	{
		stop();
		for (int i = 0; i < 4; i++)
		{
			m_sockets[i].Close();
		}
		for (int i = 0; i < m_threads.size(); i++)
		{
			StunServerThread* thread = m_threads[i];
			delete thread;
			m_threads[i] = NULL;
		}
		m_threads.clear();
		return 0;
	}

	int StunServer::start()
	{
		for (int i = 0; i < m_threads.size(); i++)
		{
			StunServerThread* thread = m_threads[i];
			thread->start();
		}
		return 0;
	}

	int StunServer::stop()
	{
		return 0;
	}
}
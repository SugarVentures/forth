/*
	Interfaces of transport protocols
*/


#ifndef OPPVS_TRANSPORT_ADAPTER_HPP
#define OPPVS_TRANSPORT_ADAPTER_HPP

namespace oppvs
{
	enum PROTOCOL_TYPE
	{
		SRTP
	};

	class TransportAdapter
	{
	public:
		void startStream();
	private:
		PROTOCOL_TYPE m_protocol
	};
}

#endif

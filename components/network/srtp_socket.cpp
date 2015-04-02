
#include "srtp_socket.hpp"
#include <fcntl.h>

namespace oppvs
{
	SRTPSocket::SRTPSocket()
	{
		m_sender = NULL;
		m_receiver = NULL;
		m_timestamp = 0;
	}

	SRTPSocket::~SRTPSocket()
	{

	}

	int SRTPSocket::Accept(const SocketAddress& remote)
	{
		m_remoteAddress = remote;
		return m_socketfd;
	}

	int SRTPSocket::Connect(const SocketAddress& dest)
	{
		m_remoteAddress = dest;
		return 0;
	}


	int SRTPSocket::initSRTPLib()
	{
		/* initialize srtp library */
		err_status_t status = srtp_init();
		setSecurityService(0);
		m_ssrc = 0xab43de12;	//fixed temporarily
		if (status)
			return -1;
		return 0;
	}

	void SRTPSocket::setSecurityService(int mode)
	{
		switch (mode)
		{
			case 0:
				m_securityService = sec_serv_none;
				break;
			case 1:
				m_securityService = sec_serv_conf;
				break;
			case 2:
				m_securityService = sec_serv_auth;
				break;
		}
	}

	void SRTPSocket::setPolicy(char* key)
	{
		m_policy.key = (uint8_t*)key;
		m_policy.ssrc.type = ssrc_specific;
		m_policy.ssrc.value = m_ssrc;
		m_policy.rtp.cipher_type = NULL_CIPHER;
		m_policy.rtp.cipher_key_len = 0;
		m_policy.rtp.auth_type = NULL_AUTH;
		m_policy.rtp.auth_key_len = 0;
		m_policy.rtp.auth_tag_len = 0;
		m_policy.rtp.sec_serv = m_securityService;
		m_policy.rtcp.cipher_type = NULL_CIPHER;
		m_policy.rtcp.cipher_key_len = 0;
		m_policy.rtcp.auth_type = NULL_AUTH;
		m_policy.rtcp.auth_key_len = 0;
		m_policy.rtcp.auth_tag_len = 0;
		m_policy.rtcp.sec_serv = m_securityService;
		m_policy.window_size = 0;
		m_policy.allow_repeat_tx = 0;
		m_policy.ekt = NULL;
		m_policy.next = NULL;

	}

	
	int SRTPSocket::initSender()
	{
		rtp_sender_ctx_t* snd = (rtp_sender_ctx_t*)malloc(sizeof(rtp_sender_ctx_t));

		if (snd == NULL) {
			printf("error: rtp sender alloc memory failed\n");
			return -1;
		}

		struct sockaddr_in dest_addr;
		m_remoteAddress.toSocketAddr(&dest_addr);

		rtp_sender_init(snd, m_socketfd, dest_addr, m_ssrc);
		int status = rtp_sender_init_srtp(snd, &m_policy);
		if (status) {
			printf("error: rtp_sender_init_srtp failed with code %d\n", status);
			return -1;
		}
		m_sender = snd;
		return 0;
	}

	int SRTPSocket::SendTo(const void* msg, int len, uint32_t ts)
	{
		//printHashCode(msg, len);

		if (!m_sender)
		{
			printf("Sender object null\n");
			return -1;
		}
		return rtp_sendto(m_sender, msg, len, ts);
	}

	int SRTPSocket::RecvFrom(void* msg, int *len)
	{
		if (!m_receiver)
		{
			return -1;
		}
		return rtp_recvfrom(m_receiver, msg, len);
	}

	int SRTPSocket::RecvFrom(void* msg, int *len, bool *isNext)
	{
		int recvLen = rtp_recvfrom(m_receiver, msg, len);
		uint32_t ts = ntohl(m_receiver->message.header.ts);
		//printf("Timestamp: %u seq: %d\n", ts, ntohs(m_receiver->message.header.seq));
		
		//printHashCode(msg, recvLen - SRTP_HEADER_LEN);

		if (recvLen < 0)
			return recvLen;
		if (m_timestamp < ts)
		{
			*isNext = true;
			m_timestamp = ts;
		}
		else
			*isNext = false;
		//printf("Timestamp: %u next: %d seq: %u\n", ts, *isNext, ntohs(m_receiver->message.header.seq));
		return recvLen - SRTP_HEADER_LEN;
	}

	void SRTPSocket::releaseSender()
	{
		if (m_sender)
		{
			rtp_sender_deinit_srtp(m_sender);
	    	rtp_sender_dealloc(m_sender);
	    }
	}

	void SRTPSocket::releaseReceiver()
	{
		if (m_receiver)
		{
			rtp_receiver_deinit_srtp(m_receiver);
			rtp_receiver_dealloc(m_receiver);
		}
	}

	int SRTPSocket::rtp_sender_init(rtp_sender_ctx_t* sender, int sock, const struct sockaddr_in& dest, uint32_t ssrc)
	{
		/* set header values */
		sender->message.header.ssrc    = htonl(ssrc);
		sender->message.header.ts      = 0;
		//sender->message.header.seq     = (uint16_t) rand();
		sender->message.header.seq     = 0;
		sender->message.header.m       = 0;
		sender->message.header.pt      = 0x1;
		sender->message.header.version = 2;
		sender->message.header.p       = 0;
		sender->message.header.x       = 0;
		sender->message.header.cc      = 0;

		/* set other stuff */
		sender->socket = sock;
		sender->addr = dest;

		return 0;
	}
	
	int SRTPSocket::rtp_sender_init_srtp(rtp_sender_ctx_t* sender, const srtp_policy_t *policy)
	{
		return srtp_create(&sender->srtp_ctx, policy);
	}

	int SRTPSocket::initReceiver()
	{
		/*int nonBlocking = 1;
		if ( fcntl(m_socketfd,  F_SETFL,  O_NONBLOCK,  nonBlocking ) == -1 )
		{
			printf( "failed to set non-blocking\n" );
			return false;
		}*/

		rtp_receiver_ctx_t* rcv = (rtp_receiver_ctx_t*)malloc(sizeof(rtp_receiver_ctx_t));
		if (rcv == NULL) {
			printf("error: rtp receiver allo memory failed\n");
			return -1;
		}

		struct sockaddr_in local_addr;
		m_localAddress.toSocketAddr(&local_addr);

		rtp_receiver_init(rcv, m_socketfd, local_addr, m_ssrc);
		int status = rtp_receiver_init_srtp(rcv, &m_policy);
		if (status) {
			printf("error: srtp_create() failed with code %d\n", status);
			return -1;
		}

		m_receiver = rcv;
		m_timestamp = 0;
		return 0;


	}

	int SRTPSocket::rtp_receiver_init(rtp_receiver_ctx_t* rcvr, int sock, const struct sockaddr_in& addr, uint32_t ssrc)
	{
		/* set header values */
		rcvr->message.header.ssrc    = htonl(ssrc);
		rcvr->message.header.ts      = 0;
		rcvr->message.header.seq     = 0;
		rcvr->message.header.m       = 0;
		rcvr->message.header.pt      = 0x1;
		rcvr->message.header.version = 2;
		rcvr->message.header.p       = 0;
		rcvr->message.header.x       = 0;
		rcvr->message.header.cc      = 0;

		/* set other stuff */
		rcvr->socket = sock;
		rcvr->addr = addr;

		return 0;
	}

	int SRTPSocket::rtp_receiver_init_srtp(rtp_receiver_ctx_t* rcv, const srtp_policy_t *policy)
	{
		return srtp_create(&rcv->srtp_ctx, policy);
	}

	int SRTPSocket::rtp_sendto(rtp_sender_ctx_t* sender, const void* msg, int len, uint32_t timestamp)
	{
		int octets_sent;
		err_status_t stat;
		int pkt_len = len + SRTP_HEADER_LEN;

		/* marshal data */
		//printHashCode(msg, len);
		//strncpy(sender->message.body, (const char*)msg, len);
		memcpy(sender->message.body, msg, len);
		//printHashCode(sender->message.body, len);
		
		/* update header */
		sender->message.header.seq = ntohs(sender->message.header.seq) + 1;
		sender->message.header.seq = htons(sender->message.header.seq);
		//sender->message.header.ts = ntohl(sender->message.header.ts) + 1;
		sender->message.header.ts = htonl(timestamp);

		/* apply srtp */
		/*stat = srtp_protect(sender->srtp_ctx, &sender->message.header, &pkt_len);
		if (stat) {
		    printf("error: srtp protection failed with code %d\n", stat);
		    return -1;
		}*/

		octets_sent = sendto(sender->socket, (void*)&sender->message,
		       pkt_len, 0, (struct sockaddr *)&sender->addr,
		       sizeof (struct sockaddr_in));

		if (octets_sent != pkt_len) {
    		printf("error: couldn't send message %s", (char *)msg);
    	}
    	//printf("Seq: %u\n", ntohs(sender->message.header.seq));
		return octets_sent;
  
	}

	int SRTPSocket::rtp_recvfrom(rtp_receiver_ctx_t* receiver, void *msg, int *len)
	{
		int octets_recvd;
		err_status_t stat;
		int pkt_len = *len + SRTP_HEADER_LEN;
  
		octets_recvd = recvfrom(receiver->socket, (void *)&receiver->message,
			 pkt_len, 0, (struct sockaddr *) NULL, 0);

		if (octets_recvd == -1) {
			*len = 0;
			return -1;
		}

		/* verify rtp header */
		if (receiver->message.header.version != 2) {
			*len = 0;
			return -1;
		}

		//printf("%d octets received from SSRC %u\n", octets_recvd, receiver->message.header.ssrc);

		/* apply srtp */
		/*stat = srtp_unprotect(receiver->srtp_ctx, &receiver->message.header, &octets_recvd);
		if (stat) {
			printf("error: srtp unprotection failed with code %d%s\n", stat,
			    stat == err_status_replay_fail ? " (replay check failed)" :
			    stat == err_status_auth_fail ? " (auth check failed)" : "");
		    return -1;
  		}*/
  		//strncpy((char*)msg, receiver->message.body, octets_recvd);
		memcpy(msg, receiver->message.body, octets_recvd);
		return octets_recvd;
	}

	int SRTPSocket::rtp_sender_deinit_srtp(rtp_sender_ctx_t* sender)
	{
		return srtp_dealloc(sender->srtp_ctx);
	}

	int SRTPSocket::rtp_receiver_deinit_srtp(rtp_receiver_ctx_t* receiver)
	{
		return srtp_dealloc(receiver->srtp_ctx);
	}

	void SRTPSocket::rtp_sender_dealloc(rtp_sender_ctx_t* rtp_ctx)
	{
		free(rtp_ctx);
	}

	void SRTPSocket::rtp_receiver_dealloc(rtp_receiver_ctx_t* rtp_ctx)
	{
		free(rtp_ctx);
	}

	void SRTPSocket::printHashCode(const void* msg, int len)
	{
		sha1_ctx_t ctx;
		uint32_t hashcode[5];
		sha1_init(&ctx);

		sha1_update(&ctx, (uint8_t*)msg, len);
		sha1_final(&ctx, hashcode);
			
		printf("len: computed hash value:  %d, %u %u %u %u\n", len, 
			 hashcode[0], hashcode[1], hashcode[2], hashcode[3]);
	}
}

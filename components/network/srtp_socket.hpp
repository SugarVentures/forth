/*
	SRTP Wrapper - Real time streaming
*/

#ifndef OPPVS_SRTP_SOCKET_HPP
#define OPPVS_SRTP_SOCKET_HPP

/* SRTP modules */
#include "srtp.h"
#include "rtp_priv.h"


extern "C"
{
	#include "crypto_kernel.h"
	#include "sha1.h"

	
}

#include "physical_socket.hpp"


namespace oppvs
{
	const static uint8_t MAX_SRTP_KEY_LENGTH = 96;
	/*
	* RTP_HEADER_LEN indicates the size of an RTP header
	*/
	const static uint8_t SRTP_HEADER_LEN = 12;

	/* 
	 * RTP_MAX_BUF_LEN defines the largest RTP packet
	 */
	const static uint16_t SRTP_MAX_BUF_LEN = 16384;


	class SRTPSocket : public PhysicalSocket
	{
	public:
		SRTPSocket();
		SRTPSocket(uint32_t ssrc);
		~SRTPSocket();
		void setPolicy(char* key);
		static int initSRTPLib();
		static void setSecurityService(int mode);
		int initSender();
		int initReceiver();

		int SendTo(const void* msg, int len, uint32_t ts);
		int RecvFrom(void* msg, int *len);
		int RecvFrom(void* msg, int *len, bool *isNext);

		int Accept(SocketAddress& remote);
		int Connect(const SocketAddress& remote);
		int Close();

		void releaseSender();
		void releaseReceiver();

	private:
		srtp_policy_t m_policy;
		static sec_serv_t m_securityService;
		uint32_t m_ssrc;

		int rtp_sender_init(rtp_sender_ctx_t* sender, int sock, const struct sockaddr_in& dest, uint32_t ssrc);
		int rtp_sender_init_srtp(rtp_sender_ctx_t* sender, const srtp_policy_t *policy);

		int rtp_receiver_init(rtp_receiver_ctx_t* rcvr, int sock, const struct sockaddr_in& addr, uint32_t ssrc);
		int rtp_receiver_init_srtp(rtp_receiver_ctx_t* sender, const srtp_policy_t *policy);

		int rtp_sendto(rtp_sender_ctx_t* sender, const void* msg, int len, uint32_t timestamp);
		int rtp_recvfrom(rtp_receiver_ctx_t* receiver, void *msg, int *len);

		int rtp_sender_deinit_srtp(rtp_sender_ctx_t* sender);
		int rtp_receiver_deinit_srtp(rtp_receiver_ctx_t* receiver);

		void rtp_sender_dealloc(rtp_sender_ctx_t* rtp_ctx);
		void rtp_receiver_dealloc(rtp_receiver_ctx_t* rtp_ctx);


		rtp_sender_ctx_t* m_sender;
		rtp_receiver_ctx_t* m_receiver;

		uint32_t m_timestamp;
		uint16_t m_seq;

		void printHashCode(const void* msg, int len);

	};


}

#endif
/*
	Test functions of network engine
*/

#include "../interface/network.hpp"
#include <iostream>

#include <signal.h> //Ctrl C catch

#include <errno.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>		/* for close () socket function */

#include "../thread/concurrent_queue.hpp"

int sockid;
bool interrupt;
#define MAX_DATA_LENGTH 3686400

void signalvideohandler(int param)
{
 	interrupt = 1;
}

void senddata(const char* filename, oppvs::SRTPSocket& svr, int dest)
{
	char word[MAX_DATA_LENGTH];
	sha1_ctx_t ctx;
	uint32_t hashcode[5];
	sha1_init(&ctx);

	FILE* infile = fopen (filename, "r");
	if (infile == NULL) {
		printf("error: could not open file %s\n", filename);
		exit(1);
	}

	uint32_t ts = 0;
	
	while (!interrupt && !feof(infile)) {
		memset(word, 0, sizeof(word));
		int msgLength = fread(word, 1, MAX_DATA_LENGTH, infile);
		//printf("Send: %s \n", word);
		int sendLength = msgLength > OPPVS_NETWORK_PACKET_LENGTH ? OPPVS_NETWORK_PACKET_LENGTH : msgLength;
		const uint8_t* curPos = (uint8_t*)word;
		uint count = 0;

		

		while (msgLength > 0)
		{
			int len = svr.SendTo(curPos, sendLength, ts);
			if (len < 0)
			{
				printf("Send failed %s\n", strerror(errno));
				break;
			}


			/*unsigned char debugbuf[sendLength];
			memset(debugbuf, 0, sendLength);
			memcpy(debugbuf, curPos, sendLength);
			printf("Sent: %s\n", debugbuf);
			printf("length: %d %d\n", len, sendLength);*/
			curPos += sendLength;
			msgLength -= sendLength;
			count++;
			sendLength = msgLength > OPPVS_NETWORK_PACKET_LENGTH ? OPPVS_NETWORK_PACKET_LENGTH : msgLength;
		}
		printf("Number of packet per frame: %d\n", count);
		ts++;
		usleep(500);
	}

	fclose(infile);
}

int main(int argc, char* argv[])
{
	char key[oppvs::MAX_SRTP_KEY_LENGTH];
	sha1_ctx_t ctx;
	uint32_t hashcode[5];
	sha1_init(&ctx);

	std::cout << "Test network engine\n";
	int opt;
	if (argc > 1)
		opt = atoi(argv[1]);
	else
	{
		std::cout << "Enter 0 for sender, 1 for receiver\n";
		return -1;
	}


	
	interrupt = 0;

	if (opt == 0)
	{
		oppvs::ServerSocket server;

		oppvs::IPAddress ipAddr;
		oppvs::SocketAddress socketAddr;
		socketAddr.setIP(ipAddr);
		socketAddr.setPort(33432);
		printf("Init server \n");

		if (server.Create(AF_INET, SOCK_STREAM, 0) < 0)
		{
			printf("Open socket error %s\n", strerror(errno));
			return -1;
		}
		if (server.Bind(socketAddr) < 0)
		{
			printf("Binding error %s\n", strerror(errno));
			return -1;
		}

		if (server.Listen() < 0)
			return -1;

		oppvs::SocketAddress remoteAddr;
		while (1)
		{
			
			int sockfd = server.Accept(remoteAddr);
			if (sockfd >= 0)
			{
				printf("New subscriber \n");
				server.Close(sockfd);
				break;
			}
			usleep(500);
		}
		server.Close();

		socketAddr.setPort(33432);
		oppvs::SRTPSocket sender;
		sender.initSRTPLib();

		if ((sockid = sender.Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
			printf("Open socket error %s\n", strerror(errno));

		if (sender.Bind(socketAddr) < 0)
			printf("Binding error %s\n", strerror(errno));

		//if (server.Listen() < 0)
		//	printf("Listen error: %s\n", strerror(errno));

		remoteAddr.setPort(33433);

		//while (1)
		//{
			int newsockfd = sender.Accept(remoteAddr);
			if (newsockfd >= 0)
			{
				printf("Init sender \n");
				sender.setPolicy(key);
				sender.initSender();
				//int result = server.Send(newsockfd, (void*)msg, sizeof(msg));
				//printf("Send: ok: %d %s\n", result, strerror(errno));
				senddata(argv[2], sender, newsockfd);

				//server.Close(newsockfd);
			}
			//usleep(500);
		//}
		sender.releaseSender();
		sender.Close();
	}
	else if (opt == 1)
	{
		oppvs::SocketAddress socketAddr;
		socketAddr.setPort(33433);
		struct in_addr in;
		inet_pton(AF_INET, "127.0.0.1", &in);

		oppvs::IPAddress ip_server = oppvs::IPAddress(in);
		oppvs::SocketAddress serverAddr = oppvs::SocketAddress();
		serverAddr.setIP(ip_server);
		serverAddr.setPort(33432);

		oppvs::ClientSocket cli;
		oppvs::SocketAddress cliAddr;
		cliAddr.setPort(32120);
		int sid = cli.Create(AF_INET, SOCK_STREAM, 0);
		cli.Bind(cliAddr);
		if (cli.Connect(serverAddr) < 0)
			printf("Connect error %s\n", strerror(errno));
		
		cli.Close();

		oppvs::SRTPSocket client;
		client.initSRTPLib();

		serverAddr.setPort(33432);

		if ((sockid = client.Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
			printf("Open socket error %s\n", strerror(errno));

		if (client.Bind(socketAddr) < 0)
			printf("Client: Binding error %s\n", strerror(errno));

		
		client.setPolicy(key);

		if (client.Connect(serverAddr) < 0)
			printf("Connect error %s\n", strerror(errno));

		client.initReceiver();
		printf("Init SRTP receiver\n");
		/*
		char buffer[256];
		int outlen = client.Receive(buffer, sizeof(buffer));
		while (outlen > 0)
		{
			printf("%s\n", buffer);
			outlen = client.Receive(buffer, sizeof(buffer));
		}*/

		unsigned char outmsg[MAX_DATA_LENGTH];
		unsigned char buffer[OPPVS_NETWORK_PACKET_LENGTH];
		int len = OPPVS_NETWORK_PACKET_LENGTH;
		bool isNextFrame = false;
		uint32_t curPos = 0;
		while (!interrupt)
		{
			memset(buffer, 0, sizeof(buffer));
			int recvLen = client.RecvFrom(buffer, &len, &isNextFrame);
			if (recvLen > -1)
			{
				//printf("is Next frame: %d curPos %d\n", isNextFrame, curPos);
				if (isNextFrame)
				{
					
					printf("Data: %s \n", buffer);
					printf("Length: %d %d\n", len, recvLen);
					memcpy(outmsg, buffer, recvLen);
					curPos = recvLen;
				}
				else
				{
					memcpy(outmsg + curPos, buffer, recvLen);					
					curPos += recvLen;
				}
			}
			else
				printf("Error: %s\n", strerror(errno));
		}
		printf("%s \n", outmsg);
		client.releaseReceiver();
		client.Close();
	}
}
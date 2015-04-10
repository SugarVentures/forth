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


			unsigned char debugbuf[sendLength];
			memset(debugbuf, 0, sendLength);
			memcpy(debugbuf, curPos, sendLength);
			printf("Sent: %s\n", debugbuf);
			printf("length: %d %d\n", len, sendLength);
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
		oppvs::IPAddress localIpAddr;
    
	    oppvs::SocketAddress localSocketAddr; //Socket for signaling
	    localSocketAddr.setIP(localIpAddr);
	    localSocketAddr.setPort(32120);
	    
	    struct in_addr in;
	    inet_pton(AF_INET, "127.0.0.1", &in);
	    oppvs::IPAddress serverIpAddr = oppvs::IPAddress(in);
	    oppvs::SocketAddress serverSocketAddr;  //Address of signaling server
	    serverSocketAddr.setIP(serverIpAddr);
	    serverSocketAddr.setPort(33432);
	    
	    oppvs::ClientSocket signalSocket;
	    int sid = signalSocket.Create(AF_INET, SOCK_STREAM, 0);
	    if (sid < 0)
	    {
	       	printf("Create signaling socket failed\n");
	        return -1;
	    }
	    if (signalSocket.Bind(localSocketAddr) < 0)
	    {
	        printf("Binding socket error\n");
	        return -1;
	    }
	    if (signalSocket.Connect(serverSocketAddr) < 0)
	    {
	        printf("Connect error %s\n", strerror(errno));
	        return -1;
	    }
	    signalSocket.Close();
	    
	    char key[oppvs::MAX_SRTP_KEY_LENGTH];
	    oppvs::SRTPSocket clientSocket;
	    clientSocket.initSRTPLib();
	    clientSocket.setPolicy(key);
	    
	    localSocketAddr.setPort(33433);
	    if (clientSocket.Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP) < 0)
	    {
	        printf("Cant create srtp socket \n");
	        return -1;
	    }
	    if (clientSocket.Bind(localSocketAddr) < 0)
	    {
	        printf("Binding error\n");
	        return -1;
	    }
	    if (clientSocket.Connect(serverSocketAddr) < 0)
	    {
	        printf("Cannot connect streaming server\n");
	        return -1;
	    }
	    clientSocket.initReceiver();


		unsigned char outmsg[MAX_DATA_LENGTH];
		unsigned char buffer[OPPVS_NETWORK_PACKET_LENGTH];
		int len = OPPVS_NETWORK_PACKET_LENGTH;
		bool isNextFrame = false;
		uint32_t curPos = 0;
		clientSocket.setReceiveTimeOut(2);
		while (!interrupt)
		{
			memset(buffer, 0, sizeof(buffer));
			int recvLen = clientSocket.RecvFrom(buffer, &len, &isNextFrame);
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
					printf("Length: %d %d\n", len, recvLen);
					curPos += recvLen;
				}
			}
			else
			{
				printf("Error: %s\n", strerror(errno));
				interrupt = 1;
			}
		}
		printf("%s \n", outmsg);
		clientSocket.releaseReceiver();
		clientSocket.Close();
	}
}
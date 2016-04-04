#include "datatypes.hpp"
#include "physical_socket.hpp"

using namespace oppvs;

#define MAX_DATA_LENGTH 3686400

bool interrupt;

void senddata(const char* filename, PhysicalSocket& svr, SocketAddress& server)
{
	char word[MAX_DATA_LENGTH];

	FILE* infile = fopen (filename, "r");
	if (infile == NULL) {
		printf("error: could not open file %s\n", filename);
		return;
	}
	
	while (!interrupt && !feof(infile)) {
		memset(word, 0, sizeof(word));
		int msgLength = fread(word, 1, MAX_DATA_LENGTH, infile);
		//printf("Send: %s \n", word);
		int sendLength = msgLength > OPPVS_NETWORK_PACKET_LENGTH ? OPPVS_NETWORK_PACKET_LENGTH : msgLength;
		const uint8_t* curPos = (uint8_t*)word;
		uint count = 0;		

		while (msgLength > 0)
		{
			int len = svr.Send(svr.getSocketHandle(), (void*)curPos, sendLength, server);
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
		
		usleep(500);
	}

	fclose(infile);
}

int main(int argc, char const *argv[])
{
	PhysicalSocket socket;
	SocketAddress forwardingServerAddress;
	if (socket.Create(AF_INET, SOCK_STREAM, 0) < 0)
	{
		return -1;
	}

	if (socket.Bind(forwardingServerAddress) < 0)
	{
		return -1;
	}

	printf("Open socket to connect forwarding server %s\n", socket.getLocalAddress().toString().c_str());

	forwardingServerAddress.setIP(IPAddress("192.168.0.104"));
    forwardingServerAddress.setPort(DEFAULT_FORWARDING_SERVER_PORT);
	if (socket.Connect(forwardingServerAddress) < 0)
	{
		printf("Cannot connect forwarding server %s\n", strerror(errno));
		return -1;
	}

	if (argc > 1)
	{
		senddata(argv[1], socket, forwardingServerAddress);
	}
	socket.Close();
	return 0;
}
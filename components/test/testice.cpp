#include "ice_common.hpp"
#include <iostream>
#include <cstdint>
#include <cstring>

#include "ice_manager.hpp"
#include "signaling_manager.hpp"

#include <signal.h>

bool interrupt;

void signalhandler(int param)
{
 	interrupt = 1;
}

void callbackCandidateGatheringDone(void* object, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates)
{
	std::cout << "call back " << std::endl;
	std::cout << "Local credential: " << username << " " << password << std::endl;

	oppvs::SignalingManager *sigManager = (oppvs::SignalingManager*)object;
	sigManager->sendRequest(username, password, candidates);

	for (int i = 0; i < candidates.size(); i++)
	{
        std::cout << "Candidate: " << candidates[i].component << " "
			  << candidates[i].foundation << " "
			  << candidates[i].priority << " "
			  << candidates[i].ip << " "
			  << candidates[i].protocol << " "
			  << candidates[i].port << " "
			  << candidates[i].type << std::endl;
	}
}

int main(int argc, char* argv[])
{
	std::string address, username, password;
	int opt = 0;
	signal(SIGINT, signalhandler);

	if (argc > 4)
	{
		address = argv[1];
		username = argv[2];
		password = argv[3];
		opt = atoi(argv[4]);
	}
	else
	{
		address = std::string("127.0.0.1");
		username = "";
		password = "";
	}

	oppvs::IceServerInfo stunServer(address, oppvs::DEFAULT_STUN_PORT, "", "");
	oppvs::IceServerInfo turnServer(address, oppvs::DEFAULT_STUN_PORT, username, password);

	oppvs::SocketAddress signalingServerAddress;
	signalingServerAddress.setIP(oppvs::IPAddress("127.0.0.1"));
	signalingServerAddress.setPort(33333);

	oppvs::SignalingManager sigManager(signalingServerAddress);
	sigManager.init();
	//oppvs::IceManager iceManager((void*)&sigManager);
	//iceManager.init(stunServer, turnServer);

	std::cout << "Server: " << turnServer.serverAddress << " port: " << turnServer.port << " user: " << turnServer.username << " " << turnServer.password << std::endl;
	std::string streamKey = std::string("1234", oppvs::STREAM_KEY_SIZE);
	if (opt == 0)
	{
		std::cout << "Broadcaster" << std::endl;
		sigManager.sendStreamRegister(streamKey);
	}
	else
	{
		std::cout << "Viewer" << std::endl;
	}	
	
	
	/*iceManager.registerCallback(callbackCandidateGatheringDone);

	oppvs::IceStream* stream = iceManager.createStream();
	stream->requestLocalCandidates();*/

	while (!interrupt)
	{
		usleep(1000);
	}
	//delete stream;
	//iceManager.release();
	return 0;
}
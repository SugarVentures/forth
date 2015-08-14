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

// This routine will set SIGINT And SIGTERM to "blocked" status only so that the
// child worker threads will never have these events raised on them.
int initAppExitListener()
{
    sigset_t sigs;   
    int ret;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGTERM);

    ret = pthread_sigmask(SIG_BLOCK, &sigs, NULL);
    if (ret > 0)
    	std::cout << "exit error: " << ret << std::endl;
    return 0;
}

void waitForAppExitSignal()
{
    while (true)
    {
        sigset_t sigs;
        sigemptyset(&sigs);
        sigaddset(&sigs, SIGINT);
        sigaddset(&sigs, SIGTERM);
        int sig = 0;
        
        int ret = sigwait(&sigs, &sig);
        if ((sig == SIGINT) || (sig == SIGTERM))
        {
            break;
        }
    }
}

void callbackCandidateGatheringDone(void* object, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates)
{
	std::cout << "call back " << std::endl;
	std::cout << "Local credential: " << username << " " << password << std::endl;

	oppvs::SignalingManager *sigManager = (oppvs::SignalingManager*)object;
	sigManager->sendIceResponse(username, password, candidates);

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

void callbackOnIceRequest(void* object)
{
	std::cout << "call back on ice request" << std::endl;

	oppvs::IceManager *iceManager = (oppvs::IceManager*)object;
	oppvs::IceStream* stream = iceManager->createStream();
	stream->requestLocalCandidates();
}

int main(int argc, char* argv[])
{
	std::string address, username, password;
	int opt = 0;
	signal(SIGPIPE, SIG_IGN);
    
    initAppExitListener();

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
	

	oppvs::IceManager iceManager((void*)&sigManager);
	iceManager.init(stunServer, turnServer);

	sigManager.registerCallback(callbackOnIceRequest, (void*)&iceManager);
	iceManager.registerCallback(callbackCandidateGatheringDone);

	std::cout << "Server: " << turnServer.serverAddress << " port: " << turnServer.port << " user: " << turnServer.username << " " << turnServer.password << std::endl;
	std::string streamKey = std::string("1234", oppvs::STREAM_KEY_SIZE);
	if (opt == 0)
	{
		std::cout << "Broadcaster" << std::endl;
		sigManager.sendStreamRegister(streamKey);
		sigManager.waitResponse();
	}
	else
	{
		std::cout << "Viewer" << std::endl;
	}	
	
	waitForAppExitSignal();
	sigManager.signalForStop();
	iceManager.release();
	return 0;
}
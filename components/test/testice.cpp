#include "ice_common.hpp"
#include <iostream>
#include <cstdint>
#include <cstring>

#include "ice_manager.hpp"
#include "signaling_handler.hpp"

#include <signal.h>

bool interrupt;
int opt = 0;
oppvs::IceStream* sendstream;
oppvs::SignalingManager* pManager;

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
        printf("quit %d\n", ret);
        if ((sig == SIGINT) || (sig == SIGTERM))
        {
        	
            break;
        }
    }
}

void callbackOnReceiveImpl(void* object, uint8_t* data, uint32_t len)
{
	if (opt == 0)
	{
		
	}
	std::cout<< "Receive packets\n";
}


void callbackCandidateGatheringDone(void* object, void* icemgr, uint32_t streamid, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates)
{
	std::cout << "call back " << std::endl;
	std::cout << "Local credential: " << username << " " << password << std::endl;

	if (opt == 0)
	{
		((oppvs::IceManager*)icemgr)->establishPeerConnection(streamid);
	}
	else if (opt == 1)
	{
		oppvs::SignalingManager* sig = (oppvs::SignalingManager*)object;
		int ret = sig->sendStreamRequest(username, password, candidates);
		std::cout << "Send stream request " << ret << "\n";
	}
	else
	{
		((oppvs::IceManager*)icemgr)->establishPeerConnection(streamid);
	}

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

void callbackOnIceResponse(void* object, std::string& username, std::string& password, std::vector<oppvs::IceCandidate>& candidates)
{
	std::cout << "call back on ice response" << std::endl;
	//candidates.erase(candidates.begin(), candidates.begin() + candidates.size() - 1);
	oppvs::IceManager* icemgr = (oppvs::IceManager*)object;
	icemgr->setPeerInfo(username, password, candidates);
	oppvs::IceStream* stream = icemgr->createStream();
	icemgr->attachCallbackEvent(callbackOnReceiveImpl, object);
	stream->requestLocalCandidates();
	if (opt == 1)
		opt = 3;
	
	for (int i = 0; i < candidates.size(); i++)
	{
        std::cout << "Remote Candidate: " << candidates[i].component << " "
			  << candidates[i].foundation << " "
			  << candidates[i].priority << " "
			  << candidates[i].ip << " "
			  << candidates[i].protocol << " "
			  << candidates[i].port << " "
			  << candidates[i].type << std::endl;
	}
}

void callbackNewSubscriberImpl(void* object, oppvs::IceStream* stream)
{
	std::cout<<"New subs\n";
	sendstream = stream;

	gchar msg[] = "hello world!";
	printf("Send message %d\n", sizeof(msg));
	sendstream->send(sizeof(msg), (uint8_t*)msg, 1);
		
}

int updateStreamInfo(const oppvs::ServiceInfo& info)
{
	std::cout << "Stream response\n";
	pManager->sendPeerRegister();
	return 0;
}


int main(int argc, char* argv[])
{
	std::string address, username, password;
	
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
	signalingServerAddress.setIP(address);
	signalingServerAddress.setPort(33333);

	oppvs::SignalingManager sigManager(signalingServerAddress);
	oppvs::ServiceInfo info;
	info.videoStreamInfo.noSources = 1;
	info.videoStreamInfo.sources = new oppvs::VideoSourceInfo[1];
	info.videoStreamInfo.sources[0].width = 1280;
	info.videoStreamInfo.sources[0].height = 780;

	oppvs::IceManager* icemgr = new oppvs::IceManager();
	pManager = &sigManager;

	if (sigManager.init() < 0)
		return -1;

	if (opt == 0)
	{
		if (sigManager.sendStreamRegister("7116f0d7-5c27-44e6-8aa4-bc4ddeea9935", info) < 0)
		{	
			return -1;
		}
		icemgr->init(stunServer, turnServer, 1);
		icemgr->attachCallbackEvent(callbackCandidateGatheringDone, (void*)&sigManager);
		icemgr->attachCallbackEvent(callbackNewSubscriberImpl, NULL);

		sigManager.attachCallbackEvent(callbackOnIceResponse, (void*)icemgr);
	}
	else
	{
		sigManager.setStreamKey("7116f0d7-5c27-44e6-8aa4-bc4ddeea9935");
		sigManager.attachCallback(updateStreamInfo);
		icemgr->init(stunServer, turnServer, 0);
		icemgr->attachCallbackEvent(callbackCandidateGatheringDone, (void*)&sigManager);
		icemgr->attachCallbackEvent(callbackNewSubscriberImpl, NULL);

		icemgr->attachCallbackEvent(callbackOnReceiveImpl, (void*)icemgr);
		oppvs::IceStream* stream = icemgr->createStream();
		stream->requestLocalCandidates();
		sigManager.attachCallbackEvent(callbackOnIceResponse, (void*)icemgr);
	}
	
	std::thread waitThread(&oppvs::SignalingManager::waitResponse, &sigManager);
	waitThread.join();
	waitForAppExitSignal();
	
	icemgr->release();
	delete icemgr;
	return 0;
}
#include <iostream>
#include <errno.h>
#include "crc.hpp"
#include <iomanip>
#include <string>
#include <signal.h>

#include "forwarding_server.h"


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
    	std::cout << "Server exit error: " << ret << std::endl;
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

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Please enter primary server address: [local address] [public address]" << std::endl;
		return -1;
	}

	char* localAddress = argv[1];

	signal(SIGPIPE, SIG_IGN);
    
    initAppExitListener();
	
	oppvs::ForwardingServer server;
    oppvs::ForwardingServerConfiguration config;
    config.addressListen.setIP(oppvs::IPAddress(localAddress));
    config.addressListen.setPort(oppvs::DEFAULT_FORWARDING_SERVER_PORT);
	std::cout << "Start server" << std::endl;


	server.init(config);
	server.start();
	waitForAppExitSignal();
    server.shutdown();
	return 0;
}
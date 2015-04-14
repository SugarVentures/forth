/*
	Test video engine from command line
*/
#include "../interface/video_capture.hpp"
#include "../video/mac_video_capture.hpp"
#include "../error/error.hpp"
#include <stdio.h>

#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include <vector>
#include <signal.h> //Ctrl C catch
#include <unistd.h>


void fcallback(oppvs::PixelBuffer& pf);

oppvs::MacVideoEngine* ve;

void signalvideohandler(int param)
{
    if (ve != NULL)
    {
        ve->stopRecording();
        delete ve;
    }
    exit(1);
}

int main(int argc, char* argv[])
{
	void* cap;
    int user1 = 1;
    int user2 = 2;
    void* user;

    signal(SIGINT, signalvideohandler);

    oppvs::window_rect_t rect(0, 0, 500, 500);

	ve = new oppvs::MacVideoEngine(fcallback, user);

	std::vector<oppvs::VideoScreenSource> v;
    std::vector<oppvs::VideoCaptureDevice> devices;

    ve->getListCaptureDevices(devices);
    int device_index = 0;
    for (std::vector<oppvs::VideoCaptureDevice>::const_iterator i = devices.begin(); i != devices.end(); ++i)
    {
        /*std::cout << "Device: " << i->device_id << ' ';
        std::cout << "name: " << i->device_name << ' ';
        std::cout << "Cap: " << i->capabilities.front().width << ' ' << i->capabilities.front().height;
        std::cout << "Format: " << i->capabilities.front().fps;
        std::cout << '\n';*/
        ve->addSource(oppvs::VST_WEBCAM, device_index, 1, rect, &user1);
        device_index++;
    }

	ve->getListVideoSource(v);
    
	/*std::cout << "Number of available windows: " << v.size() << "\n";
	for (std::vector<oppvs::VideoScreenSource>::const_iterator i = v.begin(); i != v.end(); ++i)
	{
    	std::cout << "Window id: " << i->id << ' ';
    	std::cout << "Window Title: " << i->title << ' ';
    	std::cout << "App id: " << i->app_id << ' ';
    	std::cout << "App name: " << i->app_name << ' ';
    	std::cout << '\n';
        uint32_t capability = 1;
        ve->addSource(oppvs::VST_WINDOW, i->id, capability, rect);
    }*/

    ve->addSource(oppvs::VST_WINDOW, 0, 1, rect, &user2);
    std::string str = "FaceTime";
    ve->getDeviceID(str);
    
	ve->setupCaptureSessions();
    ve->startRecording();

    while (1)
    {
        usleep(100);
    }
}

void fcallback(oppvs::PixelBuffer& buffer)
{
    printf("Process frame callback %d \n", *(int*)buffer.user);
    //printf("Frame callback: %lu bytes, stride: %lu width: %d height: %d\n", buffer.nbytes, buffer.stride[0],
    //    buffer.width, buffer.height);
    //printf("%s %d", (const char*)buffer.plane[0], buffer.nbytes);
}



/*
	Test video engine from command line
*/
#include "../interface/video_capture.hpp"
#include "../video/mac_video_capture.hpp"
#include "../error/error.hpp"
#include "../streaming/video_stream.hpp"
#include "../interface/stream.hpp"
#include "../streaming/streaming_engine.hpp"

#include <stdio.h>

#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include <vector>
#include <signal.h> //Ctrl C catch
#include <unistd.h>

#include <time.h>
#include <chrono>


void fcallback(oppvs::PixelBuffer& pf);
static int frames = 0;
clock_t starttime = 0;

oppvs::MacVideoEngine* ve;
typedef std::chrono::high_resolution_clock Clock;
oppvs::PixelBuffer sharedBuffer;

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

    sharedBuffer.width[0] = 1280;
    sharedBuffer.height[0] = 780;
    sharedBuffer.nbytes = 1280*780*4;
    sharedBuffer.plane[0] = new uint8_t[sharedBuffer.nbytes];
    sharedBuffer.format = oppvs::PF_BGRA32;
    oppvs::ControllerLinker controller;
    controller.render = &user1;

    signal(SIGINT, signalvideohandler);

    oppvs::window_rect_t rect1(0, 1280, 780, 0);
    oppvs::window_rect_t rect2(0, 1000, 1000, 0);

	ve = new oppvs::MacVideoEngine(fcallback, user);

	std::vector<oppvs::VideoScreenSource> v;
    std::vector<oppvs::VideoCaptureDevice> devices;

    ve->getListCaptureDevices(devices);
    int device_index = 0;
    oppvs::VideoActiveSource *source;
    for (std::vector<oppvs::VideoCaptureDevice>::const_iterator i = devices.begin(); i != devices.end(); ++i)
    {
        std::cout << "Device: " << i->device_id << ' ';
        std::cout << "name: " << i->device_name << ' ';
        std::cout << "Cap: " << i->capabilities.front().width << ' ' << i->capabilities.front().height;
        std::cout << "Format: " << i->capabilities.front().fps;
        std::cout << '\n';
        //source = ve->addSource(oppvs::VST_WEBCAM, i->device_id, 30, rect1, &controller);
        
        device_index++;
    }

    std::vector<oppvs::Monitor> monitors;
    ve->getListMonitors(monitors);
     for (std::vector<oppvs::Monitor>::const_iterator it = monitors.begin(); it != monitors.end(); ++it)
    {

        source = ve->addSource(oppvs::VST_WINDOW, std::to_string(it->id), 30, rect1, &controller);
    }
    
	/*ve->getListVideoSource(v);
    
	std::cout << "Number of available windows: " << v.size() << "\n";
	for (std::vector<oppvs::VideoScreenSource>::const_iterator i = v.begin(); i != v.end(); ++i)
	{
    	std::cout << "Window id: " << i->id << ' ';
    	std::cout << "Window Title: " << i->title << ' ';
    	std::cout << "App id: " << i->app_id << ' ';
    	std::cout << "App name: " << i->app_name << ' ';
    	std::cout << '\n';
        uint32_t capability = 1;
        ve->addSource(oppvs::VST_WINDOW, std::to_string(i->id), 30, rect1, &controller);
    }

    ve->printListActiveSources();
    ve->removeSource(oppvs::VST_WINDOW, std::to_string(60));
    ve->printListActiveSources();
    return 0;*/
    //ve->addSource(oppvs::VST_WINDOW, 0, 30, rect2, &user2);
    //std::string str = "FaceTime";
    //ve->getDeviceID(str);
    
	//ve->setupCaptureSessions();
    ve->setupCaptureSession(*source);
    ve->startCaptureSession(*source);
    auto t1 = Clock::now();
    //ve->startRecording();

    int count = 0;
    while (1)
    {
        /*if (count++ == 20000)
        {
            printf("Upate Configuration \n");
            source.rect.top = 200;
            source.rect.right = 200;
            //ve->updateConfiguration(source);
            auto t2 = Clock::now();
            std::cout <<std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << '\n';
            std::cout <<frames << '\n';
            signalvideohandler(SIGINT);


        }*/
        usleep(100);
    }
}

void fcallback(oppvs::PixelBuffer& buffer)
{
    ++frames;
    
    printf("Frame callback: %lu bytes, stride: %lu width: %d height: %d bpr: %d origin: %d %d\n", buffer.nbytes, buffer.stride[0],
        buffer.width[0], buffer.height[0], buffer.stride[0]/buffer.width[0], buffer.originx, buffer.originy);
    /*for (int i = 0; i < (uint32_t)buffer.height[0]; i++)
    {
        uint32_t offset = buffer.stride[0]*(buffer.originy + i) + buffer.originx*4;
        memcpy(sharedBuffer.plane[0], buffer.plane[0] + offset, buffer.width[0]*4);
    }*/
    //printf("%s %d", (const char*)buffer.plane[0], buffer.nbytes);
}



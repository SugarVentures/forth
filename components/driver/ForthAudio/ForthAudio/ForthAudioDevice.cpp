
#include "ForthAudioDevice.h"

//	System Includes
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOTimerEventSource.h>

#define super IOService

OSDefineMetaClassAndStructors(com_sugar_driver_ForthAudioDevice, IOService)

bool ForthAudioDevice::start(IOService *inProvider)
{
    bool theAnswer = IOService::start(inProvider);
    if(theAnswer)
    {
        //	create the work loop
        mWorkLoop = IOWorkLoop::workLoop();
        FailIfNULL(mWorkLoop, theAnswer = kIOReturnNoResources, Failure, "ForthAudioDevice::start: couldn't allocate the work loop");
        
        //	create the command gate
        mCommandGate = IOCommandGate::commandGate(this);
        FailIfNULL(mWorkLoop, theAnswer = kIOReturnNoResources, Failure, "ForthAudioDevice::start: couldn't allocate the command gate");
        
        //	attach it to the work loop
        mWorkLoop->addEventSource(mCommandGate);
        
        //	initialize the stuff tracked by the IORegistry
        mSampleRate = INITIAL_SAMPLE_RATE;
        setProperty(kForthAudio_RegistryKey_SampleRate, mSampleRate, sizeof(mSampleRate) * 8);
        
        mIOBufferFrameSize = INITIAL_IOBUFFER_FRAME_SIZE;
        setProperty(kForthAudio_RegistryKey_RingBufferFrameSize, mIOBufferFrameSize, sizeof(mIOBufferFrameSize) * 8);
        
        char theDeviceUID[128];
        snprintf(theDeviceUID, 128, "ForthAudioDevice-%d", static_cast<int>(random() % 100000));
        setProperty(kForthAudio_RegistryKey_DeviceUID, theDeviceUID);
        
        //	allocate the IO buffers
        IOReturn theError = allocateBuffers();
        FailIfError(theError, theAnswer = false, Failure, "ForthAudioDevice::start: allocating the buffers failed");
        
        //	initialize the timer that stands in for a real interrupt
        theError = initTimer();
        FailIfError(theError, freeBuffers(); theAnswer = false, Failure, "ForthAudioDevice::start: initializing the timer failed");
        
        //	initialize the controls
        theError = initControls();
        FailIfError(theError, theAnswer = false, Failure, "ForthAudioDevice::start: initializing the controls failed");
    }
    return theAnswer;
    
Failure:
    if (mCommandGate != NULL)
    {
        if(mWorkLoop != NULL)
        {
            mWorkLoop->removeEventSource(mCommandGate);
            mCommandGate->release();
            mCommandGate = NULL;
        }
    }
    
    if(mWorkLoop != NULL)
    {
        mWorkLoop->release();
        mWorkLoop = NULL;
    }
    
    freeBuffers();
    destroyTimer();
    
    return theAnswer;
}

void ForthAudioDevice::stop(IOService* inProvider)
{
    //	tear things down
    freeBuffers();
    destroyTimer();
    if(mCommandGate != NULL)
    {
        if(mWorkLoop != NULL)
        {
            mWorkLoop->removeEventSource(mCommandGate);
            mCommandGate->release();
            mCommandGate = NULL;
        }
    }
    if(mWorkLoop != NULL)
    {
        mWorkLoop->release();
        mWorkLoop = NULL;
    }
    IOService::stop(inProvider);
}



IOReturn ForthAudioDevice::allocateBuffers()
{
    IOReturn theAnswer = kIOReturnSuccess;
    
    //	The status buffer holds the zero time stamp when IO is running
    mStatusBuffer = IOBufferMemoryDescriptor::withOptions(kIOMemoryKernelUserShared, sizeof(ForthAudioStatus));
    FailIfNULL(mStatusBuffer, theAnswer = kIOReturnNoMemory, Failure, "ForthAudioDevice::allocateBuffers: failed to allocate the status buffer");
    bzero(mStatusBuffer->getBytesNoCopy(), mStatusBuffer->getCapacity());
    
    //	These are the ring buffers for transmitting the audio data
    
    //	Note that for this driver the samples are always 16 bit stereo
    mInputBuffer = IOBufferMemoryDescriptor::withOptions(kIOMemoryKernelUserShared, mIOBufferFrameSize * 2 * 2);
    FailIfNULL(mInputBuffer, theAnswer = kIOReturnNoMemory, Failure, "ForthAudioDevice::allocateBuffers: failed to allocate the input buffer");
    bzero(mInputBuffer->getBytesNoCopy(), mInputBuffer->getCapacity());
    
    mOutputBuffer = IOBufferMemoryDescriptor::withOptions(kIOMemoryKernelUserShared, mIOBufferFrameSize * 2 * 2);
    FailIfNULL(mOutputBuffer, theAnswer = kIOReturnNoMemory, Failure, "ForthAudioDevice::allocateBuffers: failed to allocate the output buffer");
    bzero(mOutputBuffer->getBytesNoCopy(), mOutputBuffer->getCapacity());
    
    return kIOReturnSuccess;
    
Failure:
    if(mStatusBuffer != NULL)
    {
        mStatusBuffer->release();
        mStatusBuffer = NULL;
    }
    
    if(mInputBuffer != NULL)
    {
        mInputBuffer->release();
        mInputBuffer = NULL;
    }
    
    if(mOutputBuffer != NULL)
    {
        mOutputBuffer->release();
        mOutputBuffer = NULL;
    }
    
    return theAnswer;
}

void ForthAudioDevice::freeBuffers()
{
    if(mStatusBuffer != NULL)
    {
        mStatusBuffer->release();
        mStatusBuffer = NULL;
    }
    
    if(mInputBuffer != NULL)
    {
        mInputBuffer->release();
        mInputBuffer = NULL;
    }
    
    if(mOutputBuffer != NULL)
    {
        mOutputBuffer->release();
        mOutputBuffer = NULL;
    }
}

IOReturn ForthAudioDevice::initTimer()
{
    IOReturn theAnswer = kIOReturnSuccess;
    
    //	create the timer event source that will be our fake interrupt
    mTimerEventSource = IOTimerEventSource::timerEventSource(this, timerFired);
    FailIfNULL(mTimerEventSource, theAnswer = kIOReturnNoResources, Failure, "ForthAudioDevice::initTimer: couldn't allocate the timer event source");
    
    //	add the timer to the work loop
    mWorkLoop->addEventSource(mTimerEventSource);
    
    //	calculate how many ticks are in each buffer
    updateTimer();
    
    return kIOReturnSuccess;
    
Failure:
    if(mTimerEventSource != NULL)
    {
        if(mWorkLoop != NULL)
        {
            mWorkLoop->removeEventSource(mTimerEventSource);
        }
        mTimerEventSource->release();
        mTimerEventSource = NULL;
    }
    
    if(mWorkLoop != NULL)
    {
        mWorkLoop->release();
        mWorkLoop = NULL;
    }
    
    return theAnswer;
}

void ForthAudioDevice::destroyTimer()
{
    if(mTimerEventSource != NULL)
    {
        if(mWorkLoop != NULL)
        {
            mWorkLoop->removeEventSource(mTimerEventSource);
        }
        mTimerEventSource->release();
        mTimerEventSource = NULL;
    }
    
    if(mWorkLoop != NULL)
    {
        mWorkLoop->release();
        mWorkLoop = NULL;
    }
}

IOReturn ForthAudioDevice::startTimer()
{
    IOReturn theAnswer = kIOReturnSuccess;
    
    if((mStatusBuffer != NULL) && (mTimerEventSource != NULL))
    {
        //	clear the status buffer
        ForthAudioStatus* theStatus = (ForthAudioStatus*)mStatusBuffer->getBytesNoCopy();
        theStatus->mSampleTime = 0;
        theStatus->mHostTime = 0;
        
        //	calculate how many ticks are in each buffer
        struct mach_timebase_info theTimeBaseInfo;
        clock_timebase_info(&theTimeBaseInfo);
        mHostTicksPerBuffer = (mIOBufferFrameSize * 1000000000ULL) / mSampleRate;
        mHostTicksPerBuffer = (mHostTicksPerBuffer * theTimeBaseInfo.denom) / theTimeBaseInfo.numer;
        
        //	start the timer, the first time stamp will be taken when it goes off
        union { UInt64 mUInt64; AbsoluteTime mAbsoluteTime; } theNextWakeTime;
        theNextWakeTime.mUInt64 = mHostTicksPerBuffer;
        mTimerEventSource->setTimeout(theNextWakeTime.mAbsoluteTime);
    }
    else
    {
        theAnswer = kIOReturnNoResources;
    }
    
    return theAnswer;
}

void ForthAudioDevice::stopTimer()
{
    if(mTimerEventSource != NULL)
    {
        mTimerEventSource->cancelTimeout();
    }
}

void ForthAudioDevice::updateTimer()
{
    struct mach_timebase_info theTimeBaseInfo;
    clock_timebase_info(&theTimeBaseInfo);
    mHostTicksPerBuffer = (mIOBufferFrameSize * 1000000000ULL) / mSampleRate;
    mHostTicksPerBuffer = (mHostTicksPerBuffer * theTimeBaseInfo.denom) / theTimeBaseInfo.numer;
}



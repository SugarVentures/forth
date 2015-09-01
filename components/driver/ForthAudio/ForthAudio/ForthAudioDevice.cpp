
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

IOBufferMemoryDescriptor*	ForthAudioDevice::getBuffer(int inBufferType)
{
    //	we gate the external methods onto the work loop for thread safety
    IOBufferMemoryDescriptor* theAnswer = NULL;
    if(mCommandGate != NULL)
    {
        mCommandGate->runAction(_getBuffer, &inBufferType, &theAnswer);
    }
    return theAnswer;
}

IOReturn ForthAudioDevice::startHardware()
{
    //	we gate the external methods onto the work loop for thread safety
    IOReturn theAnswer = kIOReturnSuccess;
    if(mCommandGate != NULL)
    {
        theAnswer = mCommandGate->runAction(_startHardware);
    }
    else
    {
        theAnswer = kIOReturnNoResources;
    }
    return theAnswer;
}

void ForthAudioDevice::stopHardware()
{
    //	we gate the external methods onto the work loop for thread safety
    if(mCommandGate != NULL)
    {
        mCommandGate->runAction(_stopHardware);
    }
}

IOReturn ForthAudioDevice::setSampleRate(UInt64 inNewSampleRate)
{
    //	we gate the external methods onto the work loop for thread safety
    IOReturn theAnswer = kIOReturnSuccess;
    if(mCommandGate != NULL)
    {
        theAnswer = mCommandGate->runAction(_setSampleRate, &inNewSampleRate);
    }
    else
    {
        theAnswer = kIOReturnNoResources;
    }
    return theAnswer;
}

IOReturn ForthAudioDevice::_getBuffer(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3)
{
#pragma unused(inArg2, inArg3)
    IOReturn theAnswer = kIOReturnSuccess;
    ForthAudioDevice* theDriver = NULL;
    const int* theBufferType = NULL;
    IOBufferMemoryDescriptor** theBuffer = NULL;
    
    //	cast the arguments back to what they need to be
    theDriver = OSDynamicCast(ForthAudioDevice, inTarget);
    FailIfNULL(theDriver, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: this is not a ForthAudioDevice");
    
    theBufferType = reinterpret_cast<const int*>(inArg0);
    FailIfNULL(theBufferType, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: no buffer type");
    
    theBuffer = reinterpret_cast<IOBufferMemoryDescriptor**>(inArg1);
    FailIfNULL(theBuffer, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: no buffer");
    
    switch(*theBufferType)
    {
        case kForthAudio_Buffer_Status:
            *theBuffer = theDriver->mStatusBuffer;
            break;
            
        case kForthAudio_Buffer_Input:
            *theBuffer = theDriver->mInputBuffer;
            break;
            
        case kForthAudio_Buffer_Output:
            *theBuffer = theDriver->mOutputBuffer;
            break;
    };
    
Done:
    return theAnswer;
}

IOReturn ForthAudioDevice::_startHardware(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3)
{
    //	This driver uses a work loop timer to simulate an interrupt to driver the timing
    
#pragma unused(inArg0, inArg1, inArg2, inArg3)
    IOReturn theAnswer = kIOReturnSuccess;
    
    //	cast the arguments back to what they need to be
    ForthAudioDevice* theDriver = OSDynamicCast(ForthAudioDevice, inTarget);
    FailIfNULL(theDriver, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: this is not a ForthAudioDevice");
    
    if(!theDriver->mIsRunning)
    {
        if((theDriver->mInputBuffer != NULL) && (theDriver->mOutputBuffer != NULL))
        {
            //	clear the buffers
            bzero(theDriver->mInputBuffer->getBytesNoCopy(), theDriver->mInputBuffer->getCapacity());
            bzero(theDriver->mOutputBuffer->getBytesNoCopy(), theDriver->mOutputBuffer->getCapacity());
            
            //	start the timer
            theAnswer = theDriver->startTimer();
            FailIfError(theAnswer, , Done, "ForthAudioDevice::_startHardware: starting the timer failed");
            
            //	update the is running state
            theDriver->mIsRunning = true;
        }
        else
        {
            theAnswer = kIOReturnNoResources;
        }
    }
    
Done:
    return theAnswer;
}

IOReturn ForthAudioDevice::_stopHardware(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3)
{
    //	cast the arguments back to what they need to be
#pragma unused(inArg0, inArg1, inArg2, inArg3)
    ForthAudioDevice* theDriver = OSDynamicCast(ForthAudioDevice, inTarget);
    if((theDriver != NULL) && theDriver->mIsRunning)
    {
        //	all we need to do is stop the timer
        theDriver->stopTimer();
        theDriver->mIsRunning = false;
    }
    return kIOReturnSuccess;
}

IOReturn	ForthAudioDevice::_setSampleRate(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3)
{
#pragma unused(inArg1, inArg2, inArg3)
    IOReturn theAnswer = kIOReturnSuccess;
    ForthAudioDevice* theDriver = NULL;
    const UInt64* theNewSampleRate = NULL;
    
    //	cast the arguments back to what they need to be
    theDriver = OSDynamicCast(ForthAudioDevice, inTarget);
    FailIfNULL(theDriver, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_setSampleRate: this is not a ForthAudioDevice");
    
    theNewSampleRate = reinterpret_cast<const UInt64*>(inArg0);
    FailIfNULL(theNewSampleRate, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_setSampleRate: no new sample rate");
    
    //	make sure that IO is stopped
    FailIf(theDriver->mIsRunning, theAnswer = kIOReturnNotPermitted, Done, "ForthAudioDevice::_setSampleRate: can't change the sample rate while IO is running");
    
    //	make sure the sample rate is something we support
    if((*theNewSampleRate == 44100) || (*theNewSampleRate == 48000))
    {
        theDriver->mSampleRate = *theNewSampleRate;
        theDriver->setProperty(kForthAudio_RegistryKey_SampleRate, theDriver->mSampleRate, sizeof(theDriver->mSampleRate) * 8);
        theDriver->updateTimer();
    }
    else
    {
        theAnswer = kIOReturnUnsupported;
    }
    
Done:
    return theAnswer;
}

IOReturn ForthAudioDevice::getVolume(int inVolumeID, UInt32& outVolume)
{
    //	we gate the external methods onto the work loop for thread safety
    IOReturn theAnswer = 0;
    if(mCommandGate != NULL)
    {
        theAnswer = mCommandGate->runAction(_getVolume, &inVolumeID, &outVolume);
    }
    return theAnswer;
}

IOReturn ForthAudioDevice::setVolume(int inVolumeID, UInt32 inNewVolume)
{
    //	we gate the external methods onto the work loop for thread safety
    IOReturn theAnswer = kIOReturnSuccess;
    if(mCommandGate != NULL)
    {
        theAnswer = mCommandGate->runAction(_setVolume, &inVolumeID, &inNewVolume);
    }
    else
    {
        theAnswer = kIOReturnNoResources;
    }
    return theAnswer;
}

IOReturn ForthAudioDevice::_getVolume(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3)
{
#pragma unused(inArg2, inArg3)
    IOReturn theAnswer = kIOReturnSuccess;
    ForthAudioDevice* theDriver = NULL;
    const int* theControlID = NULL;
    UInt32* theControlValue = NULL;
    
    //	cast the arguments back to what they need to be
    theDriver = OSDynamicCast(ForthAudioDevice, inTarget);
    FailIfNULL(theDriver, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: this is not a ForthAudioDevice");
    
    theControlID = reinterpret_cast<const int*>(inArg0);
    FailIfNULL(theControlID, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: no control ID");
    
    theControlValue = reinterpret_cast<UInt32*>(inArg1);
    FailIfNULL(theControlValue, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: no control value");
    
    switch(*theControlID)
    {
        case kForthAudio_Control_MasterInputVolume:
            *theControlValue = theDriver->mMasterInputVolume;
            break;
            
        case kForthAudio_Control_MasterOutputVolume:
            *theControlValue = theDriver->mMasterOutputVolume;
            break;
    };
    
Done:
    return theAnswer;
}

IOReturn ForthAudioDevice::_setVolume(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3)
{
#pragma unused(inArg2, inArg3)
    IOReturn theAnswer = kIOReturnSuccess;
    ForthAudioDevice* theDriver = NULL;
    const int* theControlID = NULL;
    const UInt32* theNewControlValue = NULL;
    
    //	cast the arguments back to what they need to be
    theDriver = OSDynamicCast(ForthAudioDevice, inTarget);
    FailIfNULL(theDriver, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: this is not a ForthAudioDevice");
    
    theControlID = reinterpret_cast<const int*>(inArg0);
    FailIfNULL(theControlID, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: no control ID");
    
    theNewControlValue = reinterpret_cast<UInt32*>(inArg1);
    FailIfNULL(theNewControlValue, theAnswer = kIOReturnBadArgument, Done, "ForthAudioDevice::_startHardware: no control value");
    
    switch(*theControlID)
    {
        case kForthAudio_Control_MasterInputVolume:
            theDriver->mMasterInputVolume = *theNewControlValue;
            if(theDriver->mMasterInputVolume > kForthAudio_Control_MaxRawVolumeValue)
            {
                theDriver->mMasterInputVolume = kForthAudio_Control_MaxRawVolumeValue;
            }
            break;
            
        case kForthAudio_Control_MasterOutputVolume:
            theDriver->mMasterOutputVolume = *theNewControlValue;
            if(theDriver->mMasterOutputVolume > kForthAudio_Control_MaxRawVolumeValue)
            {
                theDriver->mMasterOutputVolume = kForthAudio_Control_MaxRawVolumeValue;
            }
            break;
    };
    
Done:
    return theAnswer;
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



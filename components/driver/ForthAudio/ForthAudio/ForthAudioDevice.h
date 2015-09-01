#ifndef FORTH_AUDIO_DEVICE_H
#define FORTH_AUDIO_DEVICE_H

#include "ForthAudioTypeDef.h"

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>


#define ForthAudioDevice    com_sugar_driver_ForthAudioDevice

class IOBufferMemoryDescriptor;

class ForthAudioDevice : public IOService
{
    OSDeclareDefaultStructors(com_sugar_driver_ForthAudioDevice)
public:
    virtual bool start(IOService *inProvider);
    virtual void stop(IOService *provider);
    
    //	IO Management
public:
    IOBufferMemoryDescriptor*	getBuffer(int inBufferType);
    IOReturn					startHardware();
    void						stopHardware();
    IOReturn					setSampleRate(UInt64 inNewSampleRate);
    
private:
    static IOReturn				_getBuffer(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3);
    static IOReturn				_startHardware(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3);
    static IOReturn				_stopHardware(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3);
    static IOReturn				_setSampleRate(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3);
    
    //	Controls
public:
    IOReturn					getVolume(int inVolumeID, UInt32& outVolume);
    IOReturn					setVolume(int inVolumeID, UInt32 inNewVolume);
    
private:
    static IOReturn				_getVolume(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3);
    static IOReturn				_setVolume(OSObject* inTarget, void* inArg0, void* inArg1, void* inArg2, void* inArg3);

    
private:
    IOReturn					allocateBuffers();
    void						freeBuffers();
    
    IOReturn					initTimer();
    void						destroyTimer();
    IOReturn					startTimer();
    void						stopTimer();
    void						updateTimer();
    static void					timerFired(OSObject* inTarget, IOTimerEventSource* inSender);
    
    IOReturn					initControls();
    
    IOWorkLoop*                 mWorkLoop;
    IOCommandGate*              mCommandGate;
    
    IOBufferMemoryDescriptor*	mStatusBuffer;
    IOBufferMemoryDescriptor*	mInputBuffer;
    IOBufferMemoryDescriptor*	mOutputBuffer;
    UInt64						mIOBufferFrameSize;
    
    IOTimerEventSource*			mTimerEventSource;
    bool						mIsRunning;
    UInt64						mSampleRate;
    UInt64						mHostTicksPerBuffer;
    
    UInt32						mMasterInputVolume;
    UInt32						mMasterOutputVolume;

  
};

//==================================================================================================
//	Macros for error handling
//==================================================================================================

#define	DebugMsg(inFormat, ...)	IOLog(inFormat "\n", ## __VA_ARGS__)

#define	FailIf(inCondition, inAction, inHandler, inMessage)									\
            {                                                                               \
                bool __failed = (inCondition);												\
                if(__failed)																\
                {																			\
                    DebugMsg(inMessage);													\
                    { inAction; }															\
                    goto inHandler;															\
                }																			\
            }

#define	FailIfError(inError, inAction, inHandler, inMessage)								\
            {																				\
                IOReturn __Err = (inError);													\
                if(__Err != 0)																\
                {																			\
                    DebugMsg(inMessage ", Error: %d (0x%X)", __Err, (unsigned int)__Err);	\
                    { inAction; }															\
                    goto inHandler;															\
                }																			\
            }

#define	FailIfNULL(inPointer, inAction, inHandler, inMessage)								\
            if((inPointer) == NULL)															\
            {																				\
                DebugMsg(inMessage);														\
                { inAction; }																\
                goto inHandler;																\
            }


#endif

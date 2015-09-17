#include "mac_audio_resampler.hpp"
#include "mac_audio_tool.hpp"

namespace oppvs {
	MacAudioResampler::MacAudioResampler()
	{
		m_converter = NULL;
	}

	MacAudioResampler::~MacAudioResampler()
	{
		if (m_converter)
			cleanup();
	}

	int MacAudioResampler::init(CAStreamBasicDescription& inputFormat, CAStreamBasicDescription& outputFormat)
	{
		OSStatus err = noErr;
		UInt32 size = sizeof(outputFormat);
 
		err = AudioConverterNew(&inputFormat, &outputFormat, &m_converter);
		if (err)
		{
			printf("Failed to create audio converter %d\n", err);
			return -1;
		}
		size = sizeof(inputFormat);
		err = AudioConverterGetProperty(m_converter, kAudioConverterCurrentInputStreamDescription, &size, &inputFormat);
		if (err)
		{
			return -1;
		}

		size = sizeof(outputFormat);
		err = AudioConverterGetProperty(m_converter, kAudioConverterCurrentOutputStreamDescription, &size, &outputFormat);
		if (err)
		{
            return -1;
		}

		size = sizeof(UInt32);
		UInt32 maxOutputSize = 0;
		err = AudioConverterGetProperty(m_converter, kAudioConverterPropertyMaximumOutputPacketSize, &size, &maxOutputSize);

		UInt32 uFlag = kAudioConverterQuality_Max;
        err = AudioConverterSetProperty(m_converter, kAudioConverterSampleRateConverterQuality,
                                        sizeof(uFlag), &uFlag);
        checkResult(err, "Set Sample Rate Converter Quality");
        
		return 0;
	}

	void MacAudioResampler::cleanup()
	{
		AudioConverterDispose(m_converter);
	}

	OSStatus MacAudioResampler::convert(AudioConverterComplexInputDataProc proc, void* userData, UInt32 *ioOutputDataPackets, AudioBufferList* abl, UInt32 inNumFrames, UInt32 bytesPerFrame)
	{
		OSStatus err = noErr;
        //Calculate correct number output frames
        UInt32 availableInputFrames = inNumFrames;
        UInt32 actualOutputFrames;
        
        UInt32 availableInputBytes = availableInputFrames * bytesPerFrame;
        UInt32 availableOutputBytes = availableInputBytes;
        UInt32 availableOutputFrames = availableOutputBytes / bytesPerFrame;
        UInt32 propertySize = sizeof (availableOutputBytes);
        err = AudioConverterGetProperty(m_converter,
                                        kAudioConverterPropertyCalculateOutputBufferSize,
                                        &propertySize,
                                        &availableOutputBytes);
        checkResult(err, "AudioConverterGetProperty CalculateOutputBufferSize");
        
        availableOutputFrames = availableOutputBytes / bytesPerFrame;
        UInt32 inputBytes = availableOutputBytes;
        propertySize = sizeof(availableOutputBytes);
        err = AudioConverterGetProperty(m_converter,
                                        kAudioConverterPropertyCalculateInputBufferSize,
                                        &propertySize,
                                        &inputBytes);
        checkResult(err, "AudioConverterGetProperty CalculateInputBufferSize");
        
        printf("input bytes: %d %d\n", inputBytes, availableInputBytes);
        if(inputBytes < availableInputBytes) {
            // OK to zero pad the input a little
            availableOutputFrames += 1;
            availableOutputBytes = availableOutputFrames * bytesPerFrame;
        }
        
        printf("%d %d\n", availableOutputFrames, availableOutputBytes);
        
        *ioOutputDataPackets = availableOutputFrames;
		AudioConverterReset(m_converter);
		return AudioConverterFillComplexBuffer(m_converter, proc, userData, ioOutputDataPackets, abl, NULL);
	}

    AudioConverterRef MacAudioResampler::getConverter()
    {
        return m_converter;
    }
} // oppvs
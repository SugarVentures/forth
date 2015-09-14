#include "mac_audio_resampler.hpp"
#include "mac_audio_tool.hpp"

namespace oppvs {
	MacAudioResampler::MacAudioResampler()
	{
		m_converter = NULL;
		m_ioOutputDataPackets = 0;
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
		err = AudioConverterGetProperty(m_converter, kAudioConverterCurrentInputStreamDescription, &size, &outputFormat);
		if (err)
		{
			printf("Failed to set output format\n");
            return -1;
		}

		size = sizeof(UInt32);
		UInt32 maxOutputSize = 0;
		err = AudioConverterGetProperty(m_converter, kAudioConverterPropertyMaximumOutputPacketSize, &size, &maxOutputSize);

		UInt32 uFlag = kAudioConverterQuality_Max;
        err = AudioConverterSetProperty(m_converter, kAudioConverterSampleRateConverterQuality,
                                        sizeof(uFlag), &uFlag);

        /*Float64 sr = 48000;
        size = sizeof(Float64);
        err = AudioConverterSetProperty(m_converter,kAudioConverterEncodeAdjustableSampleRate, size, &sr);
		err = AudioConverterGetProperty(m_converter,kAudioConverterEncodeAdjustableSampleRate, &size, &sr);
		printf("Encode out rate: %f\n", sr);*/

		//Setup buffers
		int outputSizePerPacket = outputFormat.mBytesPerPacket;
		UInt32 theOutputBufSize = 4096;
		printf("output size per packet %d %d\n", outputSizePerPacket, theOutputBufSize);
		m_ioOutputDataPackets = theOutputBufSize / outputSizePerPacket;
		printf("m_ioOutputDataPackets %d\n", m_ioOutputDataPackets);

		m_buffer = allocateAudioBufferListWithNumChannels(2, theOutputBufSize);
		if (m_buffer == NULL)
		{
			printf("Failed to init audio resampler \n");
			return -1;
		}

		size = sizeof(outputFormat);
		err = AudioConverterGetProperty(m_converter, kAudioConverterCurrentOutputStreamDescription, &size, &outputFormat);
		return 0;
	}

	void MacAudioResampler::cleanup()
	{
		AudioConverterDispose(m_converter);
	}

	int MacAudioResampler::convert(UInt32 numberPCMFrames, const AudioBufferList* inputData, AudioBufferList* outputData)
	{
		OSStatus err = noErr;
		err = AudioConverterConvertComplexBuffer(m_converter, numberPCMFrames, inputData, outputData);
		if (err)
		{
			printf("Failed to resample\n");
			return -1;
		}
		return 0;
	}

	int MacAudioResampler::convert(AudioConverterComplexInputDataProc proc, void* userData, UInt32 *ioOutputDataPackets)
	{
		OSStatus err = noErr;
		AudioConverterReset(m_converter);
		err = AudioConverterFillComplexBuffer(m_converter, proc, userData, &m_ioOutputDataPackets, m_buffer, NULL);
		if (err)
		{
			printf("Failed to resample audio data %d\n", err);
			int i = 0;
			while(i < 4 )
                printf("%c", *(((char*)&err)+i++) );
            printf("\n");
			return -1;
		}
		*ioOutputDataPackets = m_ioOutputDataPackets;
		return 0;
	}

	AudioBufferList* MacAudioResampler::getBuffer()
	{
		return m_buffer;
	}

} // oppvs
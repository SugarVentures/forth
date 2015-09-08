#include "mac_audio_tool.hpp"

namespace oppvs {
	AudioBufferList* allocateAudioBufferListWithNumChannels(UInt32 numChannels, UInt32 size, bool noAllocData)
	{
		AudioBufferList*			list = NULL;
		UInt32						i = 0;
		
		list = (AudioBufferList*) calloc(1, sizeof(AudioBufferList) + numChannels * sizeof(AudioBuffer));
		if (list == NULL)
			return NULL;
		
		list->mNumberBuffers = numChannels;
		
		for (i = 0; i < numChannels; ++i)
		{
			list->mBuffers[i].mNumberChannels = 1;
			list->mBuffers[i].mDataByteSize = size;
			if (!noAllocData)
				list->mBuffers[i].mData = malloc(size);
			if (list->mBuffers[i].mData == NULL)
			{
				destroyAudioBufferList(list, noAllocData);
				return NULL;
			}
		}
		return list;
		
	}

	void destroyAudioBufferList(AudioBufferList* list, bool noAllocData)
	{
		UInt32 i = 0;
		printf("Destroy audio buffer list\n");
		if (list)
		{
			if (!noAllocData)
			{
				for (i = 0; i < list->mNumberBuffers; i++)
				{
					if (list->mBuffers[i].mData)
					{
						free(list->mBuffers[i].mData);
					}
				}
			}
			free(list);
		}
	}

	void convertABLToGenericABL(AudioBufferList* abl, GenericAudioBufferList* gbl)
	{
		if (abl != NULL && gbl != NULL)
		{
			if (gbl->nBuffers != abl->mNumberBuffers)
			{
				if (gbl->nBuffers != 0)
				{
					delete [] gbl->buffers;
				}
				gbl->buffers = new GenericAudioBuffer[abl->mNumberBuffers];
				gbl->nBuffers = abl->mNumberBuffers;
			}
			for (unsigned i = 0; i < abl->mNumberBuffers; ++i) {
				gbl->buffers[i].numberChannels = abl->mBuffers[i].mNumberChannels;
				gbl->buffers[i].dataLength = abl->mBuffers[i].mDataByteSize;
				//Point to the memory instead of copying
				gbl->buffers[i].data = abl->mBuffers[i].mData;
			}
		}
	}

	void convertGenericABLToABL(GenericAudioBufferList* gbl, AudioBufferList*& abl)
	{
		if (gbl == NULL)
			return;
		if (abl == NULL)
		{
			abl = allocateAudioBufferListWithNumChannels(gbl->nBuffers, true);
		}
		for (unsigned i = 0; i < gbl->nBuffers; ++i) {
			abl->mBuffers[i].mNumberChannels = gbl->buffers[i].numberChannels;
			abl->mBuffers[i].mDataByteSize = gbl->buffers[i].dataLength;
			//Point to memory
			abl->mBuffers[i].mData = gbl->buffers[i].data;
		}

	}

	void makeBufferSilent(AudioBufferList* ioData)
	{
		for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
			memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
	}
} // oppvs
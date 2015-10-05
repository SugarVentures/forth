#include "audio_ring_buffer.h"
#include <string.h>
#include <algorithm>

namespace oppvs {
	AudioRingBuffer::AudioRingBuffer() : RingBuffer(), m_bytesPerFrame(0),
		m_maxFrames(0), m_startTime(0), m_endTime(0)
	{

	}
    
    void AudioRingBuffer::allocate(uint32_t bytesPerFrame, uint32_t maxFrames)
    {
        m_bytesPerFrame = bytesPerFrame;
        m_maxFrames = maxFrames;
        RingBuffer::allocate(bytesPerFrame * maxFrames);
    }
    
    RingBufferError AudioRingBuffer::store(uint32_t* ioFrames, const void* data, uint64_t timeStamp)
    {
        if (isFull() || m_startTime == m_endTime)
        {
            printf("Reset buffer\n");
            reset();
        }
        uint64_t startWrite = timeStamp;
        uint64_t endWrite = timeStamp + *ioFrames;
        uint64_t startTime = m_startTime;
        uint64_t endTime = m_endTime;
        
        //printf("Store %lld %lld %lld %lld \n", startWrite, endWrite, startTime, endTime);
        if (startWrite < m_endTime)
        {
            startWrite = m_endTime;
            endWrite = startWrite + *ioFrames;
        }
        
        uint32_t inLen = *ioFrames * m_bytesPerFrame;
        uint32_t outLen = 0;
        
        RingBufferError err = RingBufferError_OK;
        if (startTime == endTime)
        {
            //printf("size: %u cap: %u %u %u\n", m_size, m_capacity, m_fetchIndex, m_storeIndex);
            err = RingBuffer::store((uint8_t*)data, inLen, &outLen);
            *ioFrames = outLen / m_bytesPerFrame;
            m_startTime = startWrite;
            m_endTime = endWrite;
            //printf("*******************\n");
            return RingBufferError_OK;
        }
        
        //Fill zeros in the gap
        if (startWrite - m_endTime > 0)
        {
            //printf("Fill zero %lld %lld\n", startWrite, m_endTime);
            uint32_t len = (startWrite - m_endTime) * m_bytesPerFrame;
            uint8_t* zeros = new uint8_t[len];
            memset(zeros, 0, len);
            err = RingBuffer::store(zeros, len, &len);
            delete [] zeros;
        }
        
        if (err == RingBufferError_OK)
        {
            err = RingBuffer::store((uint8_t*)data, inLen, &outLen);
            *ioFrames = outLen / m_bytesPerFrame;
            //printf("Fetch index: %d Store index: %d\n", m_fetchIndex, m_storeIndex);
            if (!err)
            {
                m_endTime = endWrite;
                if (m_startTime == 0)
                    m_startTime = startTime;
                //printf("End Time: %lld\n", m_endTime);
            }
        }
        return err;
    }
    
    RingBufferError AudioRingBuffer::fetch(uint32_t ioFrames, void* data, uint64_t timeStamp)
    {
        //No packet to fetch, exit
        if (ioFrames == 0)
            return RingBufferError_OK;
        
        uint64_t startRead = timeStamp;
        uint64_t endRead = timeStamp + ioFrames;
        uint64_t startTime = m_startTime;
        uint64_t endTime = m_endTime;
        
        //printf("Fetch %lld %lld %lld %lld \n", startRead, endRead, startTime, endTime);
        
        if (startRead > endTime || endRead < startTime)
        {
            //printf("%lld %lld\n", startRead - endTime, endRead - startTime);
            return RingBufferError_InvalidArgument;
        }
        startRead = std::max(startRead, startTime);
        endRead = std::min(endRead, endTime);
        
        if (startRead >= endRead)
        {
            memset(data, 0, ioFrames * m_bytesPerFrame);
            return RingBufferError_OK;
        }
        
        int32_t byteSize = (endRead - startRead) * m_bytesPerFrame;
        
        int32_t startOffset = std::max((int32_t)0, (int32_t)((startRead - timeStamp) * m_bytesPerFrame));
        if (startOffset > 0)
        {
            memset(data, 0, std::min((int32_t)(ioFrames * m_bytesPerFrame), startOffset));
            //printf("fill zero in starting\n");
        }
        
        int32_t endOffset = std::max((int32_t)0, (int32_t)((timeStamp + ioFrames - endRead) * m_bytesPerFrame));
        //printf("start offset: %d end offset: %d byteSize: %d\n", startOffset, endOffset, byteSize);
        if (endOffset > 0)
        {
            memset((uint8_t*)data + startOffset + byteSize, 0, endOffset);
            //printf("fill zero in ending\n");
        }
        ioFrames = endRead - startRead;
        
        //printf("Need to fetch %d\n", ioFrames);
        uint32_t inLen = ioFrames * m_bytesPerFrame;
        uint32_t outLen = 0;
        
        RingBufferError err = RingBuffer::fetch((uint8_t*)data + startOffset, inLen, &outLen);
        //printf("Fetch index: %d Store index: %d error: %d\n", m_fetchIndex, m_storeIndex, err);
        if (!err)
        {
            m_startTime = endRead;
        }

        return err;
    }
    
    uint32_t AudioRingBuffer::getNumberFrames()
    {
        return size() / m_bytesPerFrame;
    }
    
    uint64_t AudioRingBuffer::getStartTime()
    {
        return m_startTime;
    }
} // oppvs
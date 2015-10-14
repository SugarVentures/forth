#include "video_packetizer.h"

namespace oppvs {

	VideoPacketizer::VideoPacketizer(): m_isRunning(true)
	{
		p_thread = new Thread(VideoPacketizer::run, this);
		p_segmentPool = NULL;
		m_timestamp = 0;
	}

	VideoPacketizer::~VideoPacketizer()
	{
		delete p_thread;
		p_segmentPool = NULL;
		m_encoder.release();
	}

	void VideoPacketizer::init(VideoStreamInfo& info, tsqueue<SharedDynamicBufferRef>* queue)
	{
		m_encoder.init(info);
		p_segmentPool = queue;
	}

	void VideoPacketizer::start()
	{
		p_thread->create();
	}

	bool VideoPacketizer::isRunning()
	{
		return m_isRunning;
	}

	void VideoPacketizer::pushFrame(const PixelBuffer& pf)
	{
		if (m_framePool.size() >= MAX_FRAMES_IN_POOL)
			return;
		std::shared_ptr<PixelBuffer> pixelbuffer(new PixelBuffer);
		pixelbuffer->source = pf.source;
		pixelbuffer->order = pf.order;
		pixelbuffer->plane[0] = new uint8_t[pf.nbytes];
		memcpy(pixelbuffer->plane[0], pf.plane[0], pf.nbytes);
		pixelbuffer->nbytes = pf.nbytes;
		pixelbuffer->width[0] = pf.width[0];
		pixelbuffer->height[0] = pf.height[0];
		pixelbuffer->stride[0] = pf.stride[0];
		pixelbuffer->timestamp = pf.timestamp;
		m_framePool.push(pixelbuffer);
	}

	void VideoPacketizer::pullFrame()
	{
		uint8_t* data = NULL;
		uint32_t encodingLength, sendLength, sentLength = 0;
		bool isKey = false;
		int picID = -1;

		if (m_framePool.empty())
			return;

		std::shared_ptr<std::shared_ptr<PixelBuffer>> ptrFrame = m_framePool.pop();
		if (ptrFrame.get() == NULL)
		{
			return;
		}

		std::shared_ptr<PixelBuffer> frame = *ptrFrame;
		PixelBuffer pf = *frame;
		//uint32_t duration = pf.timestamp - m_timestamp;
		m_timestamp = pf.timestamp;

		//Encoding		
		if (m_encoder.encode(pf, &encodingLength, &data, &picID, &isKey, 1) < 0)
		{
			delete [] pf.plane[0];
			return;
		}
		delete [] pf.plane[0];

		//printf("Encoding Length %u Key: %d timestamp: %u\n", encodingLength, isKey, 1);
		sendLength = encodingLength;
		uint8_t* curPos = data;
		do
		{
			bool flag = (sendLength == encodingLength);
			SharedDynamicBufferRef segment = SharedDynamicBufferRef(new DynamicBuffer());
			segment->setSize(100);
			m_builder.reset();
			m_builder.getStream().attach(segment, true);

			if (m_builder.addRTPHeader(m_timestamp, pf.source, VP8_PAYLOAD_TYPE) < 0)
				return;

			if (m_builder.addVideoCommonHeader(flag, picID) < 0)
				return;
			
			if (flag)
			{
				if (m_builder.addVideoPayloadHeader(isKey, encodingLength) < 0)
					return;
				sentLength = sendLength > (OPPVS_NETWORK_PACKET_LENGTH - VP8_MAX_HEADER_SIZE) ? (OPPVS_NETWORK_PACKET_LENGTH - VP8_MAX_HEADER_SIZE) : sendLength;
			}
			else
			{
				sentLength = sendLength > (OPPVS_NETWORK_PACKET_LENGTH - VP8_COMMON_HEADER_SIZE - RTP_HEADER_SIZE) ? 
				(OPPVS_NETWORK_PACKET_LENGTH - VP8_COMMON_HEADER_SIZE - RTP_HEADER_SIZE) : sendLength;
			}
			if (m_builder.addPayload(curPos, sentLength) < 0)
				return;
			sendLength -= sentLength;
			curPos += sentLength;

			//printf("Sent length: %u\n", sentLength);
			p_segmentPool->push(segment);
		}
		while (sendLength > 0);
		//m_timestamp++;
	}

	void* VideoPacketizer::run(void* object)
	{
		VideoPacketizer* handler = (VideoPacketizer*)object;
		while (handler->isRunning())
		{
			handler->pullFrame();
			usleep(10000);
		}
		return NULL;
	}


} // oppvs
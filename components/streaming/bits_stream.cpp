#include "bits_stream.hpp"

namespace oppvs
{
	BitsStream::BitsStream() : m_isOverloading(false)
	{
		m_thread = new Thread(BitsStream::init, (void*)this);
		m_thread->create();
	}

	BitsStream::~BitsStream()
	{
		delete m_thread;
	}

	bool BitsStream::isOverloading()
	{
		return m_isOverloading;
	}

	void BitsStream::setOverloading(bool value)
	{
		m_isOverloading = value;
	}

	void* BitsStream::init(void* object)
	{
		printf("Bitsstream is running\n");
		BitsStream* stream = (BitsStream*)object;
		while (1)
		{
			if (stream->isOverloading())
			{
				stream->throttle();
			}
			usleep(100);
		}
		return NULL;
	}

	void BitsStream::throttle()
	{
		printf("Process data size: %d\n", m_waitingQueue.size());
		while (m_waitingQueue.size() > MAX_WAITING_QUEUE_SIZE)
		{
			m_waitingQueue.erase(m_waitingQueue.begin(), m_waitingQueue.begin() + MAX_WAITING_QUEUE_SIZE/4);
		}
		setOverloading(false);
	}

	void BitsStream::pushData(const PixelBuffer& pf)
	{
		if (!isOverloading())
		{
			m_waitingQueue.push_back(pf);
			printf("Push data\n");
			
			while (isOverloading())
			{
				printf("Waiting process\n");
				usleep(1000);
			}
		}
	}
}
#ifndef _ASYNC_LOGGING_QUEUE_H_ 
#define _ASYNC_LOGGING_QUEUE_H_

#include "LogFile.h"

#include "../thread/BlockingQueue.h"
#include "../thread/BoundedBlockingQueue.h"
#include "../thread/CountDownLatch.h"
#include "../thread/Thread.h"

#include <string>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

template<typename MSG, template<typename> class QUEUE>
class AsyncLoggingT
	: private boost::noncopyable
{
public:
	AsyncLoggingT(const string& rStrBaseName, size_t unRollSize)
		: m_bRunning(false)
		, m_strBaseName(rStrBaseName)
		, m_unRollSize(unRollSize)
		, m_Thread(boost::bind(&AsyncLoggingT::threadFunc, this), "Logging")
		, m_Latch(1)
	{

	}
	AsyncLoggingT(const string& rStrBaseName, size_t unRollSize, int nQueueSize) 
		: m_bRunning(false)
		, m_strBaseName(rStrBaseName)
		, m_unRollSize(unRollSize)
		, m_Thread(boost::bind(&AsyncLoggingT::threadFunc, this), "Logging")
		, m_Latch(1)
		, m_Queue(nQueueSize)
	{

	}
	~AsyncLoggingT() {
		if (m_bRunning) {
			Stop();
		}
	}

	void			Append(const char* pStrLine, int nLength) {
		m_Queue.Put(MSG(pStrLine, nLength));
	}
	void			Start() {
		m_bRunning = true;
		m_Thread.Start();
		m_Latch.Wait();
	}
	void			Stop() {
		m_bRunning = false;
		m_Queue.Put(MSG());
		m_Thread.Join();
	}

private:
	void			threadFunc() {
		assert(m_bRunning);
		m_Latch.CountDown();
		LogFile output(m_strBaseName, m_unRollSize, false);
		// time_t lastFlush = time(NULL);
		while (true) {
			MSG msg(m_Queue.Take());
			if (msg.empty()) {
				assert(!m_bRunning);
				break;
			}
			output.Append(msg.data(), msg.length());
		}
		output.Flush();
	}

private:
	bool			m_bRunning;
	string			m_strBaseName;
	size_t			m_unRollSize;
	Thread			m_Thread;
	CountDownLatch	m_Latch;
	QUEUE<MSG>		m_Queue;

};

typedef AsyncLoggingT<string, BlockingQueue> AsyncLoggingUnboundedQueue;
typedef AsyncLoggingT<string, BoundedBlockingQueue> AsyncLoggingBoundedQueue;

struct LogMessage
{
	LogMessage(const char* pStrMsg, size_t unLength)
		: m_unLength(unLength)
	{
		assert(unLength <= sizeof(m_Data));
		::memcpy(m_Data, pStrMsg, unLength);
	}

	LogMessage()
		: m_unLength(0)
	{
	}

	LogMessage(const LogMessage& rLogMsg)
		: m_unLength(rLogMsg.m_unLength)
	{
		assert(m_unLength <= sizeof(m_Data));
		::memcpy(m_Data, rLogMsg.m_Data, m_unLength);
	}

	LogMessage&	operator=(const LogMessage& rhs) {
		m_unLength = rhs.m_unLength;
		assert(m_unLength <= sizeof(m_Data));
		::memcpy(m_Data, rhs.m_Data, m_unLength);

		return *this;
	}

	const char* data() const {
		return m_Data;
	}
	size_t		length() {
		return m_unLength;
	}
	bool		empty() {
		return 0 == m_unLength;
	}

	char		m_Data[4000];
	size_t		m_unLength;

};

typedef AsyncLoggingT<LogMessage, BlockingQueue> AsyncLoggingUnboundedQueueL;
typedef AsyncLoggingT<LogMessage, BoundedBlockingQueue> AsyncLoggingBoundedQueueL;

#endif // _ASYNC_LOGGING_QUEUE_H_

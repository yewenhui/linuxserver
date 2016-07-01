#ifndef _ASYNC_LOGGING_DOUBLE_BUFFERING_
#define _ASYNC_LOGGING_DOUBLE_BUFFERING_

#include "LogStream.h"

#include "../thread/BlockingQueue.h"
#include "../thread/BoundedBlockingQueue.h"
#include "../thread/CountDownLatch.h"
#include "../thread/Mutex.h"
#include "../thread/Thread.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class AsyncLoggingDoubleBuffering
	: private boost::noncopyable
{
	typedef detail::FixedBuffer<detail::LARGE_BUFFER> Buffer;
	typedef boost::ptr_vector<Buffer> BufferVector;
	typedef BufferVector::auto_type BufferPtr;

public:
	AsyncLoggingDoubleBuffering(const string& strBaseName, size_t unRollSize, int nFlushInterval = 3);
	~AsyncLoggingDoubleBuffering();

	void			Append(const char* pStrLogLine, int nLength);

	void			Start();
	void			Stop();

private:
	void			threadFunc();

private:
	const int		m_nFlushInterval;
	bool			m_bRunning;
	string			m_strBaseName;
	size_t			m_unRollSize;
	
	Thread			m_Thread;
	CountDownLatch	m_Latch;
	MutexLock		m_Mutex;
	Condition		m_Cond;

	BufferPtr		m_pCurBuffer;
	BufferPtr		m_pNextBuffer;
	BufferVector	m_Buffers;

};

#endif // _ASYNC_LOGGING_DOUBLE_BUFFERING_

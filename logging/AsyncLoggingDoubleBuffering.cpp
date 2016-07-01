#include "AsyncLoggingDoubleBuffering.h"
#include "LogFile.h"
#include <stdio.h>

AsyncLoggingDoubleBuffering::AsyncLoggingDoubleBuffering(const string& strBaseName
														, size_t unRollSize
														, int nFlushInterval /*= 3*/) 
	: m_nFlushInterval(nFlushInterval)
	, m_bRunning(false)
	, m_strBaseName(strBaseName)
	, m_unRollSize(unRollSize)
	, m_Thread(boost::bind(&AsyncLoggingDoubleBuffering::threadFunc, this), "Logging")
	, m_Latch(1)
	, m_Mutex()
	, m_Cond(m_Mutex)
	, m_pCurBuffer(new Buffer)
	, m_pNextBuffer(new Buffer)
	, m_Buffers()
{
	m_pCurBuffer->BZero();
	m_pNextBuffer->BZero();
	m_Buffers.reserve(16);
}

AsyncLoggingDoubleBuffering::~AsyncLoggingDoubleBuffering() {
	if (m_bRunning) {
		Stop();
	}
}

void AsyncLoggingDoubleBuffering::Append(const char* pStrLogLine, int nLength) {
	MutexLockGuard lock(m_Mutex);
	if (m_pCurBuffer->GetAvail() > nLength) {
		m_pCurBuffer->Append(pStrLogLine, nLength);
	} else {
		m_Buffers.push_back(m_pCurBuffer.release());

		if (NULL != m_pNextBuffer) {
			m_pCurBuffer = boost::ptr_container::move(m_pNextBuffer);
		} else {
			m_pCurBuffer.reset(new Buffer);
		}
		m_pCurBuffer->Append(pStrLogLine, nLength);
		m_Cond.Notify();
	}
}

void AsyncLoggingDoubleBuffering::Start() {
	m_bRunning = true;
	m_Thread.Start();
	m_Latch.Wait();
}

void AsyncLoggingDoubleBuffering::Stop() {
	m_bRunning = false;
	m_Cond.Notify();
	m_Thread.Join();
}

void AsyncLoggingDoubleBuffering::threadFunc() {
	assert(m_bRunning);

	m_Latch.CountDown();
	LogFile output(m_strBaseName, m_unRollSize, false);
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->BZero();
	newBuffer2->BZero();

	boost::ptr_vector<Buffer> buffersToWrite;
	buffersToWrite.reserve(16);
	while (m_bRunning) {
		assert(NULL != newBuffer1 && 0 == newBuffer1->GetLength());
		assert(NULL != newBuffer2 && 0 == newBuffer2->GetLength());
		assert(buffersToWrite.empty());

		{
			MutexLockGuard lock(m_Mutex);
			if (!m_Buffers.empty()) {
				m_Cond.WaitForSeconds(m_nFlushInterval);
			}
			m_Buffers.push_back(m_pCurBuffer.release());
			m_pCurBuffer = boost::ptr_container::move(newBuffer1);
			buffersToWrite.swap(m_Buffers);

			if (NULL == m_pNextBuffer) {
				m_pNextBuffer = boost::ptr_container::move(newBuffer2);
			}

		}

		assert(!buffersToWrite.empty());

		if (buffersToWrite.size() > 25) {
			const char* dropMsg = "Dropped log messages\n";
			fprintf(stderr, "%s", dropMsg);
			output.Append(dropMsg, strlen(dropMsg));
			buffersToWrite.erase(buffersToWrite.begin(), buffersToWrite.end() - 2);
		}

		for (size_t n = 0; n < buffersToWrite.size(); ++n) {
			output.Append(buffersToWrite[n].GetData(), buffersToWrite[n].GetLength());
		}

		if (buffersToWrite.size() > 2) {
			buffersToWrite.resize(2);
		}

		if (NULL == newBuffer1) {
			assert(!buffersToWrite.empty());
			newBuffer1 = buffersToWrite.pop_back();
			newBuffer1->Reset();
		}

		if (NULL == newBuffer2) {
			assert(!buffersToWrite.empty());
			newBuffer2 = buffersToWrite.pop_back();
			newBuffer2->Reset();
		}

		buffersToWrite.clear();
		output.Flush();
	}

	output.Flush();
}


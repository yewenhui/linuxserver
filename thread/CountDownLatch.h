#ifndef _COUNT_DOWN_LATCH_
#define _COUNT_DOWN_LATCH_

#include "Mutex.h"
#include "Condition.h"

#include <boost/noncopyable.hpp>

class CountDownLatch
	: private boost::noncopyable
{
public:
	explicit CountDownLatch(int nCnt)
		: m_Mutex()
		, m_Cond(m_Mutex)
		, m_nCnt(nCnt)
	{
	}

	void Wait() {
		MutexLockGuard lock(m_Mutex);
		while (m_nCnt > 0) {
			m_Cond.Wait();
		}
	}

	void CountDown() {
		MutexLockGuard lock(m_Mutex);
		-- m_nCnt;

		if (0 == m_nCnt) {
			m_Cond.NotifyAll();
		}
	}

	int GetCount() const {
		MutexLockGuard lock(m_Mutex);
		return m_nCnt;
	}

private:
	mutable MutexLock		m_Mutex;
	Condition				m_Cond;
	int						m_nCnt;

};

#endif // _COUNT_DOWN_LATCH_

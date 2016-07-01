#ifndef _BLOCKING_QUEUE_H_
#define _BLOCKING_QUEUE_H_

#include "Condition.h"

#include <boost/noncopyable.hpp>
#include <deque>
#include <assert.h>

template<typename T>
class BlockingQueue
	: private boost::noncopyable
{
public:
	BlockingQueue()
		: m_Mutex()
		, m_NotEmptyCond(m_Mutex)
		, m_Queue()
	{
	}

	void Put(const T& t) {
		MutexLockGuard lock(m_Mutex);
		m_Queue.push_back(t);
		m_NotEmptyCond.Notify();
	}

	T Take() {
		MutexLockGuard lock(m_Mutex);
		while (m_Queue.empty()) {
			m_NotEmptyCond.Wait();
		}

		assert(!m_Queue.empty());
		T front(m_Queue.front());
		m_Queue.pop_front();
		return front;
	}

	size_t Size() const {
		MutexLockGuard lock(m_Mutex);
		return m_Queue.size();
	}

private:
	mutable MutexLock		m_Mutex;
	Condition				m_NotEmptyCond;
	std::deque<T>			m_Queue;

};

#endif // _BLOCKING_QUEUE_H_
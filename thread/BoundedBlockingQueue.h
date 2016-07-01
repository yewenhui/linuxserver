#ifndef _BOUNDED_BLOCKING_QUEUE_H_
#define _BOUNDED_BLOCKING_QUEUE_H_

#include "Condition.h"

#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include <assert.h>

template<typename T>
class BoundedBlockingQueue
	: private boost::noncopyable
{
public:
	explicit BoundedBlockingQueue(int nMaxSize)
		: m_Mutex()
		, m_NotEmptyCond(m_Mutex)
		, m_NoeFullCond(m_Mutex)
		, m_Queue(nMaxSize)
	{
	}

	void Put(const T& t) {
		MutexLockGuard lock(m_Mutex);
		while (m_Queue.full()) {
			m_NoeFullCond.Wait();
		}
		assert(!m_Queue.full());
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
		m_NoeFullCond.Notify();
		return front;
	}

	bool Empty() const {
		MutexLockGuard lock(m_Mutex);
		return m_Queue.empty();
	}

	bool Full() const {
		MutexLockGuard lock(m_Mutex);
		return m_Queue.full();
	}

	size_t Size() const {
		MutexLockGuard lock(m_Mutex);
		return m_Queue.size();
	}

	size_t Capacity() const {
		MutexLockGuard lock(m_Mutex);
		return m_Queue.capacity();
	}

private:
	mutable MutexLock		m_Mutex;
	Condition				m_NotEmptyCond;
	Condition				m_NoeFullCond;
	boost::circular_buffer<T> m_Queue;

};

#endif // _BOUNDED_BLOCKING_QUEUE_H_
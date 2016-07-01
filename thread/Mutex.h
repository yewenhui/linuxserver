#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "Thread.h"

#include <assert.h>
#include <pthread.h>

#include <boost/noncopyable.hpp>

class MutexLock
	: private boost::noncopyable
{
public:
	explicit MutexLock() 
		: m_Holder(0) 
	{
		pthread_mutex_init(&m_Mutex, NULL);
	}

	~MutexLock() {
		assert(0 == m_Holder);
		pthread_mutex_destroy(&m_Mutex);
	}

	bool			IsLockedByThisThread() {
		return m_Holder == CurrentThread::GetTid();
	}

	void			AssertLocked() {
		assert(IsLockedByThisThread());
	}

	void			Lock() {
		pthread_mutex_lock(&m_Mutex);
		m_Holder = CurrentThread::GetTid();
	}

	void			Unlock() {
		m_Holder = 0;
		pthread_mutex_unlock(&m_Mutex);
	}

	pthread_mutex_t* GetPthreadMutex() {
		return &m_Mutex;
	}

private:
	pthread_mutex_t	m_Mutex;
	pid_t			m_Holder;	

};

class MutexLockGuard
	: private boost::noncopyable
{
public:
	explicit MutexLockGuard(MutexLock& mutex)
		: m_Mutex(mutex) 
	{
		m_Mutex.Lock();
	}

	~MutexLockGuard() {
		m_Mutex.Unlock();
	}

private:
	MutexLock&		m_Mutex;

};

// ∑¿÷π≥ˆœ÷£∫ MutexLockGuard(mutex);
#define MutexLockGuard(x) error "Missing guard object name"

#endif // _MUTEX_H_

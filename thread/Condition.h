#ifndef _CONDITION_H_
#define _CONDITION_H_

#include "Mutex.h"

#include <pthread.h>
#include <errno.h>

#include <boost/noncopyable.hpp>

class Condition
	: private boost::noncopyable
{
public:
	explicit Condition(MutexLock& mutex)
		: m_Mutex(mutex)
	{
		pthread_cond_init(&m_PCond, NULL);
	}

	~Condition() {
		pthread_cond_destroy(&m_PCond);
	}

	void			Wait() {
		pthread_cond_wait(&m_PCond, m_Mutex.GetPthreadMutex());
	}

	bool			WaitForSeconds(int nSeconds) {
		struct timespec abstime;
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec = nSeconds;
		return ETIMEDOUT == pthread_cond_timedwait(&m_PCond, m_Mutex.GetPthreadMutex(), &abstime);
	}

	void			Notify() {
		pthread_cond_signal(&m_PCond);
	}

	void			NotifyAll() {
		pthread_cond_broadcast(&m_PCond);
	}

private:
	MutexLock&		m_Mutex;
	pthread_cond_t	m_PCond;

};

#endif // _CONDITION_H_

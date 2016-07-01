#ifndef _EVENT_LOOP_THREAD_H_
#define _EVENT_LOOP_THREAD_H_

#include "../thread/Thread.h"
#include "../thread/Mutex.h"
#include "../thread/Condition.h"

#include <boost/noncopyable.hpp>

class EventLoop;

class EventLoopThread
	: private boost::noncopyable
{
public:
	EventLoopThread();
	~EventLoopThread();

	EventLoop*		StartLoop();

private:
	void			threadFunc();

private:
	EventLoop*		m_pEventLoop;
	bool			m_bExiting;
	Thread			m_Thread;
	MutexLock		m_Mutex;
	Condition		m_Cond;

};

#endif // _EVENT_LOOP_THREAD_H_

#include "EventLoopThread.h"
#include "EventLoop.h"

#include <boost/bind.hpp>

EventLoopThread::EventLoopThread()
	: m_pEventLoop(NULL)
	, m_bExiting(false)
	, m_Thread(boost::bind(&EventLoopThread::threadFunc, this))
	, m_Mutex()
	, m_Cond(m_Mutex)
{

}

EventLoopThread::~EventLoopThread() {
	m_bExiting = true;
	m_pEventLoop->Quit();
	m_Thread.Join();
}

EventLoop* EventLoopThread::StartLoop() {
	assert(!m_Thread.Started());
	m_Thread.Start();

	{
		MutexLockGuard lockGuard(m_Mutex);
		while (NULL == m_pEventLoop) {
			m_Cond.Wait();
		}
	}

	return m_pEventLoop;
}

void EventLoopThread::threadFunc() {
	EventLoop eventLoop;
	
	{
		MutexLockGuard lockGuard(m_Mutex);
		m_pEventLoop = &eventLoop;
		m_Cond.Notify();
	}
	
	eventLoop.Loop();
}

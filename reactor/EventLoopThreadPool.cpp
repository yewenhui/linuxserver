#include "EventLoopThreadPool.h"

#include "EventLoop.h"
#include "EventLoopThread.h"

#include <boost/bind.hpp>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* pBaseLoop)
	: m_pBaseLoop(pBaseLoop)
	, m_bStarted(false)
	, m_nNumThreads(0)
	, m_nNext(0)
{

}

EventLoopThreadPool::~EventLoopThreadPool() {

}

void EventLoopThreadPool::Start() {
	assert(!m_bStarted);
	m_pBaseLoop->AssertInLoopThread();

	m_bStarted = true;

	for (int n = 0; n < m_nNumThreads; ++n) {
		EventLoopThread* pLoopThread = new EventLoopThread;
		m_vThreads.push_back(pLoopThread);
		m_vLoops.push_back(pLoopThread->StartLoop());
	}
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
	m_pBaseLoop->AssertInLoopThread();
	EventLoop* pLoop = m_pBaseLoop;
	if (!m_vLoops.empty()) {
		pLoop = m_vLoops[m_nNext];
		++ m_nNext;
		if (static_cast<size_t>(m_nNext) >= m_vLoops.size()) {
			m_nNext = 0;
		}
	}

	return pLoop;
}

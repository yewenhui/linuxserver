#ifndef _EVENT_LOOP_THREAD_POOL_H_
#define _EVENT_LOOP_THREAD_POOL_H_

#include "../thread/Condition.h"
#include "../thread/Mutex.h"
#include "../thread/Thread.h"

#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
	: private boost::noncopyable
{
public:
	EventLoopThreadPool(EventLoop* pBaseLoop);
	~EventLoopThreadPool();

	void				SetThreadNum(int nNumThreads) {
		m_nNumThreads = nNumThreads;
	}

	void				Start();
	EventLoop*			GetNextLoop();

private:
	EventLoop*			m_pBaseLoop;
	bool				m_bStarted;
	int					m_nNumThreads;
	int					m_nNext;
	boost::ptr_vector<EventLoopThread> m_vThreads;
	std::vector<EventLoop*>	m_vLoops;

};

#endif // _EVENT_LOOP_THREAD_POOL_H_

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <deque>

class ThreadPool
	: private boost::noncopyable
{
public:
	typedef boost::function<void()> Task;
	
	explicit ThreadPool(const std::string& strName = std::string());
	~ThreadPool();

	void				Start(int nNumThreads);
	void				Stop();

	void				Run(const Task& rTask);

private:
	void				runInThread();
	Task				take();

private:
	MutexLock			m_Mutex;
	Condition			m_Cond;
	std::string			m_StrName;
	boost::ptr_vector<Thread> m_vThreads;
	std::deque<Task>	m_Queue;
	bool				m_bRunning;

};

#endif // _THREAD_POOL_H_

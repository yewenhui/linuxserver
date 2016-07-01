#include "ThreadPool.h"
#include "Exception.h"

#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>

ThreadPool::ThreadPool(const std::string& strName /*= std::string()*/)
	: m_Mutex()
	, m_Cond(m_Mutex)
	, m_StrName(strName)
	, m_bRunning(false)
{

}

ThreadPool::~ThreadPool() {
	if (m_bRunning) {
		Stop();
	}
}

void ThreadPool::Start(int nNumThreads) {
	if (nNumThreads <= 0 || !m_vThreads.empty()) {
		assert(false);
		return;
	}
	m_bRunning = true;
	m_vThreads.reserve(nNumThreads);
	for (int n = 0; n < nNumThreads; ++ n) {
		char chId[32];
		snprintf(chId, sizeof(chId), "%d", n);
		m_vThreads.push_back(new Thread(boost::bind(&ThreadPool::runInThread, this), m_StrName + chId));
		m_vThreads[n].Start();
	}
	
}

void ThreadPool::Stop() {
	m_bRunning = false;
	m_Cond.NotifyAll();

	for_each(m_vThreads.begin()
		, m_vThreads.end()
		, boost::bind(&Thread::Join, _1));
}

void ThreadPool::Run(const Task& rTask) {
	if (m_vThreads.empty()) {
		rTask();
	} else {
		MutexLockGuard lockGuard(m_Mutex);
		m_Queue.push_back(rTask);
		m_Cond.Notify();
	}
}

ThreadPool::Task ThreadPool::take() {
	MutexLockGuard lockGuard(m_Mutex);
	while (m_Queue.empty() && m_bRunning) {
		m_Cond.Wait();
	}
	Task task;
	if (!m_Queue.empty()) {
		task = m_Queue.front();
		m_Queue.pop_front();
	}
	return task;
}

void ThreadPool::runInThread() {
	try {
		while (m_bRunning) {
			Task task(take());
			if (task) {
				task();
			}
		}
	} catch (const Exception &except) {
		fprintf(stderr, "exception caught in ThreadPool %s\n", m_StrName.c_str());
		fprintf(stderr, "reason: %s\n", except.what());
		fprintf(stderr, "stack trace: %s\n", except.StackTrace());
		abort();
		
	} catch (const std::exception& except) {
		fprintf(stderr, "exception caught in ThreadPool %s\n", m_StrName.c_str());
		fprintf(stderr, "reason: %s\n", except.what());
		abort();

	} catch (...) {
		fprintf(stderr, "unknown exception caught in ThreadPool %s\n", m_StrName.c_str());
		abort();
	}
}

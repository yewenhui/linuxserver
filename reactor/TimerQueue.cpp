#define __STDC_LIMIT_MACROS

#include "TimerQueue.h"
#include "Timer.h"
#include "TimerId.h"
#include "EventLoop.h"
#include "../logging/Logging.h"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <sys/timerfd.h>

namespace detail
{
	struct timespec howMuchTimeFromNow(TimeStamp when) {
		int64_t microSeconds = when.GetMicroSecondsSinceEpoch() - TimeStamp::Now().GetMicroSecondsSinceEpoch();
		if (microSeconds < 100) {
			microSeconds = 100;
		}

		struct timespec ts;
		ts.tv_sec = static_cast<time_t>(microSeconds / TimeStamp::MICRO_SECOND_PER_SECOND);
		ts.tv_nsec = static_cast<long>((microSeconds % TimeStamp::MICRO_SECOND_PER_SECOND) * 1000);

		return ts;
	}

	int createTimerfd() {
		int nTimerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
		if (nTimerFd < 0) {
			LOG_SYSFATAL << "Failed in timerfd_create";
		}
		return nTimerFd;
	}

	void readTimerFd(int nTimerFd, TimeStamp now) {
		uint64_t howmany;
		ssize_t n = ::read(nTimerFd, &howmany, sizeof(howmany));
		LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.ToString();
		if (n != sizeof(howmany)) {
			LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
		}
	}

	void resetTimerFd(int nTimerFd, TimeStamp expiration) {
		struct itimerspec newValue;
		::bzero(&newValue, sizeof(newValue));
		struct itimerspec oldValue;
		::bzero(&oldValue, sizeof(oldValue));

		newValue.it_value = howMuchTimeFromNow(expiration);
		int nRet = ::timerfd_settime(nTimerFd, 0, &newValue, &oldValue);
		if (0 != nRet) {
			 LOG_SYSERR << "timerfd_settime()";
		}
	}
}

using namespace detail;

TimerQueue::TimerQueue(EventLoop* pEventLoop) 
	: m_pEventLoop(pEventLoop)
	, m_nTimerFd(createTimerfd())
	, m_TimerFdChannel(pEventLoop, m_nTimerFd)
	, m_TimerList()
	, m_bCallingExpiredTimer(false)
{
	m_TimerFdChannel.SetReadCallBack(boost::bind(&TimerQueue::handleRead, this));
	m_TimerFdChannel.EnableReading();
}

TimerQueue::~TimerQueue() {
	::close(m_nTimerFd);
	for (TimerList::iterator itor = m_TimerList.begin(), end = m_TimerList.end()
		; end != itor
		; ++ itor)
	{
		delete itor->second;
	}
}

TimerId TimerQueue::AddTimer(const TimerCallBack& rCallBack, TimeStamp when, double dInterval) {
	Timer* pTimer = new Timer(rCallBack, when, dInterval);
	m_pEventLoop->RunInLoop(boost::bind(&TimerQueue::addTimerInLoop, this, pTimer));
	return TimerId(pTimer);
}

void TimerQueue::Cancel(TimerId timerId) {
	m_pEventLoop->RunInLoop(boost::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* pTimer) {
	m_pEventLoop->AssertInLoopThread();
	bool bEariestChanged = insert(pTimer);
	if (bEariestChanged) {
		resetTimerFd(m_nTimerFd, pTimer->GetExpiration());
	}
}

void TimerQueue::cancelInLoop(TimerId timerId) {
	m_pEventLoop->AssertInLoopThread();
	assert(m_TimerList.size() == m_ActiveTimers.size());
	ActiveTimer timer(timerId.m_pTimer, timerId.m_Seq);
	ActiveTimerSet::iterator itor = m_ActiveTimers.find(timer);
	if (itor != m_ActiveTimers.end()) {
		size_t n = m_TimerList.erase(Entry(itor->first->GetExpiration(), itor->first));
		assert(1 == n);
		delete itor->first;
		m_ActiveTimers.erase(itor);
	} else if (m_bCallingExpiredTimer) {
		m_CancelTimers.insert(timer);
	}
	assert(m_TimerList.size() == m_ActiveTimers.size());
}

void TimerQueue::handleRead() {
	m_pEventLoop->AssertInLoopThread();
	TimeStamp now(TimeStamp::Now());
	readTimerFd(m_nTimerFd, now);

	std::vector<Entry> vExpired = getExpired(now);

	m_bCallingExpiredTimer = true;
	m_CancelTimers.clear();
	// 
	for (std::vector<Entry>::iterator itor = vExpired.begin(), end = vExpired.end()
		; end != itor; ++ itor)
	{
		itor->second->Run();
	}
	m_bCallingExpiredTimer = false;

	reset(vExpired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now) {
	assert(m_TimerList.size() == m_ActiveTimers.size());
	std::vector<Entry> vExpired;
	Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
	TimerList::iterator itor = m_TimerList.lower_bound(sentry);
	assert(itor == m_TimerList.end() || now < itor->first);
	std::copy(m_TimerList.begin(), itor, back_inserter(vExpired));
	m_TimerList.erase(m_TimerList.begin(), itor);

	BOOST_FOREACH(Entry entry, vExpired) {
		ActiveTimer timer(entry.second, entry.second->GetSequence());
		size_t n = m_ActiveTimers.erase(timer);
		assert(1 == n);
	}

	assert(m_TimerList.size() == m_ActiveTimers.size());
	return vExpired;
}

void TimerQueue::reset(const std::vector<Entry>& rExpired, TimeStamp now) {
	TimeStamp nextExpired;

	for (std::vector<Entry>::const_iterator itor = rExpired.begin(), end = rExpired.end()
		; end != itor; ++ itor)
	{
		ActiveTimer timer(itor->second, itor->second->GetSequence());
		if (itor->second->IsReapeat()
			&& m_CancelTimers.find(timer) == m_CancelTimers.end())
		{
			itor->second->Restart(now);
			insert(itor->second);
		} else {
			delete itor->second;
		}
	}

	if (!m_TimerList.empty()) {
		nextExpired = m_TimerList.begin()->second->GetExpiration();
	}

	if (nextExpired.IsValid()) {
		resetTimerFd(m_nTimerFd, nextExpired);
	}
}

bool TimerQueue::insert(Timer* timer) {
	m_pEventLoop->AssertInLoopThread();
	assert(m_TimerList.size() == m_ActiveTimers.size());

	bool bEariestChanged = false;
	TimeStamp when = timer->GetExpiration();
	TimerList::iterator itor = m_TimerList.begin();
	if (m_TimerList.end() == itor || when < itor->first) {
		bEariestChanged = true;
	}

	{
		std::pair<TimerList::iterator, bool> result = m_TimerList.insert(std::make_pair(when, timer));
		assert(result.second);
	}

	{
		std::pair<ActiveTimerSet::iterator, bool> result = m_ActiveTimers.insert(ActiveTimer(timer, timer->GetSequence()));
		assert(result.second);
	}
	
	assert(m_TimerList.size() == m_ActiveTimers.size());
	return bEariestChanged;
}

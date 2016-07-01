#ifndef _TIMER_QUEUE_H_
#define _TIMER_QUEUE_H_

#include "Channel.h"
#include "CallBacks.h"
#include "../datetime/TimeStamp.h"
#include "../thread/Mutex.h"

#include <set>
#include <vector>
#include <boost/noncopyable.hpp>

class EventLoop;
class Timer;
class TimerId;

class TimerQueue
	: private boost::noncopyable
{
	typedef std::pair<TimeStamp, Timer*> Entry;
	typedef std::set<Entry> TimerList;
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;

public:
	TimerQueue(EventLoop* pEventLoop);
	~TimerQueue();

	TimerId			AddTimer(const TimerCallBack& rCallBack, TimeStamp when, double dInterval);

	void			Cancel(TimerId timerId);

private:
 	void			addTimerInLoop(Timer* pTimer);
 	void			cancelInLoop(TimerId timerId);
	void			handleRead();

	std::vector<Entry> getExpired(TimeStamp now);
	void			reset(const std::vector<Entry>& rExpired, TimeStamp now);

	bool			insert(Timer* timer);

private:
	EventLoop*		m_pEventLoop;
	const int		m_nTimerFd;
	Channel			m_TimerFdChannel;
	TimerList		m_TimerList;

	// for cancel
	bool			m_bCallingExpiredTimer;
	ActiveTimerSet	m_ActiveTimers;
	ActiveTimerSet	m_CancelTimers;

};

#endif // _TIMER_QUEUE_H_

#ifndef _EVENT_LOOP_H_ 
#define _EVENT_LOOP_H_

#include "CallBacks.h"
#include "TimerId.h"
#include "../thread/Thread.h"
#include "../thread/Mutex.h"
#include "../datetime/TimeStamp.h"

#include <vector>
#include <boost/scoped_ptr.hpp>

class Channel;
// class Poller;
class EPoller;
class TimerQueue;

class EventLoop
	: private boost::noncopyable
{
	enum {
		POLL_TIME_MS		= 1000,
	};

	typedef std::vector<Channel*> ChannelList;

public:
	typedef boost::function<void()> Functor;

public:
	EventLoop();
	~EventLoop();

	// 必须在创建此对象线程调用
	void			Loop();

	void			Quit();

	TimeStamp		PollReturnTime() const {
		return m_PollReturnTime;
	}

	/*
		*InLoop函数可以在非创建对象线程调用
	*/
	void			RunInLoop(const Functor& rCallBack);
	void			QueueInLoop(const Functor& rCallBack);

	/*
		Run*函数可以在非创建对象线程调用
	*/
	TimerId			RunAt(const TimeStamp& rTime, const TimerCallBack& rCallBack);
	TimerId			RunAfter(double dDelay, const TimerCallBack& rCallBack);
	TimerId			RunEvery(double dInterval, const TimerCallBack& rCallBack);

	void			Cancel(TimerId timerId);

	void			WakeUp();
	void			UpdateChannel(Channel* pChannel);
	void			RemoveChannel(Channel* pChannel);

	void			AssertInLoopThread();
	bool			IsInLoopThread() const;

private:
	void			abortNotInLoopThread();
	void			handleRead();
	void			doPendingFunctors();

private:
	bool			m_bLooping;				// atomic
	bool			m_bQuit;				// atomic
	TimeStamp		m_PollReturnTime;
	bool			m_bCallingFunctors;		// atomic
	const pid_t		m_threadId;

//	boost::scoped_ptr<Poller> m_pPoller;
	boost::scoped_ptr<EPoller> m_pPoller;
	boost::scoped_ptr<TimerQueue> m_pTimerQueue;
	int				m_nWakeUpFd;
	boost::scoped_ptr<Channel> m_pWakeUpChannel;
	ChannelList		m_ActiveChannels;
	MutexLock		m_Mutex;
	std::vector<Functor> m_vPendingFunctors;

};


#endif // _EVENT_LOOP_H_

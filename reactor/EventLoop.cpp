#include "EventLoop.h"
#include "Channel.h"
// #include "Poller.h"
#include "EPoller.h"
#include "TimerQueue.h"
#include "../logging/Logging.h"

#include <boost/bind.hpp>

#include <assert.h>
#include <signal.h>
#include <sys/eventfd.h>

__thread EventLoop* t_pLoopInThisThread = 0;

static int createEventFd() {
	int nEvtFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (nEvtFd < 0) {
		LOG_SYSERR << "Failed in evevntfd";
		::abort();
	}
	return nEvtFd;
}

//  对一个对端已经关闭的socket调用两次write, 第二次将会生成SIGPIPE信号, 该信号默认结束进程.
class IgnoreSigPipe {
public:
	IgnoreSigPipe() {
		::signal(SIGPIPE, SIG_IGN);
	}
};

IgnoreSigPipe initObj;

EventLoop::EventLoop()
	: m_bLooping(false)
	, m_bQuit(false)
	, m_bCallingFunctors(false)
	, m_threadId(CurrentThread::GetTid())
	//, m_pPoller(new Poller(this))
	, m_pPoller(new EPoller(this))
	, m_pTimerQueue(new TimerQueue(this))
	, m_nWakeUpFd(createEventFd())
	, m_pWakeUpChannel(new Channel(this, m_nWakeUpFd))
{
//	LOG_TRACE << "EventLoop Created" << this << "in thread" << m_threadId;
	if (NULL != t_pLoopInThisThread) {
		LOG_FATAL << "Another EventLoop " << t_pLoopInThisThread << " exists in this thread " << m_threadId;
	} else {
		t_pLoopInThisThread = this;
	}

	m_pWakeUpChannel->SetReadCallBack(boost::bind(&EventLoop::handleRead, this));
	m_pWakeUpChannel->EnableReading();
}

EventLoop::~EventLoop() {
	assert(!m_bLooping);
	t_pLoopInThisThread = NULL;
}

void EventLoop::Loop() {
	assert(!m_bLooping);
	AssertInLoopThread();
	m_bLooping = true;
	m_bQuit = false;

	while (!m_bQuit) {
		m_ActiveChannels.clear();
		m_PollReturnTime = m_pPoller->Poll(POLL_TIME_MS, &m_ActiveChannels);
		for (ChannelList::const_iterator itor = m_ActiveChannels.begin(), end = m_ActiveChannels.end()
			; end != itor
			; ++ itor)
		{
			(*itor)->HandleEvent(m_PollReturnTime);
		}
		doPendingFunctors();
	}

//	LOG_TRACE << "EventLoop " << this << " stop looping";
	m_bLooping = false;
}

void EventLoop::Quit() {
	m_bQuit = true;
	if (!IsInLoopThread()) {
		WakeUp();
	}
}

void EventLoop::RunInLoop(const Functor& rCallBack) {
	if (IsInLoopThread()) {
		rCallBack();
	} else {
		QueueInLoop(rCallBack);
	}
}

void EventLoop::QueueInLoop(const Functor& rCallBack) {
	{
		MutexLockGuard lock(m_Mutex);
		m_vPendingFunctors.push_back(rCallBack);
	}

	if (!IsInLoopThread() || m_bCallingFunctors) {
		WakeUp();
	}
}

TimerId EventLoop::RunAt(const TimeStamp& rTime, const TimerCallBack& rCallBack) {
	return m_pTimerQueue->AddTimer(rCallBack, rTime, 0.0);
}

TimerId EventLoop::RunAfter(double dDelay, const TimerCallBack& rCallBack) {
	TimeStamp time(AddTime(TimeStamp::Now(), dDelay));
	return RunAt(time, rCallBack);
}

TimerId EventLoop::RunEvery(double dInterval, const TimerCallBack& rCallBack) {
	TimeStamp time(AddTime(TimeStamp::Now(), dInterval));
	return m_pTimerQueue->AddTimer(rCallBack, time, dInterval);
}

void EventLoop::UpdateChannel(Channel* pChannel) {
	assert(pChannel->OwnerLoop() == this);
	AssertInLoopThread();
	m_pPoller->UpdateChannel(pChannel);
}

void EventLoop::RemoveChannel(Channel* pChannel) {
	assert(pChannel->OwnerLoop() == this);
	AssertInLoopThread();
	m_pPoller->RemoveChannel(pChannel);
}

void EventLoop::AssertInLoopThread() {
	if (!IsInLoopThread()) {
		abortNotInLoopThread();
	}
}

void EventLoop::Cancel(TimerId timerId) {
	return m_pTimerQueue->Cancel(timerId);
}

void EventLoop::WakeUp() {
	uint64_t oneVal = 1;
	ssize_t n = ::write(m_nWakeUpFd, &oneVal, sizeof(oneVal));
	if (n != sizeof(oneVal)) {
		LOG_ERROR << "EventLoop::WakeUp() writes " << n << " bytes instead of 8";
	}
}

bool EventLoop::IsInLoopThread() const {
	return CurrentThread::GetTid() == m_threadId;
}

void EventLoop::abortNotInLoopThread() {
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
			<< " was created in threadId_ = " << m_threadId
			<< ", current thread id = " <<  CurrentThread::GetTid();
}

void EventLoop::handleRead() {
	uint64_t oneVal = 1;
	ssize_t n = ::read(m_nWakeUpFd, &oneVal, sizeof(oneVal));
	if (n != sizeof(oneVal)) {
		LOG_ERROR << "EventLoop::handleRead() writes " << n << " bytes instead of 8";
	}
}

void EventLoop::doPendingFunctors() {
	std::vector<Functor> vFunctors;
	m_bCallingFunctors = true;

	{
		MutexLockGuard lock(m_Mutex);
		vFunctors.swap(m_vPendingFunctors);
	}

	for(size_t n = 0; n < vFunctors.size(); ++ n) {
		vFunctors[n]();
	}
	m_bCallingFunctors = false;
}

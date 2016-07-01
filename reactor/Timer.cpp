#include "Timer.h"

AtomicInt64 Timer::s_NumCreated;

Timer::Timer(const TimerCallBack& rCallBack, TimeStamp when, double dInterval) 
	: m_CallBack(rCallBack)
	, m_Expiration(when)
	, m_dInterval(dInterval)
	, m_bRepeat(dInterval > 0.0)
	, m_Sequence(s_NumCreated.IncrementAndGet())
{

}

Timer::~Timer() {

}

void Timer::Run() const {
	m_CallBack();
}

TimeStamp Timer::GetExpiration() const {
	return m_Expiration;
}

bool Timer::IsReapeat() const {
	return m_bRepeat;
}

void Timer::Restart(TimeStamp now) {
	if (m_bRepeat) {
		m_Expiration = AddTime(now, m_dInterval);
	} else {
		m_Expiration = TimeStamp::Invalid();
	}
}

#ifndef _TIMER_H_ 
#define _TIMER_H_

#include "CallBacks.h"
#include "../datetime/TimeStamp.h"
#include "../thread/Atomic.h"

#include <boost/noncopyable.hpp>

class Timer
	: private boost::noncopyable
{
public:
	Timer(const TimerCallBack& rCallBack, TimeStamp when, double dInterval);
	~Timer();

	void				Run() const;

	TimeStamp			GetExpiration() const;
	bool				IsReapeat() const;

	int64_t				GetSequence() const {
		return m_Sequence;
	}

	void				Restart(TimeStamp now);

private:
	const TimerCallBack	m_CallBack;
	TimeStamp			m_Expiration;
	const double		m_dInterval;
	const bool			m_bRepeat;
	const int64_t		m_Sequence;

	static AtomicInt64	s_NumCreated;
};

#endif // _TIMER_H_

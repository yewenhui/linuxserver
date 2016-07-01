#ifndef _TIMER_ID_H_ 
#define _TIMER_ID_H_

class Timer;

class TimerId
{
public:
	friend class TimerQueue;

public:
	explicit TimerId(Timer* pTimer = NULL, int64_t seq = 0)
		: m_pTimer(pTimer)
		, m_Seq(seq)
	{
	}

private:
	Timer*		m_pTimer;
	int64_t		m_Seq;

};

#endif // _TIMER_ID_H_

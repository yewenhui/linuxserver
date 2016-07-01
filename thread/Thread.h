#ifndef _THREAD_H_
#define _THREAD_H_

#include "Atomic.h"

#include <pthread.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

class Thread
	: private boost::noncopyable
{
public:
	typedef boost::function<void()> ThreadFunc;

	explicit Thread(const ThreadFunc& func, const std::string& name = std::string());
	~Thread();

	void		Start();
	void		Join();

	bool		Started() const { return m_bStarted; }
	pid_t		GetTid() const { return *m_tIdPtr; }
	const std::string& Name() const { return m_strName; }

	static int	GetNumCreated() { return m_AtomicNumCreated.Get(); }

private:
	bool		m_bStarted;
	bool		m_bJoined;
	pthread_t	m_threadId;
	boost::shared_ptr<pid_t> m_tIdPtr;
	ThreadFunc	m_ThreadFunc;
	std::string	m_strName;

	static AtomicInt32 m_AtomicNumCreated;

};

namespace CurrentThread
{
	pid_t			GetTid();
	const char*		Name();
	bool			IsMainThread();

}

#endif // _THREAD_H_
#ifndef _POLLER_H_
#define _POLLER_H_

#include "EventLoop.h"
#include "../datetime/TimeStamp.h"

#include <map>
#include <vector>

struct pollfd;

class Channel;

class Poller
	: private boost::noncopyable
{
	typedef std::vector<pollfd> PollFdList;
	typedef std::map<int, Channel*> ChannelMap;

public:
	typedef std::vector<Channel*> ChannelList;

public:
	Poller(EventLoop* pEventLoop);
	~Poller();

	TimeStamp		Poll(int nTimeOutMs, ChannelList* pActiveChannels);

	// 必须在Loop线程调用
	void			UpdateChannel(Channel* pChannel);
	// 同上
	void			RemoveChannel(Channel* pChannel);

	void			AssertInLoopChannels() {
		m_pOwnerLoop->AssertInLoopThread();
	}

private:
	void			fillActiveChannels(int nNumEvents, ChannelList* pActiveChannels) const;

private:
	EventLoop*		m_pOwnerLoop;
	PollFdList		m_PollFds;
	ChannelMap		m_mpChannels;

};

#endif // _POLLER_H_

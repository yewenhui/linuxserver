#ifndef _EPOLLER_H_
#define _EPOLLER_H_

#include "EventLoop.h"
#include "../datetime/TimeStamp.h"

#include <vector>
#include <map>

struct epoll_event;

class Channel;

class EPoller
	: private boost::noncopyable
{
	typedef std::vector<struct epoll_event> EventList;
	typedef std::map<int, Channel*> ChannelMap;

	enum {
		INIT_EVENT_LIST_SIZE		= 16,

		NEW							= -1,
		ADDED						= 1,
		DELETED						= 2,
	};

public:
	typedef std::vector<Channel*> ChannelList;

public:
	EPoller(EventLoop* pEventLoop);
	~EPoller();

	// 必须在loop线程调用
	TimeStamp			Poll(int nTimeoutMs, ChannelList* pActiveChannels);
	// 必须在loop线程调用
	void				UpdateChannel(Channel* pChannel);
	// 必须在loop线程调用
	void				RemoveChannel(Channel* pChannel);

	void				AssertInLoopThread() {
		m_pOwnerLoop->AssertInLoopThread();
	}

private:
	void				fillActiveChannels(int nNumEvents, ChannelList* pActiveChannels) const;
	void				update(int nOperation, Channel* pChannel);

private:
	EventLoop*			m_pOwnerLoop;
	int					m_nEpollFd;
	EventList			m_vEvents;
	ChannelMap			m_mpChannel;

};

#endif // _EPOLLER_H_

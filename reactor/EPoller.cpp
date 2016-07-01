#include "EPoller.h"

#include "Channel.h"
#include "../logging/Logging.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

#include <boost/static_assert.hpp>

// 在linux，epoll 和 poll的常量应该是相等的
BOOST_STATIC_ASSERT(EPOLLIN == POLLIN);
BOOST_STATIC_ASSERT(EPOLLPRI == POLLPRI);
BOOST_STATIC_ASSERT(EPOLLOUT == POLLOUT);
BOOST_STATIC_ASSERT(EPOLLRDHUP == POLLRDHUP);
BOOST_STATIC_ASSERT(EPOLLERR == POLLERR);
BOOST_STATIC_ASSERT(EPOLLHUP == POLLHUP);


EPoller::EPoller(EventLoop* pEventLoop) 
	: m_pOwnerLoop(pEventLoop)
	, m_nEpollFd(::epoll_create1(EPOLL_CLOEXEC))
	, m_vEvents(INIT_EVENT_LIST_SIZE)
{
	if (m_nEpollFd < 0) {
		LOG_SYSFATAL << "create epoll err";
	}
}

EPoller::~EPoller() {
	::close(m_nEpollFd);
}

TimeStamp EPoller::Poll(int nTimeoutMs, ChannelList* pActiveChannels) {
	int nNumEvents = epoll_wait(m_nEpollFd
									, &*m_vEvents.begin()
									, static_cast<int>(m_vEvents.size())
									, nTimeoutMs);

	TimeStamp now(TimeStamp::Now());
	if (nNumEvents > 0) {
		LOG_TRACE << nNumEvents << " events happends";
		fillActiveChannels(nNumEvents, pActiveChannels);
		if (static_cast<size_t>(nNumEvents) == m_vEvents.size()) {
			m_vEvents.resize(2 * m_vEvents.size());
		}
	} else if (0 == nNumEvents) {
		LOG_TRACE << "no event";
	} else {
		LOG_SYSERR << "epoll wait err";
	}

	return now;
}

void EPoller::UpdateChannel(Channel* pChannel) {
	AssertInLoopThread();
	LOG_TRACE << "fd = " << pChannel->GetFd() << "events = " << pChannel->GetEvents();
	const int nIndex = pChannel->GetIndex();

	int nFd = pChannel->GetFd();
	if (NEW == nIndex || DELETED == nIndex) {
		if (NEW == nIndex) {
			assert(m_mpChannel.find(nFd) == m_mpChannel.end());
			m_mpChannel[nFd] = pChannel;
		} else {
			assert(m_mpChannel.find(nFd) != m_mpChannel.end());
			assert(m_mpChannel[nFd] == pChannel);
		}
		pChannel->SetIndex(ADDED);
		update(EPOLL_CTL_ADD, pChannel);
	} else {
		assert(m_mpChannel.find(nFd) != m_mpChannel.end());
		assert(m_mpChannel[nFd] == pChannel);
		assert(ADDED == nIndex);
		if (pChannel->IsNoneEvent()) {
			update(EPOLL_CTL_DEL, pChannel);
			pChannel->SetIndex(DELETED);
		} else {
			update(EPOLL_CTL_MOD, pChannel);
		}
	}
}

void EPoller::RemoveChannel(Channel* pChannel) {
	AssertInLoopThread();
	int nFd = pChannel->GetFd();
	LOG_TRACE << "fd = " << nFd;
	assert(m_mpChannel.find(nFd) != m_mpChannel.end());
	assert(m_mpChannel[nFd] == pChannel);
	assert(pChannel->IsNoneEvent());
	int nIndex = pChannel->GetIndex();
	assert(ADDED == nIndex || DELETED == nIndex);
	size_t n = m_mpChannel.erase(nFd);
	assert(1 == n);
	if (ADDED == nIndex) {
		update(EPOLL_CTL_DEL, pChannel);
	}
	pChannel->SetIndex(NEW);
}

void EPoller::fillActiveChannels(int nNumEvents, ChannelList* pActiveChannels) const {
	assert(static_cast<size_t>(nNumEvents) <= m_vEvents.size());
	for (int n = 0; n < nNumEvents; ++n) {
		Channel* pChannel = static_cast<Channel*>(m_vEvents[n].data.ptr);
		int nFd = pChannel->GetFd();
		ChannelMap::const_iterator itor = m_mpChannel.find(nFd);
		assert(itor != m_mpChannel.end());
		assert(itor->second == pChannel);
		pChannel->SetREvents(m_vEvents[n].events);
		pActiveChannels->push_back(pChannel);
	}
}

void EPoller::update(int nOperation, Channel* pChannel) {
	struct epoll_event event;
	::bzero(&event, sizeof(event));
	event.events = pChannel->GetEvents();
	event.data.ptr = pChannel;
	int nFd = pChannel->GetFd();
	if (::epoll_ctl(m_nEpollFd, nOperation, nFd, &event) < 0) {
		if (EPOLL_CTL_DEL == nOperation) {
			LOG_SYSERR << "epoll_ctl op = " << nOperation << " fd = " << nFd;
		} else {
			LOG_SYSFATAL << "epoll_ctl op = " << nOperation << " fd = " << nFd;
		}
	}
}

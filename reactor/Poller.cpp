#include "Poller.h"
#include "Channel.h"
#include "../logging/Logging.h"

#include <assert.h>
#include <poll.h>

Poller::Poller(EventLoop* pEventLoop)
	: m_pOwnerLoop(pEventLoop)
{

}

Poller::~Poller() {

}

TimeStamp Poller::Poll(int nTimeOutMs, ChannelList* pActiveChannels) {
	int nNumEvents = ::poll(&*m_PollFds.begin(), m_PollFds.size(), nTimeOutMs);
	TimeStamp now(TimeStamp::Now());
	if (nNumEvents > 0) {
		LOG_TRACE << nNumEvents << " events happended";
		fillActiveChannels(nNumEvents, pActiveChannels);
	} else if (0 == nNumEvents) {
		LOG_TRACE << " nothing happended";
	} else {
		LOG_SYSERR << "Poller::Poll()";
	}

	return now;
}

void Poller::UpdateChannel(Channel* pChannel) {
	AssertInLoopChannels();
	LOG_TRACE << "fd = " << pChannel->GetFd() << " events = " << pChannel->GetEvents();
	if (pChannel->GetIndex() < 0) {
		assert(m_mpChannels.end() == m_mpChannels.find(pChannel->GetFd()));
		struct pollfd pfd;
		pfd.fd = pChannel->GetFd();
		pfd.events = static_cast<short>(pChannel->GetEvents());
		pfd.revents = 0;
		m_PollFds.push_back(pfd);

		int nIndex = static_cast<int>(m_PollFds.size()) - 1;
		pChannel->SetIndex(nIndex);
		m_mpChannels[pfd.fd] = pChannel;
	} else {
		assert(m_mpChannels.end() != m_mpChannels.find(pChannel->GetFd()));
		assert(m_mpChannels[pChannel->GetFd()] == pChannel);
		int nIndex = pChannel->GetIndex();
		assert(0 <= nIndex && nIndex < static_cast<int>(m_PollFds.size()));
		
		struct pollfd& pfd = m_PollFds[nIndex];
		assert(pfd.fd == pChannel->GetFd() || -pChannel->GetFd() - 1 == pfd.fd);
		pfd.events = static_cast<short>(pChannel->GetEvents());
		pfd.revents = 0;
		if (pChannel->IsNoneEvent()) {
			pfd.fd = -pChannel->GetFd() - 1;
		}
	}
}

void Poller::RemoveChannel(Channel* pChannel) {
	AssertInLoopChannels();
	LOG_TRACE << "fd = " << pChannel->GetFd();
	assert(m_mpChannels.find(pChannel->GetFd()) != m_mpChannels.end());
	assert(m_mpChannels[pChannel->GetFd()] == pChannel);
	assert(pChannel->IsNoneEvent());
	int nIndex = pChannel->GetIndex();
	assert(0 <= nIndex && nIndex < static_cast<int>(m_PollFds.size()));
	const struct pollfd& pfd = m_PollFds[nIndex];
	assert(pfd.fd == -pChannel->GetFd() - 1 && pfd.events == pChannel->GetEvents());
	// assert(pfd.fd == pChannel->GetFd() && pfd.events == pChannel->GetEvents());
	size_t n = m_mpChannels.erase(pChannel->GetFd());
	assert(1 == n); // map×î¶à1
	if ((unsigned int)nIndex == m_PollFds.size() -1) {
		m_PollFds.pop_back();
	} else {
		int nChannelAtEnd = m_PollFds.back().fd;
		iter_swap(m_PollFds.begin() + nIndex, m_PollFds.end() - 1);
		if (nChannelAtEnd < 0) {
			nChannelAtEnd = -nChannelAtEnd - 1;
		}
		m_mpChannels[nChannelAtEnd]->SetIndex(nIndex);
		m_PollFds.pop_back();
	}
}

void Poller::fillActiveChannels(int nNumEvents, ChannelList* pActiveChannels) const {
	for (PollFdList::const_iterator itor = m_PollFds.begin(), end = m_PollFds.end()
		; end != itor && nNumEvents > 0
		; ++ itor)
	{
		if (itor->revents > 0) {
			-- nNumEvents;
			ChannelMap::const_iterator channelItor = m_mpChannels.find(itor->fd);
			assert(m_mpChannels.end() != channelItor);
			Channel* pChannel = channelItor->second;
			assert(pChannel->GetFd() == itor->fd);
			pChannel->SetREvents(itor->revents);
			pActiveChannels->push_back(pChannel);
		}
	}
}

#include "Channel.h"
#include "EventLoop.h"
#include "../logging/Logging.h"

#include <sstream>

Channel::Channel(EventLoop* pEventLoop, int nFd) 
	: m_pEventLoop(pEventLoop)
	, m_Fd(nFd)
	, m_nEvents(0)
	, m_nREvents(0)
	, m_nIndex(-1)
	, m_bEventHandling(false)
{

} 

Channel::~Channel() {
	assert(!m_bEventHandling);
}

void Channel::HandleEvent(TimeStamp receiveTime) {
	m_bEventHandling = true;

	if (m_nREvents & POLLNVAL) {
		LOG_WARNING << "Channel::HandleEvent() POLLNVAL";
	}

	if ((m_nREvents & POLLHUP) && !(m_nREvents & POLLIN)) {
		LOG_WARNING << "POLLHUP";
		if (m_CloseCallBack) {
			m_CloseCallBack();
		}
	}

	if (m_nREvents & (POLLERR | POLLNVAL)) {
		if (m_ErrorCallBack) {
			m_ErrorCallBack();
		}
	}

	if (m_nREvents & (POLLIN | POLLPRI | POLLRDHUP)) {
		if (m_ReadCallBack) {
			m_ReadCallBack(receiveTime);
		}
	}

	if (m_nREvents & POLLOUT) {
		if (m_WriteCallBack) {
			m_WriteCallBack();
		}
	}

	m_bEventHandling = false;
}

void Channel::update() {
	m_pEventLoop->UpdateChannel(this);
}

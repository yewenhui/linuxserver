#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "../datetime/TimeStamp.h"

#include <poll.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

class EventLoop;

class Channel
	: private boost::noncopyable
{
	enum {
		NONE_EVENT		= 0,
		READ_EVENT		= POLLIN | POLLPRI,
		WRITE_EVENT		= POLLOUT,
	};

public:
	typedef boost::function<void()> EventCallBack;
	typedef boost::function<void(TimeStamp)> ReadEventCallBack;

public:
	Channel(EventLoop* pEventLoop, int nFd);
	~Channel();

	void			HandleEvent(TimeStamp receiveTime);

	void			SetReadCallBack(const ReadEventCallBack& rEventCallBack) {
		m_ReadCallBack = rEventCallBack;
	}
	void			SetWriteCallBack(const EventCallBack& rEventCallBack) {
		m_WriteCallBack = rEventCallBack;
	}
	void			SetCloseCallBack(const EventCallBack& rEventCallBack) {
		m_CloseCallBack = rEventCallBack;
	}
	void			SetErrorCallBack(const EventCallBack& rEventCallBack) {
		m_ErrorCallBack = rEventCallBack;
	}

	int				GetFd() {
		return m_Fd;
	}

	int				GetEvents() {
		return m_nEvents;
	}

	bool			IsNoneEvent() const {
		return m_nEvents == NONE_EVENT;
	}
	bool			IsWriting() const {
		return m_nEvents & WRITE_EVENT;
	}

	void			EnableReading() {
		m_nEvents |= READ_EVENT;
		update();
	}
	void			EnableWriting() {
		m_nEvents |= WRITE_EVENT;
		update();
	}

	void			DisableWriting() {
		m_nEvents &= ~WRITE_EVENT;
		update();
	}
	void			DisableAll() {
		m_nEvents = NONE_EVENT;
		update();
	}

	void			SetREvents(int nEvent) {
		m_nREvents = nEvent;
	}
	
	int				GetIndex() {
		return m_nIndex;
	}
	void			SetIndex(int nIndex) {
		m_nIndex = nIndex;
	}

	EventLoop*		OwnerLoop() {
		return m_pEventLoop;
	}

private:
	void			update();

private:
	EventLoop*		m_pEventLoop;
	
	// pollfd 属性
	const int		m_Fd;				// fd
	int				m_nEvents;			// 监听事件
	int				m_nREvents;			// 返回事件

	int				m_nIndex;
	bool			m_bEventHandling;

	ReadEventCallBack m_ReadCallBack;
	EventCallBack	m_WriteCallBack;
	EventCallBack	m_ErrorCallBack;
	EventCallBack	m_CloseCallBack;

};

#endif // _CHANNEL_H_

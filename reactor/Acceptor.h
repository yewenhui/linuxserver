#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include "Socket.h"
#include "Channel.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

class EventLoop;
class InetAddress;

class Acceptor
	: private boost::noncopyable
{
public:
	typedef boost::function<void(int, const InetAddress&)> NewConnectionCallback;

public:
	Acceptor(EventLoop* pEventLoop, const InetAddress& rListenAddr);
	~Acceptor() {}

	void			SetNewConnectionCallback(const NewConnectionCallback& rCallBack) {
		m_NewConCallBack = rCallBack;
	}

	bool			IsListenning() const {
		return m_bListenning;
	}
	void			Listen();

private:
	void			handleRead();

private:
	EventLoop*		m_pEventLoop;
	Socket			m_AccpetSocket;
	Channel			m_AcceptChannel;
	NewConnectionCallback m_NewConCallBack;
	bool			m_bListenning;

};

#endif // _ACCEPTOR_H_

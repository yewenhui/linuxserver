#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_

#include "InetAddress.h"
#include "TimerId.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

class Channel;
class EventLoop;

class Connector
	: private boost::noncopyable
{
	enum eConnectState {
		eCS_Disconnected	= 0,
		eCS_Connecting		,
		eCS_Connected		,
	};

	enum {
		MAX_RETRY_DELAY_MS		= 30 * 1000,
		INIT_RETRY_DELAY_MS		= 500,
	};

public:
	typedef boost::function<void(int)> NewConnectionCallBack;

public:
	Connector(EventLoop* pEvtLoop, const InetAddress& serverAddr);
	~Connector();

	void				SetNewConnectionCallBack(const NewConnectionCallBack& rCallBack) {
		m_NewConnectionCallBack = rCallBack;
	}

	void				Start();		// 任何线程调用
	void				Restart();		// loop线程调用
	void				Stop();			// 任何线程调用

	const InetAddress&	GetServerAddr() const {
		return m_ServerAddr;
	}

private:
	void				setState(eConnectState eState) {
		m_eConnectState = eState;
	}
	void				startInLoop();
	void				connect();
	void				connecting(int nSockFd);
	void				handleWrite();
	void				handleError();
	void				retry(int nSockFd);
	int					removeAndResetChannel();
	void				resetChannel();

private:
	EventLoop*			m_pEventLoop;
	InetAddress			m_ServerAddr;
	bool				m_bConnect;
	eConnectState		m_eConnectState;
	boost::scoped_ptr<Channel> m_pChannel;
	NewConnectionCallBack m_NewConnectionCallBack;
	int					m_nRetryDelayMs;
	TimerId				m_TimerId;

};

typedef boost::shared_ptr<Connector> ConnectorPtr;

#endif // _CONNECTOR_H_

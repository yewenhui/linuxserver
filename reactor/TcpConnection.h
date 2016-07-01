#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include "CallBacks.h"
#include "InetAddress.h"
#include "Buffer.h"
#include "../datetime/TimeStamp.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

class Channel;
class EventLoop;
class Socket;

class TcpConnection
	: private boost::noncopyable
	, public boost::enable_shared_from_this<TcpConnection>
{
	enum eTcpConnectState
	{
		eTCS_Connecting		= 0,
		eTCS_Connected		,
		eTCS_Disconnecting	,
		eTCS_Disconnected	,
		eTCS_Cnt			,
	};

public:
	TcpConnection(EventLoop* pEventLoop
					, const std::string& rStrName
					, int nSockFd
					, const InetAddress& rLocalAddr
					, const InetAddress& rPeerAddr);
	~TcpConnection();

	EventLoop*			GetLoop() const {
		return m_pEventLoop;
	}
	const std::string&	GetName() const {
		return m_strName;
	}
	const InetAddress&	GetLocalAddress() const {
		return m_LocalAddr;
	}
	const InetAddress&	GetPeerAddress() const {
		return m_PeerAddr;
	}
	bool				IsConnected() const {
		return eTCS_Connected == m_eConnectState;
	}

	// 线程安全
	void				Send(const std::string& rStrMsg);
	void				Send(Buffer* pBuf);

	// 线程安全
	void				ShutDown();
	void				SetTcpNoDelay(bool bOn);

	void				SetConnectionCallBack(const ConnectionCallBack& rConnCallBack) {
		m_ConnCallBack = rConnCallBack;
	}
	void				SetMessageCallBack(const MessageCallBack& rMsgCallBack) {
		m_MsgCallBack = rMsgCallBack;
	}
	void				SetWriteCompleteCallBack(const WriteCompleteCallBack& rWriteCompleteCallBack) {
		m_WriteCompleteCallBack = rWriteCompleteCallBack;
	}
	void				SetCloseCallBack(const CloseCallBack& rCloseCallBack) {
		m_CloseCallBack = rCloseCallBack;
	}

	void				ConnectedEstablished();
	void				ConnectedDestroyed();

private:
	void				setConnectedState(eTcpConnectState eTCS) {
		if (eTCS < eTCS_Connecting || eTCS >= eTCS_Cnt) {
			assert(false);
			return;
		}
		m_eConnectState = eTCS;
	}

	void				handleRead(TimeStamp receiveTime);
	void				handleWrite();
	void				handleClose();
	void				handleError();

	void				sendInLoop(const std::string& rStrMsg);
	void				sendInLoop(const void* pData, size_t nLength);
	void				shutDownInLoop();

private:
	EventLoop*			m_pEventLoop;
	std::string			m_strName;
	eTcpConnectState	m_eConnectState;
	boost::scoped_ptr<Socket> m_pSocket;
	boost::scoped_ptr<Channel> m_pChannel;
	InetAddress			m_LocalAddr;
	InetAddress			m_PeerAddr;

	ConnectionCallBack	m_ConnCallBack;
	MessageCallBack		m_MsgCallBack;
	WriteCompleteCallBack m_WriteCompleteCallBack;
	CloseCallBack		m_CloseCallBack;

	Buffer				m_InputBuffer;
	Buffer				m_OutputBuffer;
};

#endif // _TCP_CONNECTION_H_

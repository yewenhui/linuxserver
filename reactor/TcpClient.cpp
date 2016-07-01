#include "TcpClient.h"

#include "Connector.h"
#include "EventLoop.h"
#include "SocketOps.h"

#include "../logging/Logging.h"

#include <boost/bind.hpp>

#include <stdio.h>

namespace detail
{
	void removeConnection(EventLoop* pEventLoop, const TcpConnectionPtr& pTcpConn) {
		pEventLoop->QueueInLoop(boost::bind(&TcpConnection::ConnectedDestroyed, pTcpConn));
	}

	void removeConnector(const ConnectorPtr& pConnector) {
		//pConnector->
	}
}

TcpClient::TcpClient(EventLoop* pEvtLoop, const InetAddress& rServerAddr)
	: m_pEventLoop(CHECK_NOTNULL(pEvtLoop))
	, m_pConnector(new Connector(m_pEventLoop, rServerAddr))
	, m_bRetry(false)
	, m_bConnect(false)
	, m_nNextConnId(1)
{
	m_pConnector->SetNewConnectionCallBack(boost::bind(&TcpClient::newConnection, this, _1));
}

TcpClient::~TcpClient() {
	TcpConnectionPtr pTcpConn;
	{
		MutexLockGuard lock(m_Mutex);
		pTcpConn = m_pTcpConnection;
	}

	if (NULL != pTcpConn) {
		// 非线程安全
		CloseCallBack closeCallBack = boost::bind(&detail::removeConnection, m_pEventLoop, _1);
		m_pEventLoop->RunInLoop(boost::bind(&TcpConnection::SetCloseCallBack, pTcpConn, closeCallBack));
	} else {
		m_pConnector->Stop();
		// Fixme: HACK
		m_pEventLoop->RunAfter(1, boost::bind(&detail::removeConnector, m_pConnector));
	}
}

void TcpClient::Connect() {
	assert(!m_bConnect);
	
	m_bConnect = true;
	m_pConnector->Start();
}

void TcpClient::DisConnect() {
	m_bConnect = false;
	{
		MutexLockGuard lock(m_Mutex);
		if (m_pTcpConnection) {
			m_pTcpConnection->ShutDown();
		}
	}
}

void TcpClient::Stop() {
	m_bConnect = false;
	m_pConnector->Stop();
}

void TcpClient::newConnection(int nSockFd) {
	m_pEventLoop->AssertInLoopThread();
	InetAddress peerAddr(Sockets::GetPeerAddr(nSockFd));
	char buf[32];
	snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.ToHostPort().c_str(), m_nNextConnId);
	++ m_nNextConnId;
	string strConnName = buf;

	InetAddress localAddr(Sockets::GetLocalAddr(nSockFd));
	TcpConnectionPtr pTcpConn(new TcpConnection(m_pEventLoop, strConnName, nSockFd, localAddr, peerAddr));
	pTcpConn->SetConnectionCallBack(m_ConnectionCallBack);
	pTcpConn->SetMessageCallBack(m_MessageCallBack);
	pTcpConn->SetWriteCompleteCallBack(m_WriteCompleteCallBack);
	// 非线程安全
	pTcpConn->SetCloseCallBack(boost::bind(&TcpClient::removeConnection, this, _1));

	{
		MutexLockGuard lock(m_Mutex);
		m_pTcpConnection = pTcpConn;
	}

	pTcpConn->ConnectedEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& pTcpConn) {
	m_pEventLoop->AssertInLoopThread();
	assert(m_pTcpConnection->GetLoop() == m_pEventLoop);

	{
		MutexLockGuard lock(m_Mutex);
		assert(pTcpConn == m_pTcpConnection);
		m_pTcpConnection.reset();
	}

	m_pEventLoop->QueueInLoop(boost::bind(&TcpConnection::ConnectedDestroyed, pTcpConn));
	if (m_bRetry && m_bConnect) {
		m_pConnector->Restart();
	}
}

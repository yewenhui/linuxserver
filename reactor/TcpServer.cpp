#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"
#include "../logging/Logging.h"

#include <stdio.h>  // snprintf

#include <boost/bind.hpp>

TcpServer::TcpServer(EventLoop* pEventLoop, const InetAddress& rListenAddr)
	: m_pEventLoop(pEventLoop)
	, m_strName(rListenAddr.ToHostPort())
	, m_pAcceptor(new Acceptor(pEventLoop, rListenAddr))
	, m_pThreadPool(new EventLoopThreadPool(m_pEventLoop))
	, m_ConnCallBack(DefaultConnectionCallBack)
	, m_MsgCallBack(DefaultMessageCallBack)
	, m_bStared(false)
	, m_nNextConnId(1)
{
	m_pAcceptor->SetNewConnectionCallback(
		boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
	
}

void TcpServer::SetThreadNum(int nNumThread) {
	assert(0 <= nNumThread);
	m_pThreadPool->SetThreadNum(nNumThread);
}

void TcpServer::Start() {
	if (!m_bStared) {
		m_bStared = true;
		m_pThreadPool->Start();
	}

	if (!m_pAcceptor->IsListenning()) {
		m_pEventLoop->RunInLoop(boost::bind(&Acceptor::Listen, get_pointer(m_pAcceptor)));
	}
}

void TcpServer::newConnection(int nSockFd, const InetAddress& rPeerAddr) {
	m_pEventLoop->AssertInLoopThread();
	char buf[32];
	snprintf(buf, sizeof(buf), "#%d", m_nNextConnId);
	++ m_nNextConnId;
	std::string strConnName = m_strName + buf;

	InetAddress localAddr(Sockets::GetLocalAddr(nSockFd));
	EventLoop* pIoEventLoop = m_pThreadPool->GetNextLoop();
	TcpConnectionPtr pTcpConn(new TcpConnection(pIoEventLoop, strConnName, nSockFd, localAddr, rPeerAddr));
	m_mpConnection[strConnName] = pTcpConn;
	pTcpConn->SetConnectionCallBack(m_ConnCallBack);
	pTcpConn->SetMessageCallBack(m_MsgCallBack);
	pTcpConn->SetWriteCompleteCallBack(m_WriteCompleteCallBack);
	pTcpConn->SetCloseCallBack(boost::bind(&TcpServer::removeConnection, this, _1));
	// unsafe
	pIoEventLoop->RunInLoop(boost::bind(&TcpConnection::ConnectedEstablished, pTcpConn));

}

void TcpServer::removeConnection(const TcpConnectionPtr& pTcpConn) {
	// unsafe
	m_pEventLoop->RunInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, pTcpConn));

// 	m_pEventLoop->AssertInLoopThread();
// // 	LOG_INFO << "[" << m_strName << "] - Connection " << pTcpConn->GetName();
// 	size_t n = m_mpConnection.erase(pTcpConn->GetName());
// 	assert(1 == n);
// 	m_pEventLoop->QueueInLoop(boost::bind(&TcpConnection::ConnectedDestroyed, pTcpConn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& pTcpConn) {
	m_pEventLoop->AssertInLoopThread();
// 	LOG_INFO << "[" << m_strName << "] - Connection " << pTcpConn->GetName();
	size_t n = m_mpConnection.erase(pTcpConn->GetName());
	assert(1 == n);
	EventLoop* pIoEventLoop = m_pThreadPool->GetNextLoop();
	pIoEventLoop->QueueInLoop(boost::bind(&TcpConnection::ConnectedDestroyed, pTcpConn));
}

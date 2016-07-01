#include "TcpConnection.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketOps.h"
#include "../logging/Logging.h"

#include <errno.h>

#include <boost/bind.hpp>

void DefaultConnectionCallBack(const TcpConnectionPtr& pTcpCon) {
	LOG_TRACE << pTcpCon->GetLocalAddress().ToIpPort() << " -> "
		<< pTcpCon->GetPeerAddress().ToIpPort() << " is "
		<< (pTcpCon->IsConnected() ? "UP" : "DOWN");
}

void DefaultMessageCallBack(const TcpConnectionPtr pTcpCon, Buffer* pBuf, TimeStamp reveiveTime) {
	pBuf->RetrieveAll();
}

TcpConnection::TcpConnection(EventLoop* pEventLoop 
				, const std::string& rStrName 
				, int nSockFd 
				, const InetAddress& rLocalAddr 
				, const InetAddress& rPeerAddr) 
	: m_pEventLoop(pEventLoop)
	, m_strName(rStrName)
	, m_eConnectState(eTCS_Connecting)
	, m_pSocket(new Socket(nSockFd))
	, m_pChannel(new Channel(m_pEventLoop, nSockFd))
	, m_LocalAddr(rLocalAddr)
	, m_PeerAddr(rPeerAddr)
{
	LOG_DEBUG << m_strName << " fd = " << nSockFd;
	m_pChannel->SetReadCallBack(boost::bind(&TcpConnection::handleRead, this, _1));
	m_pChannel->SetWriteCallBack(boost::bind(&TcpConnection::handleWrite, this));
	m_pChannel->SetCloseCallBack(boost::bind(&TcpConnection::handleClose, this));
	m_pChannel->SetErrorCallBack(boost::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
	LOG_DEBUG << m_strName << " fd = " << m_pChannel->GetFd();
}

void TcpConnection::Send(const std::string& rStrMsg) {
	if (eTCS_Connected == m_eConnectState) {
		if (m_pEventLoop->IsInLoopThread()) {
			sendInLoop(rStrMsg);
		} else {
			m_pEventLoop->RunInLoop(boost::bind(&TcpConnection::sendInLoop, this, rStrMsg));
		}
	}
}

void TcpConnection::Send(Buffer* pBuf) {
	if (NULL == pBuf) {
		return;
	}

	if (eTCS_Connected == m_eConnectState) {
		if (m_pEventLoop->IsInLoopThread()) {
			sendInLoop(pBuf->Peek(), pBuf->ReadableBytes());
			pBuf->RetrieveAll();
		} else {
			m_pEventLoop->RunInLoop(boost::bind(&TcpConnection::sendInLoop, this, pBuf->RetrieveAsString()));
		}
	}
}

void TcpConnection::ShutDown() {
	if (eTCS_Connected == m_eConnectState) {
		setConnectedState(eTCS_Disconnecting);
		m_pEventLoop->RunInLoop(boost::bind(&TcpConnection::shutDownInLoop, this));
	}
}

void TcpConnection::SetTcpNoDelay(bool bOn) {
	m_pSocket->SetTcpNoDelay(bOn);
}

void TcpConnection::ConnectedEstablished() {
	m_pEventLoop->AssertInLoopThread();
	assert(eTCS_Connecting == m_eConnectState);
	setConnectedState(eTCS_Connected);
	m_pChannel->EnableReading();

	if (m_ConnCallBack) {
		m_ConnCallBack(shared_from_this());
	}
}

void TcpConnection::ConnectedDestroyed() {
	m_pEventLoop->AssertInLoopThread();
	assert(eTCS_Connected == m_eConnectState || eTCS_Disconnecting == m_eConnectState);
	setConnectedState(eTCS_Disconnected);
	m_pChannel->DisableAll();
//	m_ConnCallBack(shared_from_this());
	m_pEventLoop->RemoveChannel(get_pointer(m_pChannel));
}

void TcpConnection::handleRead(TimeStamp receiveTime) {
	int nSavedErrno = 0;
	ssize_t unReadSize = m_InputBuffer.ReadFd(m_pChannel->GetFd(), &nSavedErrno);
	if (unReadSize > 0) {
		if (m_MsgCallBack) {
			m_MsgCallBack(shared_from_this(), &m_InputBuffer, receiveTime);
		}
	} else if (0 == unReadSize) {
		handleClose();
	} else {
//		errno = nSavedErrno;
		handleError();
	}
	
}

void TcpConnection::handleWrite() {
	m_pEventLoop->AssertInLoopThread();
	if (m_pChannel->IsWriting()) {
		ssize_t nWrite = ::write(m_pChannel->GetFd(), m_OutputBuffer.Peek(), m_OutputBuffer.ReadableBytes());
		if (nWrite > 0) {
			m_OutputBuffer.Retrieve(nWrite);
			if (0 == m_OutputBuffer.ReadableBytes()) {
				m_pChannel->DisableWriting();
				if (m_WriteCompleteCallBack) {
					m_pEventLoop->QueueInLoop(boost::bind(m_WriteCompleteCallBack, shared_from_this()));
				}
				if (eTCS_Disconnecting == m_eConnectState) {
					shutDownInLoop();
				}
			} else {
				LOG_TRACE << "";
			}
		} else {
			LOG_FATAL << "";
		}
	} else {
		LOG_TRACE << "Connection is down, no more writing.";
	}
}

void TcpConnection::handleClose() {
	m_pEventLoop->AssertInLoopThread();
	LOG_TRACE << "Connection State = " << m_eConnectState;
	assert(eTCS_Connected == m_eConnectState || eTCS_Disconnecting == m_eConnectState);
	m_pChannel->DisableAll();
	m_CloseCallBack(shared_from_this());
}

void TcpConnection::handleError() {
	int nErr = Sockets::GetSocketError(m_pChannel->GetFd());
	LOG_ERROR << "[" << m_strName << "] - SO_ERROR" << nErr << " " << StrError_tl(nErr);
}

void TcpConnection::sendInLoop(const std::string& rStrMsg) {
	sendInLoop(rStrMsg.c_str(), rStrMsg.size());
}

void TcpConnection::sendInLoop(const void* pData, size_t nLength) {
	m_pEventLoop->AssertInLoopThread();
	if (eTCS_Disconnected == m_eConnectState) {
		LOG_WARNING << "disconnected, give up writing";
		return;
	}

	ssize_t nWrite = 0;
	size_t nRemaining = nLength;
	if (!m_pChannel->IsWriting() && 0 == m_OutputBuffer.ReadableBytes()) {
		nWrite = ::write(m_pChannel->GetFd(), pData, nLength);
		if (nWrite >= 0) {
			nRemaining = nLength - nWrite;
			if (0 == nRemaining && m_WriteCompleteCallBack) {
				m_pEventLoop->QueueInLoop(boost::bind(m_WriteCompleteCallBack, shared_from_this()));
			}
		} else {
			nWrite = 0;
			if (EWOULDBLOCK != errno) {
				LOG_SYSERR << "TcpConnection::sendInLoop";
			}
		}
	}

	assert(nRemaining <= nLength);
	if (nRemaining > 0) {
		m_OutputBuffer.Append(static_cast<const char*>(pData) + nWrite, nRemaining);
		if (!m_pChannel->IsWriting()) {
			m_pChannel->EnableWriting();
		}
	}
}

void TcpConnection::shutDownInLoop() {
	m_pEventLoop->AssertInLoopThread();
	if (!m_pChannel->IsWriting()) {
		m_pSocket->ShutDownWrite();
	}
}

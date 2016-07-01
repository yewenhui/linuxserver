#include "Connector.h"

#include "Channel.h"
#include "EventLoop.h"
#include "SocketOps.h"

#include "../logging/Logging.h"

#include <errno.h>

#include <boost/bind.hpp>

Connector::Connector(EventLoop* pEvtLoop, const InetAddress& serverAddr) 
	: m_pEventLoop(pEvtLoop)
	, m_ServerAddr(serverAddr)
	, m_bConnect(false)
	, m_eConnectState(eCS_Disconnected)
	, m_nRetryDelayMs(INIT_RETRY_DELAY_MS)
{

}

Connector::~Connector() {
	m_pEventLoop->Cancel(m_TimerId);
	assert(NULL == m_pChannel);
}

void Connector::Start() {
	m_bConnect = true;
	// unsafe
	m_pEventLoop->RunInLoop(boost::bind(&Connector::startInLoop, this));
}

void Connector::Restart() {
	m_pEventLoop->AssertInLoopThread();
	setState(eCS_Disconnected);
	m_nRetryDelayMs = INIT_RETRY_DELAY_MS;
	m_bConnect = true;
	startInLoop();
}

void Connector::Stop() {
	m_bConnect = false;
	m_pEventLoop->Cancel(m_TimerId);
}

void Connector::startInLoop() {
	m_pEventLoop->AssertInLoopThread();
	assert(eCS_Disconnected == m_eConnectState);
	if (m_bConnect) {
		connect();
	} else {
		LOG_DEBUG << "Do not connect";
	}
}

void Connector::connect() {
	int nSockFd = Sockets::CreateNonBlockingOrDie();
	int nRet = Sockets::Connect(nSockFd, m_ServerAddr.GetSockAddrInet());
	int nSavedErrno = (0 == nRet) ? 0 : errno;
	switch (nSavedErrno) {
	case 0:
	case EINPROGRESS:
	case EINTR:
	case EISCONN:
		connecting(nSockFd);
		break;

	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:
		retry(nSockFd);
		break;

	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_SYSERR << "Connect error" << nSavedErrno;
		Sockets::Close(nSockFd);
		break;

	default:
		LOG_SYSERR << "Unexpected error" << nSavedErrno;
		Sockets::Close(nSockFd);
		break;
	}
}

void Connector::connecting(int nSockFd) {
	setState(eCS_Connecting);
	assert(NULL == m_pChannel);
	m_pChannel.reset(new Channel(m_pEventLoop, nSockFd));
	
	// unsafe
	m_pChannel->SetWriteCallBack(boost::bind(&Connector::handleWrite, this));
	// unsafe
	m_pChannel->SetErrorCallBack(boost::bind(&Connector::handleError, this));

	m_pChannel->EnableWriting();
}

void Connector::handleWrite() {
	LOG_TRACE << m_eConnectState;
	if (eCS_Connecting == m_eConnectState) {
		int nSockFd = removeAndResetChannel();
		int nErr = Sockets::GetSocketError(nSockFd);
		if (0 != nErr) {
			LOG_WARNING << "SO_ERROR = " << nErr << " " << StrError_tl(nErr);
		} else if (Sockets::IsSelfConnect(nSockFd)) {
			LOG_WARNING << "Self connect";
			retry(nSockFd);
		} else {
			setState(eCS_Connected);
			if (m_bConnect) {
				m_NewConnectionCallBack(nSockFd);
			} else {
				Sockets::Close(nSockFd);
			}
		}
	} else {
		assert(eCS_Connected == m_eConnectState);
	}
}

void Connector::handleError() {
	LOG_ERROR << "";
	assert(eCS_Connecting == m_eConnectState);

	int nSockFd = removeAndResetChannel();
	int nErr = Sockets::GetSocketError(nSockFd);
	LOG_TRACE << "SO_ERROR = " << nErr << " " << StrError_tl(nErr);
	retry(nSockFd);
}

void Connector::retry(int nSockFd) {
	Sockets::Close(nSockFd);
	setState(eCS_Connected);
	if (m_bConnect) {
		LOG_INFO << "Retry connecting to" << m_ServerAddr.ToHostPort() << "in "
			<< m_nRetryDelayMs << "milisceonds";

		m_TimerId = m_pEventLoop->RunAfter(m_nRetryDelayMs / 1000.0, boost::bind(&Connector::startInLoop, this));
		m_nRetryDelayMs = std::min(m_nRetryDelayMs * 2, (int)MAX_RETRY_DELAY_MS);

	} else {
		LOG_DEBUG << "do not connect";
	}
}

int Connector::removeAndResetChannel() {
	m_pChannel->DisableAll();
	m_pEventLoop->RemoveChannel(get_pointer(m_pChannel));
	int nSockFd = m_pChannel->GetFd();
	// Con't reset m_pChannel here, because we are inside Channel::HandleEvent
	// unsafe
	m_pEventLoop->QueueInLoop(boost::bind(&Connector::resetChannel, this));

	return nSockFd;
}

void Connector::resetChannel() {
	m_pChannel.reset();
}

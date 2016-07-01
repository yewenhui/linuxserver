#include "SocketOps.h"
#include "../logging/Logging.h"

#include <stdio.h>		// snprintf
#include <strings.h>	// bzero

// fcntl
#include <unistd.h>	
#include <fcntl.h>

#include <errno.h>
#include <sys/socket.h>	// socket

#include <boost/implicit_cast.hpp>

namespace
{
	const struct sockaddr* sockaddr_cast(const struct sockaddr_in* pSockAddr) {
		return static_cast<const struct sockaddr*>(boost::implicit_cast<const void*>(pSockAddr));
	}

	struct sockaddr* sockaddr_cast(struct sockaddr_in* pSockAddr) {
		return static_cast<struct sockaddr*>(boost::implicit_cast<void*>(pSockAddr));
	}

	void setNonBlockingAndCloseOnExec(int nSockFd) {
		int nFlags = ::fcntl(nSockFd, F_GETFL, 0);
		nFlags |= O_NONBLOCK;
		int nRet = ::fcntl(nSockFd, F_SETFL, nFlags);
		assert(-1 != nRet);

		nFlags = ::fcntl(nSockFd, F_GETFD, 0);
		nFlags |= FD_CLOEXEC;
		nRet = ::fcntl(nSockFd, F_SETFD, nFlags);
		assert(-1 != nRet);
	}
}

int Sockets::CreateNonBlockingOrDie() {
	int nSockFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	if (nSockFd < 0) {
		LOG_SYSERR << "Sockets::CreateNonBlockingOrDie";
	}
	return nSockFd;
}

int Sockets::Connect(int nSockFd, const struct sockaddr_in& rAddr) {
	return ::connect(nSockFd, sockaddr_cast(&rAddr), sizeof(rAddr));
}

void Sockets::BindOrDie(int nSockFd, const struct sockaddr_in& rAddr) {
	int nRet = ::bind(nSockFd, sockaddr_cast(&rAddr), sizeof(rAddr));
	if (nRet < 0) {
		LOG_SYSERR << "Sockets::BindOrDie";
	}
}

void Sockets::ListenOrDie(int nSockFd) {
	// /pro/sys/net/core/somaxconn
	int nRet = ::listen(nSockFd, SOMAXCONN);
	if (nRet < 0) {
		LOG_SYSERR << "Sockets::ListenOrDie";
	}
}

int Sockets::Accept(int nSockFd, struct sockaddr_in& rAddr) {
	socklen_t addrLen = sizeof(rAddr);
	int nConnFd = ::accept4(nSockFd, sockaddr_cast(&rAddr), &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (nConnFd < 0) {
		int nSavedErrno = errno;
		LOG_SYSERR << "Socket::accept";
		switch (nSavedErrno) {
		case EAGAIN:
		case ECONNABORTED:
		case EINTR:
		case EPROTO: // ???
		case EPERM:
		case EMFILE: // per-process lmit of open file desctiptor ???
			// expected errors
			errno = nSavedErrno;
			break;
		case EBADF:
		case EFAULT:
		case EINVAL:
		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
		case ENOTSOCK:
		case EOPNOTSUPP:
			// unexpected errors
			LOG_FATAL << "unexpected error of ::accept " << nSavedErrno;
			break;
		default:
			LOG_FATAL << "unknown error of ::accept " << nSavedErrno;
			break;
		}
	}
	return nConnFd;
}

void Sockets::Close(int nSockFd) {
	int nRet = ::close(nSockFd);
	if (nRet < 0) {
		LOG_SYSERR << "Sockets::Close";
	}
}

void Sockets::ShutDownWrite(int nSockFd) {
	int nRet = ::shutdown(nSockFd, SHUT_WR) < 0;
	if (nRet < 0) {
		LOG_SYSERR << "Sockets::ShutDownWrite";
	}
}

void Sockets::ToHostPort(char* pStrBuf, size_t unSize, const struct sockaddr_in& rAddr) {
	char host[INET_ADDRSTRLEN] = "INVALID";
	::inet_ntop(AF_INET, &rAddr.sin_addr, host, sizeof(host));
	uint16_t usPort = Sockets::NetworkToHost16(rAddr.sin_port);
	snprintf(pStrBuf, unSize, "%s:%u", host, usPort);
}

void Sockets::FromHostPort(const char* pStrIp, uint16_t usPort, struct sockaddr_in& rAddr) {
	rAddr.sin_family = AF_INET;
	rAddr.sin_port = Sockets::HostToNetwork16(usPort);

	if (::inet_pton(AF_INET, pStrIp, &rAddr.sin_addr) <= 0) {
		LOG_SYSERR << "Sockets::FromHostPort";
	}
}

struct sockaddr_in Sockets::GetLocalAddr(int nSockFd) {
	struct sockaddr_in localAddr;
	::bzero(&localAddr, sizeof(localAddr));
	socklen_t addrlen = sizeof(localAddr);
	if (::getsockname(nSockFd, sockaddr_cast(&localAddr), &addrlen) < 0) {
		LOG_SYSERR << "Sockets::GetLocalAddr";
	}

	return localAddr;
}

struct sockaddr_in Sockets::GetPeerAddr(int nSockFd) {
	struct sockaddr_in peerAddr;
	::bzero(&peerAddr, sizeof(peerAddr));
	socklen_t addrlen = sizeof(peerAddr);
	if (::getpeername(nSockFd, sockaddr_cast(&peerAddr), &addrlen) < 0) {
		LOG_SYSERR << "Sockets::GetPeerAddr";
	}

	return peerAddr;
}

int Sockets::GetSocketError(int nSockFd) {
	int nOptVal;
	socklen_t optLen = sizeof(nOptVal);
	if (::getsockopt(nSockFd, SOL_SOCKET, SO_ERROR, &nOptVal, &optLen) < 0) {
		return errno;
	}

	return nOptVal;
}

bool Sockets::IsSelfConnect(int nSockFd) {
	struct sockaddr_in localAddr = GetLocalAddr(nSockFd);
	struct sockaddr_in peerAddr = GetPeerAddr(nSockFd);
	return localAddr.sin_port == peerAddr.sin_port
		&& localAddr.sin_addr.s_addr == peerAddr.sin_addr.s_addr;
}

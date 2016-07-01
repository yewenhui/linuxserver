#include "TcpStream.h"
#include "InetAddress.h"

#include <assert.h>

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

TcpStream::TcpStream(Socket&& rSocket)
	: m_Socket(std::move(rSocket))
{

}

TcpStreamPtr TcpStream::Connect(const InetAddress& rServerAddr) {
	TcpStreamPtr tcpStreamPtr;
	Socket socket(Socket::CreateTcp());
	if (0 == socket.Connet(rServerAddr)) {
		tcpStreamPtr.reset(new TcpStream(std::move(socket)));
	}
	return tcpStreamPtr;
}

TcpStreamPtr TcpStream::Connect(const InetAddress& rServerAddr, const InetAddress& rLocalAddr) {
	TcpStreamPtr tcpStreamPtr;
	Socket socket(Socket::CreateTcp());
	socket.Bind(rLocalAddr);
	if (0 == socket.Connet(rServerAddr)) {
		tcpStreamPtr.reset(new TcpStream(std::move(socket)));
	}
	return tcpStreamPtr;
}

int TcpStream::ReadAll(void* pBuf, int nLength) {
// 	if (NULL == pBuf || nLength <= 0) {
// 		assert(false);
// 		return;
// 	}
	return ::recv(m_Socket.GetFd(), pBuf, nLength, MSG_WAITALL);
}

int TcpStream::ReadSome(void* pBuf, int nLength) {
// 	if (NULL == pBuf || nLength <= 0) {
// 		assert(false);
// 		return;
// 	}
	return ::read(m_Socket.GetFd(), pBuf, nLength);
}

int TcpStream::SendAll(void* pBuf, int nLength) {
// 	if (NULL == pBuf || nLength <= 0) {
// 		assert(false);
// 		return;
// 	}
// 	
	int nWrite = 0;
	while (nWrite < nLength) {
		int nRet = ::write(m_Socket.GetFd(), pBuf, nLength);
		if (nRet > 0) {
			nWrite += nRet;
		} else if (EINTR != nRet || 0 == nRet) {
			break;
		}
	}
	return nWrite;
}

int TcpStream::SendSome(void* pBuf, int nLength) {
// 	if (NULL == pBuf || nLength <= 0) {
// 		assert(false);
// 		return;
// 	}
	return ::write(m_Socket.GetFd(), pBuf, nLength);
}

void TcpStream::SetNoDelay(bool bOn) {
	m_Socket.SetTcpNoDelay(bOn);
}

void TcpStream::ShutDownWrite() {
	m_Socket.ShutdownWrite();
}

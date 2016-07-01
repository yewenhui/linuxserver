#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <boost/noncopyable.hpp>

class InetAddress;

class Socket
	//: private boost::noncopyable
{
public:
	explicit Socket(int nSockFd);

	~Socket();

	int				GetFd() const;

	void			Bind(const InetAddress& rAddr);
	void			Listen();
	int				Accpet(InetAddress* pPeerAddr);

	/*
		在bind之前调用
		如果端口忙，TCP状态位于TIME_WAIT，可以重用端口。
		如果端口忙，而TCP状态位于其他状态，重用端口时依旧得到一个错误信息，指明"地址已经使用中"。
		如果你的服务程序停止后想立即重启，而新套接字依旧使用同一端口，此时 SO_REUSEADDR 选项非常有用
	*/
	void			SetReuseAddr(bool bOn);

	void			ShutDownWrite();

	// Nagle's algorithm
	void			SetTcpNoDelay(bool bOn);

private:
	const int		m_nSockFd;

};

#endif // _SOCKET_H_

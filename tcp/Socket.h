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

	int				Connet(const InetAddress& rAddr);
	void			ShutdownWrite();

	void			SetTcpNoDelay(bool bOn);
	void			SetReuseAddr(bool bOn);

	static Socket	CreateTcp();

private:
	int				m_nSockFd;

};

#endif // _SOCKET_H_

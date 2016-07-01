#ifndef _TCP_STREAM_H_
#define _TCP_STREAM_H_

#include "Socket.h"

#include <memory>

#include <boost/noncopyable.hpp>

class InetAddress;
class TcpStream;
typedef std::unique_ptr<TcpStream> TcpStreamPtr;

class TcpStream
	//: private boost::noncopyable
{
public:
	explicit TcpStream(Socket&& rSocket);
//	~TcpStream() = default;

// 	TcpStream(TcpStream&& rhs) = default;
// 	TcpStream& operator=(TcpStream&& rhs) = default;

	static TcpStreamPtr	Connect(const InetAddress& rServerAddr);
	static TcpStreamPtr Connect(const InetAddress& rServerAddr, const InetAddress& rLocalAddr);

	int					ReadAll(void* pBuf, int nLength);
	int					ReadSome(void* pBuf, int nLength);

	int					SendAll(void* pBuf,  int nLength);
	int					SendSome(void* pBuf,  int nLength);

	void				SetNoDelay(bool bOn);
	void				ShutDownWrite();

private:
	Socket				m_Socket;

};


#endif // _TCP_STREAM_H_

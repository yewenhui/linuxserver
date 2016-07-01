#ifndef _CALL_BACKS_H_
#define _CALL_BACKS_H_

#include "../datetime/TimeStamp.h"

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

template<typename To, typename From>
inline boost::shared_ptr<To> down_pointer_cast(const boost::shared_ptr<From>& pFrom) {
#ifndef NODEBUG
	assert(NULL == pFrom || dynamic_cast<To*>(get_pointer(f)) != NULL);
#endif
	return boost::static_pointer_cast<To>(pFrom);
}

class TcpConnection;
class Buffer;

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef boost::function<void()> TimerCallBack;

typedef boost::function<void(const TcpConnectionPtr&)> ConnectionCallBack;
typedef boost::function<void(const TcpConnectionPtr&, Buffer*, TimeStamp)> MessageCallBack;
typedef boost::function<void(const TcpConnectionPtr&)> WriteCompleteCallBack;
typedef boost::function<void(const TcpConnectionPtr&)> CloseCallBack;

typedef boost::function<void(const TcpConnectionPtr&, Buffer*, TimeStamp)> MessageCallBack;

void DefaultConnectionCallBack(const TcpConnectionPtr& pTcpCon);
void DefaultMessageCallBack(const TcpConnectionPtr pTcpCon, Buffer* pBuf, TimeStamp reveiveTime);

#endif // _CALL_BACKS_H_

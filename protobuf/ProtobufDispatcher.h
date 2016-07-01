#ifndef _PROTOBUF_DISPATCHER_H_
#define _PROTOBUF_DISPATCHER_H_

#include "../reactor/CallBacks.h"

#include <google/protobuf/message.h>

#include <map>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#ifndef NODEBUG
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>
#endif

typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

class CallBack
	: private boost::noncopyable
{
public:
	virtual ~CallBack() {}
	virtual void	OnMessage(const TcpConnectionPtr& pTcpCon, const MessagePtr& pMessage, TimeStamp time) const = 0;
};

template<typename T>
class CallBackT 
	: public CallBack
{
#ifndef NODEBUG
	BOOST_STATIC_ASSERT((boost::is_base_of<google::protobuf::Message, T>::value));
#endif

public:
	typedef boost::function<void(const TcpConnectionPtr&, const boost::shared_ptr<T>&, TimeStamp)> ProtobufMessageTCallBack;

public:
	CallBackT(const ProtobufMessageTCallBack& rCallBack)
		: m_CallBack(rCallBack)
	{

	}

	virtual void		OnMessage(const TcpConnectionPtr& pTcpCon, const MessagePtr& pMessage, TimeStamp receiveTime) const {
		boost::shared_ptr<T> pConcrete = down_pointer_cast<T>(pMessage);
		assert(NULL != pConcrete);
		m_CallBack(pTcpCon, pConcrete, receiveTime);
	}

private:
	ProtobufMessageTCallBack m_CallBack;

};

class ProtobufDispatcher
	: private boost::noncopyable
{
	typedef std::map<const google::protobuf::Descriptor*, boost::shared_ptr<CallBack> > CallBackMap;

public:
	typedef boost::function<void(const TcpConnectionPtr&, const MessagePtr&, TimeStamp)> ProtobufMessageTCallBack;

public:
	explicit ProtobufDispatcher(const ProtobufMessageTCallBack& rDefaultCB)
		: m_DefaultCallBack(rDefaultCB)
	{

	}

	void				OnProtobufMessage(const TcpConnectionPtr& pTcpCon, const MessagePtr& pMessage, TimeStamp receiveTime) const {
		CallBackMap::const_iterator itor = m_mpCallBack.find(pMessage->GetDescriptor());
		if (itor != m_mpCallBack.end()) {
			itor->second->OnMessage(pTcpCon, pMessage, receiveTime);
		} else {
			m_DefaultCallBack(pTcpCon, pMessage, receiveTime);
		}
	}

	template<typename T>
	void				RegisterMessageCallBack(const typename CallBackT<T>::ProtobufMessageTCallBack& rCallBack) {
		boost::shared_ptr<CallBackT<T> > pCallBack(new CallBackT<T>(rCallBack));
		m_mpCallBack[T::descriptor()] = pCallBack;
	}

private:
	CallBackMap			m_mpCallBack;
	ProtobufMessageTCallBack m_DefaultCallBack;

};

#endif // _PROTOBUF_DISPATCHER_H_

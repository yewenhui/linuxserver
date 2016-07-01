#ifndef _PROTOBUF_CODEC_H_
#define _PROTOBUF_CODEC_H_

#include "../reactor/Buffer.h"
#include "../reactor/TcpConnection.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <google/protobuf/message.h>

typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

class ProtobufCodec
	: private boost::noncopyable
{
	enum {
		HEADER_LENGTH		= sizeof(int32_t),
		MIN_MESSAGE_LENGTH	= 2 * HEADER_LENGTH + 2,
		MAX_MESSAGE_LENGTH	= 64 * 1024 * 1024,
	};

public:
	enum eProtobufCodecError
	{
		ePCE_NoError		= 0,
		ePCE_InvalidLength	,
		ePCE_CheckSumError	,
		ePCE_InvalidNameLen	,
		ePCE_UnknownMessageType,
		ePCE_ParseError		,
	};

	typedef boost::function<void(const TcpConnectionPtr&, const MessagePtr, TimeStamp)> ProtobufMessageCallBack;
	typedef boost::function<void(const TcpConnectionPtr&, Buffer*, TimeStamp, eProtobufCodecError)> ErrorCallBack;

public:
	explicit ProtobufCodec(const ProtobufMessageCallBack& rMessageCB)
		: m_MessageCallBack(rMessageCB)
		, m_ErrorCallBack(defaultErrorCallBack)
	{

	}

	ProtobufCodec(const ProtobufMessageCallBack& rMessageCB, const ErrorCallBack& rErrorCB)
		: m_MessageCallBack(rMessageCB)
		, m_ErrorCallBack(rErrorCB)
	{

	}

	void			OnMessage(const TcpConnectionPtr& pTcpCon, Buffer* pBuf, TimeStamp reveiveTime);
	void			Send(const TcpConnectionPtr& pTcpCon, const google::protobuf::Message& rMessage);

	static const string& ErrorCodeToString(eProtobufCodecError eErrorCode);
	static void		FillEmptyBuffer(Buffer* pBuf, const google::protobuf::Message& rMessage);
	static google::protobuf::Message* CreateMessage(const string& strTypeName);
	static MessagePtr Parse(const char* buf, int nLength, eProtobufCodecError& rErrorcode);

private:
	static void		defaultErrorCallBack(const TcpConnectionPtr& pTcpCon, Buffer* pBuf, TimeStamp stamp, eProtobufCodecError eError);

private:
	ProtobufMessageCallBack m_MessageCallBack;
	ErrorCallBack	m_ErrorCallBack;


};

#endif // _PROTOBUF_CODEC_H_

#include "ProtobufCodec.h"

#include "google-inl.h"
#include "../logging/Logging.h"
#include "../reactor/SocketOps.h"

#include <google/protobuf/descriptor.h>

#include <zlib.h> // adler32

int32_t AsInt32(const char* pStrBuf) {
	int32_t nVal32 = 0;
	::memcpy(&nVal32, pStrBuf, sizeof(nVal32));
	return Sockets::NetworkToHost32(nVal32);
}

void ProtobufCodec::OnMessage(const TcpConnectionPtr& pTcpCon, Buffer* pBuf, TimeStamp reveiveTime) {
	while (pBuf->ReadableBytes() >= MIN_MESSAGE_LENGTH + HEADER_LENGTH) {
		const int32_t nLen = pBuf->PeekInt32();
		if (nLen > MAX_MESSAGE_LENGTH || nLen < MIN_MESSAGE_LENGTH) {
			m_ErrorCallBack(pTcpCon, pBuf, reveiveTime, ePCE_InvalidLength);
			break;
		} else if (pBuf->ReadableBytes() >= implicit_cast<size_t>(nLen + HEADER_LENGTH)) {
			eProtobufCodecError eErr = ePCE_NoError;
			MessagePtr pMessage = Parse(pBuf->Peek() + HEADER_LENGTH, nLen, eErr);
			if (ePCE_NoError == eErr && pMessage) {
				m_MessageCallBack(pTcpCon, pMessage, reveiveTime);
				pBuf->Retrieve(HEADER_LENGTH + nLen);
			} else {
				m_ErrorCallBack(pTcpCon, pBuf, reveiveTime, eErr);
				break;
			}
		} else {
			break;
		}
	}
}

void ProtobufCodec::Send(const TcpConnectionPtr& pTcpCon, const google::protobuf::Message& rMessage) {
	Buffer buf;
	FillEmptyBuffer(&buf, rMessage);
	pTcpCon->Send(&buf);
}

namespace {
	const string strNoError = "NoError";
	const string strInvalidLength = "InvalidLength";
	const string strCheckSumError = "CheckSumError";
	const string strInvalidNameLen = "InvalidNameLen";
	const string strUnknownMessageType = "UnknownMessageType";
	const string strParseError = "ParseError";
	const string strUnknownError = "UnknownError";
}

const string& ProtobufCodec::ErrorCodeToString(eProtobufCodecError eErrorCode) {
	switch (eErrorCode) {
	case ePCE_NoError:
		return strNoError;
	case ePCE_InvalidLength:
		return strInvalidLength;
	case ePCE_CheckSumError:
		return strCheckSumError;
	case ePCE_InvalidNameLen:
		return strInvalidNameLen;
	case ePCE_UnknownMessageType:
		return strUnknownMessageType;
	case ePCE_ParseError:
		return strParseError;
	default:
		return strUnknownError;
	}
}

void ProtobufCodec::FillEmptyBuffer(Buffer* pBuf, const google::protobuf::Message& rMessage) {
	if(NULL == pBuf) {
		assert(false);
		return;
	}
	assert(0 == pBuf->ReadableBytes());
	const string& rStrTypeName = rMessage.GetTypeName();
	int32_t nNameLen = static_cast<int32_t>(rStrTypeName.size() + 1);
	pBuf->AppendInt32(nNameLen);
	pBuf->Append(rStrTypeName.c_str(), nNameLen);

	GOOGLE_DCHECK(rMessage.IsInitialized()) << InitializationErrorMessage("serialize", rMessage);

	int nByteSize = rMessage.ByteSize();
	pBuf->EnsureWritableBytes(nByteSize);

	uint8_t* pStart = reinterpret_cast<uint8_t*>(pBuf->BeginWrite());
	uint8_t* pEnd = rMessage.SerializeWithCachedSizesToArray(pStart);
	if (pEnd - pStart != nByteSize) {
		ByteSizeConsistencyError(nByteSize, rMessage.ByteSize(), static_cast<int>(pEnd - pStart));
	}
	pBuf->HasWritted(nByteSize);

	int32_t nCheckSum = static_cast<int32_t>(::adler32(1
		, reinterpret_cast<const Bytef*>(pBuf->Peek()), static_cast<int>(pBuf->ReadableBytes())));

	pBuf->AppendInt32(nCheckSum);
	assert(pBuf->ReadableBytes() == sizeof(nNameLen) + nNameLen + nByteSize + sizeof(nCheckSum));
	int32_t nLen = Sockets::HostToNetwork32(static_cast<int32_t>(pBuf->ReadableBytes()));
	pBuf->Prepend(&nLen, sizeof(nLen));
}

google::protobuf::Message* ProtobufCodec::CreateMessage(const string& strTypeName) {
	google::protobuf::Message* pMessage = NULL;
	const google::protobuf::Descriptor* pDescriptor = 
		google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(strTypeName);
	
	if (NULL != pDescriptor) {
		const google::protobuf::Message* pProtoType
			= google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);

		if (NULL != pProtoType) {
			pMessage = pProtoType->New();
		}
	}

	return pMessage;
}

MessagePtr ProtobufCodec::Parse(const char* buf, int nLength, eProtobufCodecError& rErrorcode) {
	MessagePtr pMessage;
	int32_t nExpectedCheckSum = AsInt32(buf + nLength - HEADER_LENGTH);
	int32_t nCheckSum = static_cast<int32_t>(::adler32(1
		, reinterpret_cast<const Bytef*>(buf), static_cast<int>(nLength - HEADER_LENGTH)));

	if (nCheckSum == nExpectedCheckSum) {
		int32_t nNameLength = AsInt32(buf);
		if (nNameLength >= 2 && nNameLength <= nLength - 2 * HEADER_LENGTH) {
			string strTypeName(buf + HEADER_LENGTH, buf + HEADER_LENGTH + nNameLength - 1);
			pMessage.reset(CreateMessage(strTypeName));
			if (NULL != pMessage) {
				const char* pData = buf + HEADER_LENGTH + nNameLength;
				int32_t nDataLen = nLength - nNameLength - 2 * HEADER_LENGTH;
				if (pMessage->ParseFromArray(pData, nDataLen)) {
					rErrorcode = ePCE_NoError;
				} else {
					rErrorcode = ePCE_ParseError;
				}
			} else {
				rErrorcode = ePCE_UnknownMessageType;
			}
		} else {
			rErrorcode = ePCE_InvalidNameLen;
		}
	} else {
		rErrorcode = ePCE_CheckSumError;
	}

	return pMessage;
}

void ProtobufCodec::defaultErrorCallBack(const TcpConnectionPtr& pTcpCon, Buffer* pBuf, TimeStamp stamp, eProtobufCodecError eError) {
	LOG_ERROR << "ProtobufCodec::defaultErrorCallBack - " << ErrorCodeToString(eError);
	if (NULL != pTcpCon && pTcpCon->IsConnected()) {
		pTcpCon->ShutDown();
	}
}

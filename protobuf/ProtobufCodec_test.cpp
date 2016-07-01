#include "ProtobufCodec.h"
#include "../reactor/SocketOps.h"
#include "query.pb.h"

#include <stdio.h>
#include <zlib.h> // adler32

void print(const Buffer& rBuf) {
	printf("encoded to %zd bytes\n", rBuf.ReadableBytes());
	for (size_t n = 0; n < rBuf.ReadableBytes(); ++ n) {
		unsigned char ucVal = static_cast<unsigned char>(rBuf.Peek()[n]);
		printf("%2zd:  0x%02x  %c\n", n, ucVal, isgraph(ucVal) ? ucVal : ' ');
	}
}

void testQuery() {
	Test::Query query;
	query.set_id(1);
	query.set_questioner("leon ye");
	query.add_question("running");

	Buffer buf;
	ProtobufCodec::FillEmptyBuffer(&buf, query);
	print(buf);

	const int32_t nLength = buf.ReadInt32();
	assert(nLength == static_cast<int32_t>(buf.ReadableBytes()));

	ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
	MessagePtr message = ProtobufCodec::Parse(buf.Peek(), nLength, error);
	assert(ProtobufCodec::ePCE_NoError == error);
	assert(NULL != message);

	message->PrintDebugString();
	assert(message->DebugString() == query.DebugString());

	boost::shared_ptr<Test::Query> pQuery = down_pointer_cast<Test::Query>(message);
	assert(NULL != pQuery);
}

void testAnswer() {
	Test::Answer answer;
	answer.set_id(1);
	answer.set_questioner("leon ye");
	answer.set_answerer("www.baidu.com");
	answer.add_solution("Jump!");
	answer.add_solution("Win!");

	Buffer buf;
	ProtobufCodec::FillEmptyBuffer(&buf, answer);
	print(buf);

	const int32_t nLength = buf.ReadInt32();
	assert(nLength == static_cast<int32_t>(buf.ReadableBytes()));

	ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
	MessagePtr message = ProtobufCodec::Parse(buf.Peek(), nLength, error);
	assert(ProtobufCodec::ePCE_NoError == error);
	assert(NULL != message);

	message->PrintDebugString();
	assert(message->DebugString() == answer.DebugString());

	boost::shared_ptr<Test::Answer> pAnswer = down_pointer_cast<Test::Answer>(message);
	assert(NULL != pAnswer);
}

void testEmpty() {
	Test::Empty empty;

	Buffer buf;
	ProtobufCodec::FillEmptyBuffer(&buf, empty);
	print(buf);

	const int32_t nLength = buf.ReadInt32();
	assert(nLength == static_cast<int32_t>(buf.ReadableBytes()));

	ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
	MessagePtr message = ProtobufCodec::Parse(buf.Peek(), nLength, error);
	assert(ProtobufCodec::ePCE_NoError == error);
	assert(NULL != message);

	message->PrintDebugString();
	assert(message->DebugString() == empty.DebugString());
}

void redoCheckSum(string& strData, int nLength) {
	int32_t unCheckSum = Sockets::HostToNetwork32(static_cast<int32_t>(::adler32(1
		, reinterpret_cast<const Bytef*>(strData.c_str()), static_cast<int>(nLength - 4))));

	strData[nLength - 4] = reinterpret_cast<const char*>(&unCheckSum)[0];
	strData[nLength - 3] = reinterpret_cast<const char*>(&unCheckSum)[1];
	strData[nLength - 2] = reinterpret_cast<const char*>(&unCheckSum)[2];
	strData[nLength - 1] = reinterpret_cast<const char*>(&unCheckSum)[3];

}

void testBadBuffer() {
	Test::Empty empty;
	empty.set_id(43);

	Buffer buf;
	ProtobufCodec::FillEmptyBuffer(&buf, empty);

	const int32_t nLength = buf.ReadInt32();
	assert(nLength == static_cast<int32_t>(buf.ReadableBytes()));

	{
		string strData(buf.Peek(), nLength);
		ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
		MessagePtr message = ProtobufCodec::Parse(strData.c_str(), nLength - 1, error);
		assert(NULL == message);
		assert(ProtobufCodec::ePCE_CheckSumError == error);
	}

	{
		string strData(buf.Peek(), nLength);
		ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
		strData[nLength - 1] ++;
		MessagePtr message = ProtobufCodec::Parse(strData.c_str(), nLength, error);
		assert(NULL == message);
		assert(ProtobufCodec::ePCE_CheckSumError == error);
	}

	{
		string strData(buf.Peek(), nLength);
		ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
		strData[0] ++;
		MessagePtr message = ProtobufCodec::Parse(strData.c_str(), nLength, error);
		assert(NULL == message);
		assert(ProtobufCodec::ePCE_CheckSumError == error);
	}

	{
		string strData(buf.Peek(), nLength);
		ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
		strData[3] = 0;
		redoCheckSum(strData, nLength);
		MessagePtr message = ProtobufCodec::Parse(strData.c_str(), nLength, error);
		assert(NULL == message);
		assert(ProtobufCodec::ePCE_InvalidNameLen == error);
	}

	{
		string strData(buf.Peek(), nLength);
		ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
		strData[3] = 100;
		redoCheckSum(strData, nLength);
		MessagePtr message = ProtobufCodec::Parse(strData.c_str(), nLength, error);
		assert(NULL == message);
		assert(ProtobufCodec::ePCE_InvalidNameLen == error);
	}

	{
		string strData(buf.Peek(), nLength);
		ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
		strData[3] --;
		redoCheckSum(strData, nLength);
		MessagePtr message = ProtobufCodec::Parse(strData.c_str(), nLength, error);
		assert(NULL == message);
		assert(ProtobufCodec::ePCE_UnknownMessageType == error);
	}

	{
		string strData(buf.Peek(), nLength);
		ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
		strData[4] = 'M';
		redoCheckSum(strData, nLength);
		MessagePtr message = ProtobufCodec::Parse(strData.c_str(), nLength, error);
		assert(NULL == message);
		assert(ProtobufCodec::ePCE_UnknownMessageType == error);
	}

	{
		// FIXME: reproduce parse error
		string strData(buf.Peek(), nLength);
		ProtobufCodec::eProtobufCodecError error = ProtobufCodec::ePCE_NoError;
		redoCheckSum(strData, nLength);
		MessagePtr message = ProtobufCodec::Parse(strData.c_str(), nLength, error);
		// assert(message == NULL);
		// assert(error == ProtobufCodec::kParseError);
	}
}

int g_count = 0;

void onMessage(const TcpConnectionPtr& conn, const MessagePtr& message, TimeStamp receiveTime) {
	g_count++;
}

void testOnMessage()
{
	Test::Query query;
	query.set_id(1);
	query.set_questioner("Chen Shuo");
	query.add_question("Running?");

	Buffer buf1;
	ProtobufCodec::FillEmptyBuffer(&buf1, query);

	Test::Empty empty;
	empty.set_id(43);
	empty.set_id(1982);

	Buffer buf2;
	ProtobufCodec::FillEmptyBuffer(&buf2, empty);

	size_t totalLen = buf1.ReadableBytes() + buf2.ReadableBytes();
	Buffer all;
	all.Append(buf1.Peek(), buf1.ReadableBytes());
	all.Append(buf2.Peek(), buf2.ReadableBytes());
	assert(all.ReadableBytes() == totalLen);
	TcpConnectionPtr conn;
	TimeStamp t;
	ProtobufCodec codec(onMessage);
	for (size_t len = 0; len <= totalLen; ++len) {
		Buffer input;
		input.Append(all.Peek(), len);
		g_count = 0;
		codec.OnMessage(conn, &input, t);
		int expected = len < buf1.ReadableBytes() ? 0 : 1;
		if (len == totalLen) expected = 2;
		assert(g_count == expected); (void) expected;
		// printf("%2zd %d\n", len, g_count);

		input.Append(all.Peek() + len, totalLen - len);
		codec.OnMessage(conn, &input, t);
		assert(g_count == 2);
	}
}


int main() {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	testQuery();
	puts("");

	testAnswer();
	puts("");

	testEmpty();
	puts("");

	testBadBuffer();
	puts("");

	testOnMessage();
	puts("");

	puts("All pass!!!");
	
	google::protobuf::ShutdownProtobufLibrary();
}

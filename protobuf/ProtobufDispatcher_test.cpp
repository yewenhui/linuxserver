#include "ProtobufDispatcher.h"

#include "query.pb.h"

#include <iostream>

using namespace std;

typedef boost::shared_ptr<Test::Query> QueryPtr;
typedef boost::shared_ptr<Test::Answer> AnswerPtr;

void test_down_pointer_cast() {
	boost::shared_ptr<google::protobuf::Message> msg(new Test::Query);
	boost::shared_ptr<Test::Query> query(down_pointer_cast<Test::Query>(msg));
	assert(msg && query);
	if (NULL == query) {
		abort();
	}
};

void onQuery(const TcpConnectionPtr& pTcpCon, const QueryPtr& msg, TimeStamp time) {
	cout << "onQuery: " << msg->GetTypeName() << endl;
}

void onAnswer(const TcpConnectionPtr& pTcpCon, const AnswerPtr& msg, TimeStamp time) {
	cout << "onAnswer: " << msg->GetTypeName() << endl;
}

void onUnknownMessageType(const TcpConnectionPtr& pTcpCon, const MessagePtr& msg, TimeStamp time) {
	cout << "onUnknownMessageType: " << msg->GetTypeName() << endl;
}

int main() {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	test_down_pointer_cast();

	ProtobufDispatcher dispatcher(onUnknownMessageType);
	dispatcher.RegisterMessageCallBack<Test::Query>(onQuery);
	dispatcher.RegisterMessageCallBack<Test::Answer>(onAnswer);

	TcpConnectionPtr pTcpCon;
	TimeStamp time;

	boost::shared_ptr<Test::Query> query(new Test::Query); 
	boost::shared_ptr<Test::Answer> answer(new Test::Answer);
	boost::shared_ptr<Test::Empty> empty(new Test::Empty);

	dispatcher.OnProtobufMessage(pTcpCon, query, time);
	dispatcher.OnProtobufMessage(pTcpCon, answer, time);
	dispatcher.OnProtobufMessage(pTcpCon, empty, time);

	google::protobuf::ShutdownProtobufLibrary();
}

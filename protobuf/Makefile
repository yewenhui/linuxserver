LIBDIR=/usr/local/lib

CXXFLAGS = -O0 -g -Wall
LDFLAGS = -lprotobuf -Wl,-rpath -Wl,$(LIBDIR) -lz

BASE_SRC = ../logging/LogFile.cpp \
			../logging/Logging.cpp \
			../logging/LogStream.cpp \
			../thread/Thread.cpp \
			../datetime/TimeStamp.cpp \
			
LIB_SRC =	../reactor/EPoller.cpp \
			../reactor/TcpClient.cpp \
			../reactor/Connector.cpp \
			../reactor/EventLoopThreadPool.cpp \
			../reactor/Buffer.cpp \
			../reactor/TcpConnection.cpp \
			../reactor/TcpServer.cpp \
			../reactor/Acceptor.cpp \
			../reactor/InetAddress.cpp \
			../reactor/Socket.cpp \
			../reactor/SocketOps.cpp \
			../reactor/EventLoopThread.cpp \
			../reactor/TimerQueue.cpp \
			../reactor/Timer.cpp \
			../reactor/Channel.cpp \
			../reactor/Poller.cpp \
			../reactor/EventLoop.cpp


BINARIES = protobufdispatcher_test \
			protobufcodec_test \
			protobuf_writetest \
			protobuf_readtest

all: $(BINARIES)

protobufdispatcher_test: ProtobufDispatcher_test.cpp query.pb.cc
protobufdispatcher_test: CXXFLAGS += $(BASE_SRC) $(LIB_SRC)

protobufcodec_test: ProtobufCodec_test.cpp ProtobufCodec.cpp query.pb.cc
protobufcodec_test: CXXFLAGS += $(BASE_SRC) $(LIB_SRC)

protobuf_writetest: addressbook.pb.cc protobuf_writetest.cpp 
protobuf_readtest: addressbook.pb.cc protobuf_readtest.cpp 


$(BINARIES):
	g++ $(CXXFLAGS) -o $@  $(filter %.cpp,$^) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core*
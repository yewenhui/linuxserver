LIB_SRC =	EPoller.cpp \
			TcpClient.cpp \
			Connector.cpp \
			EventLoopThreadPool.cpp \
			Buffer.cpp \
			TcpConnection.cpp \
			TcpServer.cpp \
			Acceptor.cpp \
			InetAddress.cpp \
			Socket.cpp \
			SocketOps.cpp \
			EventLoopThread.cpp \
			TimerQueue.cpp \
			Timer.cpp \
			Channel.cpp \
			Poller.cpp \
			EventLoop.cpp

BINARIES = eventloop_unittest13 eventloop_unittest10 \
#		eventloop_unittest12 \
#		eventloop_unittest11 \
#		eventloop_unittest10 \
#		eventloop_unittest9 \
#		eventloop_unittest8 \
#		eventloop_unittest7 \
#		eventloop_unittest6 \
#		eventloop_unittest5	\
#		eventloop_unittest4 \
#		eventloop_unittest3 \
#		eventloop_unittest1 \
#		eventloop_unittest2 \

all: $(BINARIES)

include ./reactor.mk

eventloop_unittest1: test/EventLoop_unittest1.cpp
eventloop_unittest2: test/EventLoop_unittest2.cpp
eventloop_unittest3: test/EventLoop_unittest3.cpp
eventloop_unittest4: test/EventLoop_unittest4.cpp
eventloop_unittest5: test/EventLoop_unittest5.cpp
eventloop_unittest6: test/EventLoop_unittest6.cpp
eventloop_unittest7: test/EventLoop_unittest7.cpp
eventloop_unittest8: test/EventLoop_unittest8.cpp
eventloop_unittest9: test/EventLoop_unittest9.cpp
eventloop_unittest10: test/EventLoop_unittest10.cpp
eventloop_unittest11: test/EventLoop_unittest11.cpp
eventloop_unittest12: test/EventLoop_unittest12.cpp
eventloop_unittest13: test/EventLoop_unittest13.cpp




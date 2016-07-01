# CXXFLAGS=-g -Wall -rdynamic -march=native
CXXFLAGS=-g -Wall
CXXFLAGS+=-O2

HEADERS = $(wildcard *.h)
TESTS = boundedblockingqueue_unitTest \
		blockingqueue_unittest \
		threadpool_unittest \
		factory_unittest \
		copyandwrite_unitTest \
		observersafe_unittest \
		atomic_unittest \
		thread_unittest \
		singleton_unittest \
		threadlocal_unittest \
		threadlocalsingleton_unitest \
		singletonthreadlocal_unittest \
		exception_unittest \
		observer_unittest \

all: $(TESTS)

# $(TESTS): $(HEADERS)

$(TESTS):
	g++ $(CXXFLAGS) -o $@ $(filter %.cpp, $^) $(LDFLAGS) -lpthread

atomic_unittest: test/Atomic_UnitTest.cpp
thread_unittest: test/Thread_UnitTest.cpp Thread.cpp

singleton_unittest: test/Singleton_UnitTest.cpp Thread.cpp
singleton_unittest: CXXFLAGS += -ftemplate-depth-1000

threadlocal_unittest: test/ThreadLocal_UnitTest.cpp Thread.cpp
threadlocalsingleton_unitest: test/ThreadLocalSingleton_UnitTest.cpp Thread.cpp

singletonthreadlocal_unittest: test/SingletonThreadLocal_UnitTest.cpp Thread.cpp
exception_unittest: test/Exception_UnitTest.cpp Exception.cpp

observer_unittest: test/Observer.cpp
observersafe_unittest: test/ObserverSafe.cpp Thread.cpp
factory_unittest: test/Factory.cpp Thread.cpp

copyandwrite_unitTest: test/CopyAndWrite_UnitTest.cpp Thread.cpp

threadpool_unittest: test/ThreadPool_UnitTest.cpp ThreadPool.cpp Exception.cpp Thread.cpp
blockingqueue_unittest: test/BlockingQueue_UnitTest.cpp Thread.cpp
boundedblockingqueue_unitTest: test/BoundedBlockingQueue_UnitTest.cpp Thread.cpp

clean:
	rm -f $(TESTS)

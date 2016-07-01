CXXFLAGS = -g -O0 -Wall 

TESTS = asynclogging_test \
#		logging_test \
#		logfile_test \
#		logstream_bench \
#		logstream_unittest \

all: $(TESTS)

$(TESTS):
	g++ $(CXXFLAGS) -o $@ $(filter %.cpp, $^)

logstream_unittest: LogStream.h LogStream.cpp test/LogStream_test.cpp
logstream_unittest: CXXFLAGS +=-lboost_unit_test_framework -DBOOST_TEST_DYN_LINK

logstream_bench: LogStream.h LogStream.cpp ../datetime/TimeStamp.cpp test/LogStream_bench.cpp 

logfile_test: ../thread/Thread.cpp LogFile.cpp Logging.cpp LogStream.cpp ../datetime/TimeStamp.cpp test/LogFile_test.cpp 
logfile_test: CXXFLAGS += -lpthread

logging_test: ../thread/Thread.cpp LogFile.cpp Logging.cpp LogStream.cpp ../datetime/TimeStamp.cpp test/Logging_test.cpp 
logging_test: CXXFLAGS += -lpthread

asynclogging_test: ../thread/Thread.cpp ../datetime/TimeStamp.cpp LogFile.cpp Logging.cpp LogStream.cpp AsyncLoggingDoubleBuffering.cpp test/AsyncLogging_test.cpp
asynclogging_test: CXXFLAGS += -lpthread -lrt

# –L：指明动态库所在的目录
# -l：指明动态库的名称，该名称是处在头lib和后缀.so中的名称，如上动态库libtest.so的l参数为-l test。
# asynclogging_test: test/AsyncLogging_test.cpp ../thread/Thread.cpp ../datetime/TimeStamp.cpp
# asynclogging_test: CXXFLAGS += -lpthread -lrt -L . -l logging

# LOGGING_FILES = AsyncLoggingDoubleBuffering.cpp \
#				LogFile.cpp \
#				Logging.cpp \
#				LogStream.cpp \

# –fPIC：表示编译为位置独立的代码，不用此选项的话编译后的代码是位置相关的所以动态载入时是通过代码拷贝的方式来满足不同进程的需要，而不能达到真正代码段共享的目的。
# –shared：指明编译成动态库。
# lib:
#	g++ $(LOGGING_FILES) -fPIC -shared -o liblogging.so
#	cp -rf liblogging.so /usr/lib64

clean:
	rm -f $(TESTS)
CXXFLAGS = -O0 -g -Wall -pthread
LDFLAGS = -lpthread -ldl -lrt -lmysqlclient -L/usr/lib64/mysql -I/usr/include/mysql/ 

BASE_SRC = DBRecord.cpp \
			DBRecordSet.cpp \
			DBConnection.cpp \
			DBConnectionPool.cpp \
			../datetime/TimeStamp.cpp \
			../thread/Thread.cpp \
			../thread/ThreadPool.cpp \
			../thread/Exception.cpp \


BINARIES = dbconnection_unittest2
#			dbconnection_unittest1


all: $(BINARIES)

dbconnection_unittest2: test/dbconnection_unittest2.cpp
dbconnection_unittest1: test/dbconnection_unittest1.cpp

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cpp,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core*

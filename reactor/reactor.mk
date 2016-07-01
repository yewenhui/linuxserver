CXXFLAGS = -O0 -g -Wall -pthread
LDFLAGS = -lpthread
BASE_SRC = ../logging/LogFile.cpp ../logging/Logging.cpp ../logging/LogStream.cpp ../thread/Thread.cpp ../datetime/TimeStamp.cpp

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cpp,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core

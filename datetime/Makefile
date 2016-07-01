CXXFLAGS=-g -Wall 
CXXFLAGS+=-O0

TESTS = timestamp_unittest \

all: $(TESTS)

$(TESTS):
	g++ $(CXXFLAGS) $(LDFLAGS) -lpthread -o $@ $(filter %.cpp, $^) -D__STDC_FORMAT_MACROS

timestamp_unittest: TimeStamp.h TimeStamp.cpp test/TimeStamp_Unittest.cpp

clean:
	rm -f $(TESTS)
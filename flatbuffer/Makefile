CXXFLAGS = -O0 -g -Wall -std=c++11

BASE_SRC = 

BINARIES = flatbuffers_test

all: $(BINARIES)

flatbuffers_test: flatbuffers_test.cpp

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cpp,$^)

clean:
	rm -f $(BINARIES) core*
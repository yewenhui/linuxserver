CXXFLAGS = -O0 -g -Wall -pthread
LDFLAGS = -lpthread -I../logging -I../reactor -I../datetime

BASE_SRC =

BINARIES = discard

all: $(BINARIES)

discard: discard/Main.cpp discard/Discard.cpp

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cpp,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core*

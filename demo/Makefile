.PHONY: clean all

srcs := $(wildcard *.cpp)
targets := $(patsubst %.cpp,%,$(srcs))

CC=g++
CXXFLAGS=-g
CFLAGS=-g
LDFLAGS:=
LIBS:=-levent

all: discover discover_server

discover: discover.cpp sdn_share_protocol.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

discover_server: discover_server.cpp sdn_share_protocol.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@


clean:
	rm -rf $(targets) *.o *.so *.a *.dll

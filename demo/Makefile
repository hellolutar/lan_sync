.PHONY: clean all

srcs := $(wildcard *.cpp)
targets := $(patsubst %.cpp,%,$(srcs))

CC=g++
CXXFLAGS=-g
CFLAGS=-g
LDFLAGS:=
LIBS:=-levent

all: ${targets}

discover: discover.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LIBS) -o $@

discover_server: discover_server.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LIBS) -o $@


clean:
	rm -rf $(targets) *.o *.so *.a *.dll

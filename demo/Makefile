.PHONY: clean all

srcs := $(wildcard *.cpp)
targets := $(patsubst %.cpp,%,$(srcs))
srcutils := $(wildcard utils/*.cpp)
proto_srcs := resource_manager.cpp lan_share_protocol.cpp

CC=g++
CXXFLAGS=-g
CFLAGS=-g
LDFLAGS:=
LIBS:=-levent -lcrypto -lssl -lgtest #-lspdlog

all: discover discover_server 

discover: discover.cpp ${proto_srcs} ${srcutils}
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

discover_server: discover_server.cpp ${proto_srcs} ${srcutils}
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

wc:
	git ls-files | xargs cat | wc -l
clean:
	rm -rf $(targets) *.o *.so *.a *.dll \
	 discover discover_server 

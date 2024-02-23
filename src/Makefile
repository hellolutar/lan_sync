.PHONY: clean cleanall all utils

SRC_C := $(wildcard *.cpp)
targets := $(patsubst %.cpp,%,$(SRC_C))
srcutils := $(wildcard utils/*.cpp)
srccomms := $(wildcard comm/*.cpp)
srcresource := $(wildcard resource/*.cpp)
OBJS := ${srcutils:.cpp=.o}
OBJS += ${srccomms:.cpp=.o}
OBJS += ${srcresource:.cpp=.o}

CC=g++
CXXFLAGS=-g -Wno-format-contains-nul -Wno-write-strings
CFLAGS=${CXXFLAGS}
LDFLAGS:=
LIBS:=-levent -lcrypto -lssl -lgtest -lfmt -lspdlog
workspace:=/home/lutar/code/c/lan_sync/src
# all: discover discover_server 
all: ${OBJS} discover discover_server

%.o: %.cpp
	$(CC) -I${workspace} -c $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

discover: discover.cpp ${proto_SRC_C} ${OBJS} 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

discover_server: discover_server.cpp ${proto_SRC_C} ${OBJS} 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

wc:
	git ls-files | xargs cat | wc -l
clean:
	rm -rf $(targets) logs *.o *.so *.a *.dll discover discover_server 

cleanall: clean
	rm -rf ${OBJS}

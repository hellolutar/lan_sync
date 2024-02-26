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
LDFLAGS:=
LIBS:=-levent -lcrypto -lssl -lgtest -lfmt -lspdlog
workspace:=/home/lutar/code/c/lan_sync/src
C_INCLUDE_PATH +=${workspace}
CPLUS_INCLUDE_PATH +=${workspace}
all: ${OBJS} sync_cli sync_server

%.o: %.cpp
	$(CC) -I${CPLUS_INCLUDE_PATH} -c $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

sync_cli: sync_cli.cpp ${proto_SRC_C} ${OBJS} 
	$(CC) -I${CPLUS_INCLUDE_PATH} $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

sync_server: sync_server.cpp ${proto_SRC_C} ${OBJS} 
	$(CC) -I${CPLUS_INCLUDE_PATH} $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

wc:
	git ls-files | xargs cat | wc -l
clean:
	rm -rf $(targets) logs *.o *.so *.a *.dll sync_cli sync_server 

cleanall: clean
	rm -rf ${OBJS}

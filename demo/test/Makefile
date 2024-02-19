.PHONY: clean all

base_src:=..

srcs := $(wildcard *.cpp)
targets := $(patsubst %.cpp,%,$($(base_src)/srcs))
srcutils := $(wildcard $(base_src)/utils/*.cpp)

CC=g++
CXXFLAGS=-g -I$(base_src)
CFLAGS=-g -I$(base_src)
LDFLAGS:=
LIBS:=-levent -lcrypto -lssl -lgtest #-lspdlog

all: resource_manager_test

resource_manager_test: resource_manager_test.cpp $(base_src)/resource_manager.cpp ${srcutils}
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@


clean:
	rm -rf $(targets) *.o *.so *.a *.dll \
	 discover discover_server resource_manager_test

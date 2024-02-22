.PHONY: clean all

base_src:=..

srcs := $(wildcard *.cpp)
targets := $(patsubst %.cpp,%,$($(base_src)/srcs))
srcutils := $(wildcard $(base_src)/utils/*.cpp)

CC=g++
CXXFLAGS=-g -I$(base_src) -I$(base_src)/utils -Wno-write-strings -Wno-format-contains-nul
CFLAGS=$(CXXFLAGS)
LDFLAGS:=
LIBS:=-levent -lcrypto -lssl -lgtest #-lspdlog

all: resource_manager_test lan_share_protocol_test properties_parse_test

resource_manager_test: resource_manager_test.cpp $(base_src)/resource_manager.cpp ${srcutils}
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

lan_share_protocol_test: lan_share_protocol_test.cpp  $(base_src)/comm/lan_share_protocol.cpp ${srcutils}
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

properties_parse_test: properties_parse_test.cpp $(base_src)/utils/properties_parse.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

clean:
	rm -rf $(targets) *.o *.so *.a *.dll \
	 resource_manager_test lan_share_protocol_test properties_parse_test

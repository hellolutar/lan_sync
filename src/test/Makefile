.PHONY: clean all


WORKSPACE:=/home/lutar/code/c/lan_sync/src
SRC_C := $(wildcard *.cpp)
SRC_UTILS := $(wildcard $(WORKSPACE)/utils/*.cpp)
SRC_COMMS := $(wildcard $(WORKSPACE)/comm/*.cpp)
SRC_RESOURCE := $(wildcard $(WORKSPACE)/resource/*.cpp)
COMMON_OBJS := ${SRC_UTILS:.cpp=.o}
COMMON_OBJS += ${SRC_COMMS:.cpp=.o}
COMMON_OBJS += ${SRC_RESOURCE:.cpp=.o}
TARGET_OBJS += ${SRC_C:.cpp=.o} 
OBJS+=${COMMON_OBJS}
OBJS+=${TARGET_OBJS}
TARGETS := $(patsubst %.cpp,%,$(SRC_C))

CC=g++
CXXFLAGS=-g -Wno-format-contains-nul -Wno-write-strings
LDFLAGS:=
LIBS:=-levent -lcrypto -lssl -lgtest -lfmt -lspdlog
CPLUS_INCLUDE_PATH +=${WORKSPACE}
COMPILE_OPT:=RELEASE#RELEASE or DEV
RELEASE:=RELEASE

all: ${OBJS} test

%.o: %.cpp
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH} -c $(CXXFLAGS) $(LDFLAGS) -D${COMPILE_OPT}  $^ $(LIBS) -o $@

resource_manager_test: resource_manager_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

lan_share_protocol_test: lan_share_protocol_test.o  ${COMMON_OBJS}
	@echo $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

properties_parse_test: properties_parse_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

test: resource_manager_test lan_share_protocol_test properties_parse_test
	./resource_manager_test && \
	./lan_share_protocol_test && \
	./properties_parse_test

clean:
	rm -rf $(TARGETS) *.o *.so *.a *.dll 

.PHONY: clean cleanall all utils

WORKSPACE:=/home/lutar/code/c/lan_sync
SRC_PATH:=/home/lutar/code/c/lan_sync/src
SRC_C := $(wildcard *.cpp)
SRC_UTILS := $(wildcard ${SRC_PATH}/utils/*.cpp)
SRC_PROTOS := $(wildcard ${SRC_PATH}/proto/*.cpp)
SRC_COMPONENTS := $(wildcard ${SRC_PATH}/components/*.cpp)
SRC_NETS := $(wildcard ${SRC_PATH}/net/*.cpp)
SRC_TIMER:= $(wildcard ${SRC_PATH}/timer/*.cpp)
SRC_MODULES:= $(wildcard ${SRC_PATH}/modules/*.cpp)
SRC_LOGICS:= $(wildcard ${SRC_PATH}/logics/*.cpp)
SRC_LOGICS += $(wildcard ${SRC_PATH}/logics/cli/*.cpp)
SRC_LOGICS += $(wildcard ${SRC_PATH}/logics/srv/*.cpp)
SRC_VOS:= $(wildcard ${SRC_PATH}/vo/*.cpp)
SRC_DTOS:= $(wildcard ${SRC_PATH}/vo/dto/*.cpp)
SRC_TASKS:=$(wildcard ${SRC_PATH}/modules/task/*.cpp)
COMMON_OBJS := ${SRC_UTILS:.cpp=.o}
COMMON_OBJS += ${SRC_PROTOS:.cpp=.o}
COMMON_OBJS += ${SRC_COMPONENTS:.cpp=.o}
COMMON_OBJS += ${SRC_NETS:.cpp=.o}
COMMON_OBJS += ${SRC_TIMER:.cpp=.o}
COMMON_OBJS += ${SRC_MODULES:.cpp=.o}
COMMON_OBJS += ${SRC_LOGICS:.cpp=.o}
COMMON_OBJS += ${SRC_VOS:.cpp=.o}
COMMON_OBJS += ${SRC_DTOS:.cpp=.o}
COMMON_OBJS += ${SRC_TASKS:.cpp=.o} 
TARGET_OBJS += ${SRC_C:.cpp=.o} 
OBJS+=${COMMON_OBJS}
OBJS+=${TARGET_OBJS}
TARGETS := $(patsubst %.cpp,%,$(SRC_C))
BIN_TARGETS:= $(patsubst %.cpp,%.bin,$(SRC_C))

CC=g++
CXXFLAGS=-g -Wno-format-contains-nul -Wno-write-strings
LDFLAGS:=
LIBS:=-levent -lcrypto -lssl -lgtest -lfmt -lspdlog
CPLUS_INCLUDE_PATH +=${SRC_PATH}
CPLUS_INCLUDE_PATH +=-I${WORKSPACE}/libs/include
COMPILE_OPT:=DEV#RELEASE or DEV
RELEASE:=RELEASE

all: ${OBJS} ${BIN_TARGETS}

%.bin: %.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH} $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

%.o: %.cpp
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH} -c $(CXXFLAGS)  -D${COMPILE_OPT}  $<  -o $@

sync_cli: sync_cli.o ${COMMON_OBJS} 
ifneq (${COMPILE_OPT} , ${RELEASE})
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH} $(CXXFLAGS) $(LDFLAGS) -D${COMPILE_OPT} $^ $(LIBS) -o $@
else
	@echo "skip sync_cli"
endif

sync_server: sync_server.o ${COMMON_OBJS} 
ifneq (${COMPILE_OPT}, ${RELEASE})
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH} $(CXXFLAGS) $(LDFLAGS) -D${COMPILE_OPT} $^ $(LIBS) -o $@
else
	@echo "skip sync_server"
endif

# sync: ${COMMON_OBJS} ${TARGET_OBJS}
# ifeq (${COMPILE_OPT}, ${RELEASE})
# 	@echo $@
# 	@$(CC) -I${CPLUS_INCLUDE_PATH} -DRELEASE $(CXXFLAGS) -DRELEASE $(LDFLAGS) $^ $(LIBS) -o $@
# else
# 	@echo "skip sync"
# endif


wc:
	git ls-files | xargs cat | wc -l
clean:
	rm -rf ${TARGETS} ${BIN_TARGETS} logs *.o *.so *.a *.dll sync_cli sync_server 

cleanall: clean
	rm -rf ${COMMON_OBJS}

.PHONY: clean cleanall all utils

WORKSPACE:=/home/lutar/code/c/lan_sync/src
SRC_C := $(wildcard *.cpp)
SRC_UTILS := $(wildcard ${WORKSPACE}/utils/*.cpp)
SRC_PROTOS := $(wildcard ${WORKSPACE}/proto/*.cpp)
SRC_RESOURCE := $(wildcard ${WORKSPACE}/resource/*.cpp)
SRC_COMPONENTS := $(wildcard ${WORKSPACE}/components/*.cpp)
SRC_NETS := $(wildcard ${WORKSPACE}/net/*.cpp)
COMMON_OBJS := ${SRC_UTILS:.cpp=.o}
COMMON_OBJS += ${SRC_PROTOS:.cpp=.o}
COMMON_OBJS += ${SRC_RESOURCE:.cpp=.o}
COMMON_OBJS += ${SRC_COMPONENTS:.cpp=.o}
COMMON_OBJS += ${SRC_NETS:.cpp=.o}
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

all: ${OBJS} ${TARGETS}


%.o: %.cpp
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH} -c $(CXXFLAGS) $(LDFLAGS) -D${COMPILE_OPT}  $^ $(LIBS) -o $@

sync_cli: sync_cli.o ${COMMON_OBJS} 
ifneq (${COMPILE_OPT} , ${RELEASE})
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH} $(CXXFLAGS) $(LDFLAGS) -D${COMPILE_OPT} $^ $(LIBS) -o $@
else
	@echo "skip sync_cli"
endif

# sync_server: sync_server.o ${COMMON_OBJS} 
# ifneq (${COMPILE_OPT}, ${RELEASE})
# 	@echo $@
# 	@$(CC) -I${CPLUS_INCLUDE_PATH} $(CXXFLAGS) $(LDFLAGS) -D${COMPILE_OPT} $^ $(LIBS) -o $@
# else
# 	@echo "skip sync_server"
# endif

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
	rm -rf $(TARGETS) logs *.o *.so *.a *.dll sync_cli sync_server 

cleanall: clean
	rm -rf ${COMMON_OBJS}

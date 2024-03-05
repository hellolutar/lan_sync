include ../Makefile

all := tcpserver ${OBJS} ${TARGETS}


tcpserver: tcpserver.o ${COMMON_OBJS} 
	$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@
	
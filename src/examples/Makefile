include ../Makefile

all := tcpserver ${OBJS} ${TARGETS}


tcpserver: tcpserver.o ${COMMON_OBJS} 
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

udpserver: udpserver.o ${COMMON_OBJS} 
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@
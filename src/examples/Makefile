include ../Makefile

%.bin: %.o ${COMMON_OBJS} 
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@
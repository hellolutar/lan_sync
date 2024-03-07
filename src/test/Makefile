include ../Makefile

all := ${OBJS} ${TARGETS}

resource_manager_test: resource_manager_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

lan_share_protocol_test: lan_share_protocol_test.o  ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

properties_parse_test: properties_parse_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

io_utils_test: io_utils_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

net_addr_test: net_addr_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

test: ${TARGETS}
	./io_utils_test && \
	./resource_manager_test && \
	./lan_share_protocol_test && \
	./properties_parse_test
	./net_addr_test

clean:
	rm -rf $(TARGETS) *.o *.so *.a *.dll 

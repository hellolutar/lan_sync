include ../src/Makefile

all := ${OBJS} ${BIN_TARGETS}

resource_manager_test.bin: resource_manager_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

lan_share_protocol_test.bin: lan_share_protocol_test.o  ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

properties_parse_test.bin: properties_parse_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

io_utils_test.bin: io_utils_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

net_addr_test.bin: net_addr_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

openssl_utils_test.bin : openssl_utils_test.o ${COMMON_OBJS}
	@echo $@
	@$(CC) -I${CPLUS_INCLUDE_PATH}  $(CXXFLAGS)  $(LDFLAGS) $^ $(LIBS) -o $@

test: ${BIN_TARGETS}
	./io_utils_test.bin && \
	./resource_manager_test.bin && \
	./lan_share_protocol_test.bin && \
	./properties_parse_test.bin && \
	./net_addr_test.bin && \
	./openssl_utils_test.bin

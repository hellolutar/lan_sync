#include "abst_net_logic.h"

void LogicTcp::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    recv_tcp(data, data_len, ctx);
}

void LogicUdp::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    recv_udp(data, data_len, ctx);
}
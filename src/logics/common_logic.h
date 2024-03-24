#ifndef __COMMON_LOGIC_H_
#define __COMMON_LOGIC_H_

#include "net/abst_net_logic.h"
#include "proto/lan_share_protocol.h"
#include "utils/logger.h"

class SyncCommonLoigc : public AbstNetLogic
{
public:
    SyncCommonLoigc(){};
    ~SyncCommonLoigc(){};

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    uint64_t isExtraAllDataNow(void *data, uint64_t data_len) override;
};


#endif
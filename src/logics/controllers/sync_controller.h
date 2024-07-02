#ifndef __SYNC_CONTROLLER_H_
#define __SYNC_CONTROLLER_H_

#include "components/sync_network_conn_ctx.h"
#include "logics/services/sync_service.h"
#include "net/abst_net_logic.h"
#include "modules/conn/mod_conn.h"

class SyncController : public LogicTcp, public LogicUdp
{
private:
    SyncService &syncService;

public:
    SyncController(SyncService &service) : syncService(service){};
    ~SyncController();

    void recv_udp(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx) override;
    void recv_tcp(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx) override;
    uint64_t isExtraAllDataNow(void *data, uint64_t data_len) override;
};

#endif
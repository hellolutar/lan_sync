#ifndef __SYNC_SERVICE_H_
#define __SYNC_SERVICE_H_

#include <cstdint>
#include "components/sync_network_conn_ctx.h"
#include "components/sync_io_read_monitor.h"
#include "modules/task/req_rs_task.h"
#include "modules/task/task_manager.h"
#include "modules/rs_manager.h"
#include "modules/config_manager.h"
#include "modules/conn/mod_conn.h"


class SyncService : public ModConnAbility
{
private:
    /* data */
public:
    SyncService(AbsModConnMediator *med) : ModConnAbility(med, MODULE_NAME_SYNC_SERVICE){};
    ~SyncService();

    void mod_conn_recv(std::string from, std::string uri, void *data) override;

    void handleHello(SyncNetworkConnCtx *ctx);
    void handleHelloAck(SyncNetworkConnCtx *ctx);

    void handleReqTableIndex(SyncNetworkConnCtx *ctx);
    void handleReqResource(SyncNetworkConnCtx *ctx);

    void handleReplyTableIndex(void *data, uint64_t data_len, SyncNetworkConnCtx *ctx);
    void handleReplyResource(void *data, uint64_t data_len, SyncNetworkConnCtx *ctx);
    void add_req_task(SyncNetworkConnCtx *ctx);

    void exit(void *data, uint64_t data_len, SyncNetworkConnCtx *ctx);
};

#endif
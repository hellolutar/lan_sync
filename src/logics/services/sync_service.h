#ifndef __SYNC_SERVICE_H_
#define __SYNC_SERVICE_H_

#include <cstdint>

#include "components/sync_network_conn_ctx.h"
#include "components/sync_io_read_monitor.h"
#include "modules/rs_manager.h"
#include "modules/config_manager.h"
#include "modules/conn/mod_conn.h"
#include "vo/dto/mod_conn_task_dto.h"
#include "modules/task2/task_coordinator.h"

class SyncService : public ModConnAbility
{
private:
    TaskCoordinator &coor_;

public:
    SyncService(AbsModConnMediator *med, TaskCoordinator &coor)
        : coor_(coor), ModConnAbility(med, MODULE_NAME_SYNC_SERVICE){};
    ~SyncService();

    void mod_conn_recv(std::string from, std::string uri, void *data) override;

    void handleHello(std::shared_ptr<SyncNetworkConnCtx> ctx);
    void handleHelloAck(std::shared_ptr<SyncNetworkConnCtx> ctx);

    void handleReqTableIndex(std::shared_ptr<SyncNetworkConnCtx> ctx);
    void handleReqResource(std::shared_ptr<SyncNetworkConnCtx> ctx);

    void handleReplyTableIndex(void *data, uint64_t data_len, std::shared_ptr<SyncNetworkConnCtx> ctx);
    void handleReplyResource(void *data, uint64_t data_len, std::shared_ptr<SyncNetworkConnCtx> ctx);

    void exit(void *data, uint64_t data_len, std::shared_ptr<SyncNetworkConnCtx> ctx);
};

#endif
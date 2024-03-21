#ifndef __REQ_RS_TASK_H_
#define __REQ_RS_TASK_H_

#include <random>

#include "abs_task.h"
#include "net/network_layer.h"
#include "net/net_framework_impl_with_event.h"
#include "modules/rs_manager.h"
#include "proto/lan_share_protocol.h"
#include "components/buf_base_on_event.h"
#include "modules/task/task_manager.h"

class ReqRsTask : public AbsTask
{
private:
    std::string uri;
    void sendRsReq(NetworkConnCtx *ctx, Block b);

public:
    ReqRsTask(std::string name, std::string uri)
        : AbsTask(name), uri(uri){};
    ~ReqRsTask(){};
    void run() override;
};

#endif
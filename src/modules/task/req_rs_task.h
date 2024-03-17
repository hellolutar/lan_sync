#ifndef __REQ_RS_TASK_H_
#define __REQ_RS_TASK_H_

#include "abs_task.h"
#include "net/network_layer.h"
#include "modules/rs_manager.h"
#include "proto/lan_share_protocol.h"
#include "components/buf_base_on_event.h"

class ReqRsTask : public AbsTask
{
private:
    NetworkConnCtx *nctx;
    std::string uri;
    std::vector<Block> reqBlocks;

public:
    ReqRsTask(std::string name, NetworkConnCtx *nctx, std::string uri, std::vector<Block> reqBlocks)
        : AbsTask(name), nctx(nctx), uri(uri), reqBlocks(reqBlocks){};
    ~ReqRsTask(){};
    void run() override;
};

#endif
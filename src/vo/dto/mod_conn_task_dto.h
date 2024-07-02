#ifndef __MOD_CONN_TASK_DTO_H_
#define __MOD_CONN_TASK_DTO_H_

#include <string>
#include <memory>

#include "modules/task2/range.h"
#include "net/network_layer.h"

struct ModconnTaskDto
{
    std::string uri;
    Range2 range;
    std::shared_ptr<NetworkConnCtx> ctx;
};

#endif
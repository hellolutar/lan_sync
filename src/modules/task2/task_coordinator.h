#ifndef __COORDINATOR_H_
#define __COORDINATOR_H_

#include <map>
#include <string>
#include <algorithm>
#include <memory>

#include "modules/task2/task.h"
#include "modules/task2/resource.h"

/**
 * 负责协调任务
 * - 由TaskManager2具体管理任务
 */
class TaskCoordinator
{
private:
    std::shared_ptr<TaskManager2> tm_;

    std::map<std::string, ResourceInfo> res_;

    void analysis_resource(ResourceInfo info, std::shared_ptr<NetworkConnCtx> ctx);

    void assignTask(ResourceInfo &info);

public:
    TaskCoordinator(std::shared_ptr<TaskManager2> tm) : tm_(tm){};
    ~TaskCoordinator();

    void tick(std::uint64_t tick);

    std::shared_ptr<TaskManager2> &taskManager();

    void add_resource(std::string uri, Range2 range, std::shared_ptr<NetworkConnCtx> ctx);

    void reAssignTask(const std::string uri, const Block2 blk, const std::shared_ptr<NetworkConnCtx> oldCtx);
};

#endif
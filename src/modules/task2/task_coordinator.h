#ifndef __COORDINATOR_H_
#define __COORDINATOR_H_

#include <map>
#include <string>
#include <algorithm>
#include <memory>

#include "modules/task2/task.h"
#include "modules/task2/resource.h"

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

    /**
     * case1: 新增一个资源服务器
     * case2: 在已有的资源服务器上，新增一个资源服务器
     * case3：在已有的资源服务器上，新增一个更新的资源服务器
     */
    void add_resource(std::string uri, Range2 range, std::shared_ptr<NetworkConnCtx> ctx);

    void reAssignTask(const std::string uri, const Block2 blk, const std::shared_ptr<NetworkConnCtx> oldCtx);
};

#endif
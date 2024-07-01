#include "modules/task2/coordinator.h"

using namespace std;

void Coordinator::analysis_resource(ResourceInfo info, std::shared_ptr<NetworkConnCtx> ctx)
{
    std::string uri = info.getUri();
    auto r_iter = res_.find(uri);

    if (r_iter == res_.end()) // 为新资源
        res_.insert(std::pair<std::string, ResourceInfo>(info.getUri(), info));
    else
    {
        // 找到了， 判断是否需要更新资源
        ResourceInfo &old_r = r_iter->second;
        if (old_r.getRange() < info.getRange())
        {
            tm_->cancelTask(uri);                                                   // 取消任务， todo 删除本地文件
            res_.erase(r_iter);                                                     // 资源表中移除
            res_.insert(std::pair<std::string, ResourceInfo>(info.getUri(), info)); // 向资源表插入新资源信息
        }
        else if (old_r.getRange() == info.getRange())
            old_r.addNetCtx(ctx);
        else
            return; // 新资源为过时资源，故跳过
    }
    r_iter = res_.find(uri);
    ResourceInfo &r = r_iter->second;
    assignTask(r); // 指派任务
}

void Coordinator::assignTask(ResourceInfo &info)
{
    const std::vector<std::shared_ptr<NetworkConnCtx>> &ctxs = info.getCtxs();
    if (ctxs.size() == 0)
    {
        // todo log_warn
        return;
    }

    tm_->stopPendingTask(info.getUri());

    uint16_t who = 0;
    if (info.size() <= BLOCK_SIZE)
    {
        string uri = info.getUri();
        std::shared_ptr<NetworkConnCtx> ctx = ctxs[who]; // 可能存在拷贝陷进
        tm_->addTask({uri, Block(0, info.size()), ctx});
    }
    else
    {
        uint num = info.size() / BLOCK_SIZE + 1; // 向上取整
        for (uint i = 0; i < num; i++)
        {
            if (who >= ctxs.size())
                who = 0;
            uint64_t bitPos = Block::bitPos(i);
            if (bitPos >= info.size())
                break;

            uint64_t end = min(info.size(), bitPos + BLOCK_SIZE);
            string uri = info.getUri();
            std::shared_ptr<NetworkConnCtx> ctx = ctxs[who++]; // 可能存在拷贝陷进
            tm_->addTask({uri, Block(bitPos, end), ctx});
        }
    }
}

Coordinator::~Coordinator()
{
}

void Coordinator::tick(std::uint64_t t)
{
    auto f = bind(&Coordinator::reAssignTask, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    tm_->tick(t, f);
}

std::shared_ptr<TaskManager2> &Coordinator::taskManager()
{
    return tm_;
}

void Coordinator::add_resource(std::string uri, Range2 range, std::shared_ptr<NetworkConnCtx> ctx)
{
    ResourceInfo r{uri, range};
    r.addNetCtx(ctx);
    analysis_resource(r, ctx);
}

void Coordinator::reAssignTask(const std::string uri, const Block blk, const std::shared_ptr<NetworkConnCtx> oldCtx)
{
    auto r_iter = res_.find(uri);
    ResourceInfo &r = r_iter->second;

    std::shared_ptr<NetworkConnCtx> ctx;
    const std::vector<std::shared_ptr<NetworkConnCtx>> &ctxs = r.getCtxs();
    for (size_t i = 0; i < ctxs.size(); i++)
    {
        if (ctxs[i]->getPeer() == oldCtx->getPeer())
        {
            if (i == ctxs.size() - 1)
                ctx = ctxs[0];
            else
            {
                ctx = ctxs[i + 1];
            }
            tm_->addTask({r.getUri(), blk, ctx});
            break;
        }
    }
}

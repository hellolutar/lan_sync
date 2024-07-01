#include "modules/task2/resource.h"

ResourceInfo::~ResourceInfo()
{
}

const Range2 &ResourceInfo::getRange() const
{
    return range_;
}

const std::string ResourceInfo::getUri() const
{
    return uri_;
}

void ResourceInfo::addNetCtx(std::shared_ptr<NetworkConnCtx> ctx)
{
    ctxs_.push_back(ctx);
}

const std::vector<std::shared_ptr<NetworkConnCtx>> &ResourceInfo::getCtxs() const
{
    return ctxs_;
}

const uint64_t ResourceInfo::size() const
{
    return range_.size();
}

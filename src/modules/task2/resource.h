#ifndef __RESOURCE_H_
#define __RESOURCE_H_

#include <vector>
#include <string>
#include <memory>

#include "modules/task2/range.h"
#include "modules/task2/common.h"
#include "net/network_layer.h"


// class SyncingInfo
// {
// private:
//     Range range_;
//     NetworkConnCtx &ctx_;

// public:
//     SyncingInfo(Range range, NetworkConnCtx &ctx) : range_(range), ctx_(ctx) {}
//     ~SyncingInfo() {}
// };

class ResourceInfo
{
private:
    std::string uri_;
    Range2 range_;
    std::vector<std::shared_ptr<NetworkConnCtx>> ctxs_;
    // std::vector<SyncingInfo> syncings_;

public:
    ResourceInfo(std::string uri, Range2 range) : uri_(uri), range_(range){};
    ~ResourceInfo();

    const Range2 &getRange() const;

    const std::string getUri() const;

    void addNetCtx(std::shared_ptr<NetworkConnCtx> ctx);

    const std::vector<std::shared_ptr<NetworkConnCtx>> &getCtxs() const;

    const uint64_t size() const;
};

#endif
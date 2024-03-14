#ifndef __RS_SYNC_MANAGER_H_
#define __RS_SYNC_MANAGER_H_

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <filesystem>

#include "net/net_addr.h"
#include "modules/rs_manager.h"
#include "constants/constants.h"
class Block
{
public:
    uint64_t start;
    uint64_t end;
    Block(){};
    Block(uint64_t start, uint64_t end) : start(start), end(end){};
    ~Block(){};

    bool operator==(const Block &other);
};

class SyncingRange
{
private:
public:
    NetAddr peer;
    Block block; // [start, end)
    time_t timeOut;
    SyncingRange(){};
    SyncingRange(NetAddr peer, Block block);
    ~SyncingRange(){};
};

class SyncRs
{
public:
    std::string uri;
    std::uint64_t size;
    std::string hash;
    std::vector<Block> block;
    std::vector<NetAddr> owner;
    std::vector<SyncingRange> syncing;

    SyncRs();
    SyncRs(std::string uri, std::uint64_t size, std::string hash, NetAddr owner);
    ~SyncRs(){};
};

class RsSyncManager
{
private:
    std::map<std::string, SyncRs> uriRs;

public:
    RsSyncManager();
    ~RsSyncManager();

    void refreshSyncingRsByTbIdx(NetAddr peer, struct Resource *table, uint64_t rs_size);
    bool regSyncRs(NetAddr peer, std::string uri, std::string hash, uint64_t size);
    bool updateSyncRs(NetAddr peer, std::string uri, std::string hash, uint64_t size);
    Block regReqSyncRsAuto(NetAddr peer, std::string uri);
    // vector<uint64_t, uint64_t> regReqSyncRsCplt(NetAddr peer, std::string uri) { return {}; };
    // vector<uint64_t, uint64_t> regReqSyncRsRang(NetAddr peer, std::string uri, uint64_t offset, uint64_t size) { return {}; };
    void syncingRangeDone(NetAddr peer, string uri, Block block);
};

#endif
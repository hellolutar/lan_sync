#ifndef __RS_SYNC_MANAGER_H_
#define __RS_SYNC_MANAGER_H_

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <filesystem>
#include <mutex>

#include "net/net_addr.h"
#include "constants/constants.h"
#include "modules/rs_local_manager.h"
#include "modules/block.h"

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
    bool success = false;

    SyncRs();
    SyncRs(std::string uri, std::uint64_t size, std::string hash, NetAddr owner);
    ~SyncRs(){};
};

class RsSyncManager
{
private:
    std::mutex mut;
    std::map<std::string, SyncRs> uriRs;
    RsLocalManager &rlm;

public:
    RsSyncManager(RsLocalManager &rm) : rlm(rm){};
    ~RsSyncManager(){};

    std::map<std::string, SyncRs> &getAllUriRs();
    bool isSyncSuccess(std::string);

    void refreshSyncingRsByTbIdx(NetAddr peer, struct Resource *table, uint64_t rs_size);
    bool regSyncRs(NetAddr peer, std::string uri, std::string hash, uint64_t size);
    bool updateSyncRs(NetAddr peer, std::string uri, std::string hash, uint64_t size);
    Block regReqSyncRsAuto(NetAddr peer, std::string uri);
    void unregReqSyncRsByBlock(NetAddr peer, Block b, std::string uri);
    void unregAllReqSyncRsByPeer(NetAddr peer, std::string uri);
    // vector<uint64_t, uint64_t> regReqSyncRsCplt(NetAddr peer, std::string uri) { return {}; };
    // vector<uint64_t, uint64_t> regReqSyncRsRang(NetAddr peer, std::string uri, uint64_t offset, uint64_t size) { return {}; };
    void syncingRangeDone(NetAddr peer, string uri, Block block);
    void syncingRangeDoneAndValid(NetAddr peer, string uri, Block block,bool valid);
    uint getOwnerSize(string uri);
    uint getBlockSize(string uri);
    uint getSyncingSize(string uri);
};

#endif
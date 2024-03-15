#ifndef __RS_LOCAL_MANAGER_H_
#define __RS_LOCAL_MANAGER_H_

#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>

#include "vo/dto/resource.h"
#include "utils/openssl_utils.h"
#include "utils/logger.h"
#include "vo/want_to_sync_vo.h"
#include "utils/io_utils.h"
#include "net/net_addr.h"

class RsLocalManager
{
private:
    IoUtil io;
    map<std::string, struct Resource *> table;
    std::string rsHome;
    filesystem::file_time_type last_update_time;
    std::vector<struct Resource *> recurPath(filesystem::path p);
    void freeTable();

public:
    RsLocalManager(std::string path);
    ~RsLocalManager();

    std::vector<struct Resource *> getTable();

    const struct Resource *queryByUri(std::string uri);
    std::string getRsHome();

    void refreshTable();

    void setRsHomePath(std::string path);

    bool validRes(std::string uri, std::string hash);

    std::vector<struct Resource *> genResources(std::string p);
    std::vector<struct Resource> cmpThenRetNeedToSyncTable(std::vector<struct Resource> peer_table);
    std::vector<struct Resource> cmpThenRetNeedToSyncTable(struct Resource *table, uint64_t rs_size);
    std::string resourcePosition(std::string uri);
    bool saveLocal(std::string uri, void *data, uint64_t offset, uint64_t data_len);
};

#endif
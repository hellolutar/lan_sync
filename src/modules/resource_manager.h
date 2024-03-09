#ifndef __RESOURCE_MANAGER_H
#define __RESOURCE_MANAGER_H

#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>

#include "vo/dto/resource.h"
#include "utils/openssl_utils.h"
#include "utils/logger.h"
#include "vo/want_to_sync_vo.h"
#include "utils/io_utils.h"

class ResourceManager
{
private:
    IoUtil io;
    map<std::string, struct Resource *> table;
    std::vector<struct Resource *> recurPath(filesystem::path p);
    std::vector<struct Resource *> genResources(std::string p);
    void freeTable();
    std::string rsHome;

    std::map<std::string, WantToSyncVO> syncTable;   // TODO need to care about <connection, rs>

    OpensslUtil opensslUtil;

    filesystem::file_time_type last_update_time;

public:
    ResourceManager(std::string path);
    ~ResourceManager();

    std::vector<struct Resource *> getTable();

    const struct Resource *queryByUri(std::string uri);
    std::string getRsHome();

    void refreshTable();

    void setRsHomePath(std::string path);

    bool validRes(std::string uri, std::string hash);

    std::vector<struct Resource> queryNeedToSync(std::vector<struct Resource> peer_table);
    std::vector<struct Resource> queryNeedToSync(struct Resource *table, uint64_t rs_size);
    void analysisThenUpdateSyncTable(struct Resource *table, uint64_t rs_size);
    std::map<string, WantToSyncVO> &getSyncTable();
    std::string resourcePosition(std::string uri);
    bool saveLocal(std::string uri, void *data, uint64_t offset, uint64_t data_len);

    void updateSyncEntryStatus(std::string uri, WantSyncResourceStatusEnum status);
    void updateSyncEntryLastUpteTime(std::string uri, time_t t);
    void delSyncEntry(std::string uri);

    static ResourceManager& getRsm();
    static void init(string home);

};

#endif
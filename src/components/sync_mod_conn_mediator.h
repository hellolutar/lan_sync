#ifndef __Sync_Mod_Conn_Mediator_h_
#define __Sync_Mod_Conn_Mediator_h_

#include <map>
#include <string>

#include "modules/conn/mod_conn.h"
#include "utils/logger.h"

class SyncModConnMediator : public AbsModConnMediator
{
private:
    std::map<std::string, ModConnAbility *> mods;

public:
    SyncModConnMediator(/* args */){};
    ~SyncModConnMediator(){};
    bool mod_tel(std::string from, std::string to, std::string uri, void *data) override;
    bool add(ModConnAbility *);
    void del(std::string name);
};

#endif
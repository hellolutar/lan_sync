#include "sync_mod_conn_mediator.h"

using namespace std;

bool SyncModConnMediator::mod_tel(std::string from, std::string to, std::string uri, void *data)
{
    if (mods.find(to) == mods.end())
    {
        LOG_WARN("SyncModConnMediator::mod_tel() : to[{}] is not found!", to);
        return false;
    }

    if (mods.find(from) == mods.end())
    {
        LOG_WARN("SyncModConnMediator::mod_tel() : from[{}] is not found!", from);
        return false;
    }
    mods[to]->mod_conn_recv(from, uri, data);
    return true;
}

bool SyncModConnMediator::add(ModConnAbility *mod)
{
    if (mods.find(mod->name) != mods.end())
    {
        return false;
    }

    mods[mod->name] = mod;
    return true;
}

void SyncModConnMediator::del(std::string name)
{
    if (mods.find(name) == mods.end())
    {
        return;
    }

    for (auto iter = mods.begin(); iter != mods.end(); iter++)
    {
        string mod_name = (*iter).second->name;
        if (mod_name == name)
        {
            mods.erase(iter);
            return;
        }
    }
}
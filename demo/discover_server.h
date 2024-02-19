#include <fcntl.h>

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <string>
#include <vector>

#include <unistd.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

// #include <spdlog/spdlog.h>

#include "lan_share_protocol.h"
#include "resource_manager.h"

using namespace std;

class DiscoverServer
{
private:
    vector<local_inf_info> inf_infos;
    
public:
    enum state st;
    struct event_base *base;
    DiscoverServer(struct event_base *base);
    ~DiscoverServer();
    void start();
    ResourceManager rm = ResourceManager("resources/server");
};

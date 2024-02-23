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

#include "comm/lan_share_protocol.h"
#include "resource/resource_manager.h"
#include "utils/logger.h"
#include "constants.h"

using namespace std;

class DiscoverServer
{
private:
    
public:
    enum state st;
    struct event_base *base;
    DiscoverServer(struct event_base *base);
    ~DiscoverServer();
    void start();
    ResourceManager rm = ResourceManager("static/server");
};

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <string>
#include <vector>

#include <unistd.h>

#include "sdn_share_protocol.h"

using namespace std;

class DiscoverServer
{
private:
    enum state st;
    vector<local_inf_info> inf_infos;

public:
    DiscoverServer(/* args */);
    ~DiscoverServer();
    void start();
};

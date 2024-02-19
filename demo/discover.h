#ifndef __DISCOVER_H
#define __DISCOVER_H

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <stdlib.h>

#include <string>
#include <vector>
#include <map>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <event2/event.h>

#include "lan_share_protocol.h"
#include "resource_manager.h"

using namespace std;

class Discover
{
private:
public:
    enum state st;
    struct event_base *base;
    struct local_inf_info *inf_infos;
    uint8_t inf_infos_len = 0;
    struct sockaddr_in *target_sock_addrs;
    int udp_sock;
    ResourceManager rm = ResourceManager("resources/cli");

    Discover(struct event_base *base);
    ~Discover();
    void start();
    void do_connect(evutil_socket_t sock, short event, void *arg);
};

#endif
#ifndef __DISCOVER_H
#define __DISCOVER_H

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <stdlib.h>

#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <event2/event.h>

#include "sdn_share_protocol.h"

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
    int sock;
    Discover(struct event_base *base);
    ~Discover();
    void start();
    void do_connect(evutil_socket_t sock, short event, void *arg);
    bool handle_hello_recv(struct bufferevent *bev);
};

#endif
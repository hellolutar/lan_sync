#ifndef __DISCOVER_H
#define __DISCOVER_H

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cerrno>

#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include "sdn_share_protocol.h"

using namespace std;

class Discover
{
private:
    enum state st;
    bool handle_hello_recv(int sock, const sockaddr_in & target_sock_addr);

public:
    Discover(/* args */);
    ~Discover();
    void start();
};

#endif
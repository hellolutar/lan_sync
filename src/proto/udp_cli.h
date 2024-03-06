#ifndef __UDP_CLI_H_
#define __UDP_CLI_H_

#include "lan_share_protocol.h"
#include <event2/event.h>

class udp_cli
{
private:
    int udp_sock;
    struct event_base *base;
    struct sockaddr_in t_addr;
    int be_port;

public:
    udp_cli(int udp_sock,
            struct event_base *base,
            struct sockaddr_in t_addr) : udp_sock(udp_sock),
                                         base(base),
                                         t_addr(t_addr){};
    ~udp_cli();

    void send(struct evbuffer *inbuf);
};

#endif
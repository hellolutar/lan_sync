#ifndef __UDP_CLI_H_
#define __UDP_CLI_H_

#include "lan_share_protocol.h"
#include <event2/event.h>

class udp_cli
{
private:
    int udp_sock;
    struct event_base *base;
    struct in_addr t_sin_addr;

    void configure_sock(struct sockaddr_in *t_sock_addr);

public:
    udp_cli(int udp_sock,
            struct event_base *base,
            struct in_addr t_sin_addr) : udp_sock(udp_sock),
                                         base(base),
                                         t_sin_addr(t_sin_addr){};
    ~udp_cli();

    void send(struct evbuffer *inbuf);
};

#endif
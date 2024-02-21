#include "udp_cli.h"

udp_cli::~udp_cli()
{
}

void udp_cli::configure_sock(struct sockaddr_in *t_sock_addr)
{
    t_sock_addr->sin_family = AF_INET;
    t_sock_addr->sin_addr = t_sin_addr;
    t_sock_addr->sin_port = htons(DISCOVER_SERVER_UDP_PORT);
}

void udp_cli::send(struct evbuffer *inbuf)
{
    struct cb_arg *arg = cb_arg_new(base); // free in lan_share_protocol#writecb
    configure_sock(arg->target_addr);

    evbuffer_add_buffer(arg->buf, inbuf);
    evbuffer_free(inbuf);

    struct event *write_e = event_new(base, udp_sock, EV_WRITE, writecb, arg);
    event_add(write_e, nullptr);
}



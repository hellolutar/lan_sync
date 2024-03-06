#include "udp_cli.h"

udp_cli::~udp_cli()
{
}

void udp_cli::send(struct evbuffer *inbuf)
{
    struct cb_arg *arg = cb_arg_new(base); // free in lan_share_protocol#writecb
    arg->target_addr->sin_family = t_addr.sin_family;
    arg->target_addr->sin_addr = t_addr.sin_addr;
    arg->target_addr->sin_port = t_addr.sin_port;

    evbuffer_add_buffer(arg->buf, inbuf);
    evbuffer_free(inbuf);

    struct event *write_e = event_new(base, udp_sock, EV_WRITE, writecb, arg);
    event_add(write_e, nullptr);
}



#include "sdn_share_protocol.h"

void *encapsulate(lan_discover_header_t header, void *data, int data_len)
{
    int total_len = sizeof(header) + data_len;
    lan_discover_header_t *ret_header = (lan_discover_header_t *)malloc(total_len);
    memcpy(ret_header, &header, sizeof(header));
    memcpy(ret_header + 1, data, data_len);
    return ret_header;
}

void debug_print_header(struct lan_discover_header *header, struct in_addr addr)
{
    char data[4096] = {0};
    memcpy(data, header + 1, header->data_len);
    data[header->data_len] = '\0';
    printf("[DEBUG] recive msg from :%s \n"
           "\t version: %d \n"
           "\t type: %d \n"
           "\t len: %d \n",
           inet_ntoa(addr),
           header->version,
           header->type,
           header->data_len);
}

struct cb_arg *cb_arg_new(struct event_base *base)
{
    assert(base);
    struct cb_arg *arg = (struct cb_arg *)malloc(sizeof(struct cb_arg));
    struct evbuffer *buf = evbuffer_new();
    struct sockaddr_in *target_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    arg->buf = buf;
    arg->target_addr = target_addr;
    arg->base = base;
    return arg;
}

void cb_arg_free(struct cb_arg *arg)
{
    free(arg->target_addr);
    free(arg->buf);
    free(arg);
}

void writecb(evutil_socket_t fd, short events, void *ctx)
{
    struct cb_arg *arg = (struct cb_arg *)ctx;
    int datalen = evbuffer_get_length(arg->buf);
    char data[datalen];
    assert(evbuffer_copyout(arg->buf, data, datalen) == datalen);
    data[datalen] = '\0';
    int sent = sendto(fd, data, datalen, 0, (struct sockaddr *)arg->target_addr, sizeof(struct sockaddr_in));
    printf("sent to [%s:%d]: %d , data_len:%d \n", inet_ntoa(arg->target_addr->sin_addr), ntohs(arg->target_addr->sin_port), sent, datalen);

    cb_arg_free(arg);
}
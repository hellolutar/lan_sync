#include "lan_share_protocol.h"

void *lan_discover_encapsulate(lan_discover_header_t header, void *data, int data_len)
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

void evbuffer_cb_for_free(struct evbuffer *buffer, const struct evbuffer_cb_info *info, void *arg)
{
    if (evbuffer_get_length(buffer) == 0)
    {
        if (arg)
        {
            printf("[DEBUG] evbuffer is clear! \n");
            printf("[DEBUG] free header! \n");
            evbuffer_remove_cb(buffer, evbuffer_cb_for_free, arg);
            free(arg);
            arg = nullptr;
        }
    }
}

void lan_sync_encapsulate(struct evbuffer *out, lan_sync_header_t header, void *data, int data_len)
{
    int total_len = sizeof(header) + data_len;
    lan_sync_header_t *ret_header = (lan_sync_header_t *)malloc(total_len);

    memcpy(ret_header, &header, sizeof(header));
    if (data_len > 0)
    {
        memcpy(ret_header + 1, data, data_len);
    }
    evbuffer_add(out, (void *)ret_header, total_len);

    // 这里有些疑问，多次add_cb, 不知道会发生什么
    printf("[DEBUG] lan_sync_encapsulate");
    evbuffer_add_cb(out, evbuffer_cb_for_free, ret_header);
}

struct Resource *lan_sync_parseTableToData(vector<struct Resource *> table)
{
    int num = table.size();
    int size = num * sizeof(struct Resource);
    struct Resource *ret = (struct Resource *)malloc(size);
    struct Resource *retp = ret;
    memset(ret, 0, size);
    for (int i = 0; i < table.size(); i++)
    {
        memcpy(&(retp[i].name), table[i]->name, strlen(table[i]->name));
        memcpy(&(retp[i].uri), table[i]->uri, strlen(table[i]->uri));
        memcpy(&(retp[i].path), table[i]->path, strlen(table[i]->path));
        memcpy(&(retp[i].hash), table[i]->hash, strlen(table[i]->hash));
        retp[i].size = table[i]->size;
    }

    return ret;
}

void lan_sync_parseStructToMem()
{
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
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
            printf("[DEBUG] free header! \n");
            evbuffer_remove_cb(buffer, evbuffer_cb_for_free, arg);
            free(arg);
            arg = nullptr;
        }
    }
}

void lan_sync_encapsulate(struct evbuffer *out, lan_sync_header_t *header)
{
    evbuffer_add(out, header, header->total_len);
    // 这里有些疑问，多次add_cb, 不知道会发生什么
    evbuffer_add_cb(out, evbuffer_cb_for_free, header);
    printf("[DEBUG] [TCP] send pkt, header info: total_len:%ld, header_len:%d \n", header->total_len, header->header_len);
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

lan_sync_header_t *lan_sync_header_set_data(lan_sync_header_t *header, void *data, int datalen)
{
    int ori_total_len = header->total_len;

    int total_len = header->header_len + datalen;
    lan_sync_header_t *newHeader = (lan_sync_header_t *)malloc(total_len);
    memcpy(newHeader, header, header->header_len);
    newHeader->total_len = total_len;

    char *data_point = (char *)((char *)newHeader + header->header_len);

    memcpy(data_point, data, datalen);

    free(header);
    // memset(header, 0, ori_total_len);
    // header = nullptr;

    return newHeader;
}

lan_sync_header_t *lan_sync_header_add_xheader(lan_sync_header_t *header, const string key, const char *value)
{
    int ori_total_len = header->total_len;
    int ori_header_len = header->header_len;
    int ori_xheader_len = header->header_len - lan_sync_header_len;
    int ori_data_len = header->total_len - header->header_len;

    char *ori_data_point = (char *)((char *)header + ori_header_len);

    int append_x_header_len = key.size() + strlen(value) + FLAG_KEY_VALUE_SPLIT;

    // update header 信息
    header->total_len = ori_total_len + append_x_header_len;
    header->header_len = ori_header_len + append_x_header_len;

    lan_sync_header_t *newHeader = (lan_sync_header_t *)malloc(header->total_len);

    // 先拷贝header
    memcpy(newHeader, header, ori_header_len);

    // 然后添加新xheader
    char *append_xheader = (char *)malloc(append_x_header_len);
    sprintf(append_xheader, "%s:%s\0", key.data(), value);

    char *append_xheader_point = (char *)((char *)newHeader + ori_header_len);
    memcpy(append_xheader_point, append_xheader, append_x_header_len);
    free(append_xheader);

    // 最后拷贝data
    char *data = (char *)(append_xheader_point + append_x_header_len);
    memcpy(data, ori_data_point, ori_data_len);

    // 释放原有的内存，并更新指针
    free(header);
    // memset(header, 0, ori_total_len);
    // header = nullptr;

    return newHeader;
}

void lan_sync_header_extract_xheader(const lan_sync_header_t *header, char *to)
{
    int total_len = header->total_len;
    int data_len = header->total_len - header->header_len;
    int xheader_len = header->header_len - lan_sync_header_len;

    char *xheader = (char *)(header + 1);

    // 根据\0分隔符号，字符串
    memcpy(to, xheader, xheader_len);
}

string lan_sync_header_query_xheader(const lan_sync_header_t *header, string keystr)
{
    int xheader_len = header->header_len - lan_sync_header_len;
    char *xhd = (char *)malloc(xheader_len);

    lan_sync_header_extract_xheader(header, xhd);

    int start = 0;
    for (int i = 0; i < xheader_len; i++)
    {
        char ch = xhd[i];
        if (ch == 0)
        {
            // 这里需要截取字符串
            char kv[1024] = {0};
            memcpy(kv, xhd + start, (i - start + 1));
            start = i + 1;

            string kvstr(kv);
            int index = kvstr.find(keystr);
            if (index >= 0)
            {
                free(xhd);
                return kvstr.substr(keystr.size() + 1, kvstr.size() - keystr.size());
            }
        }
    }
    free(xhd);

    return "";
}

void lan_sync_header_extract_data(const lan_sync_header_t *header, char *to)
{
    int total_len = header->total_len;
    int data_len = header->total_len - header->header_len;
    int xheader_len = header->header_len - lan_sync_header_len;

    char *hdp = (char *)header;
    char *data = (char *)(hdp + header->header_len);
    memcpy(to, data, data_len);
}

lan_sync_header_t *lan_sync_header_new(enum lan_sync_version version, enum lan_sync_type_enum type)
{
    lan_sync_header_t *header = (lan_sync_header_t *)malloc(lan_sync_header_len);
    header->version = version;
    header->type = type;
    header->total_len = lan_sync_header_len;
    header->header_len = lan_sync_header_len;

    return header;
}
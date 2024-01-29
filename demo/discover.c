#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "sdn_share_protocol.h"

struct local_sock_info
{
    char name[IFNAMSIZ];
    struct sockaddr_in addr;
    struct sockaddr_in broad_addr;
    struct sockaddr_in subnet_mask;
};

int getLoclInfo(struct local_sock_info *infos)
{
    int fd;
    assert((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0);

    struct ifreq ifreqs[10];

    struct ifconf ifc;
    ifc.ifc_len = sizeof(ifreqs);
    ifc.ifc_buf = (caddr_t)ifreqs;

    assert(ioctl(fd, SIOCGIFCONF, (char *)&ifc) >= 0);
    int interface_num = ifc.ifc_len / sizeof(struct ifreq);

    for (size_t i = 0; i < interface_num; i++)
    {
        memcpy(infos[i].name, ifreqs[i].ifr_name, strlen(ifreqs[i].ifr_name));

        // ip
        assert(ioctl(fd, SIOCGIFADDR, &ifreqs[i]) >= 0); // 获取ip，并将其存储到ifreq中
        infos[i].addr = *(struct sockaddr_in *)&(ifreqs[i].ifr_addr);

        // // broad address of this interface
        assert(ioctl(fd, SIOCGIFBRDADDR, &ifreqs[i]) >= 0); // 获取广播地址，并将其存储到ifreq中
        infos[i].broad_addr = *(struct sockaddr_in *)&(ifreqs[i].ifr_broadaddr);

        // // subnet mask
        assert(ioctl(fd, SIOCGIFNETMASK, &ifreqs[i]) >= 0);
        infos[i].subnet_mask = *(struct sockaddr_in *)&(ifreqs[i].ifr_netmask);
    }

    return interface_num;
}

void *encapsulate(sdn_share_proto_header_t header, void *data, int data_len)
{
    int total_len = sizeof(header) + data_len;
    sdn_share_proto_header_t *ret_header = malloc(total_len);
    memcpy(ret_header, &header, sizeof(header));
    memcpy(ret_header + 1, data, data_len);
    return ret_header;
}

int main(int argc, char const *argv[])
{
    struct local_sock_info infos[10];
    memset(infos, 0, sizeof(struct local_sock_info) * 10);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock > 0);

    int optval = 1; // 这个值一定要设置，否则可能导致sendto()失败
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));

    for (size_t i = 0; i < getLoclInfo(infos); i++)
    {

        struct local_sock_info info = infos[i];

        if (info.broad_addr.sin_addr.s_addr == 0)
            continue;

        printf("want to send udp to %s \n", inet_ntoa(info.broad_addr.sin_addr));
        struct sockaddr_in target_sock_addr;
        target_sock_addr.sin_family = AF_INET;
        target_sock_addr.sin_addr = info.broad_addr.sin_addr;
        target_sock_addr.sin_port = htons(8080);

        char *data = "hello world";
        sdn_share_proto_header_t tmp_header = {v0_1, HELLO, strlen(data)};
        sdn_share_proto_header_t *header = (sdn_share_proto_header_t *)encapsulate(tmp_header, data, strlen(data));

        int ret = sendto(sock, header, sizeof(header) + strlen(data), 0, (struct sockaddr *)&target_sock_addr, sizeof(target_sock_addr));
        free(header);
        if (ret < 0)
        {
            printf("error: %s \n", strerror(ret));
            // return -1;
        }

        printf("udp send result :%d \n", ret);
    }

    return 0;
}

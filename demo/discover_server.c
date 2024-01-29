#include <stdio.h>
#include <assert.h>
#include <error.h>
#include <unistd.h>

#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "sdn_share_protocol.h"

int main(int argc, char const *argv[])
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(fd > 0);

    struct sockaddr_in addr;
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(fd, 10);

    while (1)
    {
        struct sockaddr_in cli_addr;
        char buf[4096] = {0};
        int addr_len = sizeof(cli_addr);
        int len = recvfrom(fd, buf, 4096, 0, (struct sockaddr *)&cli_addr, &addr_len);

        if (len > 0)
        {
            struct sdn_share_proto_header *header = (sdn_share_proto_header_t *)buf;
            printf("recive msg  from :%s \n", inet_ntoa(cli_addr.sin_addr));
            printf("version: %d \n", header->version);
            printf("type: %d \n", header->type);
            printf("len: %d \n", header->data_len);

            char data[4096] = {0};
            memcpy(data, header + 1, header->data_len);
            data[header->data_len] = '\0';
            printf("data: %s\n", data);

            printf("%s \n", buf);
        }
    }

    return 0;
}

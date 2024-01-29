#ifndef __SDN_SHARE_PROTOCOL_H
#define __SDN_SHARE_PROTOCOL_H

#include <stdint.h>


enum sdn_proto_version
{
    v0_1 = 1,
};

enum sdn_proto_type_enum
{
    HELLO = 1,
    HELLO_REPLY = 2,
    SYNC_INDEX = 3,
};

typedef struct sdn_share_proto_header
{
    uint8_t version;
    uint8_t type;
    uint16_t data_len; // data lenth
} sdn_share_proto_header_t;

#endif
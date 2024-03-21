#ifndef __CONSTANTS_H_
#define __CONSTANTS_H_

#include <cstdint>

#define SERVICE_NAME_DISCOVER_HELLO "discover_hello"

#define SERVICE_NAME_REQ_TABLE_INDEX "req_table_index"
#define SERVICE_NAME_REQ_RESOURCE "req_resource"
#define SERVICE_NAME_REPLY_TABLE_INDEX "reply_table_index"
#define SERVICE_NAME_REPLY_REQ_RESOURCE "reply_req_resource"

#define EVENT_KEY_CONFIG_REQ_RESOURCE_PERIODICALLY "config_req_resource_periodically"

#define CONFIG_KEY_DISCOVER_IPS "ip.discovery"
#define CONFIG_KEY_PROTO_DISCOVER_SERVER_UDP_PORT "proto.udp.server.port"
#define CONFIG_KEY_PROTO_SYNC_SERVER_TCP_PORT "proto.tcp.server.port"

#define CONFIG_KEY_LOCAL_DISCOVER_SERVER_UDP_PORT "local.udp.server.port"
#define CONFIG_KEY_LOCAL_SYNC_SERVER_TCP_PORT "local.tcp.server.port"

#define CONFIG_KEY_RESOURCE_HOME "resource.home"
#define CONFIG_DEFAULT_NAME "properties.properties"

#define SIZE_1KByte 1024
#define SIZE_50_KByte 50 * SIZE_1KByte
#define SIZE_1MByte 1024 * SIZE_1KByte

#define ONCE_MAX_READ_SIZE SIZE_1MByte * 2
#define BLOCK_SIZE SIZE_1MByte // TODO(LUTAR, 123): support server reply block < sync_table block

#define DOWNLOAD_LIMIT 5

#endif
#ifndef __CONSTANTS_H_
#define __CONSTANTS_H_

#include <cstdint>

#define EVENT_KEY_CONFIG_REQ_RESOURCE_PERIODICALLY "config_req_resource_periodically"

#define CONFIG_KEY_DISCOVER_IPS "ip.discovery"
#define CONFIG_KEY_PROTO_DISCOVER_SERVER_UDP_PORT "proto.udp.server.port"
#define CONFIG_KEY_PROTO_SYNC_SERVER_TCP_PORT "proto.tcp.server.port"

#define CONFIG_KEY_LOCAL_DISCOVER_SERVER_UDP_PORT "local.udp.server.port"
#define CONFIG_KEY_LOCAL_SYNC_SERVER_TCP_PORT "local.tcp.server.port"

#define CONFIG_KEY_RESOURCE_HOME "resource.home"
#define CONFIG_DEFAULT_NAME "properties.properties"
#define CONFIG_KEY_LOG_LEVEL "log.level"

#define SIZE_1KByte 1024
#define SIZE_50_KByte 50 * SIZE_1KByte
#define SIZE_1MByte 1024 * SIZE_1KByte

#define ONCE_MAX_READ_SIZE SIZE_1MByte * 2
#define BLOCK_SIZE SIZE_1MByte // TODO(LUTAR, 123): support server reply block < sync_table block

#define DOWNLOAD_LIMIT 5

#define MODULE_NAME_SYNC_SRV "mod_sync_srv"
#define MODULE_NAME_SYNC_CLI "mod_sync_cli"
#define MODULE_NAME_SYNC_SERVICE "mod_syn_service"
#define MODULE_NAME_DISCOVERY "mod_discovery"
#define MODULE_NAME_PERIOD_REQ_TB_IDX "mod_req_table_idx"

#define MODULE_CONN_URI_DISCOVER_ADD "/discover/add"
#define MODULE_CONN_URI_DISCOVER_DEL "/discover/del"
#define MODULE_CONN_URI_PERIOD_REQ_TB_IDX_ADD "/req_tb_idx/add"
#define MODULE_CONN_URI_PERIOD_REQ_TB_IDX_DEL "/req_tb_idx/del"

#endif
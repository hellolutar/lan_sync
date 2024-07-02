#ifndef __CONSTANTS_H_
#define __CONSTANTS_H_

#include <cstdint>
#include <string>

const std::string EVENT_KEY_CONFIG_REQ_RESOURCE_PERIODICALLY = "config_req_resource_periodically";

const std::string CONFIG_KEY_DISCOVER_IPS = "ip.discovery";
const std::string CONFIG_KEY_PROTO_DISCOVER_SERVER_UDP_PORT = "proto.udp.server.port";
const std::string CONFIG_KEY_PROTO_SYNC_SERVER_TCP_PORT = "proto.tcp.server.port";

const std::string CONFIG_KEY_RESOURCE_HOME = "resource.home";
const std::string CONFIG_DEFAULT_NAME = "properties.properties";
const std::string CONFIG_KEY_LOG_LEVEL = "log.level";

const std::uint64_t SIZE_1KByte = 1024;
const std::uint64_t SIZE_50_KByte = 50 * SIZE_1KByte;
const std::uint64_t SIZE_1MByte = 1024 * SIZE_1KByte;

const std::uint64_t ONCE_MAX_READ_SIZE = SIZE_1MByte * 2;
const std::uint64_t BLOCK_SIZE = SIZE_1MByte; // TODO(LUTAR, 123): support server reply block < sync_table block

const std::uint16_t DOWNLOAD_LIMIT = 5;
const std::uint8_t MAX_RETRY = 5;

const std::string MODULE_NAME_SYNC_SRV = "mod_sync_srv";
const std::string MODULE_NAME_SYNC_CLI = "mod_sync_cli";
const std::string MODULE_NAME_SYNC_SERVICE = "mod_syn_service";
const std::string MODULE_NAME_DISCOVERY = "mod_discovery";
const std::string MODULE_NAME_PERIOD_REQ_TB_IDX = "mod_req_table_idx";
const std::string MODULE_NAME_DISCOVER_LOGIC = "mod_discovery_logic";
const std::string MODULE_NAME_REQ_TB_IDX_LOGIC = "mod_req_table_idx_logic";
const std::string MODULE_NAME_TASK_COORDINATOR_TRIGGER_MODCONN = "mod_task_coordinator_trigger_modconn";

namespace module_conn_uri
{
    const std::string DISCOVER_ADD = "/discover/add";
    const std::string DISCOVER_DEL = "/discover/del";
    const std::string PERIOD_REQ_TB_IDX_ADD = "/req_tb_idx/add";
    const std::string PERIOD_REQ_TB_IDX_DEL = "/req_tb_idx/del";

    const std::string TASK_COORDINATOR_TRIGGER_MODCONN_ADD_RESOURCE = "task_trigger/resource/add";
    const std::string TASK_COORDINATOR_TRIGGER_MODCONN_STOP = "task_trigger/stop";
}

#endif
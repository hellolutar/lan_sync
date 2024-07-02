#include "task_coordinator_trigger_modconn.h"

using namespace module_conn_uri;

void TaskCoordinatorTriggerModconn::mod_conn_recv(std::string from, std::string uri, void *data)
{
    if (uri == TASK_COORDINATOR_TRIGGER_MODCONN_ADD_RESOURCE)
    {
        TaskCoordinator &coor = trigger_.getTaskCoordinator();

        ModconnTaskDto *dto = static_cast<ModconnTaskDto *>(data);
        coor.add_resource(dto->uri, dto->range, dto->ctx);
    }
    else if (uri == TASK_COORDINATOR_TRIGGER_MODCONN_STOP)
    {
        // todo
    }
}
#ifndef __SYNC_TASK_COORDINATOR_TRIGGER_MODCONN_H_
#define __SYNC_TASK_COORDINATOR_TRIGGER_MODCONN_H_

#include "modules/conn/mod_conn.h"
#include "modules/task2/task_coordinator_trigger.h"
#include "constants/constants.h"
#include "vo/dto/mod_conn_task_dto.h"

class TaskCoordinatorTriggerModconn : public ModConnAbility
{
private:
    TaskCoordinatorTrigger &trigger_;

public:
    TaskCoordinatorTriggerModconn(AbsModConnMediator *med, TaskCoordinatorTrigger &trg)
        : trigger_(trg), ModConnAbility(med, MODULE_NAME_TASK_COORDINATOR_TRIGGER_MODCONN){};
    ~TaskCoordinatorTriggerModconn() {}

    void mod_conn_recv(std::string from, std::string uri, void *data) override;
};

#endif
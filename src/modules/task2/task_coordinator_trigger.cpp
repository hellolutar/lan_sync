#include "modules/task2/task_coordinator_trigger.h"

#include <chrono>
#include "task_coordinator_trigger.h"

using namespace std;

void TaskCoordinatorTrigger::trigger()
{
    auto now = chrono::system_clock::now();
    int64_t duration = chrono::duration_cast<chrono::milliseconds>((now - last_)).count();
    last_ = now;
    coor_.tick(duration);
}
TaskCoordinator &TaskCoordinatorTrigger::getTaskCoordinator()
{
    return coor_;
}
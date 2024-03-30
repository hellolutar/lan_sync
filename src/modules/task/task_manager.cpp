#include "task_manager.h"

using namespace std;

TaskManager::TaskManager()
{
    thread t(&TaskManager::start, this);
    t.detach();
}
TaskManager::~TaskManager()
{
    for (auto iter = tasks.end() - 1; iter >= tasks.begin(); iter--)
    {
        delete *iter;
    }
    cout << "TaskManager::~TaskManager()" << endl;
}

TaskManager *TaskManager::tm;

TaskManager *TaskManager::getTaskManager()
{
    if (tm == nullptr)
    {
        tm = new TaskManager();
    }
    return tm;
}

void TaskManager::start()
{
    while (!stopFlg)
    {
        unique_lock<std::mutex> lk(mut);
        cond.wait(lk, [this]
                  { return tasks.size() > 0; });
        auto tsk = tasks.front();
        tasks.erase(tasks.begin());
        lk.unlock();

        tsk->run();
        delete tsk;
    }
    cout << "TaskManager exit while" << endl;
    delete this;
}

bool TaskManager::addTask(AbsTask *t)
{
    scoped_lock<std::mutex> lk(mut);
    for (auto &tsk : tasks)
    {
        if (tsk->getId() == t->getId())
            return false;
    }
    tasks.push_back(t);
    cond.notify_one();
    return true;
}

void TaskManager::rmTask(AbsTask *t)
{
    scoped_lock<std::mutex> lk(mut);
    for (auto iter = tasks.begin(); iter != tasks.end(); iter++)
    {
        if (t->getId() == t->getId())
            tasks.erase(iter);
    }
}

void TaskManager::stop()
{
    stopFlg = true;
    cond.notify_all();
}
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
}



// int main(int argc, char const *argv[])
// {
//     TaskManager *tm = new TaskManager();
//     for (size_t i = 0; i < 15; i++)
//     {
//         tm->addTask(new ConcreteTask(to_string(i)));
//     }
//     thread t(&TaskManager::start, tm);
//     t.detach();

//     this_thread::sleep_for(chrono::seconds(2));
//     tm->stop();
//     this_thread::sleep_for(chrono::seconds(15));
//     cout << "main exit" << endl;

//     return 0;
// }

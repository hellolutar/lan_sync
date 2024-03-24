#ifndef __TASK_MANAGER_H_
#define __TASK_MANAGER_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <cstdio>
#include <map>
#include <chrono>
#include <iostream>

#include "abs_task.h"

class TaskManager
{
protected:
    bool stopFlg = false;
    std::vector<AbsTask *> tasks;
    std::mutex mut;
    std::condition_variable cond;
    TaskManager();
    virtual ~TaskManager();
    virtual void start();

    static TaskManager *tm;

public:
    virtual bool addTask(AbsTask *);
    virtual void rmTask(AbsTask *);
    virtual void stop();

    static TaskManager *getTaskManager();
};

#endif
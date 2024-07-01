#ifndef __TEST_TASK_MANAGER_H_
#define __TEST_TASK_MANAGER_H_

#include <map>

#include "modules/task2/task.h"

using namespace std;

class TaskManagerForTest : public TaskManager2
{
public:
    TaskManagerForTest(/* args */) {};
    ~TaskManagerForTest() {};

    const uint64_t tasks_ctx_num(string uri) const
    {
        map<std::shared_ptr<NetworkConnCtx>, int> ctx_map;
        auto uri_iter = tasks_.find(uri);
        if (uri_iter == tasks_.end())
        {
            return 0;
        }

        auto ts = uri_iter->second;

        for (size_t i = 0; i < ts.size(); i++)
        {
            auto t = ts[i];
            ctx_map[t.getCtx()] = 1;
        }

        return ctx_map.size();
    }

    const std::vector<Task> tasks(std::string uri) const
    {
        map<std::shared_ptr<NetworkConnCtx>, int> ctx_map;
        auto uri_iter = tasks_.find(uri);
        if (uri_iter == tasks_.end())
            return {};

        return uri_iter->second;
    }
};

#endif
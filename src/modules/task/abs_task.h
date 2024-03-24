#ifndef __ABS_TASK_H_
#define __ABS_TASK_H_

#include <string>

class AbsTask
{
protected:
    std::string id;

public:
    AbsTask(std::string id) : id(id){};
    virtual ~AbsTask(){};
    virtual void run() = 0;
    virtual std::string getId();
};

#endif
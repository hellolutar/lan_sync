#ifndef __TASK_H_
#define __TASK_H_

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

#include "net/network_layer.h"
#include "modules/task2/range.h"
#include "modules/task2/common.h"

class Block
{
public:
    uint64_t start;
    uint64_t end;
    Block(){};
    Block(uint64_t start, uint64_t end) : start(start), end(end){};
    ~Block(){};

    const bool operator==(const Block &other);

    const bool operator==(const Block &other) const;

    static const uint64_t pos(uint64_t offset);
    static const uint64_t bitPos(uint64_t pos);
};

enum class TaskStatusEnum
{
    Pendding,
    Syning,
    Stop,
    Success,
    Failed
};

class Task
{
private:
    Block block_;
    std::shared_ptr<NetworkConnCtx> ctx_;
    std::string uri_;
    uint8_t retry_ = 0;
    uint64_t passed_time_ = 0;
    TaskStatusEnum status_ = TaskStatusEnum::Pendding;

    const uint8_t *prepareHeader() const;

public:
    Task(std::string uri, Block blk, std::shared_ptr<NetworkConnCtx> ctx) : uri_(uri), block_(blk), ctx_(ctx){};
    ~Task();

    void req();
    void tick(uint64_t t);
    bool isFailed();
    void stop();
    TaskStatusEnum getStatus() const;
    void setStatus(TaskStatusEnum st);

    const std::string getUri() const;
    const Block getBlock() const;

    const std::shared_ptr<NetworkConnCtx> getCtx() const;
};

class TaskManager2
{
protected:
    std::map<std::string, std::vector<Task>> tasks_;
    std::uint8_t download_num_ = 0;

public:
    TaskManager2();
    virtual ~TaskManager2();

    void addTask(Task t);
    void cancelTask(std::string uri);

    void tick(uint64_t t, std::function<void(const std::string uri, const Block blk, const std::shared_ptr<NetworkConnCtx> oldCtx)> reAssignTaskFunc);

    uint64_t stopPendingTask(std::string);

    void success(std::string uri, Block block);

    const bool isSuccess(std::string uri) const;

    const uint64_t downloadNum() const;
};

#endif
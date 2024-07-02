#include "modules/task2/task.h"

using namespace std;

const uint8_t *Task::prepareHeader() const
{
    return nullptr;
}

Task::~Task()
{
}

void Task::req()
{
    status_ = TaskStatusEnum::Syning;
    const uint8_t *data = prepareHeader();

    LanSyncPkt pkt(lan_sync_version::VER_0_1, LAN_SYNC_TYPE_GET_RESOURCE);

    string range_hdr = Range(block_.start, block_.end - block_.start).to_string();
    pkt.addXheader(XHEADER_URI, uri_);
    pkt.addXheader(XHEADER_RANGE, range_hdr);

    BufBaseonEvent buf;
    pkt.write(buf);
    ctx_->write(buf.data(), buf.size());
}

void Task::tick(uint64_t t)
{
    passed_time_ += t;
    if (passed_time_ >= 1000 * 60)
    {
        retry_++;
        passed_time_ = 0;
        // todo(lutar, 20240628) may be request again
    }
    if (retry_ >= MAX_RETRY)
        status_ = TaskStatusEnum::Failed;
}

bool Task::isFailed()
{
    if (retry_ >= 5)
        return true;
    return false;
}

void Task::stop()
{
    status_ = TaskStatusEnum::Stop;
}

TaskStatusEnum Task::getStatus() const
{
    return status_;
}

void Task::setStatus(TaskStatusEnum st)
{
    status_ = st;
}

const std::string Task::getUri() const
{
    return uri_;
}

const Block2 Task::getBlock() const
{
    return block_;
}

const std::shared_ptr<NetworkConnCtx> Task::getCtx() const
{
    return ctx_;
}

TaskManager2::TaskManager2(/* args */)
{
}

TaskManager2::~TaskManager2()
{
}

void TaskManager2::addTask(Task t)
{
    auto uri = t.getUri();
    auto uri_iter = tasks_.find(uri);

    if (uri_iter == tasks_.end())
    {
        tasks_[uri] = std::vector<Task>();
        tasks_[uri].push_back(t);
    }
    else
    {
        // todo 要确认是否重复
        uint64_t pos = Block2::pos(t.getBlock().start);
        if (pos >= tasks_[uri].size())
            tasks_[uri].push_back(t);
        else
        {
            if (tasks_[uri][pos].getStatus() == TaskStatusEnum::Success)
                return;
            tasks_[uri][pos] = t;
        }
    }
}

void TaskManager2::cancelTask(std::string uri)
{
    auto iter = tasks_.find(uri);
    if (iter != tasks_.end())
    {
        // todo( lutar,20240701) remove local tmp file
        tasks_.erase(iter);
    }
}

void TaskManager2::tick(uint64_t t, std::function<void(const std::string uri, const Block2 blk, const std::shared_ptr<NetworkConnCtx> oldCtx)> reAssignTaskFunc)
{
    for (auto uri_iter = tasks_.begin(); uri_iter != tasks_.end(); uri_iter++)
    {
        std::vector<Task> &ts = uri_iter->second;

        for (auto t_iter = ts.begin(); t_iter != ts.end(); t_iter++)
        {
            Task &tsk = *t_iter;
            if (tsk.getStatus() == TaskStatusEnum::Success)
                continue;

            if (tsk.getStatus() == TaskStatusEnum::Pendding)
            {
                if (download_num_ >= DOWNLOAD_LIMIT)
                    continue;
                tsk.req();
                download_num_++;
            }
            else if (tsk.getStatus() == TaskStatusEnum::Syning)
            {
                tsk.tick(t);
            }
            else if (tsk.getStatus() == TaskStatusEnum::Failed)
            {
                reAssignTaskFunc(tsk.getUri(), tsk.getBlock(), tsk.getCtx());
            }
        }
    }
}

uint64_t TaskManager2::stopPendingTask(std::string uri)
{
    auto uri_iter = tasks_.find(uri);
    if (uri_iter == tasks_.end())
        return 0;

    std::vector<Task> &ts = uri_iter->second;
    int i = 0;

    for (auto t_iter = ts.begin(); t_iter != ts.end(); t_iter++)
    {
        Task &tsk = *t_iter;
        if (tsk.getStatus() == TaskStatusEnum::Pendding)
        {
            tsk.stop();
            i++;
        }
    }
    return i;
}

void TaskManager2::success(std::string uri, Block2 block)
{
    auto uri_iter = tasks_.find(uri);
    if (uri_iter == tasks_.end())
        return;

    std::vector<Task> &ts = uri_iter->second;
    for (uint64_t i = 0; i < ts.size(); i++)
    {
        if (ts[i].getBlock() == block)
        {
            ts[i].setStatus(TaskStatusEnum::Success);
            download_num_--;
        }
    }
}

const bool TaskManager2::isSuccess(std::string uri) const
{
    auto uri_iter = tasks_.find(uri);
    if (uri_iter == tasks_.end())
        return false;

    const std::vector<Task> &ts = uri_iter->second;
    for (uint64_t i = 0; i < ts.size(); i++)
    {
        if (ts[i].getStatus() != TaskStatusEnum::Success)
            return false;
    }
    return true;
}

const uint64_t TaskManager2::downloadNum() const
{
    return download_num_;
}

const bool Block2::operator==(const Block2 &other)
{
    return static_cast<const Block2 &>(*this) == other;
}

const bool Block2::operator==(const Block2 &other) const
{
    return start == other.start && end == other.end;
}

const uint64_t Block2::pos(uint64_t offset)
{
    if (offset < BLOCK_SIZE)
        return 0;

    uint64_t pos = offset / BLOCK_SIZE;

    return pos;
}

const uint64_t Block2::bitPos(uint64_t pos)
{
    return pos * BLOCK_SIZE;
}

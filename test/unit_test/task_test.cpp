#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <algorithm>

#include "modules/task2/task_coordinator.h"
#include "test_task_manager.h"
#include "net/net_logic_container.h"
using namespace std;

class NetAbilityConcrete : public NetAbility
{
private:
    /* data */
public:
    NetAbilityConcrete(NetAddr addr) : NetAbility(addr) {}
    ~NetAbilityConcrete() {}

    void recv(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx) override {};
    uint64_t isExtraAllDataNow(void *data, uint64_t data_len) override { return 1; };
    NetAddr &getAddr() override { return addr; };
    void setSock(int sock) override {};
};

class NetworkCtxConcrete : public NetworkConnCtx
{
private:
    /* data */
public:
    NetworkCtxConcrete(/* args */) {}
    NetworkCtxConcrete(std::vector<std::shared_ptr<NetworkConnCtx>> *head, NetAbility *ne, NetAddr peer) : NetworkConnCtx(head, ne, peer) {};
    ~NetworkCtxConcrete() {}

    virtual uint64_t write(void *data, uint64_t data_len) { return 1; };
};

class CoordinatorSimpleTest : public testing::Test
{

protected:
    TaskManagerForTest *tmft = new TaskManagerForTest();
    shared_ptr<TaskManager2> tm{tmft};
    TaskCoordinator *cor;
    void SetUp() override
    {
        cor = new TaskCoordinator(tm);
        printf("SetUp \n");
    }

    void TearDown() override
    {
        printf("TearDown \n");
        delete cor;
    }

public:
    /**
     * @brief simple test
     *
     * @param ctx_num
     * @param uri
     * @param size
     */
    void simple_case(uint16_t ctx_num, string uri, uint64_t size)
    {
        std::vector<std::shared_ptr<NetworkConnCtx>> head;
        NetAbilityConcrete na(NetAddr("127.0.0.1:8080"));
        vector<std::shared_ptr<NetworkCtxConcrete>> ctxs;
        for (size_t i = 0; i < ctx_num; i++)
        {
            std::shared_ptr<NetworkConnCtx> ctx = std::make_shared<NetworkCtxConcrete>(&head, &na, NetAddr(to_string(i)));
            cor->add_resource(uri, Range2(0, size), ctx);
        }

        ASSERT_EQ(ctx_num, tmft->tasks_ctx_num(uri));

        uint64_t blk_nums = (size / BLOCK_SIZE + 1);
        uint64_t offset = 0;

        cor->tick(1000 * 60);

        ASSERT_GT(tm->downloadNum(), 0);

        uint i = 0;
        if (size / BLOCK_SIZE > DOWNLOAD_LIMIT)
        {
            for (; i < DOWNLOAD_LIMIT; i++)
            {
                uint64_t end = min(size, offset + BLOCK_SIZE);
                tm->success(uri, Block2(offset, end));
                offset = end;
                if (offset >= size)
                    break;
            }

            ASSERT_FALSE(tm->isSuccess(uri));
        }

        for (; i < blk_nums; i++)
        {
            uint64_t end = min(size, offset + BLOCK_SIZE);
            tm->success(uri, Block2(offset, end));
            offset = end;
            if (offset >= size)
                break;
        }

        ASSERT_TRUE(tm->isSuccess(uri));
    }

    /**
     * @brief timeout test
     *
     * @param ctx_num
     * @param uri
     * @param size
     */
    void timeout_case(uint16_t ctx_num, string uri, uint64_t size)
    {
        std::vector<std::shared_ptr<NetworkConnCtx>> head;
        NetAbilityConcrete na(NetAddr("127.0.0.1:8080"));
        vector<std::shared_ptr<NetworkCtxConcrete>> ctxs;
        for (size_t i = 0; i < ctx_num; i++)
        {
            std::shared_ptr<NetworkCtxConcrete> ctx = make_shared<NetworkCtxConcrete>(&head, &na, NetAddr(to_string(i)));
            cor->add_resource(uri, Range2(0, size), ctx);
        }

        ASSERT_EQ(ctx_num, tmft->tasks_ctx_num(uri));

        vector<Task> old_tasks = tmft->tasks(uri);
        if (old_tasks.size() <= DOWNLOAD_LIMIT)
        {
            for (uint i = 0; i < 7; i++)
                cor->tick(1000 * 60);

            vector<Task> tasks = tmft->tasks(uri);
            ASSERT_EQ(old_tasks.size(), tasks.size());
            for (size_t i = 0; i < tasks.size(); i++)
                ASSERT_NE(old_tasks[i].getCtx()->getPeer().str(), tasks[i].getCtx()->getPeer().str());
        }
        else
        {
            for (size_t i = 0; i < old_tasks.size(); i += DOWNLOAD_LIMIT)
            {
                for (uint i = 0; i < 7; i++)
                {
                    cor->tick(1000 * 60);
                }
                for (size_t j = 0; j < DOWNLOAD_LIMIT; j++)
                {
                    uint64_t start = Block2::bitPos(i + j);
                    tm->success(uri, Block2(start, start + BLOCK_SIZE));
                }
            }

            vector<Task> tasks = tmft->tasks(uri);
            ASSERT_EQ(old_tasks.size(), tasks.size());
            for (size_t i = 0; i < tasks.size(); i++)
                ASSERT_NE(old_tasks[i].getCtx()->getPeer().str(), tasks[i].getCtx()->getPeer().str()) << "i:" << i;
        }
    }

    void ctx_join(uint16_t ctx_num, string uri, uint64_t size)
    {
        size = min(static_cast<uint64_t>(SIZE_1MByte * 1024), size);
        std::vector<std::shared_ptr<NetworkConnCtx>> head;
        NetAbilityConcrete na(NetAddr("127.0.0.1:8080"));

        uint64_t pos = 0;
        for (size_t i = 0; i < ctx_num; i++)
        {
            std::shared_ptr<NetworkCtxConcrete> ctx = make_shared<NetworkCtxConcrete>(&head, &na, NetAddr(to_string(i)));
            cor->add_resource(uri, Range2(0, size), ctx);

            cor->tick(1000 * 60);

            for (; pos < 5;)
            {
                uint64_t start = Block2::bitPos(pos++);
                tm->success(uri, Block2(start, start + BLOCK_SIZE));
            }
        }

        while (!tm->isSuccess(uri))
        {
            uint64_t start = Block2::bitPos(pos++);
            tm->success(uri, Block2(start, start + BLOCK_SIZE));
        }

        ASSERT_EQ(ctx_num, tmft->tasks_ctx_num(uri));
    }
};

TEST_F(CoordinatorSimpleTest, single_ctx_block_blk_nums_lt_download_limit)
{
    simple_case(1, "rs1", SIZE_1MByte * 50);
}
TEST_F(CoordinatorSimpleTest, single_ctx_block_blk_nums_gt_download_limit)
{
    simple_case(1, "rs1", SIZE_1MByte * 1024);
}

TEST_F(CoordinatorSimpleTest, many_ctx_block_blk_nums_lt_download_limit)
{
    simple_case(10, "rs1", SIZE_1MByte * 50);
}

TEST_F(CoordinatorSimpleTest, many_ctx_block_blk_nums_gt_download_limit)
{
    simple_case(10, "rs1", SIZE_1MByte * 1024);
}

TEST_F(CoordinatorSimpleTest, timout)
{
    timeout_case(10, "rs1", SIZE_1MByte * 50);
}

TEST_F(CoordinatorSimpleTest, timout_gt_download_limit)
{
    timeout_case(10, "rs1", SIZE_1MByte * 1024);
}

TEST_F(CoordinatorSimpleTest, ctx_join_simple)
{
    ctx_join(10, "rs1", SIZE_1MByte * 1024);
}

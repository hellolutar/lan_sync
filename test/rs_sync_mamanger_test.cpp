#include <gtest/gtest.h>

#include <sstream>
#include <cstring>
#include <algorithm>
#include "modules/rs_sync_mamanger.h"

using namespace std;

filesystem::path test_peer_dir("test_peer_dir");
filesystem::path test_peer_small_file("test_peer_dir/small.txt");
filesystem::path test_peer_big_file("test_peer_dir/big.txt");

uint64_t test_peer_small_file_size = SIZE_1KByte;
uint64_t test_peer_big_file_size = SIZE_1MByte * 3;

uint64_t test_small_file_size = 100;
uint64_t test_big_file_size = SIZE_1MByte;

filesystem::path test_dir("test_dir");
filesystem::path test_small_file("test_dir/small.txt");
filesystem::path test_big_file("test_dir/big.txt");

string dic = "ABCDEFGHIJKLMNOPQRSTUVWSYZ1234567890abcdefghijklmnopqrstuvwsy";

class DemoTest : public testing::Test
{

protected:
    void genFile(string path, uint64_t size)
    {

        stringstream ss;
        int len = 0;
        while (true)
        {
            size_t retlen = size - len;
            int cur_write_len = min(dic.size(), retlen);
            ss << dic.substr(0, cur_write_len);
            len += cur_write_len;
            if (len >= size)
            {
                break;
            }
        }
        string data;
        ss >> data;

        IoUtil io;
        io.writeFile(path, 0, data.data(), size);
    }

    RsSyncManager rsm;
    void SetUp() override
    {
        ResourceManager::init(test_dir.string());

        filesystem::create_directories(test_peer_dir);
        filesystem::create_directories(test_dir);

        genFile(test_big_file.string(), test_big_file_size);
        genFile(test_small_file.string(), test_small_file_size);

        genFile(test_peer_small_file.string(), test_peer_small_file_size);
        genFile(test_peer_big_file.string(), test_peer_big_file_size);
    }

    void TearDown() override
    {
        filesystem::remove_all(test_peer_dir);
        filesystem::remove_all(test_dir);
    }

    void TestUpdSyncingRsByTbIdx()
    {
        NetAddr peer("127.0.0.1:38080");
        Resource peerrs[2];
        RsLocalManager peer_rm(test_peer_dir.string());
        vector<struct Resource *> peer_rs_table = peer_rm.getTable();

        RsSyncManager rsync;
        rsync.refreshSyncingRsByTbIdx(peer, Resource::vecToArr(peer_rs_table), peer_rs_table.size());
        Block req_small_block = rsync.regReqSyncRsAuto(peer, "/small.txt");

        ASSERT_TRUE(Block(0, test_peer_small_file_size + 1) == req_small_block);

        uint64_t total_len = 0;
        Block req_big_block = rsync.regReqSyncRsAuto(peer, "/big.txt");
        total_len += req_big_block.end - req_big_block.start;
        printf("%ld\n", req_big_block.end - req_big_block.start);

        req_big_block = rsync.regReqSyncRsAuto(peer, "/big.txt");
        total_len += req_big_block.end - req_big_block.start;
        printf("%ld\n", req_big_block.end - req_big_block.start);

        req_big_block = rsync.regReqSyncRsAuto(peer, "/big.txt");
        total_len += req_big_block.end - req_big_block.start;
        printf("%ld\n", req_big_block.end - req_big_block.start);

        ASSERT_EQ(total_len - 1, test_peer_big_file_size);  // [0,total_len), so total_len need to - 1
    }

public:
};

TEST_F(DemoTest, Add)
{
    TestUpdSyncingRsByTbIdx();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}

#include <gtest/gtest.h>

#include <sstream>
#include <cstring>
#include <algorithm>
#include "modules/rs_sync_mamanger.h"
#include "modules/rs_manager.h"

using namespace std;

string big_file_uri = "/big.txt";
string small_file_uri = "/small.txt";

filesystem::path test_peer_dir("test_peer_dir");
filesystem::path test_peer_small_file("test_peer_dir/small.txt");
filesystem::path test_peer_big_file("test_peer_dir/big.txt");

filesystem::path test_peer2_dir("test_peer2_dir");
filesystem::path test_peer_bigger_file("test_peer2_dir/big.txt");
uint64_t test_peer_bigger_file_size = SIZE_1MByte * 4;

uint64_t test_peer_small_file_size = SIZE_1KByte;
uint64_t test_peer_big_file_size = SIZE_1MByte * 3;

uint64_t test_small_file_size = 100;
uint64_t test_big_file_size = SIZE_1MByte;

filesystem::path test_dir("test_dir");
filesystem::path test_small_file("test_dir/small.txt");
filesystem::path test_big_file("test_dir/big.txt");

string dic = "ABCDEFGHIJKLMNOPQRSTUVWSYZ1234567890abcdefghijklmnopqrstuvwsy";

class RsSyncManagerTest : public testing::Test
{

protected:
    void genFile(string path, uint64_t size)
    {

        stringstream ss;
        int len = 0;
        while (true)
        {
            uint64_t retlen = size - len;
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

    void SetUp() override
    {
        ResourceManager::init(test_dir.string());

        filesystem::create_directories(test_peer_dir);
        filesystem::create_directories(test_peer2_dir);
        filesystem::create_directories(test_dir);

        genFile(test_big_file.string(), test_big_file_size);
        genFile(test_small_file.string(), test_small_file_size);

        genFile(test_peer_small_file.string(), test_peer_small_file_size);
        genFile(test_peer_big_file.string(), test_peer_big_file_size);

        genFile(test_peer_bigger_file.string(), test_peer_bigger_file_size);
    }

    void TearDown() override
    {
        filesystem::remove_all(test_peer_dir);
        filesystem::remove_all(test_peer2_dir);
        filesystem::remove_all(test_dir);
    }

    Resource *findFromTbWithUri(vector<struct Resource *> table, string uri)
    {
        for (auto rs : table)
        {
            if (rs->uri == uri)
            {
                return rs;
            }
        }
        return nullptr;
    }

    void TestCaseNormal()
    {
        NetAddr peer("127.0.0.1:38080");
        RsLocalManager peer_rm(test_peer_dir.string());
        vector<struct Resource *> peer_rs_table = peer_rm.getTable();

        RsSyncManager rsync(ResourceManager::getRsLocalManager());

        uint64_t total_len = 0;
        rsync.refreshSyncingRsByTbIdx(peer, Resource::vecToArr(peer_rs_table), peer_rs_table.size());
        while (rsync.getAllUriRs()[small_file_uri].block.size() > 0)
        {
            Block b = rsync.regReqSyncRsAuto(peer, small_file_uri);
            total_len += b.end - b.start;
        }
        ASSERT_EQ(total_len, test_peer_small_file_size);

        // register rs of peer1, then syncing
        total_len = 0;
        int total_block_num = rsync.getAllUriRs()[big_file_uri].block.size();
        Block b = rsync.regReqSyncRsAuto(peer, big_file_uri);
        total_len = b.end - b.start;

        // register rs of peer2 while syncing peer1 rs
        NetAddr peer2("127.0.0.2:38080");
        rsync.refreshSyncingRsByTbIdx(peer2, Resource::vecToArr(peer_rs_table), peer_rs_table.size());
        SyncRs rs = rsync.getAllUriRs()[big_file_uri];
        ASSERT_EQ(rs.syncing.size(), 1);
        ASSERT_EQ(rs.owner.size(), 2);
        ASSERT_STREQ(rs.owner[0].str().data(), peer.str().data());
        ASSERT_STREQ(rs.owner[1].str().data(), peer2.str().data());

        Resource *bigrs = findFromTbWithUri(peer_rs_table, big_file_uri);
        ASSERT_EQ(rs.hash, bigrs->hash);
        ASSERT_EQ(rs.size, bigrs->size);

        while (rsync.getAllUriRs()[big_file_uri].block.size() > 0)
        {
            b = rsync.regReqSyncRsAuto(peer2, big_file_uri);
            ASSERT_NE(b.end, 0);
            total_len += b.end - b.start;
            ASSERT_EQ(total_block_num, rsync.getAllUriRs()[big_file_uri].block.size() + rsync.getAllUriRs()[big_file_uri].syncing.size());
        }

        ASSERT_EQ(total_len, test_peer_big_file_size); // [0,total_len), so total_len need to - 1
    }

    void TestCaseFoundBetterRsWhileSyncing()
    {
        RsSyncManager rsync(ResourceManager::getRsLocalManager());
        uint64_t total_len = 0;

        NetAddr peer("127.0.0.1:38080");
        RsLocalManager peer_rm(test_peer_dir.string());
        vector<struct Resource *> peer_rs_table = peer_rm.getTable();
        rsync.refreshSyncingRsByTbIdx(peer, Resource::vecToArr(peer_rs_table), peer_rs_table.size());

        // register rs of peer1, then syncing
        Block b = rsync.regReqSyncRsAuto(peer, big_file_uri);
        total_len += b.end - b.start;
        ASSERT_GT(total_len, 0);

        // register rs of peer2, then syncing while sycing peer 1.
        NetAddr peer2("127.0.0.2:38080");
        RsLocalManager peer2_rm(test_peer2_dir.string());
        vector<struct Resource *> peer2_rs_table = peer2_rm.getTable();
        rsync.refreshSyncingRsByTbIdx(peer2, Resource::vecToArr(peer2_rs_table), peer2_rs_table.size());
        SyncRs mustBebiggerRs = rsync.getAllUriRs()[big_file_uri];
        ASSERT_EQ(mustBebiggerRs.syncing.size(), 0);
        ASSERT_EQ(mustBebiggerRs.owner.size(), 1);
        ASSERT_STREQ(mustBebiggerRs.owner[0].str().data(), peer2.str().data());
        ASSERT_EQ(mustBebiggerRs.hash, peer2_rs_table[0]->hash);
        ASSERT_EQ(mustBebiggerRs.size, peer2_rs_table[0]->size);

        int total_block_num = rsync.getAllUriRs()[big_file_uri].block.size();
        total_len = 0;
        while (rsync.getAllUriRs()[big_file_uri].block.size() > 0)
        {
            Block b = rsync.regReqSyncRsAuto(peer2, big_file_uri);
            ASSERT_NE(b.end, 0);
            total_len += b.end - b.start;
            ASSERT_EQ(total_block_num, rsync.getAllUriRs()[big_file_uri].block.size() + rsync.getAllUriRs()[big_file_uri].syncing.size());
        }
        ASSERT_EQ(total_len, test_peer_bigger_file_size); // [0,total_len), so total_len need to - 1
    }

    void TestCaseOneConnErrWhileSyncing()
    {
        NetAddr peer("127.0.0.1:38080");
        NetAddr peer2("127.0.0.2:48080");
        RsLocalManager peer_rm(test_peer_dir.string());
        vector<struct Resource *> peer_rs_table = peer_rm.getTable();

        RsSyncManager rsync(ResourceManager::getRsLocalManager());

        uint64_t total_len = 0;
        // register rs of peer1, then syncing
        rsync.refreshSyncingRsByTbIdx(peer, Resource::vecToArr(peer_rs_table), peer_rs_table.size());
        int total_block_num = rsync.getAllUriRs()[big_file_uri].block.size();
        Block peer1_b = rsync.regReqSyncRsAuto(peer, big_file_uri);
        total_len = peer1_b.end - peer1_b.start;

        // register rs of peer1, then syncing
        rsync.refreshSyncingRsByTbIdx(peer2, Resource::vecToArr(peer_rs_table), peer_rs_table.size());
        Block b = rsync.regReqSyncRsAuto(peer2, big_file_uri);
        total_len += b.end - b.start;

        // peer1 occur connection error
        rsync.unregAllReqSyncRsByPeer(peer, big_file_uri);
        total_len -= peer1_b.end - peer1_b.start; // substract the failed part
        ASSERT_EQ(total_block_num, rsync.getAllUriRs()[big_file_uri].block.size() + rsync.getAllUriRs()[big_file_uri].syncing.size());
        ASSERT_EQ(rsync.getAllUriRs()[big_file_uri].owner.size(), 1);
        ASSERT_STREQ(rsync.getAllUriRs()[big_file_uri].owner[0].str().data(), peer2.str().data());
        while (rsync.getAllUriRs()[big_file_uri].block.size() > 0)
        {
            Block b = rsync.regReqSyncRsAuto(peer2, big_file_uri);
            ASSERT_NE(b.end, 0);
            total_len += b.end - b.start;
            ASSERT_EQ(total_block_num, rsync.getAllUriRs()[big_file_uri].block.size() + rsync.getAllUriRs()[big_file_uri].syncing.size());
        }
        ASSERT_EQ(total_len, test_peer_big_file_size); // [0,total_len), so total_len need to - 1
    }

    void TestCaseAllConnErrWhileSyncing()
    {
        RsSyncManager rsync(ResourceManager::getRsLocalManager());
        uint64_t total_len = 0;

        NetAddr peer("127.0.0.1:38080");
        RsLocalManager peer_rm(test_peer_dir.string());
        vector<struct Resource *> peer_rs_table = peer_rm.getTable();
        rsync.refreshSyncingRsByTbIdx(peer, Resource::vecToArr(peer_rs_table), peer_rs_table.size());
        int total_block_num = rsync.getAllUriRs()[big_file_uri].block.size();

        // register rs of peer1, then syncing
        Block b = rsync.regReqSyncRsAuto(peer, big_file_uri);
        total_len += b.end - b.start;
        ASSERT_GT(total_len, 0);

        // register rs of peer2, then syncing while sycing peer 1.
        NetAddr peer2("127.0.0.2:48080");
        rsync.refreshSyncingRsByTbIdx(peer2, Resource::vecToArr(peer_rs_table), peer_rs_table.size());
        SyncRs rs = rsync.getAllUriRs()[big_file_uri];
        ASSERT_EQ(rs.syncing.size(), 1);
        ASSERT_EQ(rs.owner.size(), 2);

        b = rsync.regReqSyncRsAuto(peer2, big_file_uri);
        total_len += b.end - b.start;
        ASSERT_GT(total_len, 0);

        rsync.unregAllReqSyncRsByPeer(peer, big_file_uri);
        rsync.unregAllReqSyncRsByPeer(peer2, big_file_uri);
        ASSERT_EQ(rsync.getAllUriRs()[big_file_uri].owner.size(), 0);
        ASSERT_EQ(rsync.getAllUriRs()[big_file_uri].syncing.size(), 0);
        ASSERT_EQ(rsync.getAllUriRs()[big_file_uri].block.size(), total_block_num);
    }
    void TestSyncingRangeDone()
    {
        NetAddr peer("127.0.0.1:38080");
        RsLocalManager peer_rm(test_peer_dir.string());
        vector<struct Resource *> peer_rs_table = peer_rm.getTable();

        RsSyncManager rsync(ResourceManager::getRsLocalManager());

        rsync.refreshSyncingRsByTbIdx(peer, Resource::vecToArr(peer_rs_table), peer_rs_table.size());

        // register rs of peer1, then syncing
        uint64_t total_len = 0;
        Block b;
        int total_block_num = rsync.getAllUriRs()[big_file_uri].block.size();
        while (!rsync.isSyncSuccess(big_file_uri) && rsync.getAllUriRs()[big_file_uri].block.size() > 0)
        {
            b = rsync.regReqSyncRsAuto(peer, big_file_uri);
            ASSERT_NE(b.end, 0);
            total_len += b.end - b.start;
            ASSERT_EQ(1, rsync.getAllUriRs()[big_file_uri].syncing.size());
            ASSERT_FALSE(rsync.isSyncSuccess(big_file_uri));
            rsync.syncingRangeDone(peer, big_file_uri, b);
        }
        ASSERT_EQ(total_len, test_peer_big_file_size); // [0,total_len), so total_len need to - 1
        ASSERT_TRUE(rsync.isSyncSuccess(big_file_uri));
    }

public:
};

TEST_F(RsSyncManagerTest, Add)
{
    TestCaseNormal();
    TestCaseFoundBetterRsWhileSyncing();
    TestCaseOneConnErrWhileSyncing();
    TestCaseAllConnErrWhileSyncing();
    TestSyncingRangeDone();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}

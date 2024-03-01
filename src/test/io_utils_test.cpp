#include <gtest/gtest.h>

#include <random>
#include <sstream>
#include <fstream>

#include "utils/io_utils.h"

using namespace std;

string dic = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
std::default_random_engine e;
std::uniform_int_distribution<int> u(1, dic.size() * SIZE_1KB); // 左闭右闭区间

TEST(HelloTest, BasicAssertions)
{
    EXPECT_STRNE("hello", "world");
}

string genFname()
{
    time_t now = time(0);
    string fname = to_string(now) + ".txt";
    return fname;
}

string generateData()
{
    e.seed(time(0));
    int randV = u(e);
    stringstream ss;
    for (size_t i = 0; i < randV; i++)
    {
        for (size_t j = 0; j < dic.size(); j++)
        {
            ss << dic[j];
        }
    }
    string datastr;
    ss >> datastr;

    return datastr;
}

void setup(string path)
{
    filesystem::path p;
    if (filesystem::exists(p))
        filesystem::resize_file(p, 0);
    else
    {
        int fd = open(path.data(), O_CREAT, 0644);
        close(fd);
    }
    IoUtil io;

    string datastr = generateData();

    io.writeFile(path, 0, datastr.data(), datastr.size());
}

void tearDown(string path)
{
    filesystem::path p(path);
    if (filesystem::exists(p))
        filesystem::remove(p);
}

uint8_t *readFile(string path, uint64_t fsize)
{
    ifstream ifs;
    ifs.open(path, ios::in);
    uint8_t *buf = (uint8_t *)malloc(fsize); // free by outter
    memset(buf, 0, fsize);
    while (ifs >> buf)
    {
    }
    return buf;
}

TEST(IOUtilsTest, readAll)
{
    string fname = genFname();
    setup(fname);

    IoUtil io;
    uint64_t datalen = 0;
    uint8_t *data = (uint8_t *)io.readAll(fname, datalen);

    filesystem::path p(fname);
    uint64_t fsize = filesystem::file_size(fname);
    ASSERT_EQ(fsize, datalen);

    uint8_t *buf = readFile(fname, fsize);
    for (size_t i = 0; i < datalen; i++)
    {
        uint8_t a = data[i];
        uint8_t b = (uint8_t)buf[i];
        ASSERT_EQ(a, b) << " i:" << i;
    }
    free(buf);
    free(data);

    tearDown(fname);
}

TEST(IOUtilsTest, readAll_offset)
{
    string fname = genFname();
    setup(fname);

    IoUtil io;
    uint64_t datalen = 0;

    filesystem::path p(fname);
    size_t fsize = filesystem::file_size(p);

    e.seed(time(0));
    int randV = u(e);
    int randV2 = u(e);

    uint8_t *data = (uint8_t *)io.readAll(fname, randV, randV2, datalen);
    uint8_t *buf = readFile(fname, fsize);
    string buf_str((char *)buf);
    string expected_data = buf_str.substr(randV, randV2);
    free(buf);

    for (size_t i = 0; i < datalen; i++)
    {
        uint8_t a = data[i];
        uint8_t b = (uint8_t)expected_data[i];
        ASSERT_EQ(a, b) << " i:" << i;
    }
    free(data);

    tearDown(fname);
}

void test_read_cb(uint64_t curpos, void *data, uint64_t data_len)
{
    assert(data_len > 0);
    assert(data != nullptr);
}

TEST(IOUtilsTest, readAll_cb)
{
    string fname = genFname();
    setup(fname);

    IoUtil io;
    io.setReadCb(test_read_cb);

    uint64_t len;
    io.readAll(fname, len);

    tearDown(fname);
}

TEST(IOUtilsTest, writeFile)
{
    string fname = genFname();
    setup(fname);

    for (size_t i = 0; i < 10; i++)
    {
        e.seed(time(0));
        int randV = u(e);

        string datastr = generateData();
        IoUtil io;
        io.writeFile(fname, randV, datastr.data(), SIZE_1KB);

        uint64_t readLen = 0;
        uint8_t *data = (uint8_t *)io.readAll(fname, randV, SIZE_1KB, readLen);

        for (size_t i = 0; i < readLen; i++)
        {
            uint8_t a = data[i];
            uint8_t b = (uint8_t)datastr[i];
            ASSERT_EQ(a, b) << " i:" << i;
        }
        free(data);
    }

    tearDown(fname);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
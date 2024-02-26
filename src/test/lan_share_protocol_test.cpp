#include <gtest/gtest.h>

#include "comm/lan_share_protocol.h"

TEST(HelloTest, BasicAssertions)
{
    EXPECT_STRNE("hello", "world");
}

TEST(lan_sync_header_test, set_data_test)
{
    char *data = "hello world, nihao shijie!\r\n\0";
    int data_len = strlen(data);

    lan_sync_header_t *hd = lan_sync_header_new(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    hd = lan_sync_header_set_data(hd, data, data_len);

    int expect_total_len = lan_sync_header_len + data_len;
    ASSERT_EQ(expect_total_len, hd->total_len);

    char *ret_data = (char *)malloc(data_len + 1);
    memset(ret_data, 0, data_len + 1);

    lan_sync_header_extract_data(hd, ret_data);

    ASSERT_STREQ(data, ret_data);
    free(ret_data);
}

TEST(lan_sync_header_test, add_xheader_test)
{
    char *data = "hello world, nihao shijie!\r\n\0";
    string key = XHEADER_URI;
    string value = "/network/TCPIP卷1.txt";
    char *expect_xheader = "uri:/network/TCPIP卷1.txt\0";
    int data_len = strlen(data);
    int key_len = key.size();
    int value_len = value.size();
    int xheader_len = data_len + key_len + value_len + FLAG_KEY_VALUE_SPLIT;
    int expect_total_len = lan_sync_header_len + xheader_len;

    lan_sync_header_t *hd = lan_sync_header_new(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    hd = lan_sync_header_add_xheader(hd, key, value);
    hd = lan_sync_header_set_data(hd, data, data_len);
    ASSERT_EQ(expect_total_len, hd->total_len);

    string ret_val = lan_sync_header_query_xheader(hd, key);
    ASSERT_STREQ(value.data(), ret_val.data());

    // verify data
    char *ret_data = (char *)malloc(data_len + 1);
    memset(ret_data, 0, data_len + 1);

    lan_sync_header_extract_data(hd, ret_data);

    ASSERT_STREQ(data, ret_data);
    free(ret_data);
    // verify data end

    // verify header
    char *ret_xheader = (char *)malloc(xheader_len + 1);
    memset(ret_xheader, 0, xheader_len + 1);
    lan_sync_header_extract_xheader(hd, ret_xheader);
    ASSERT_STREQ(expect_xheader, ret_xheader);
    free(ret_xheader);
    // verify header end
}

TEST(lan_sync_header_test, extract_xheader_test)
{
    char *data = "hello world, nihao shijie!\r\n\0";
    char *key = XHEADER_URI;
    string value = "/network/TCPIP卷1.txt";
    char *key2 = "path";
    string value2 = "/home/lutar/code/c/lan_sync/demo/resources/server/network";
    char *expect_xheader = "uri:/network/TCPIP卷1.txt\0path:/home/lutar/code/c/lan_sync/demo/resources/server/network\0";

    lan_sync_header_t *hd = lan_sync_header_new(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    hd = lan_sync_header_add_xheader(hd, key, value);
    hd = lan_sync_header_add_xheader(hd, key2, value2);

    // verify header
    char *ret_xheader = (char *)malloc(512);
    memset(ret_xheader, 0, 512);
    lan_sync_header_extract_xheader(hd, ret_xheader);
    ASSERT_STREQ(expect_xheader, ret_xheader);
    free(ret_xheader);
    // verify header end
}

TEST(lan_sync_header_test, query_xheader_test)
{
    char *data = "hello world, nihao shijie!\r\n\0";
    string key = XHEADER_URI;
    string value = "/network/TCPIP卷1.txt";
    string key2 = "path";
    string value2 = "/home/lutar/code/c/lan_sync/demo/resources/server/network";

    lan_sync_header_t *hd = lan_sync_header_new(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    hd = lan_sync_header_add_xheader(hd, key, value);
    hd = lan_sync_header_add_xheader(hd, key2, value2);

    string ret_value = lan_sync_header_query_xheader(hd, key);
    ASSERT_STREQ(value.data(), ret_value.data());
    string ret_value2 = lan_sync_header_query_xheader(hd, key2);
    ASSERT_STREQ(value2.data(), ret_value2.data());
}

TEST(LocalPortTest, existIp)
{

    vector<LocalPort> ports = LocalPort::query();
    if (ports.size() == 0)
        return;

    LocalPort port = ports[0];

    GTEST_ASSERT_TRUE(LocalPort::existIp(ports, port.getAddr().sin_addr));

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(1);

    GTEST_ASSERT_FALSE(LocalPort::existIp(ports, addr.sin_addr));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
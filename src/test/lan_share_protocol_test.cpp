#include <gtest/gtest.h>

#include "comm/lan_share_protocol.h"

TEST(HelloTest, BasicAssertions)
{
    EXPECT_STRNE("hello", "world");
}

bool compareChar(char *l, char *r, uint32_t cnt)
{
    for (size_t i = 0; i < cnt; i++)
    {
        if (*l != *r)
            return false;
    }
    return true;
}

void compareLanSyncPkt(LanSyncPkt &pkt, LanSyncPkt &pkt2)
{
    ASSERT_EQ(pkt.version, pkt2.version);
    ASSERT_EQ(pkt.type, pkt2.type);
    ASSERT_EQ(pkt.getTotalLen(), pkt2.getTotalLen());
    ASSERT_EQ(pkt.getHeaderLen(), pkt2.getHeaderLen());

    uint32_t data_len = pkt.getTotalLen() - pkt.getHeaderLen();
    char *ret_data = (char *)(pkt.getData());
    char *ret_data2 = (char *)(pkt2.getData());
    ASSERT_TRUE(compareChar(ret_data, ret_data2, data_len));
}

void checkLanSyncPktSerialize(LanSyncPkt &pkt)
{
    struct evbuffer *buf = evbuffer_new();
    pkt.write(buf);
    char tmp[2048];
    evbuffer_remove(buf, tmp, evbuffer_get_length(buf));
    lan_sync_header_t *hd = (lan_sync_header_t *)tmp;
    LanSyncPkt pkt2(hd);

    const map<string, string> xheaders = pkt.getXheaders();
    for (auto i = xheaders.begin(); i != xheaders.end(); i++)
    {
        string key = (*i).first;
        string value = (*i).second;
        ASSERT_STREQ(value.data(), pkt2.queryXheader(key).data());
    }
    compareLanSyncPkt(pkt, pkt2);
}

TEST(LanSyncPktTest, setData)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    pkt.addXheader("hello", "world");

    ASSERT_EQ(pkt.getHeaderLen(), strlen("hello:world ") + LEN_LAN_SYNC_HEADER_T);

    pkt.addXheader("dev", "cpp");
    ASSERT_EQ(pkt.getHeaderLen(), strlen("hello:world dev:cpp ") + LEN_LAN_SYNC_HEADER_T);

    string msg = "data 123 123 data";
    pkt.setData(msg.data(), msg.size());
    ASSERT_EQ(pkt.getTotalLen(), strlen("hello:world dev:cpp ") + msg.size() + LEN_LAN_SYNC_HEADER_T);

    checkLanSyncPktSerialize(pkt);
}

TEST(LanSyncPktTest, addXheader)
{
    char *data = "hello world, nihao shijie!\r\n\0";
    string key = XHEADER_URI;
    string value = "/network/TCPIP卷1.txt";
    char *expect_xheader = "uri:/network/TCPIP卷1.txt\0";
    uint32_t data_len = strlen(data);
    uint32_t key_len = key.size();
    uint32_t value_len = value.size();
    uint32_t xheader_len = data_len + key_len + value_len + FLAG_KEY_VALUE_SPLIT;
    uint32_t expect_total_len = LEN_LAN_SYNC_HEADER_T + xheader_len;

    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    pkt.addXheader(key, value);
    pkt.setData(data, data_len);

    ASSERT_EQ(expect_total_len, pkt.getTotalLen());

    string ret_val = pkt.queryXheader(key);
    ASSERT_STREQ(value.data(), ret_val.data());

    // verify data
    char *ret_data = (char *)(pkt.getData());
    ASSERT_STREQ(data, ret_data);
    // verify data end

    checkLanSyncPktSerialize(pkt);
}

TEST(LanSyncPktTest, queryXheader)
{
    char *data = "hello world, nihao shijie!\r\n\0";
    string key = XHEADER_URI;
    string value = "/network/TCPIP卷1.txt";
    string key2 = "path";
    string value2 = "/home/lutar/code/c/lan_sync/demo/resources/server/network";

    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    pkt.addXheader(key, value);
    pkt.addXheader(key2, value2);

    ASSERT_STREQ(value.data(), pkt.queryXheader(key).data());
    ASSERT_STREQ(value2.data(), pkt.queryXheader(key2).data());
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
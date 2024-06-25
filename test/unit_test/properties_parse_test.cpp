#include <gtest/gtest.h>

#include "constants/constants.h"
#include "utils/properties_parse.h"
#include "utils/io_utils.h"

TEST(HelloTest, BasicAssertions)
{
    EXPECT_STRNE("hello", "world");
}

class PropertiesParseTest : public testing::Test
{
private:
    string propertiesPath = "properties.properties";
    IoUtil io;

protected:
    void SetUp() override
    {
        string datastr = "ip.discovery=172.27.211.138\n"
                         "proto.tcp.server.port=58081\n"
                         "proto.udp.server.port=58080\n"
                         "local.tcp.server.port=58081\n"
                         "local.udp.server.port=58080\n"
                         "resource.home= /home/lutar/code/c/lan_sync/src/static/cli";

        io.writeFile(propertiesPath, 0, datastr.data(), datastr.size());
    }

    void TearDown() override
    {
        printf("TearDown \n");
    }

public:
    void queryTest(PropertiesParse pp)
    {
        string tcp_port = pp.query(CONFIG_KEY_PROTO_SYNC_SERVER_TCP_PORT);
        string udp_port = pp.query(CONFIG_KEY_PROTO_DISCOVER_SERVER_UDP_PORT);
        string rs_home = pp.query(CONFIG_KEY_RESOURCE_HOME);
        ASSERT_STREQ("58081", tcp_port.data());
        ASSERT_STREQ("58080", udp_port.data());
        ASSERT_GT(rs_home.size(), 0);
    }
};

TEST_F(PropertiesParseTest, queryTest)
{
    PropertiesParse pp("properties.properties");
    queryTest(pp);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
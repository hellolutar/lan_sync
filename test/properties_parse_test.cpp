#include <gtest/gtest.h>
#include "utils/properties_parse.h"

TEST(HelloTest, BasicAssertions)
{
    EXPECT_STRNE("hello", "world");
}

TEST(PropertiesParseTest, queryTest)
{
    PropertiesParse pp("../src/properties.properties");
    string tcp_port = pp.query("tcp.port");
    string udp_port = pp.query("udp.port");
    string rs_home = pp.query("resource.home");
    ASSERT_STREQ("58081", tcp_port.data());
    ASSERT_STREQ("58080", udp_port.data());
    ASSERT_GT(rs_home.size(), 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
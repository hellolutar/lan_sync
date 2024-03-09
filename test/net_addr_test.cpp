#include <string>
#include <gtest/gtest.h>

#include "net/net_addr.h"

TEST(HelloTest, BasicAssertions)
{
    EXPECT_STRNE("hello", "world");
}

TEST(NetAddrTest, ipv4)
{
    std::string ipport = "192.168.233.1:8080";
    NetAddr addr(ipport);
    sockaddr_in le_addr = addr.getAddr();

    ASSERT_EQ(3232295169, le_addr.sin_addr.s_addr);
    ASSERT_EQ(8080, le_addr.sin_port);

    NetAddr fromBe = NetAddr::fromBe(addr.getBeAddr());
    ASSERT_TRUE(fromBe == addr);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
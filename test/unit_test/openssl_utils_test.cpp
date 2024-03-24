#include <string>
#include <gtest/gtest.h>

#include "utils/openssl_utils.h"

TEST(HelloTest, BasicAssertions)
{
    EXPECT_STRNE("hello", "world");
}

TEST(OpenSslUtilsTest, mdEncodeWithSHA3_512)
{
    string ret = OpensslUtil::mdEncodeWithSHA3_512("Makefile");
    ASSERT_TRUE(ret.size() > 0);
}



int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
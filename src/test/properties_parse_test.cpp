#include <gtest/gtest.h>
#include "utils/properties_parse.h"

TEST(HelloTest, BasicAssertions)
{
    EXPECT_STRNE("hello", "world");
}

TEST(PropertiesParseTest, queryTest)
{
    PropertiesParse pp("test.properties");
    string value = pp.query("TCP_PORT");
    ASSERT_STREQ("58080", value.data());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
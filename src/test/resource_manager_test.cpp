#include <gtest/gtest.h>

#include "resource_manager.h"

TEST(HelloTest, BasicAssertions)
{
    EXPECT_STRNE("hello", "world");
}

TEST(ResourceManagerTest, getResourcesTest)
{
    ResourceManager rm("../../resources");
    vector<Resource *> rs = rm.getTable();
    ASSERT_GT(rs.size(), 0);
    ASSERT_GT(strlen(rs.at(0)->name), 0);
    ASSERT_GT(strlen(rs.at(0)->path), 0);
    ASSERT_GT(strlen(rs.at(0)->uri), 0);
    ASSERT_GT(strlen(rs.at(0)->hash), 0);
    ASSERT_GT(rs.at(0)->size, 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
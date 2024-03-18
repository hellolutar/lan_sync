#include <gtest/gtest.h>

class DemoAdd
{
public:
    DemoAdd(/* args */){};
    ~DemoAdd(){};
    int add(int a, int b)
    {
        return a + b;
    }
};

class DemoTest : public testing::Test
{
protected:
    void SetUp() override
    {
        printf("SetUp \n");
    }

    void TearDown() override
    {
        printf("TearDown \n");
    }

public:
    void Add(DemoAdd demo, int a, int b, int expected)
    {
        EXPECT_EQ(expected, demo.add(a, b)) << "a:" << a << "\tb:" << b;
    }
};

TEST_F(DemoTest, Add)
{
    DemoAdd demo;
    int j = 5;
    for (int i = 0; i < 10; i++)
    {
        j -= 1;
        Add(demo, i, j, (i + j));
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
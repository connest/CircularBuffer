#include "tst_circular_buffer_lockfree.h"
#include "tst_circular_buffer.h"

#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

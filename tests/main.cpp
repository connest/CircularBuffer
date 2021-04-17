#include "tst_circular_buffer_lockfree_mrmw.h"
#include "tst_circular_buffer_blocked_mrmw.h"
#include "tst_circular_buffer_lockfree_srsw.h"

#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

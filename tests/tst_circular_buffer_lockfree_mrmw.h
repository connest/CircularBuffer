#ifndef TST_CIRCULAR_BUFFER_LOCKFREE_MRMW_H
#define TST_CIRCULAR_BUFFER_LOCKFREE_MRMW_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

#include <circular_buffer/circular_buffer_fwd.h>
#include <circular_buffer/circular_buffer_lockfree_mrmw.h>


TEST(circular_buffer_lockfree_tests, is_empty)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(40);

    // Act

    bool is_empty = cb.empty();

    // Assert

    ASSERT_TRUE(is_empty);
}

TEST(circular_buffer_lockfree_tests, max_size)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(40);

    // Act

    size_t max_size = cb.max_size();

    // Assert

    ASSERT_EQ(max_size, 40u);
}

TEST(circular_buffer_lockfree_tests, size_and_push_back)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(40);
    cb.try_push_back(2);
    cb.try_push_back(3);

    // Act

    size_t size = cb.size();

    // Assert

    ASSERT_EQ(size, 2u);

    int value1{}, value2{};
    cb.try_pop(value1);
    cb.try_pop(value2);
    ASSERT_EQ(value1, 2);
    ASSERT_EQ(value2, 3);
}


TEST(circular_buffer_lockfree_tests, is_full)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(2);
    cb.try_push_back(1);
    cb.try_push_back(2);

    // Act

    bool full = cb.full();

    // Assert

    ASSERT_TRUE(full);
    ASSERT_EQ(cb.size(), 2u);
}

TEST(circular_buffer_lockfree_tests, push_to_full)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(2);
    cb.try_push_back(1);

    // Act

    bool to_not_full = cb.try_push_back(1);
    bool to_full = cb.try_push_back(3);
    bool full = cb.full();
    size_t size = cb.size();

    // Assert

    ASSERT_TRUE(to_not_full);
    ASSERT_FALSE(to_full);
    ASSERT_TRUE(full);
    ASSERT_EQ(size, 2u);
}

TEST(circular_buffer_lockfree_tests, is_full_zero_size)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(0);

    // Act

    bool full = cb.full();
    bool empty = cb.empty();

    // Assert

    ASSERT_TRUE(full);
    ASSERT_TRUE(empty);
    ASSERT_EQ(cb.size(), 0u);
}

TEST(circular_buffer_lockfree_tests, pop_from_empty)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(0);
    int value{};

    // Act

    bool success = cb.try_pop(value);

    // Assert
    ASSERT_FALSE(success);
    ASSERT_EQ(value, int{});

}

TEST(circular_buffer_lockfree_tests, pop)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(10);
    cb.try_push_back(100);

    int value{};
    // Act

    bool success = cb.try_pop(value);

    // Assert

    ASSERT_EQ(value,  100);
    ASSERT_TRUE(cb.empty());
    ASSERT_TRUE(success);
}


TEST(circular_buffer_lockfree_tests, pop_push)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(3);

    int v;

    // Act

    cb.try_push_back(1);
    cb.try_push_back(2);
    cb.try_push_back(3);


    cb.try_pop(v); // read value = 1
    cb.try_push_back(4);


    // Assert

    ASSERT_TRUE(cb.full());
    ASSERT_EQ(cb.size(), 3u);

    int res1, res2, res3;
    cb.try_pop(res1);
    cb.try_pop(res2);
    cb.try_pop(res3);

    ASSERT_EQ(res1, 2);
    ASSERT_EQ(res2, 3);
    ASSERT_EQ(res3, 4);
}

TEST(circular_buffer_lockfree_tests, full_pop_push)
{
    // Arrange

    connest::CircularBuffer_mrmw<int> cb(3);
    int v;

    // Act

    cb.try_push_back(1);
    cb.try_push_back(2);
    cb.try_push_back(3);

    // now full

    cb.try_pop(v); // read value = 1
    cb.try_push_back(4);
    cb.try_pop(v);
    cb.try_pop(v);

    // writePos < readPos
    cb.try_push_back(5);
    cb.try_push_back(6);


    // Assert

    ASSERT_TRUE(cb.full());
    ASSERT_EQ(cb.size(), 3u);

    int res1, res2, res3;
    cb.try_pop(res1);
    cb.try_pop(res2);
    cb.try_pop(res3);

    ASSERT_EQ(res1, 4);
    ASSERT_EQ(res2, 5);
    ASSERT_EQ(res3, 6);
}


#endif // TST_CIRCULAR_BUFFER_LOCKFREE_MRMW_H

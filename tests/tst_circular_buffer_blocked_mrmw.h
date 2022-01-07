#ifndef TST_CIRCULAR_BUFFER_BLOCKED_MRMW_H
#define TST_CIRCULAR_BUFFER_BLOCKED_MRMW_H


#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

#include "../circular_buffer_fwd.h"
#include "../circular_buffer_blocked_mrmw.h"


TEST(circular_buffer_blocked_tests, is_empty)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(40);

    // Act

    bool is_empty = cb.empty();

    // Assert

    ASSERT_TRUE(is_empty);
}

TEST(circular_buffer_blocked_tests, max_size)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(40);

    // Act

    size_t max_size = cb.max_size();

    // Assert

    ASSERT_EQ(max_size, 40u);
}

TEST(circular_buffer_blocked_tests, size_and_push_back)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(40);
    cb.try_push_back(2);
    cb.try_push_back(3);

    // Act

    size_t size = cb.size();

    // Assert

    ASSERT_EQ(size, 2u);

    int value;
    cb.try_pop(value);
    ASSERT_EQ(value, 2);

    cb.try_pop(value);
    ASSERT_EQ(value, 3);
}


TEST(circular_buffer_blocked_tests, is_full)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(2);
    cb.try_push_back(1);
    cb.try_push_back(2);

    // Act

    bool full = cb.full();

    // Assert

    ASSERT_TRUE(full);
    ASSERT_EQ(cb.size(), 2u);
}

TEST(circular_buffer_blocked_tests, push_to_full)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(2);
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

TEST(circular_buffer_blocked_tests, is_full_zero_size)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(0);

    // Act

    bool full = cb.full();
    bool empty = cb.empty();

    // Assert

    ASSERT_TRUE(full);
    ASSERT_TRUE(empty);
    ASSERT_EQ(cb.size(), 0u);
}

TEST(circular_buffer_blocked_tests, pop_from_empty)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(0);


    // Act
    int value{};
    bool success = cb.try_pop(value);

    // Assert
    ASSERT_FALSE(success);

}

TEST(circular_buffer_blocked_tests, pop)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(10);
    cb.try_push_back(100);

    // Act

    int value{};
    cb.try_pop(value);

    // Assert

    ASSERT_EQ(value, 100);
    ASSERT_TRUE(cb.empty());
}


TEST(circular_buffer_blocked_tests, pop_push)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(3);
    int value;

    // Act

    cb.try_push_back(1);
    cb.try_push_back(2);
    cb.try_push_back(3);


    cb.try_pop(value); // read value = 1
    cb.try_push_back(4);


    // Assert

    ASSERT_TRUE(cb.full());
    ASSERT_EQ(cb.size(), 3u);

    cb.try_pop(value);
    ASSERT_EQ(value, 2);

    cb.try_pop(value);
    ASSERT_EQ(value, 3);

    cb.try_pop(value);
    ASSERT_EQ(value, 4);

}

TEST(circular_buffer_blocked_tests, full_pop_push)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(3);
    int value;

    // Act

    cb.try_push_back(1);
    cb.try_push_back(2);
    cb.try_push_back(3);

    // now full

    cb.try_pop(value); // read value = 1
    cb.try_push_back(4);
    cb.try_pop(value);
    cb.try_pop(value);

    // writePos < readPos
    cb.try_push_back(5);
    cb.try_push_back(6);


    // Assert

    ASSERT_TRUE(cb.full());
    ASSERT_EQ(cb.size(), 3u);

    cb.try_pop(value);
    ASSERT_EQ(value, 4);

    cb.try_pop(value);
    ASSERT_EQ(value, 5);

    cb.try_pop(value);
    ASSERT_EQ(value, 6);

}




TEST(circular_buffer_blocked_tests, resize)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(10);

    // Act

    cb.resize(50);

    // Assert

    ASSERT_EQ(cb.max_size(), 50u);
}
TEST(circular_buffer_blocked_tests, blocked_pop_wait)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(10);

    auto read = [&cb]() mutable {
        int value{};

        cb.pop_wait(value); // wait there

        // Assert

        ASSERT_EQ(value, 666);
    };

    auto write = [&cb]() mutable {
        cb.try_push_back(666);

    };

    // Act



    auto reader = std::thread(read);

    // to be sure, that reader in wait
    std::this_thread::sleep_for(std::chrono::milliseconds(50));


    auto writter = std::thread(write);


    writter.join();
    reader.join();
}
TEST(circular_buffer_blocked_tests, blocked_push_back_wait)
{
    // Arrange

    connest::CircularBuffer_mrmw_blocked<int> cb(2);
    cb.try_push_back(1);
    cb.try_push_back(2); // full now


    auto read = [&cb]() mutable {
        int value{};
        cb.try_pop(value);
    };

    auto write = [&cb]() mutable {

        // Assert

        cb.push_back_wait(666); // wait there
        int value{};
        cb.try_pop(value);
        cb.try_pop(value);

        ASSERT_EQ(value, 666);
    };

    // Act

    auto writter = std::thread(write);

    // to be sure, that writter in wait
    std::this_thread::sleep_for(std::chrono::milliseconds(50));


    auto reader = std::thread(read);


    reader.join();
    writter.join();
}

#endif // TST_CIRCULAR_BUFFER_BLOCKED_MRMW_H

#ifndef TST_CIRCULAR_BUFFER_H
#define TST_CIRCULAR_BUFFER_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

#include "../circular_buffer.h"


TEST(circular_buffer_tests, is_empty)
{
    // Arrange

    connest::CircularBuffer<int> cb(40);

    // Act

    bool is_empty = cb.empty();

    // Assert

    ASSERT_TRUE(is_empty);
}

TEST(circular_buffer_tests, max_size)
{
    // Arrange

    connest::CircularBuffer<int> cb(40);

    // Act

    size_t max_size = cb.max_size();

    // Assert

    ASSERT_EQ(max_size, 40u);
}

TEST(circular_buffer_tests, size_and_push_back)
{
    // Arrange

    connest::CircularBuffer<int> cb(40);
    cb.push_back(2);
    cb.push_back(3);

    // Act

    size_t size = cb.size();

    // Assert

    ASSERT_EQ(size, 2u);

    int value;
    cb.pop(value);
    ASSERT_EQ(value, 2);

    cb.pop(value);
    ASSERT_EQ(value, 3);
}

TEST(circular_buffer_tests, size_and_emplace_back)
{
    // Arrange

    connest::CircularBuffer<int> cb(40);
    cb.emplace_back(2);
    cb.emplace_back(3);

    // Act

    size_t size = cb.size();

    // Assert

    ASSERT_EQ(size, 2u);

    int value;
    cb.pop(value);
    ASSERT_EQ(value, 2);

    cb.pop(value);
    ASSERT_EQ(value, 3);
}

TEST(circular_buffer_tests, is_full)
{
    // Arrange

    connest::CircularBuffer<int> cb(2);
    cb.push_back(1);
    cb.push_back(2);

    // Act

    bool full = cb.full();

    // Assert

    ASSERT_TRUE(full);
    ASSERT_EQ(cb.size(), 2u);
}

TEST(circular_buffer_tests, push_to_full)
{
    // Arrange

    connest::CircularBuffer<int> cb(2);
    cb.push_back(1);

    // Act

    bool to_not_full = cb.push_back(1);
    bool to_full = cb.push_back(3);
    bool full = cb.full();
    size_t size = cb.size();

    // Assert

    ASSERT_TRUE(to_not_full);
    ASSERT_FALSE(to_full);
    ASSERT_TRUE(full);
    ASSERT_EQ(size, 2u);
}

TEST(circular_buffer_tests, is_full_zero_size)
{
    // Arrange

    connest::CircularBuffer<int> cb(0);

    // Act

    bool full = cb.full();
    bool empty = cb.empty();

    // Assert

    ASSERT_TRUE(full);
    ASSERT_TRUE(empty);
    ASSERT_EQ(cb.size(), 0u);
}

TEST(circular_buffer_tests, pop_from_empty)
{
    // Arrange

    connest::CircularBuffer<int> cb(0);


    // Act
    int value{};
    bool success = cb.pop(value);

    // Assert
    ASSERT_FALSE(success);

}

TEST(circular_buffer_tests, pop)
{
    // Arrange

    connest::CircularBuffer<int> cb(10);
    cb.push_back(100);

    // Act

    int value{};
    cb.pop(value);

    // Assert

    ASSERT_EQ(value, 100);
    ASSERT_TRUE(cb.empty());
}


TEST(circular_buffer_tests, pop_push)
{
    // Arrange

    connest::CircularBuffer<int> cb(3);
    int value;

    // Act

    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);


    cb.pop(value); // read value = 1
    cb.push_back(4);


    // Assert

    ASSERT_TRUE(cb.full());
    ASSERT_EQ(cb.size(), 3u);

    cb.pop(value);
    ASSERT_EQ(value, 2);

    cb.pop(value);
    ASSERT_EQ(value, 3);

    cb.pop(value);
    ASSERT_EQ(value, 4);

}

TEST(circular_buffer_tests, full_pop_push)
{
    // Arrange

    connest::CircularBuffer<int> cb(3);
    int value;

    // Act

    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);

    // now full

    cb.pop(value); // read value = 1
    cb.push_back(4);
    cb.pop(value);
    cb.pop(value);

    // writePos < readPos
    cb.push_back(5);
    cb.push_back(6);


    // Assert

    ASSERT_TRUE(cb.full());
    ASSERT_EQ(cb.size(), 3u);

    cb.pop(value);
    ASSERT_EQ(value, 4);

    cb.pop(value);
    ASSERT_EQ(value, 5);

    cb.pop(value);
    ASSERT_EQ(value, 6);

}


TEST(circular_buffer_tests, at)
{
    // Arrange

    connest::CircularBuffer<int> cb(10);
    cb.push_back(1);
    cb.push_back(2);

    // Act

    auto value1 = cb.at(0);
    auto value2 = cb.at(1);

    // Assert

    ASSERT_EQ(cb.size(),  2u); // at will not change size
    ASSERT_EQ(value1, 1);
    ASSERT_EQ(value2, 2);
}


TEST(circular_buffer_tests, at_empty)
{
    // Arrange

    connest::CircularBuffer<int> cb(10);

    // Act + Assert

    ASSERT_THROW(
        cb.at(10),
        std::out_of_range
    );
}

TEST(circular_buffer_tests, at_const)
{
    // Arrange

    connest::CircularBuffer<int> cb(10);
    cb.push_back(1);
    cb.push_back(2);


    auto func_with_const =
            [](const connest::CircularBuffer<int>& cb_const) {
                    // Act

                    auto value1 = cb_const.at(0);
                    auto value2 = cb_const.at(1);

                    // Assert
                    ASSERT_EQ(cb_const.size(),  2u); // at will not change size
                    ASSERT_EQ(value1, 1);
                    ASSERT_EQ(value2, 2);
    };

    func_with_const(cb);
}


TEST(circular_buffer_tests, copy_constructor_buffer)
{
    // Arrange

    connest::CircularBuffer<int> forCopy(10);
    forCopy.push_back(1);
    forCopy.push_back(2);
    forCopy.push_back(3);
    forCopy.push_back(4);

    int value{};

    // Act
    connest::CircularBuffer<int> copy(forCopy);

    //Assert
    ASSERT_EQ(copy.size(), 4u);
    ASSERT_EQ(copy.max_size(), 10u);


    copy.pop(value);
    ASSERT_EQ(value, 1);

    copy.pop(value);
    ASSERT_EQ(value, 2);

    copy.pop(value);
    ASSERT_EQ(value, 3);

    copy.pop(value);
    ASSERT_EQ(value, 4);

    // assert that really copy (not move)

    ASSERT_EQ(forCopy.size(), 4u);
    ASSERT_EQ(forCopy.max_size(), 10u);

    forCopy.pop(value);
    ASSERT_EQ(value, 1);

    forCopy.pop(value);
    ASSERT_EQ(value, 2);

    forCopy.pop(value);
    ASSERT_EQ(value, 3);

    forCopy.pop(value);
    ASSERT_EQ(value, 4);
}

TEST(circular_buffer_tests, move_constructor_buffer)
{
    // Arrange

    connest::CircularBuffer<int> forMove(10);
    forMove.push_back(1);
    forMove.push_back(2);
    forMove.push_back(3);
    forMove.push_back(4);

    int value{};

    // Act
    connest::CircularBuffer<int> copy(std::move(forMove));

    //Assert
    ASSERT_EQ(copy.size(), 4u);
    ASSERT_EQ(copy.max_size(), 10u);


    copy.pop(value);
    ASSERT_EQ(value, 1);

    copy.pop(value);
    ASSERT_EQ(value, 2);

    copy.pop(value);
    ASSERT_EQ(value, 3);

    copy.pop(value);
    ASSERT_EQ(value, 4);
}
#endif // TST_CIRCULAR_BUFFER_H

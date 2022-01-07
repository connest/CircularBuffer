#ifndef TST_CIRCULAR_BUFFER_LOCKFREE_SRSW_H
#define TST_CIRCULAR_BUFFER_LOCKFREE_SRSW_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

#include <circular_buffer/circular_buffer_fwd.h>
#include <circular_buffer/circular_buffer_lockfree_srsw.h>


TEST(circular_buffer_tests, is_empty)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(40);

    // Act

    bool is_empty = cb.empty();

    // Assert

    ASSERT_TRUE(is_empty);
}

TEST(circular_buffer_tests, max_size)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(40);

    // Act

    size_t max_size = cb.max_size();

    // Assert

    ASSERT_EQ(max_size, 40u);
}

TEST(circular_buffer_tests, size_and_push_back)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(40);
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

TEST(circular_buffer_tests, size_and_emplace_back)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(40);
    cb.try_emplace_back(2);
    cb.try_emplace_back(3);

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

TEST(circular_buffer_tests, is_full)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(2);
    cb.try_push_back(1);
    cb.try_push_back(2);

    // Act

    bool full = cb.full();

    // Assert

    ASSERT_TRUE(full);
    ASSERT_EQ(cb.size(), 2u);
}

TEST(circular_buffer_tests, push_to_full)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(2);
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

TEST(circular_buffer_tests, is_full_zero_size)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(0);

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

    connest::CircularBuffer_srsw<int> cb(0);


    // Act
    int value{};
    bool success = cb.try_pop(value);

    // Assert
    ASSERT_FALSE(success);

}

TEST(circular_buffer_tests, pop)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(10);
    cb.try_push_back(100);

    // Act

    int value{};
    cb.try_pop(value);

    // Assert

    ASSERT_EQ(value, 100);
    ASSERT_TRUE(cb.empty());
}


TEST(circular_buffer_tests, pop_push)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(3);
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

TEST(circular_buffer_tests, full_pop_push)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(3);
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


TEST(circular_buffer_tests, at)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(10);
    cb.try_push_back(1);
    cb.try_push_back(2);

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

    connest::CircularBuffer_srsw<int> cb(10);

    // Act + Assert

    ASSERT_THROW(
        cb.at(10),
        std::out_of_range
    );
}

TEST(circular_buffer_tests, at_const)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(10);
    cb.try_push_back(1);
    cb.try_push_back(2);


    auto func_with_const =
            [](const connest::CircularBuffer_srsw<int>& cb_const) {
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


TEST(circular_buffer_tests, constructor_begin_end_iterators)
{
    // Arrange
    std::vector<int> v{1,2,3,4,5,6,7,8};

    // Act

    connest::CircularBuffer_srsw<int> cb(v.cbegin(), v.cend());

    //Assert

    ASSERT_EQ(cb.size(), 8u);

    ASSERT_EQ(cb.at(0), 1);
    ASSERT_EQ(cb.at(1), 2);
    ASSERT_EQ(cb.at(2), 3);
    ASSERT_EQ(cb.at(3), 4);
    ASSERT_EQ(cb.at(4), 5);
    ASSERT_EQ(cb.at(5), 6);
    ASSERT_EQ(cb.at(6), 7);
    ASSERT_EQ(cb.at(7), 8);

}



TEST(circular_buffer_tests, random_access_iterator)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb_for_order(10);
    cb_for_order.try_push_back(10);
    cb_for_order.try_push_back(4);
    cb_for_order.try_push_back(3);
    cb_for_order.try_push_back(2);
    cb_for_order.try_push_back(5);
    cb_for_order.try_push_back(8);
    cb_for_order.try_push_back(7);

    // Act

    std::sort(cb_for_order.begin(), cb_for_order.end());

    // Assert

    ASSERT_EQ(cb_for_order.size(),  7u);

    ASSERT_EQ(cb_for_order.at(0), 2);
    ASSERT_EQ(cb_for_order.at(1), 3);
    ASSERT_EQ(cb_for_order.at(2), 4);
    ASSERT_EQ(cb_for_order.at(3), 5);
    ASSERT_EQ(cb_for_order.at(4), 7);
    ASSERT_EQ(cb_for_order.at(5), 8);
    ASSERT_EQ(cb_for_order.at(6), 10);
}

TEST(circular_buffer_tests, const_random_access_iterator)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb_unordered(10);
    cb_unordered.try_push_back(10);
    cb_unordered.try_push_back(4);
    cb_unordered.try_push_back(3);
    cb_unordered.try_push_back(2);
    cb_unordered.try_push_back(5);
    cb_unordered.try_push_back(8);
    cb_unordered.try_push_back(7);

    // Act

    auto it = std::find(cb_unordered.cbegin(), cb_unordered.cend(), 7);

    // Assert

    ASSERT_EQ(cb_unordered.size(),  7u);

    ASSERT_TRUE(it != cb_unordered.end());
    ASSERT_TRUE(*it == 7);
    ASSERT_EQ(std::distance(cb_unordered.cbegin(), it), 6u);
}

TEST(circular_buffer_tests, push_all)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(10);
    std::vector<int> for_input {
        1,2,3,4,5,6,7,8,9
    };

    // Act
    size_t count = cb.push_back_all(for_input.cbegin(), for_input.cend());

    // Assert

    ASSERT_EQ(cb.size(),  9u);
    ASSERT_EQ(for_input.size(),  9u);
    ASSERT_EQ(count,  9u);
}

TEST(circular_buffer_tests, push_all_no_free_space)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(5);
    std::vector<int> for_input {
        1,2,3,4,5,6,7,8,9
    };

    // Act
    size_t count = cb.push_back_all(for_input.cbegin(), for_input.cend());

    // Assert

    ASSERT_EQ(cb.size(),  5u);
    ASSERT_EQ(count,  5u);
    ASSERT_EQ(for_input.size(),  9u);
}

TEST(circular_buffer_tests, pop_all)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(10);

    cb.try_push_back(10);
    cb.try_push_back(4);
    cb.try_push_back(3);
    cb.try_push_back(2);
    cb.try_push_back(5);
    cb.try_push_back(8);
    cb.try_push_back(7);

    std::vector<int> v{};
    std::vector<int> expected_v{10,4,3,2,5,8,7};

    // Act
    size_t count = cb.pop_all(v);

    // Assert

    ASSERT_EQ(cb.size(),  0u);
    ASSERT_EQ(count,  7u);
    ASSERT_EQ(v.size(),  7u);
    ASSERT_EQ(v,  expected_v);
}

TEST(circular_buffer_tests, pop_all_empty)
{
    // Arrange

    connest::CircularBuffer_srsw<int> cb(10);


    std::vector<int> v{};
    std::vector<int> expected_v{};

    // Act
    size_t count = cb.pop_all(v);

    // Assert

    ASSERT_EQ(cb.size(),  0u);
    ASSERT_EQ(count,  0u);
    ASSERT_EQ(v.size(),  0u);
    ASSERT_EQ(v,  expected_v);
}


#endif // TST_CIRCULAR_BUFFER_LOCKFREE_SRSW_H

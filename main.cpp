#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <cassert>

#include "test_for_fwd.h"
#include "circular_buffer.h"
#include "circular_buffer_lockfree.h"

#define READER_COUNT 4
#define WRITER_COUNT 4

connest::CircularBufferLockfree<size_t> queue{128};
using namespace std;

std::atomic<size_t> read_counter{0};
std::atomic<size_t> write_counter{0};
std::atomic<size_t> amount_readed{0};
std::atomic<size_t> amount_writed{0};


void reader()
{
    size_t value{0};
    size_t value_old{0};

    while(true) {
        bool success = queue.pop(value);
        if(success) {
            ++amount_readed;

            if(value_old >= value)
                std::cout << "error: old = " << value << " new = " << value;

            read_counter += value;
        }

    }

}

void writter()
{
    for(size_t i = 1; i < 1000u;)
    {
        if(queue.push_back(i)) {
            write_counter += i;
            ++i;
            amount_writed++;
        }
    }
    std::cout << "writter end with: " << write_counter << std::endl;
}
int main()
{
    connest::CircularBuffer<int> c{20};
    c.push_back(9999);

    test_for_fwd f(c);
    return 0;

    std::vector<std::thread> writters;
    std::vector<std::thread> readers;


    writters.reserve(WRITER_COUNT);
    for(int i = 0; i < WRITER_COUNT; ++i)
        writters.emplace_back(writter);

    readers.reserve(READER_COUNT);
    for(int i = 0; i < READER_COUNT; ++i)
        readers.emplace_back(reader);




    for(auto& r : readers)
        r.detach();

    // ждем пока допишут писатели
    for(auto& w : writters)
        w.join();

    // чтобы читатели успели дочитать (но, и прочитать слишком много, если в алгоритме ошибка)
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "readers end with: " << read_counter << std::endl;
    std::cout << "difference (expected 0): " << (int)(read_counter - write_counter) << std::endl;
    std::cout << "readed: " << amount_readed << std::endl;
    std::cout << "writed: " << amount_writed << std::endl;

    assert((read_counter - write_counter) == 0);

    return 0;

}

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>

#include "circular_buffer_lockfree_srsw.h"
#include "circular_buffer_blocked_mrmw.h"
#include "circular_buffer_lockfree_mrmw.h"
#define READER_COUNT 4
#define WRITER_COUNT 4
#define ELEMENTS_COUNT 1000u

// Собственно кольцевой буфер
//                   хранимый тип данных
//                          vvvvvvv
connest::CircularBuffer_mrmw<size_t> queue{16};
//                                        ^^^^
//                               размер буфера в элементах

// сумма значений считанных элементов
std::atomic<size_t> read_counter{0};
// сумма значений записанных элементов
std::atomic<size_t> write_counter{0};

// количество считанных элементов
std::atomic<size_t> amount_readed{0};
// количество записанных элементов
std::atomic<size_t> amount_written{0};


void reader()
{
    size_t value{0};
    while(true) {
        if(queue.try_pop(value)) {
            ++amount_readed;
            read_counter += value;
        }
    }

}

void writter()
{
    for(size_t i = 1; i < ELEMENTS_COUNT;) {
        if(queue.try_push_back(i)) {
            write_counter += i;
            ++i;
            ++amount_written;
        }
    }

    std::stringstream s;
    s << "writter end with: " << write_counter;
    std::cout << s.str() << std::endl;
}

int main()
{
    std::vector<std::thread> writters;
    std::vector<std::thread> readers;

    writters.reserve(WRITER_COUNT);
    for(int i = 0; i < WRITER_COUNT; ++i)
        writters.emplace_back(writter);

    readers.reserve(READER_COUNT);
    for(int i = 0; i < READER_COUNT; ++i)
        readers.emplace_back(reader);

    // не ждем читателей
    for(auto& r : readers)
        r.detach();

    // ждем пока допишут писатели
    for(auto& w : writters)
        w.join();

    // чтобы читатели успели дочитать (но, и прочитать слишком много, если в алгоритме ошибка)
    std::this_thread::sleep_for(std::chrono::seconds(3));

    assert(read_counter  == write_counter);
    assert(amount_readed == amount_written);

    return 0;
}

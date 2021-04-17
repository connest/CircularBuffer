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

using namespace std;

connest::CircularBuffer_mrmw<size_t> queue{16};

std::atomic<size_t> read_counter{0};
std::atomic<size_t> write_counter{0};
std::atomic<size_t> amount_readed{0};
std::atomic<size_t> amount_written{0};


void reader()
{
    size_t value{0};
    size_t value_old{0};

    while(true) {
        bool success = queue.try_pop(value);
//        bool success = true;
//        queue.pop_wait(value);
        if(success) {
            ++amount_readed;


            assert(value_old < value);

            read_counter += value;

//            if(read_counter == write_counter) {
//                stringstream s;
//                s << "reader at: "
//                  << read_counter;

//                std::cout << s.str() << std::endl;
//            }
        }

    }

}

void writter()
{
    for(size_t i = 1; i < ELEMENTS_COUNT;)
    {
        if(queue.try_push_back(i))
//        queue.push_back_wait(i);
        {
            write_counter += i;
            ++i;
            amount_written++;
        }
    }

    stringstream s;
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
    std::cout << "writed: " << amount_written << std::endl;

    assert((read_counter - write_counter) == 0);

    return 0;

}

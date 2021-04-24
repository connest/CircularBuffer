#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <cassert>

#include "circular_buffer_lockfree_srsw.h"
#include "circular_buffer_blocked_mrmw.h"
#include "circular_buffer_lockfree_mrmw.h"
#define READER_COUNT 4
#define WRITER_COUNT 4
#define ELEMENTS_COUNT 1000u



class CrazyCopyClass {
    std::string m_str;
    size_t m_value;

    void sleepForCrazy() const {
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> dist(m_value % 140, 150);
        std::this_thread::sleep_for(std::chrono::nanoseconds(dist(gen)));
    }

public:
    CrazyCopyClass(size_t value = 0) noexcept
        : m_str{"It is not small string. I want to be in heap!"}
        , m_value{value}
    {
        sleepForCrazy();
    }

    CrazyCopyClass(const CrazyCopyClass& other) noexcept
        : m_str{"It is not small string. I want to be in heap!"}
        , m_value{other.m_value}
    {
        sleepForCrazy();
    }
    CrazyCopyClass(CrazyCopyClass&& other) noexcept
        : m_str{std::move(other.m_str)}
        , m_value{other.m_value}
    {
        sleepForCrazy();
    }
    ~CrazyCopyClass() = default;

    const std::string& str() const {
        return m_str;
    }

    operator size_t() const {
        return m_value;
    }

    CrazyCopyClass& operator=(const CrazyCopyClass& other) noexcept // noexcept - ВАЖЕН
    {
        sleepForCrazy();
        m_value = other.m_value;
        m_str = other.m_str;
        return *this;
    }

    CrazyCopyClass& operator=(CrazyCopyClass&& other) noexcept // noexcept - ВАЖЕН
    {
        sleepForCrazy();
        m_value = other.m_value;
        m_str = std::move(other.m_str);
        return *this;
    }

};



// Собственно кольцевой буфер
//                        хранимый тип данных
//                          vvvvvvvvvvvvvvv
connest::CircularBuffer_mrmw<CrazyCopyClass> queue{16};
//                                                ^^^^
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
    /*size_t*/ CrazyCopyClass value{0};
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

        CrazyCopyClass value{i};
        if(queue.try_push_back(value)) {
            write_counter += i;
            ++i;
            ++amount_written;
        }
        assert(!value.str().empty());
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

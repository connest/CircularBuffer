#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <vector>
#include <atomic>
#include <stdexcept>

namespace connest {

/**
                R
                |
                V
  -----------------------------------------------------
  |   |   |   |XXX|XXX|XXX|XXX|XXX|XXX|XXX|   |   |RES|
  -----------------------------------------------------
  |                                     ^         |
  0                                     |         Size
                                        W
  R - Позиция чтения
  W - Позиция записи

  R <= W (В кольцевом отсчете)

  RES - зарезервированный элемент, чтобы отличать
        состояния "пуст" и "заполнен"

  R     --> W     - полезные данные
  W + 1 --> R - 1 - свободное место
 */
template <typename T>
class CircularBuffer
{
    std::vector<T> m_data;
    std::atomic<size_t> m_head;
    std::atomic<size_t> m_tail;

public:
    CircularBuffer(size_t size)
        : m_data(size + 1)  // +1 - резервный элемент, чтобы отличать состояния
                            // "пуст" и "полон"
        , m_head{0}
        , m_tail{0}
    {}

    CircularBuffer(const CircularBuffer& other)
        : m_data{other.m_data}
        , m_head{other.m_head.load(std::memory_order_acquire)}
        , m_tail{other.m_tail.load(std::memory_order_acquire)}
    {}

    CircularBuffer(CircularBuffer&& other)
        : m_data{std::move(other.m_data)}
        , m_head{other.m_head.load(std::memory_order_acquire)}
        , m_tail{other.m_tail.load(std::memory_order_acquire)}
    {}

    /**
     * @brief empty Определить пуст ли буфер
     * @return флаг пустоты буфера
     */
    bool empty() const noexcept
    {
        size_t head = m_head.load(std::memory_order_acquire);
        size_t tail = m_tail.load(std::memory_order_acquire);
        return  head == tail;
    }

    /**
     * @brief full Определить полон ли буфер
     * @return флаг полноты буфера
     */
    bool full() const noexcept
    {
        size_t head = m_head.load(std::memory_order_acquire);
        size_t tail = m_tail.load(std::memory_order_acquire);
        return  head == next(tail);
    }

    /**
     * @brief size Получить количество элементов в буфере
     * @return количетсво элементов
     */
    size_t size() const noexcept
    {
        size_t head = m_head.load(std::memory_order_acquire);
        size_t tail = m_tail.load(std::memory_order_acquire);

        if(tail < head)
            return  m_data.size() + tail - head;

        return tail - head;
    }

    /**
     * @brief max_size Получить размер буфера
     * @return маскимальное количество элементов в буфере
     */
    size_t max_size() const noexcept
    {
        // -1 так как не считается резервный элемент
        return m_data.size() - 1;
    }

    /**
     * @brief at Получить доступ к элементу буфера без изменения его состояния
     * @param pos индекс элемента
     * @return ссылка на элемент
     */
    T& at(size_t pos)
    {
        if(pos >= size())
            throw std::out_of_range("CircularBuffer::at: no such index");

        pos = next(m_head.load(std::memory_order_acquire), pos);

        return m_data.at(pos);
    }

    /**
     * @brief at Получить доступ к элементу буфера без изменения его состояния
     * @param pos индекс элемента
     * @return ссылка на элемент
     */
    const T& at(size_t pos) const
    {
        if(pos >= size())
            throw std::out_of_range("CircularBuffer::at: no such index");

        pos = next(m_head.load(std::memory_order_acquire), pos);

        return m_data.at(pos);
    }

    /**
     * @brief push_back Добавить элемент в конец буфера
     * @param value значение элемента
     * @return флаг успешности добавления (буфер может быть заполнен)
     */
    bool push_back(T&& value)
    {
        if(full())
            return false;

        size_t tail = m_tail.load(std::memory_order_acquire);

        m_data.at(tail) = std::forward<T>(value);

        m_tail.store(next(tail), std::memory_order_release);

        return true;
    }

    /**
     * @brief emplace_back Создать и добавить элемент в конец буфера
     * @param args параметры конструктора типа T
     * @return флаг успешности добавления (буфер может быть заполнен)
     */
    template<typename ... Args>
    bool emplace_back(Args&& ... args)
    {
        if(full())
            return false;

        size_t tail = m_tail.load(std::memory_order_acquire);

        m_data.at(tail) = T{std::forward<T>(args) ... };

        m_tail.store(next(tail), std::memory_order_release);

        return true;
    }

    /**
     * @brief pop Получить очередной элемент буфера
     * @param result ссылка, куда должко быть положено значение
     * @return флаг успешности добавления (буфер может быть пуст)
     */
    bool pop(T& result)
    {
        if(empty())
            return false;

        size_t head = m_head.load(std::memory_order_acquire);

        result = m_data.at(head);

        m_head.store(next(head), std::memory_order_release);

        return true;
    }

private:
    /**
     * @brief next Получить позицию в кольцевом буфере
     * @param position текущая позиция
     * @param n        количество сдвигов
     * @return новая позиция
     */
    size_t next(size_t position, size_t n = 1) const noexcept
    {
        return (position + n) % m_data.size();
    }
};

}
#endif // CIRCULAR_BUFFER_H
